# Radar System 3000 — Project Summary

A long-form writeup of the ZHAW PM4 team project: a 24 GHz radar that measures distance via FMCW, speed via CW Doppler, streams the processed spectrum to a local touchscreen UI, bridges the data over WiFi to a desktop application, and fuses the radar result with camera-based angle-of-arrival detection powered by a YOLOv8 neural network. This document is the reference for anyone writing about the project or reading the repository.

---

## 1. At a Glance

- **Academic context:** ZHAW (Zurich University of Applied Sciences), module T-ISC-ET-PM4, spring semester 2025. Internal project name: *Radar System 3000*.
- **Team:** Dennis Rathgeb, Bryan Uhlmann, Benjamin Tschopp.
- **Timeline:** Feb 2025 – May 2025, five phases (schematics → layout → hardware → firmware → demo/video).
- **Specifications:** detect car-sized objects at 1 m to 30 m range, measure speed from 10 km/h to 300 km/h, real-time visualization, optional WLAN/IoT stream.
- **Stack highlights:** STM32F429 + TouchGFX, ESP32 WiFi bridge over SPI, FastAPI backend, YOLOv8n desktop app, KiCad-designed custom analog front-end board, MATLAB + LTSpice simulation, CMSIS-DSP FFT, FreeRTOS.

## 2. The Problem & Motivation

Automotive and industrial sensing commonly rely on radar for range and velocity because radar is robust against lighting and weather conditions that defeat optical systems. The 24 GHz ISM band is a sweet spot: affordable transceivers, practical antenna size, non-licensed. The team's design goal was a complete vertical slice — from raw RF down-conversion through analog signal conditioning, DMA-driven ADC, FFT-based spectral analysis, CFAR target detection, and a touchscreen UI — with enough headroom to stream the result out wirelessly and fuse it with a camera-based angle estimate for a richer display.

Compared to "radar on a module" builds, which treat the RF part as a black box, this project goes one layer deeper: the analog amplifier chain, the VCO sweep driving the FMCW chirp, and the sample acquisition path were all designed and tuned by the team on a custom board.

## 3. RF Front-End — Two Radar Modules, Two Modulation Modes

The RF stage uses two 24 GHz radar modules working in complementary modes:

- **CW (Continuous Wave) module** — used for **velocity** measurement via the Doppler effect. A target moving at velocity *v* shifts the returning signal by *f_d = 2 · v · f_0 / c*. At 24 GHz a 1 m/s target produces ~160 Hz of Doppler shift, so the expected IF band is a few kHz up to roughly 80 kHz at 300 km/h.
- **FMCW (Frequency-Modulated CW) module** — used for **distance** measurement. The transmitter emits a linear frequency chirp; the reflection off a target arrives delayed in time, which at any instant produces a **beat frequency** between transmit and receive that is proportional to target range. The chirp is generated in-system — see §4 for how the VCO is swept.

Both modules output complex baseband (I/Q) — the in-phase and quadrature components of the down-mixed reflection — which is what the analog board amplifies and the STM32 digitizes. Handling I/Q is what preserves the sign of the Doppler shift (approaching vs. receding) and what makes target-direction ambiguity in FMCW resolvable.

Keeping two dedicated modules instead of re-using one is a deliberate choice: a single transceiver can technically do both, but the time-sharing overhead (settling, synchronization, mode switch) eats into the sample window. With separate modules the processing paths run concurrently.

## 4. Analog Signal Conditioning — The Filter Chain

The RF modules' I/Q outputs are in the microvolt range and centered on whatever DC bias the module puts out — useless to feed an ADC directly. The custom **Radar Board** (KiCad, V1 → V3, design files in `hardware/kicad/`) implements a three-stage amplifier chain for each of the I and Q channels, **six op-amp stages in total**.

Key design features:

- **Mid-rail DC bias at 1.65 V** — the amplifier is single-supply (3.3 V rail from a linear regulator, 5 V input); all stages sit on a bias that puts the zero-signal output at midpoint, giving symmetric headroom.
- **Multiple-feedback bandpass topology** (LTSpice file `hardware/simulations/MF_bandpass.asc`) — this topology offers independent control of center frequency, Q, and gain with a single op-amp. It rejects both DC (amplifier drift, FMCW transient) and the aliasing band above Nyquist.
- **Total gain ≈ 58.3 dB at 7 kHz** (verified on the bench: 57.8 dB to 58.2 dB measured across several builds).
- **Power supply ripple under 200 µV** on the 3.3 V rail (spec was < 10 mVpp — measured ~50× better).

Hardware tolerance analysis was done before board bring-up in a Jupyter notebook at `hardware/python-tools/calculations.ipynb` and in MATLAB scripts `hardware/calculations/tof.m` (time-of-flight) and `sigma.m` (noise). The Bode plot (both simulated and measured) lives in `matlab/Bodeplot/`.

The test report `docs/amplifier-stage.md` captures the bench validation: static DC offsets, no-signal noise floor, 10 kHz tone response across all three stages for I and Q. The Q channel cleared every criterion; the I channel failed one amplitude criterion — flagged as a known issue (`TODO NOTE: CHECK I Stage sths wrong here`) and left as a retrospective item (§13).

A custom power sequencer rounds out the board: a momentary button starts the 5 V rail, the MCU latches the rail via GPIO once it boots, and the auto-shutdown handler drops the latch when idle. This lets the whole device be battery-operated without a permanent quiescent drain.

## 5. MCU Data Path — DMA-Driven Acquisition on STM32F429

This is the technical centerpiece of the project. The STM32F429 Discovery board carries the full data pipeline with zero-dropped-sample guarantees.

### Dual-simultaneous ADC in I/Q lock-step

Two on-chip 12-bit ADCs run in **`ADC_DUALMODE_REGSIMULT`** — regular simultaneous mode. ADC1 samples the Q channel (PC1, `ADC1_IN11`), ADC2 samples the I channel (PC3, `ADC2_IN13`), both converting on the same clock edge. That lock-step timing is what preserves phase alignment between I and Q — the whole complex-valued demodulation story collapses without it.

Each sample pair arrives in memory as a packed 32-bit word: `{Q[15:0], I[15:0]}`.

### Circular DMA into SDRAM

A single DMA stream (DMA2 Stream 0, Channel 0) pulls the paired samples from the ADCs into a ring buffer. The buffer lives in **external SDRAM**, not SRAM, because the sample history for the FFT can be large:

```c
__attribute__((section(".sdram"), aligned(4)))
uint32_t sampleBuffer[MEAS_SAMPLE_BUFFER_LENGTH];
```

DMA is configured with `Mode = DMA_CIRCULAR`, so it loops forever without CPU intervention. Priority is `HIGH`. `PeriphIncr` is off (always read the same register pair), `MemIncr` is on (advance through the buffer).

### Ping-pong (double-buffered) processing

Here's the trick that makes real-time processing tractable on a 168 MHz MCU. The DMA fires two interrupts per full buffer:

- `HAL_ADC_ConvHalfCpltCallback` — at the midpoint (first half of the ring is full, DMA is now writing into the second half)
- `HAL_ADC_ConvCpltCallback` — at the end (second half full, DMA wraps around)

Each callback marks which half to process, then does:

```c
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
xTaskNotifyFromISR(local_taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
```

The `xTaskNotifyFromISR` + `portYIELD_FROM_ISR` pair is the canonical zero-latency wakeup: the SAMPLING_Task runs on the very next instruction after the ISR exits, no queue-based intermediate or polling loop. While the task crunches the "stable" half, DMA keeps filling the other half. At normal frame rates **not a single sample is dropped**.

### DC-offset removal, then CMSIS-DSP FFT

Inside the sampling task, before each FFT, the processing path:

1. Computes the DC average of I and Q over the half-buffer (a simple mean over `N/2` samples).
2. Converts the 16-bit ADC codes to `float32_t`, subtracting the average. This strips the 1.65 V mid-rail bias plus any amplifier offset drift from the spectrum — crucial because a single residual DC bin can overwhelm all the signal bins in an FFT.
3. Copies the adjusted samples into a rolling **FFT input buffer** (also in SDRAM, `MEAS_SAMPLE_BUFFER_LENGTH * MEAS_FFT_BUF_SIZE` floats): multiple FFT frames are retained for time-averaging or CFAR logic that needs history.
4. Runs the complex FFT via CMSIS-DSP's `arm_cfft_f32` (single-precision float, in-place, bit-reversed output).

The FFT is "complex" because we have true I/Q samples — the resulting spectrum covers both positive and negative frequencies, which the CW Doppler branch needs to distinguish approach from retreat.

### FreeRTOS task layout

Three user-level tasks run under the CMSIS-RTOS v2 wrapper over FreeRTOS:

| Task             | Stack     | Priority       | Role                                                       |
|------------------|-----------|----------------|------------------------------------------------------------|
| `defaultTask`    | 512 B     | Normal         | System watchdog / hooks                                    |
| `GUI_Task`       | 32 KB     | Normal         | TouchGFX rendering loop                                    |
| `SAMPLING_Task`  | 8 KB      | **High**       | Half-buffer processing → FFT → detection → UI model update |

`SAMPLING_Task` is pinned to high priority so the sample-to-FFT path never yields to the UI mid-frame. `GUI_Task` gets the 32 KB stack because TouchGFX's render path uses heavy local structures (widget trees, transformation matrices).

### Supporting modules

The firmware is organized into focused modules (all in `firmware/stm32/STM32CubeIDE/Application/User/`):

- `measurement.c/h` — the full ADC → DMA → FFT pipeline described above (438 LOC).
- `vco.c/h` — DAC-driven VCO control for FMCW chirp (see §6).
- `timing.c/h` — TIM2 master + TIM8 slave synchronized via `ITR1` for hardware-accurate sweep timing.
- `esp.c/h` — SPI3 master driver to the ESP32 (see §7).
- `shutdown_handler.c/h` — the power-latch state machine driving the button / auto-shutdown sequence.
- `buzzer_handler.c/h` — PWM-driven buzzer on a timer channel (500 Hz, 50 % duty when alerting).
- `log.c/h` — priority-queued logging system with `printf`-rerouted UART1 output, configurable via `config.h` (threshold, queue depth 28, max message 255 B, prefix 10 B).
- `error_handler.c/h` — `RETURN_OK()` / `RETURN_ERROR("reason")` macros that wrap every HAL call; errors bubble up with context strings and land in the log priority queue.

### FMCW chirp generation (VCO module)

The `vco.c` module drives an off-chip VCO through DAC channel 2 (PA5). A 256-sample ramp is pre-computed and pushed to the DAC via **DMA1 Stream 6 Channel 7 in circular mode**, triggered by a timer — so the chirp runs continuously in hardware without CPU intervention. The `Timing` module pairs TIM2 (master) with TIM8 (slave) via the internal trigger `ITR1` so the sweep and the ADC sampling window stay phase-locked.

## 6. Live UI — TouchGFX Stream Directly From the Processing Pipeline

The LCD on the STM32F429 Discovery board is driven by TouchGFX, STMicroelectronics' hardware-accelerated GUI framework that leverages the Chrom-ART (DMA2D) accelerator and the LTDC controller. The project uses the ILI9341 LCD driver (`Components/ili9341/ili9341.h`), SPI5 for its control interface, the on-board SDRAM as framebuffer.

The standout design decision is that there is **no polling boundary** between the signal-processing pipeline and the UI:

- The FFT output buffer in SDRAM is the same buffer the GUI model reads from. The SAMPLING_Task writes new FFT magnitudes into it; the next GUI_Task render tick reads them.
- There is no producer-consumer queue. No memcpy into a "UI buffer". No polling "is data ready" flag. A model update invalidates the spectrum widget, and Chrom-ART composites the next frame directly from the live FFT memory.

The result is a spectrum display that updates at FFT rate, with latency equal to one render frame. It looks — and is — streamed live from the radar.

Along with the spectrum, the UI shows current-state indicators for measurement (sampling / processing / idle), power (on / shutting down), and alert state (buzzer active).

## 7. ESP32 Bridge & WiFi Telemetry

The ESP32 lives downstream of the STM32 as a pure WiFi bridge. The link between them is **SPI** — the STM32 is master, the ESP32 is slave. On the STM32 side:

- **SPI3** peripheral, 8-bit data, mode 0 (CPOL=0, CPHA=0), MSB-first, hardware NSS, baud prescaler /2.
- DMA transmit (`HAL_SPI_Transmit_DMA`), with a `spi_tx_busy` flag set on submit and cleared in `HAL_SPI_TxCpltCallback` — so the caller can't queue a second transfer while the first is in flight.
- A dedicated GPIO (`ESP_EN_Pin`) power-gates the ESP32 so the host can cold-boot it or cut it for power saving.

On the ESP32 side (`firmware/esp32/`, ESP-IDF v5.x-style project):

- `app_main.c` bootstraps the SPI slave interface on `HSPI_HOST` with a handshake line (GPIO2). The slave asserts the handshake high when its receive buffer is queued and drops it when the transaction completes, giving the master a flow-control signal that doesn't require shared clocks.
- 128-byte transactions, full-duplex. The master writes a detection record (or a FFT snapshot packet) into MOSI; the slave responds on MISO.
- `wifi.c` drives the WiFi station-mode stack with `fast_scan` — the SSID/password are placeholder strings in the committed source (`MyWiFiNetwork` / `MyWiFiPassword`) and must be replaced by the operator before flashing.

The ESP32 forwards received frames to the backend (§8). The exact framing protocol between the two halves of the firmware is a work-in-progress (§13).

## 8. Machine Learning — Angle-of-Arrival via YOLOv8

Radar alone gives you speed and range — it does not naturally give you the **angle** at which a target sits in the field of view, at least not without a phased antenna array. The desktop app closes that gap with camera-based angle estimation.

The pipeline (in `desktop-app/modules/yolo.py`):

1. A webcam provides a live video feed (OpenCV, `cv2.VideoCapture(0)`).
2. **YOLOv8n** (Ultralytics' nano variant, ~6 MB, auto-downloaded at first run) runs on each frame at `conf=0.3` confidence threshold.
3. Detections are filtered by class — only cars, people, and bicycles are kept; everything else is discarded.
4. For each surviving detection, the horizontal angle is computed from the bounding-box centroid:

   ```python
   angle = ((cx - frame_width/2) / frame_width) * fov_deg
   ```

   where `fov_deg` is the camera's horizontal field of view (default 60°). A target at the center of the frame is 0°; full left is −30°; full right is +30°.

This is a deliberate engineering simplification: at the project's working distance and FOV, pixel position is a serviceable proxy for angle, given the camera lens is approximately rectilinear. Full lens-distortion calibration wasn't needed.

The model choice (YOLOv8n) trades some accuracy for speed — on commodity laptops it runs at dozens of FPS and keeps pace with the camera. Swapping to YOLOv8s/m/l is a one-line change if more accuracy is worth the throughput hit.

## 9. Desktop App — Fusion & Overlay

The orchestration script `desktop-app/collect_main.py` wires three data streams together and draws the overlay:

- **Radar** (speed) — eventually pulled from the FastAPI backend. In the current demo the radar path is stubbed; the desktop app is shaped to consume the real feed once the backend serves it.
- **Camera** (class + angle) — the YOLO pipeline of §8.
- **Ground-truth reference speed** (GPS) — from an Android phone running an app that streams NMEA-0183 `$GPVTG` / `$GNVTG` sentences as UDP packets to port 5005. The `android_speed.py` module runs a daemon thread, parses the sentences, and exposes `get_speed()` — returning `None` if the last update is older than 5 s, so stale data never reaches the overlay.

The main loop draws, for every frame, per detected object: a full-height vertical band at the object's bbox column, plus three text labels (angle in degrees, class name, speed in km/h or "No speed"). This gives a quick intuitive read of "what the radar is locked onto, where, and how fast."

## 10. Hardware Iterations V1 → V3

The custom board went through three spins during the project:

- **V1** (in `hardware/kicad/HW_Radar_V1/`) — initial schematic-first layout. Small, mostly to validate the component selection and rough topology.
- **V2** — an intermediate revision that wasn't worth keeping as a separate folder (changes rolled into V3 during iteration).
- **V3** (in `hardware/kicad/HW_Radar_V3/`) — the board that got bench-tested. `HW_Radar.kicad_pcb` (1.7 MB) is the final layout; `HW_Radar.pdf` is the exported schematic PDF.

Auto-generated KiCad backups and zip snapshots have been excluded from the published repo.

## 11. MATLAB Prototyping

Before firmware, the team prototyped the signal chain in MATLAB (folder `matlab/`):

- `matlab/Bodeplot/` — frequency-response analysis of the multi-feedback bandpass filter, used to validate that LTSpice's predictions matched the discrete-time behavior at the sample rate the MCU would run.
- `matlab/CW_Doppler/` — Doppler signal-chain simulation: synthetic target at configurable speed/range, I/Q mixer model, window + FFT, peak extraction. This was the reference implementation the STM32 FFT path was eventually validated against.

Additional hardware-side calculations in `hardware/calculations/` (MATLAB `.m`) and `hardware/python-tools/` (Jupyter).

## 12. Test & Validation

Two structured test matrices captured the bring-up campaign (extracted into this document from the original XLSX files before publication):

**Hardware test (HW_Test.xlsx extract):**

| Test                     | Condition                    | Expected            | Measured  | Verdict |
|--------------------------|------------------------------|---------------------|-----------|---------|
| Linear regulator 3.3 V   | 5 V supply                   | 3.3 V ± 50 mV       | 3.289 V   | OK      |
| DC offset voltage        | 5 V supply                   | 1.65 V ± 50 mV      | 1.64 V    | OK      |
| Supply ripple 3.3 V      | 5 V supply                   | < 10 mVpp           | 200 µV    | OK      |
| DC gain I output         | 5 V supply, input 0 V        | 1.65 V ± 50 mV      | 1.64 V    | OK      |
| DC gain Q output         | 5 V supply, input 0 V        | 1.65 V ± 50 mV      | 1.64 V    | OK      |
| AC gain I @ 7 kHz        | 1.5 V_rms in, TP4→TP6        | 58.276 dB ± 0.6 dB  | 57.81 dB  | OK      |
| AC gain Q @ 7 kHz        | 1.5 V_rms in, TP3→TP5        | 58.276 dB ± 0.6 dB  | 57.83 dB  | OK      |
| Slow start-up            | button latch / auto-shutdown | various             | all pass  | OK      |
| Buzzer PWM               | 500 Hz 50/50                 | audible             | OK        | OK      |

A second run of the same battery after revisions tightened the AC gain to 58.12 / 58.16 dB — even closer to spec.

**Firmware test (FW_Test.xlsx extract):**

| Test                 | Condition                                         | Verdict |
|----------------------|---------------------------------------------------|---------|
| VCO sweep            | Vpp = 2 V, T_on = 1 ms, T_off = 0.5 ms            | OK      |
| ADC sample integrity | 3 kHz sine at TP5 (Q-input)                       | In-spec |
| DAC / Frequency / Buzzer | subsystem-level smoke tests                   | Pass    |

The bench campaign validated every dimension that affects signal-path fidelity: rails, biases, gain, symmetry between I and Q, sweep linearity, and subsystem-level behavior. The known open item (§4) is the I-channel amplitude anomaly at 10 kHz — the Q channel passes cleanly while I shows higher-than-expected ripple (60 mV instead of the 30 mV the Q path produced). That issue was noted in `docs/amplifier-stage.md` and left as a retrospective item.

## 13. What We'd Do Differently

Three honest retrospective items:

1. **FastAPI backend remained a stub.** `backend/main.py` exposes a single `/fft/` endpoint that currently returns 256 copies of a constant float. The plumbing (CORS configured for a React dev server at `localhost:3000`) hints at the original intent — a browser-side live spectrum viewer — but the glue between the ESP32 uplink and the FastAPI ingest never closed. A second pass would wire the SPI frames through the ESP32's HTTP client into a websocket endpoint, and the React client would consume that.
2. **I-channel amplitude anomaly.** The 10 kHz bench test flagged the I stage as noisy relative to Q. Next revision: check the I-side MF-bandpass components for tolerance mismatch vs. the Q side (same BOM, so likely a solder issue or a stuffed-wrong resistor).
3. **ESP32 firmware is still ESP-IDF example code.** `app_main.c` is literally the `spi_slave_recv` example from Espressif with placeholder WiFi credentials in `wifi.c`. The framing for radar detections was designed but not shipped. Anyone forking this for their own project should treat the ESP32 side as a scaffold, not finished work.

The signal-processing pipeline (hardware + STM32 firmware + UI) is the part that runs end-to-end cleanly.

## 14. File Map — Original Submission → This Repository

| Original (ZHAW submission)                            | This repository                |
|-------------------------------------------------------|--------------------------------|
| `02_Hardware/02_Kicad/`                               | `hardware/kicad/`              |
| `02_Hardware/03_Simulations/`                         | `hardware/simulations/`        |
| `02_Hardware/04_Calculations/`                        | `hardware/calculations/`       |
| `02_Hardware/04_Python/`                              | `hardware/python-tools/`       |
| `02_Hardware/05_Pinbelegung/`                         | `hardware/pinout/`             |
| `04_Matlab/Bodeplot/`, `CW_Doppler/`                  | `matlab/`                      |
| `05_Conecpt_Design_Report/Blockdiagramm_Simpel.pdf`   | (dropped — content → SUMMARY)  |
| `05_Conecpt_Design_Report/blockdiagram_simple.png`    | `docs/block-diagram.png`       |
| `06_Firmware/Radar_v2/`                               | `firmware/stm32/`              |
| `06_Firmware/ESP32/version_0.1/versio0.1/`            | `firmware/esp32/`              |
| `06_Firmware/documentation/`                          | (dropped — public vendor PDFs) |
| `07_HW_FW_Concept_Test/01_Calculate_Tolerance/`       | `tests/hw-concept-tolerance/`  |
| `07_HW_FW_Concept_Test/HW_Test.xlsx`, `FW_Test.xlsx`  | (dropped — content → §12)      |
| `08_Test/01_hardware/Amplifierstage.md`               | `docs/amplifier-stage.md`      |
| `09_Desktop_Application/01_python/`                   | `desktop-app/`                 |
| `10_GFX/Autoshutdown_Image.png`                       | `docs/autoshutdown.png`        |
| `11_Video/Video_Script.docx`                          | (dropped — empty placeholder)  |
| `12_FastAPI/`                                         | `backend/`                     |
| `01_Admin/Timemanagment.xlsx`                         | (dropped — Gantt extract in §1 timeline) |
| *External* `PM4_Concept_Report.pdf`                   | `docs/concept-report.pdf`      |

## 15. Credits & License

ZHAW PM4 team project, spring semester 2025. Authors: Dennis Rathgeb, Bryan Uhlmann, Benjamin Tschopp.

The original collaboration took place on ZHAW's internal GitHub instance. This public mirror is a slimmed, portfolio-oriented snapshot released under the MIT License (see `LICENSE`). All three authors consented to public release; the original ZHAW repository remains the authoritative source of the academic submission.

---

## Appendix A — Reference Data

| Parameter                         | Value                                |
|-----------------------------------|--------------------------------------|
| Carrier frequency                 | 24 GHz                               |
| Target range spec                 | 1 m … 30 m                           |
| Target speed spec                 | 10 km/h … 300 km/h                   |
| Supply input                      | 5 V                                  |
| Rail (post-LDO)                   | 3.3 V                                |
| Rail ripple (measured)            | ~200 µV                              |
| Mid-rail bias                     | 1.65 V                               |
| Amplifier gain (I/Q, measured)    | 57.8 – 58.2 dB @ 7 kHz              |
| Amplifier topology                | Multiple-feedback bandpass, 3 stages |
| ADC resolution                    | 12-bit                               |
| ADC mode                          | Dual-simultaneous (regular)          |
| DMA mode                          | Circular, into SDRAM                 |
| FFT                               | CMSIS-DSP `arm_cfft_f32`             |
| FreeRTOS task stacks              | 512 B / 32 KB / 8 KB                 |
| SAMPLING_Task priority            | High                                 |
| STM32 ↔ ESP32 link                | SPI3, 8-bit, mode 0, HW NSS, DMA     |
| ESP32 SPI role                    | Slave (HSPI_HOST, 128-byte frames)   |
| ESP32 ↔ STM32 handshake           | GPIO line (ESP drives high when ready) |
| UDP GPS ingest port               | 5005 (NMEA `$GPVTG` / `$GNVTG`)      |
| YOLOv8 model                      | YOLOv8n (nano, ~6 MB)               |
| YOLO confidence threshold         | 0.3                                 |
| Tracked classes                   | car, person, bicycle                |
| Camera FOV (default)              | 60°                                 |
| VCO DAC resolution                | 12-bit                              |
| VCO ramp length (default)         | 256 samples                         |
| Buzzer PWM                        | 500 Hz, 50/50 duty                  |

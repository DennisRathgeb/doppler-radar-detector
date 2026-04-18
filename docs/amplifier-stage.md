# Hardware Amplifier Stage Test Preset

## Test Setup

- **Device Under Test (DUT):** Hardware Amplifier Stage
- **Test Equipment:**
  - Signal Generator: FY6900
  - Oscilloscope: Sigilent SDS824
  - Power Supply: Basetech BT-305 & USB from Laptop

## Test Cases

### 1. Static Tests

- **Objective:** Verify the amplifier's behavior without input signals.
- **Procedure:**
  - Power on the amplifier.
  - Measure and record the input Voltage of each Stage
  - Visual inspection of IC Orientation and solder points

### 2. Frequency Response Tests

#### a. No Input Signal

- **Objective:** Verify the amplifier's output with no input signal.
- **Procedure:**
  - Ensure no input signal is applied.
  - Measure and record the output noise level.

#### c. Input Signal at 10,000 kHz

- **Objective:** Test the amplifier's response at 10,000 kHz.
- **Procedure:**
  - Apply a 10,000 kHz sine wave to the input.
  - Measure and record the output amplitude and phase.

### 3. Bode Plot

- **Objective:** Generate a Bode plot to analyze the frequency response. from 100Hz to 100kHz
- **Procedure:**
  - Sweep the input frequency across the amplifier's operating range.
  - Record the gain and phase shift at each frequency.
  - Plot the results to visualize the frequency response.

## Results

### 1. Static Tests

**Input Voltage `PASS`**
| **Description** | **Test Point** | **Value** |
|---------------------------|----------------|-----------|
|Offset OpAmp 1|Opamp 1 Pin 8 | 3.289V |
|Offset Voltage OpAmp 2|Opamp 2 Pin 8| 3.289V |
|Offset Voltage OpAmp 3|Opamp 3 Pin 8| 3.289V |

**Offset Voltage `PASS`**
| **Description** | **Test Point** | **Value** |
|---------------------------|----------------|-----------|
|Offset OpAmp 1|Opamp 1 Pin 5 | 1.64V |
|Offset Voltage OpAmp 2|Opamp 2 Pin 5| 1.64V |
|Offset Voltage OpAmp 3|Opamp 3 Pin 5| 1.64V |

**Visual Inspection `PASS`**
| **Description** | **Result** |
|---------------------------|----------------|
| OpAmp 1 IC | Pass |
| OpAmp 2 IC | Pass |
| OpAmp 3 IC | Pass |
| Resistors Check | Pass |
| Capacitors Check | Pass |

### 2. Frequency Response Tests

#### No Input Signal

**Connections**

- TP 2 is used for GND
- Signal Input(TP3& TP4) connected to GND
- TP 5 & 6 is signal OUTPUT

**Result**
Aquire Mode with 40 Sequences was used:
**TP5**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Mean Voltage | 1.65 V | `PASS` |
| Peak-to-Peak Voltage | < 10 mV | `PASS` |
| Frequency | > 1 MHz | `PASS` |

**TP6**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Mean Voltage | 1.65 V | `PASS` |
| Peak-to-Peak Voltage | < 20 mV | `PASS` |
| Frequency | > 1 MHz | `PASS` |

#### 10kHz Input Signal

**Connections**

- TP 1 & 2 is used for GND
- Signal Input (TP3 & 4) connected to Signal
- TP 7 & 9 is signal OUTPUT stage 1
- TP 8 & 10 is signal OUTPUT stage 2
- TP 5 & 6 is signal OUTPUT stage 2
  > **signal**: 1kHz, 0.01V (resistor divider) , 1.64V Offset
  > **measure**: Aquire Mode with 40 seq, 1X Probe, DC Coupling, 1MOhm

**Result Stage 1**
**I: TP6**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Peak-to-Peak Voltage | 60mV @1V | **`FAIL`** |
| Frequency | 10kHz | `PASS` |

**Q: TP7**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Peak-to-Peak Voltage | 30mV | `PASS` |
| Frequency | 10kHz | `PASS` |

**Result Stage 2**
**I: TP10**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Peak-to-Peak Voltage | 110mV @ 1V | **`FAIL`** |
| Frequency | 10kHz | `PASS` |

**Q: TP8**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Peak-to-Peak Voltage | 150mV | `PASS` |
| Frequency | 10kHz | `PASS` |

**Result Stage 3**
**I: TP6**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Peak-to-Peak Voltage | 870mV @ 1V | **`FAIL`** |
| Frequency | 10kHz | `PASS` |

**Q: TP5**
| **Description** | **Value** | **Result** |
| -------------------- | --------- | ---------- |
| Peak-to-Peak Voltage | 1.23V | `PASS` |
| Frequency | 10kHz | `PASS` |

> **TODO NOTE: CHECK I Stage sths wrong here.**

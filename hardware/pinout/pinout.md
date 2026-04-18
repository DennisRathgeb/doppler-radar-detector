# STM32F429 Pin Assignments

## Sheet1

| STM32F429 Discovery board |  |  |  |  |  |  |  |  | hhrt@zhaw.ch | 2020-06-18 00:00:00 |  |
|---|---|---|---|---|---|---|---|---|---|---|---|
|  |  |  |  |  |  |  |  |  | Meaning of the colours: |  |  |
| Compiled by hhrt@zhaw.ch by consulting  the user manual of the evaluation board and the datasheet of the MCU |  |  |  |  |  |  |  |  | Pin is fully available |  |  |
|  |  |  |  |  |  |  |  |  | Pin available when SDRAM, Gyro resp. LED not used |  |  |
|  |  |  |  |  |  |  |  |  | GND and 3V supply |  |  |
|  |  |  |  |  |  |  |  |  | Pin is used by board function and is NOT available |  |  |
| ExtensionConnector P1 |  |  |  |  |  |  |  |  |  |  |  |
| Available
analog
functions | Was macht das bei uns??? Häää. Erklärung siehe unten | Available
alternate
functions | Board
function | Main
function | Pin | Pin | Main
function | Board
function | Available
alternate
functions | Available
analog
functions |  |
|  |  | 5V supply |  | 5V | 1 | 2 | 5V |  | 5V supply |  |  |
|  |  |  | SRRAM (A4) | PF4 | 3 | 4 | PF5 | SDRAM (A5) |  |  |  |
|  |  |  | SDRAM (A2) | PF2 | 5 | 6 | PF3 | SDRAM (A3) |  |  |  |
|  |  |  | SDRAM (A0) | PF0 | 7 | 8 | PF1 | SDRAM (A1) |  |  |  |
|  |  |  | OSC32_IN | PC14 | 9 | 10 | PC15 | OSC32_OUT |  |  |  |
|  |  | TIM9_CH2, SPI4_MOSI, SAI1_SD_A |  | PE6 | 11 | 12 | PC13 |  |  |  | SIG_SEL |
|  |  | SPI4_NSS, SAI1_FS_A |  | PE4 | 13 | 14 | PE5 |  | TIM9_CH1 , SPI4_MISO,  SAI1_SCK_A |  |  |
|  |  | SPI4_SCK, SAI1_MCLK_A |  | PE2 | 15 | 16 | PE3 |  | SAI1_SD_B |  |  |
|  |  |  | SDRAM (NBL0) | PE0 | 17 | 18 | PE1 | SDRAM (NBL1) |  |  |  |
|  |  |  | LCD | PB8 | 19 | 20 | PB9 | LCD |  |  |  |
|  |  |  | BOOT0 | BOOT0 | 21 | 22 | VDD |  |  |  |  |
|  |  |  | SDRAM (SDNE1) | PB6 | 23 | 24 | PB7 |  | TIM4_CH2, I2C1_SDA, USART1_RX |  |  |
|  | SPI3_MISO | TIM3_CH1,SPI1_MISO, SPI3_MISO, I2S3EXT_SD |  | PB4 | 25 | 26 | PB5 | SDRAM (SDCKE1) |  |  |  |
|  |  |  | SDRAM (SDNCAS) | PG15 | 27 | 28 | PB3 | SWO |  |  |  |
|  |  | SPI6_SCK, USART6_CTS (with LED indicator) | LED GREEN, LD3 | PG13 | 29 | 30 | PG14 | LED RED, LD4 | SPI6_MOSI, USART6_TX (with LED indicator) |  |  |
|  |  |  | LCD | PG11 | 31 | 32 | PG12 | LCD |  |  |  |
|  |  | USART6_RX |  | PG9 | 33 | 34 | PG10 | LCD |  |  |  |
|  |  | USART2_CK |  | PD7 | 35 | 36 | PD6 | LCD |  |  |  |
|  |  | USART2_TX |  | PD5 | 37 | 38 | PD4 |  | USART2_RTS |  |  |
|  |  |  | LCD | PD3 | 39 | 40 | PD2 |  | TIM3_ETR |  |  |
|  |  | CAN1_TX | SDRAM (D3) | PD1 | 41 | 42 | PD0 | SDRAM (D2) | CAN1_RX |  |  |
|  | SPI3_MOSI | SPI3_MOSI, I2S3_SD, USART3_CX, UART5_TX |  | PC12 | 43 | 44 | PC11 |  | I2S3EXT_SD, SPI3_MISO,USART3_RX, UART4_RX |  |  |
|  | SPI3_SCK/SPI3_NSS |  | LCD | PC10 | 45 | 46 | PA15 | TOUCH |  |  | SPI_NSS |
|  |  |  | SWCLK | PA14 | 47 | 48 | PA13 | SWDIO |  |  |  |
|  |  |  | LCD | PA12 | 49 | 50 | PA11 | LCD |  |  |  |
|  |  |  | USART1_RX (VCP) | PA10 | 51 | 52 | PA9 | USART1_TX (VCP) |  |  |  |
|  |  |  | I2C_EXT, TOUCH | PA8 | 53 | 54 | PC9 | I2C_EXT, TOUCH |  |  |  |
|  |  | TIM3_CH3, TIM8_CH3, USART6_CK |  | PC8 | 55 | 56 | PC7 | LCD |  |  |  |
|  |  |  | LCD | PC6 | 57 | 58 | PG8 | SDRAM (SDCLK) |  |  |  |
|  |  |  | LCD | PG7 | 59 | 60 | PG6 | LCD |  |  |  |
|  |  |  | SDRAM (BA1) | PG5 | 61 | 62 | PG4 | SDRAM (BA0) |  |  |  |
|  |  |  |  | GND | 63 | 64 | GND |  |  |  |  |
| Extension Connector P2 |  |  |  |  |  |  |  |  |  |  |  |
| Available
analog
functions |  | Available
alternate
functions | Board
function | Main
function | Pin | Pin | Main
function | Board
function | Available
alternate
functions | Available
analog
functions |  |
|  |  | 3V supply |  | 3V | 1 | 2 | 3V |  | 3V supply |  |  |
| ADC3_IN4 | Buzzer TIM10_CH1 | TIM10_CH1, SPI5_NSS, SAI1_SD_B, UART7_RX |  | PF6 | 3 | 4 | NC |  |  |  |  |
| ADC3_IN6 |  | SPI5_MISO, SAI1_SCK_B, TIM13_CH1 | GYRO (MISO) | PF8 | 5 | 6 | PF7 | LCD, GYRO SCL |  |  |  |
|  |  |  | LCD | PF10 | 7 | 8 | PF9 | LCD, GYRO |  |  |  |
|  |  |  | OSC_OUT | PH1 | 9 | 10 | PH0 | OSC_IN |  |  |  |
|  |  |  |  | GND | 11 | 12 | NRST | NRST |  |  |  |
| ADC123_IN11 | SIGNAL Q ADC1 |  | GYRO (CS) | PC1 | 13 | 14 | PC0 | SDRAM (SDNWE) |  |  |  |
| ADC123_IN13 | SIGNAL I ADC2 | SPI2_MOSI, I2A2_SD |  | PC3 | 15 | 16 | PC2 | LCD |  |  |  |
|  |  |  | GYRO (INT1) | PA1 | 17 | 18 | PA0 | PushButton |  |  |  |
|  |  |  | LCD | PA3 | 19 | 20 | PA2 | GYRO (INT2) |  |  |  |
| ADC12_IN5, DAC_OUT2 | Radar VCO DAC_OUT2 | TIM2_CH1, TIM2_ETR, TIM8_CH1N, SPI1_SCK |  | PA5 | 21 | 22 | PA4 | LCD |  |  |  |
|  |  |  | I2C_EXT | PA7 | 23 | 24 | PA6 | LCD |  |  |  |
|  |  |  | USB | PC5 | 25 | 26 | PC4 | USB |  |  |  |
|  |  |  | LCD | PB1 | 27 | 28 | PB0 | LCD |  |  |  |
|  |  |  |  | GND | 29 | 30 | PB2 (BOOT1) | BOOT1 |  |  |  |
|  |  |  | SDRAM (A6) | PF12 | 31 | 32 | PF11 | SDRAM (SDNRAS) |  |  |  |
|  |  |  | SDRAM (A8) | PF14 | 33 | 34 | PF13 | SDRAM (A7) |  |  |  |
|  |  |  | SDRAM (A10) | PG0 | 35 | 36 | PF15 | SDRAM (A9) |  |  |  |
|  |  | TIM1_ETR, UART7_Rx | SDRAM (D4) | PE7 | 37 | 38 | PG1 | SDRAM (A11) |  |  |  |
|  |  | TIM1_CH1, FMC_D6 | SDRAM (D6) | PE9 | 39 | 40 | PE8 | SDRAM (D5) | TIM1_CH1N, UART7_Tx |  |  |
|  |  | TIM1_CH2, SPI4_NSS | SDRAM (D8) | PE11 | 41 | 42 | PE10 | SDRAM (D7) | TIM1_CH2N, FMC_D7 |  |  |
|  |  | TIM1_CH3, SPI4_MISO | SDRAM (D10) | PE13 | 43 | 44 | PE12 | SDRAM (D9) | TIM1_CH3N, SPI4_SCK |  |  |
|  |  | TIM1_BKIN | SDRAM (D12) | PE15 | 45 | 46 | PE14 | SDRAM (D11) | TIM1_CH4, SPI4_MOSI |  |  |
|  |  |  | LCD | PB11 | 47 | 48 | PB10 | LCD |  |  |  |
|  |  |  | USB | PB13 | 49 | 50 | PB12 | USB |  |  |  |
|  |  |  | USB | (PB15) | 51 | 52 | (PB14) | USB |  |  |  |
|  |  | USART3_RX | SDRAM (D14) | PD9 | 53 | 54 | PD8 | SDRAM (D13) | USART3_TX |  |  |
|  | ESP_EN |  | LCD | PD11 | 55 | 56 | PD10 | SDRAM (D15) | USART3_CK |  |  |
|  |  |  | LCD | PD13 | 57 | 58 | PD12 | LCD |  |  |  |
|  |  | TIM4_CH4 | SDRAM (D1) | PD15 | 59 | 60 | PD14 | SDRAM (D0) | TIM4_CH3 |  |  |
|  | Input Enable PCB |  |  | PG3 | 61 | 62 | PG2 |  |  |  | Output Enable PCB |
|  |  |  |  | GND | 63 | 64 | GND |  |  |  |  |


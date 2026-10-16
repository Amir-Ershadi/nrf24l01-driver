# Multi-Platform NRF24L01+ C Driver

An enterprise-grade, highly modular, multi-layer C driver for the **NRF24L01+** 2.4GHz RF transceiver. Designed with a strict hardware abstraction layer, it seamlessly supports both **STM32** (HAL) and **ESP32** (ESP-IDF) platforms with zero modifications required in the application logic.

---

## 🌟 Key Features

- **Multi-Platform Support**: Built-in abstraction for STM32 (HAL) and ESP32 (ESP-IDF).
- **Layered Architecture**: Clean separation between Low-level Core/Hardware layers and High-level Application API.
- **Enhanced ShockBurst™ (ESB)**: Full support for Auto-ACK, Hardware Retransmissions, and Dynamic Payload Length (DPL).
- **Multi-Pipe Support**: Full configuration, address mapping, and data handling across all 6 RX pipes.
- **Advanced ESB Features**: ACK Payloads, No-ACK transmission (`W_TX_PAYLOAD_NOACK`), and Carrier Detection (CD/RPD).
- **Non-Blocking & IRQ Ready**: Interrupt-driven event callbacks (`RX_DR`, `TX_DS`, `MAX_RT`).
- **Diagnostics & Channel Scanning**: Dynamic channel occupation scanning and packet loss tracking.

---

## 🏗️ Architecture Overview

The library follows a strict 4-layer architecture to ensure high portability and maintainability:

1. **Application Layer (`NRF_App`)**: Task-oriented functions for state management, data streaming, and event handling.
2. **Device Driver Layer (`NRF`)**: Register-level configurations, shadow state management, and parameter getters/setters.
3. **Hardware Layer (`NRF_Hw`)**: SPI transaction wrappers, CSN/CE pin control, and command encoding.
4. **Core Abstraction Layer (`NRF_Core`)**: Platform-specific implementations (STM32 HAL vs ESP-IDF) covering microsecond/millisecond delays and GPIO/SPI routines.

---

## 📂 Directory Structure

```text
nrf24l01-driver/
│
├── include/                  <-- All header files (.h)
│   ├── nrf.h
│   ├── NRF_App.h
│   ├── nrf_core.h
│   ├── nrf_hw.h
│   └── nrf_reg.h
│
├── src/                      <-- All source files (.c)
│   ├── nrf.c
│   ├── NRF_App.c
│   ├── NRF_Core.c
│   └── NRF_Hw.c
│
├── examples/                 <-- Platform-specific sample applications
│   ├── stm32/
│   │   └── main.c
│   └── esp32/
│       └── main.c
│
├── LICENSE
└── README.md

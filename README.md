# 24v-to-5v-50w-buck-converter-stm32
24V to 5V 50W Digital Controlled Buck Converter Hardware, Software, and Control Design. Features an STM32-based closed-loop PI regulation, 120kHz PWM, custom PCB, and a Nextion HMI display interface.
# STM32 Digitally Controlled 24V to 5V (50W) Buck Converter

![Hardware](https://img.shields.io/badge/Hardware-Custom_PCB-blue.svg)
![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-orange.svg)
![Control](https://img.shields.io/badge/Control-Closed_Loop_PI-success.svg)
![Power](https://img.shields.io/badge/Power-50W_Max-red.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

A high-performance, fully custom **closed-loop digitally controlled buck converter** capable of stepping down a 24V DC input to a highly stable 5V DC output, providing up to 10A (50W) of continuous power. 

This project integrates custom PCB hardware, STM32 embedded software, mathematical control algorithms, and a Nextion HMI (Human-Machine Interface) into a single, cohesive industrial-grade prototype.

## 🌟 Key Features

*   **Robust Power Delivery:** 24V to 5V step-down conversion with up to 10A current capacity.
*   **Digital PI Control:** Real-time Proportional-Integral (PI) control loop running on an STM32 microcontroller to dynamically adjust the 120 kHz PWM duty cycle for perfect voltage/current regulation.
*   **Precision Measurement:** Hall-effect current sensing (ACS712) and voltage divider networks, both buffered by MCP6001 Op-Amps for noise-immune ADC readings via DMA.
*   **Advanced Gate Driving:** FAN73711 high-speed gate driver implemented to efficiently switch the N-Channel power MOSFET with minimal switching losses.
*   **Multi-Stage Auxiliary Power:** Dual XL4015 buck stages (24V→12V, 12V→5V) and an AMS1117-3.3V LDO to safely isolate and power the logic and driver circuits.
*   **Comprehensive Protection:** Input fuse, varistor for overvoltage spikes, inrush current-limiting thermistor, and a software-controlled mechanical relay for emergency cutoff.
*   **Interactive UI:** Nextion display integration via USART for real-time monitoring (V, I, P) and on-the-fly PI tuning, calibration, and mode switching.

## 🛠️ Hardware Architecture

The custom dual-layer PCB is designed with best practices in mind, separating noisy power switching nodes from sensitive analog measuring circuits.

*   **Power Stage:** Features a 33µH power inductor and low-ESR capacitors optimized through MATLAB/Simulink and LTspice simulations to minimize output ripple.
*   **Protection Layer:** Hardware-level protection blocks ensuring safe start-up and transient spike absorption.
*   **Connectivity:** Features industry-standard ports including USB Type-C (for communication/future updates), 4-pin SWD (for flashing/debugging), and a 4-pin USART header for the HMI.

## 💻 Software & Control Strategy

Developed using **STM32CubeIDE** and the HAL library, the software architecture operates in a non-blocking, interrupt-driven manner:

1.  **ADC via DMA:** Voltage and current analog signals are continuously sampled in the background.
2.  **Calibration Layer:** Raw ADC data is scaled using dynamically adjustable calibration multipliers (`V_Cal`, `I_Cal`) and offsets to compensate for real-world component tolerances.
3.  **PI Algorithm:** Compares the measured value against the user-defined reference. The accumulated error (Integral) and current error (Proportional) dynamically update the timer's compare register to adjust the 120 kHz PWM duty cycle.
4.  **UART Parser:** A custom interrupt-driven parser processes commands from the Nextion screen (e.g., `KPV:2.50`, `SETV:5.00`) to tune the system on the fly.

## 📂 Repository Structure

*(You can upload your files into these folders later)*

*   `/Hardware` - KiCad/Altium schematic and PCB layout files, Gerber outputs, and BOM.
*   `/Software` - STM32CubeIDE project workspace, `.c` and `.h` source files.
*   `/Simulation` - LTspice and MATLAB/Simulink models used for control loop and power stage verification.
*   `/Docs` - Project reports, component datasheets, and system diagrams.

## 🚀 Getting Started

1.  Clone this repository: `git clone https://github.com/HakanD1/24v-to-5v-50w-buck-converter-stm32`
2.  Open the `/Software` project in **STM32CubeIDE**.
3.  Compile and flash the firmware onto the STM32F103C8T6 via the SWD interface.
4.  Load the `.tft` file (if provided) onto your Nextion display.
5.  Supply 24V to the main input terminal, and monitor the live parameters on the Nextion screen.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

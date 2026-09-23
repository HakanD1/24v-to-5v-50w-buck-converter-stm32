# STM32 Digitally Controlled 24V to 5V (50W) Buck Converter

![Hardware](https://img.shields.io/badge/Hardware-Custom_PCB-blue.svg)
![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-orange.svg)
![Control](https://img.shields.io/badge/Control-Closed_Loop_PI-success.svg)
![Power](https://img.shields.io/badge/Power-50W_Max-red.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

A high-performance, fully custom **closed-loop digitally controlled buck converter** capable of stepping down a 24V DC input to a highly stable 5V DC output, providing up to 10A (50W) of continuous power. 

This project integrates custom PCB hardware, STM32 embedded software, mathematical control algorithms, and a Nextion HMI (Human-Machine Interface) into a single, cohesive industrial-grade prototype.
> ⚠️ **Project Status:** Rather than turning this into a physical product, I built it as an R&D exercise to combine the full design, simulation, and firmware development process into a single project and put my theoretical background into practice. As a result, the PCB, schematic, and firmware files in this repo have not been manufactured — they've only been verified through design and simulation.

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

*   `/bom` - Bill of Materials (BOM) files.
*   `/cubemx` - STM32CubeIDE project workspace and C/C++ source code.
*   `/gerber` - PCB manufacturing and Gerber files.
*   `/Nextion` - Nextion HMI display interface and GUI files.
*   `/schematic_pcb_3d` & `/STEP file` - 3D models and CAD files of the PCB.
*   `buck converter V6.*` - KiCad schematic and PCB design files.
*   `Buck_converter_24Vto5V.asc` - LTspice simulation model.
*   `buck_converter_24Vto5V.slx` - MATLAB/Simulink model.
*   `REPORT.pdf` - Comprehensive project documentation and theoretical calculations (in Turkish).

## 🚀 Getting Started

1.  Clone this repository: `git clone https://github.com/HakanD1/24v-to-5v-50w-buck-converter-stm32`
2.  Open the `/cubemx` project folder in **STM32CubeIDE**.
3.  Compile and flash the firmware onto the STM32F103C8T6 via the SWD interface.
4.  Load the `.tft` file (located in the `/Nextion` folder) onto your Nextion display.
5.  Supply 24V to the main input terminal, and monitor the live parameters on the Nextion screen.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

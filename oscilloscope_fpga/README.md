# Oscilloscope FPGA Project

This project is a digital oscilloscope implemented on an FPGA, designed to capture and visualize signals with precise timing. It demonstrates FPGA-based signal processing and integration with software for asynchronous data visualization.

---

## Features

* Analog signal capture using FPGA to control and interface to an ADC
* Trigger-based data acquisition with customizable conditions
* Frame-based UART transmission of captured data to a host PC
* Python interface for asynchronous data visualization and plotting
* Sampling rate of 1 Megasample per second
* Modular and parameterizable Verilog design for flexible use

---

## FPGA Design

The FPGA design is implemented in Verilog HDL and includes:

* A UART transmitter for communication with the host PC
* Trigger logic to capture signal frames when user-defined conditions are met
* Data buffering and sequencing to ensure reliable frame transmission
* Synchronization of asynchronous inputs to the FPGA clock domain

The design is parameterized with `CLOCKS_PER_BIT` to adapt to different baud rates for UART communication. The ADC IP from Altera is used for signal sampling at 1 MS/s.

The project was developed using **Quartus** with **Questa Intel FPGA Edition** for simulations and implemented on a **DE10-Lite development board**.

---

## Python Interface

The captured data frames are sent over UART to a Python script, which:

* Reads the incoming UART frames asynchronously
* Parses the signal samples
* Plots the waveform for analysis and visualization

The Python visualization is **asynchronous** to the FPGA sampling, so it provides a post-capture view of signals rather than real-time plotting.

---

## Technologies & Tools

* **Verilog HDL**: Digital design and signal processing
* **Quartus**: FPGA synthesis and project management
* **Questa Intel FPGA Edition**: RTL/gate level simulations and verification
* **ADC IP (Altera)**: Analog-to-digital conversion at 1 MS/s
* **DE10-Lite**: FPGA development board
* **UART**: Communication protocol between FPGA and Python script
* **Python**: Asynchronous data visualization
* **Simulation Scripts**: Testbenches and tcl scripts for design validation

---

## Contact

* GitHub: [https://github.com/EvanRJenkins](https://github.com/EvanRJenkins)
* Email: [JenkinsE@jbu.edu](mailto:JenkinsE@jbu.edu)

**Seeking professional opportunities involving FPGA development and digital signal processing.**





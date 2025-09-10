# Projects by Evan Jenkins

Welcome to my EE project collection! Here you'll find both personal projects and select coursework that showcase my passion for engineering.

---

## Repository Overview

Currently completed projects include:

* **oscilloscope_fpga**
  A digital oscilloscope implemented on an FPGA with a sampling rate 1 MS/s. Features trigger-based data capture with UART transmission to a Python-controller serial port. The design emphasizes system reliability through comprehensive testbenches and proper timing constraints. This project demonstrates organized, hierarchal design and thorough simulations.

* **automated_garden_watering**
  An interrupt-driven embedded system written in C for Arduino Uno. Uses a finite state machine driven by pin-change and hardware timer interrupts. Implements non-blocking operation with configurable parameters and robust error handling including timeout safeguards and sleep modes for reduced power consumption.

* **drop_test_ar200**
  A precision measurement system using ESP32 (Arduino, not ESP IDF) with Bluetooth control of a solenoid circuit. Achieves 8-micron resolution sampling for displacement analysis. The captured data enabled development of a predictive model with 94% accuracy (R² = 0.94).

Each project contains its own directory with source files and more details regarding its design process.

---

## Technologies & Tools

* **C/C++ & Python**
* **Verilog HDL**
* **Communication Protocols**: UART, I2C, SPI, Modbus, Bluetooth
* **Hardware Platforms**: FPGA, ESP32, Arduino

---

## Contact

* GitHub: [https://github.com/EvanRJenkins](https://github.com/EvanRJenkins)
* Email: [jenkinsejbu@gmail.com](mailto:jenkinsejbu@gmail.com) or [jenkinse@jbu.edu](mailto:jenkinse@jbu.edu)

**Seeking embedded software engineering opportunities involving MCU/FPGA development, specifically working with real-time systems.**

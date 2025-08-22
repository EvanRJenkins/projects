import serial
import matplotlib.pyplot as plt
import numpy as np

# configure serial port
"""
baud rate is gross and has small time-skew (but within +- 3% error)
fix this later  in HDL by putting  uart_clk on PLL with c0 49.9968 MHz
that will divide evenly to get 38400 baud
"""
dog = serial.Serial(port="/dev/serial0", baudrate=76687, timeout=1) 

samples_binary = []
samples_voltage = []

try:
    # collect 2048 samples (2 bytes each little-endian)
    while len(samples_binary) < 2048:
        if dog.in_waiting >= 2:
            packet = dog.read(2)
            binary_sample = int.from_bytes(packet, byteorder="little")
            
            # convert to voltage with MAX10 ADC scaling formula
            voltage = binary_sample * (2.5 / 2**12) * 2
            voltage = max(0, min(voltage, 5))  # set max range

            # ignore samples below 0.05 V (arduino inconsistency, remove later)
            if voltage < 0.05:
                continue
            
            samples_binary.append(binary_sample)
            samples_voltage.append(voltage)

    dog.close()
    print("capture done, plotting...\n")

    # x axis (time us)
    DEPTH = len(samples_voltage)
    sample_rate = 1_000_000  # 1 MS/s, change this to match FPGA ADC configuration
    sample_interval_us = 1e6 / sample_rate
    time_axis = np.arange(DEPTH) * sample_interval_us

    # plot configuration
    plt.style.use("dark_background")
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.set_xlim(0, time_axis[-1])
    ax.set_xticks(np.linspace(0, time_axis[-1], 10))
    ax.grid(True, which="both", linestyle="--", linewidth=0.6, alpha=0.7)
    ax.set_axisbelow(True)

    for spine in ax.spines.values():
        spine.set_linewidth(1.5)

    # plot voltage with clipping
    ax.plot(time_axis, samples_voltage, color="yellow", linewidth=1.5, clip_on=True)

    # fix y-axis with some blank space on extremes for aesthetics
    ax.set_ylim(-0.01, 5.01)
    ax.set_autoscale_on(False)

    # make axis labels
    ax.set_xlabel("Time (µs)", fontweight='bold')
    ax.set_ylabel("Voltage (V)", fontweight='bold')

    plt.show()

except KeyboardInterrupt:
    dog.close()
    print("\ncapture cancelled")

# Bare-Metal ATmega328P Drone & Pan-Tilt Controller

This repository contains the complete firmware for a high-performance, low-level controller for a drone or rover chassis. The code is written in C++ for the ATmega328P microcontroller (Arduino Uno/Nano) and intentionally avoids the standard Arduino abstraction layer (`digitalWrite`, `analogWrite`) to achieve maximum efficiency, precise hardware control, and a deep understanding of the AVR architecture.

The system is controlled via Bluetooth (HC-05) and features a sophisticated tri-timer architecture to concurrently manage motor speed, servo positions, and system timing without hardware conflicts.

## Core Features

-   **Direct Register Manipulation:** All I/O, timers, and interrupts are configured by directly setting AVR registers, resulting in faster and smaller code.
-   **Tri-Timer Architecture:** Utilizes all three hardware timers for independent, non-conflicting tasks:
    -   **Timer1 (16-bit):** Dedicated to high-precision 50Hz PWM for two servo motors (pan/tilt).
    -   **Timer0 (8-bit):** Dedicated to Fast PWM for dual-channel motor speed control.
    -   **Timer2 (8-bit):** Dedicated to a system tick interrupt for a low-power sleep mode counter.
-   **Robust Motor Control:** Implements a "Stop-Then-Go" logic pattern to ensure clean state transitions and eliminate intermittent motor behavior.
-   **Incremental Servo Control:** Provides smooth, step-by-step servo movement with software-defined limits to prevent hardware strain.
-   **Power Management:** An automatic deep power-down sleep mode is triggered after a period of inactivity, with a pin-change interrupt to wake the system instantly.
-   **System Reliability:** A hardware Watchdog Timer is implemented to automatically reset the microcontroller if the main program loop ever freezes.

## Hardware Setup

### Required Components
- Arduino Nano (or Uno) with ATmega328P
- L298N Motor Driver Module
- HC-05 Bluetooth Module
- 2x SG90 or similar servo motors for Pan/Tilt
- DC Motors for the chassis
- A separate, robust power supply for the motors (e.g., 2S/3S LiPo, 6xAA battery pack, capable of delivering at least 2A).

### Pinout Configuration

| Function          | Code `enum` Name | AVR Port/Pin | Arduino Pin      |
| ----------------- | ---------------- | ------------ | ---------------- |
| **Motor Speed A** | `Enb_A`          | `PD6`        | **D6** (`~`)     |
| **Motor Speed B** | `Enb_B`          | `PD5`        | **D5** (`~`)     |
| **Pan Servo**     | `pan_servo`      | `PB1`        | **D9** (`~`)     |
| **Tilt Servo**    | `tilt_servo`     | `PB2`        | **D10** (`~`)    |
| **Motor A Dir 1** | `IN1`            | `PB0`        | **D8**           |
| **Motor A Dir 2** | `IN2`            | `PD2`        | **D2**           |
| **Motor B Dir 1** | `IN3`            | `PB5`        | **D13** (LED)    |
| **Motor B Dir 2** | `IN4`            | `PD7`        | **D7**           |

## Firmware Deep Dive

### The "Stop-Then-Go" Motor Logic
To eliminate inconsistent motor behavior during rapid direction changes, the firmware employs a "Stop-Then-Go" strategy. Before any new motion command is processed, a universal stop command is issued by setting all four motor direction pins (`IN1`-`IN4`) to `LOW`. This forces the L298N into a known, stable "brake" state. Only then are the pins for the new direction set to `HIGH`. This prevents state transition conflicts and ensures reliable, predictable motor response.

### Bluetooth App Commands
The firmware expects single-character commands from a Bluetooth serial terminal application.

| Command | Action             |
| ------- | ------------------ |
| `F`     | Move Forward       |
| `B`     | Move Backward      |
| `L`     | Turn Left          |
| `R`     | Turn Right         |
| `T`     | Tilt Servo Up      |
| `X`     | Tilt Servo Down    |
| `S`     | Pan Servo Right    |
| `C`     | Pan Servo Left     |





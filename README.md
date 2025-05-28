# MTRX2700 Embedded Escape Room – Group 3

## Semester
**S1 2025** – The University of Sydney  
**Unit:** MTRX2700 – Mechatronics 2

---

## 🔧 Project Description

This project implements a multi-stage **embedded escape room** using **STM32F3 Discovery Boards**, with each board acting as a unique challenge. Inspired by **Indiana Jones** and **arcade-style games**, users must solve a series of five interactive puzzles to advance through “doors” and reach the final challenge. All systems were developed in **modular C** using **STM32CubeIDE**.

---

## 🧑‍💻 Group Members & Roles

| Name        | Role                        | Responsibilities                                                                                  |
|-------------|-----------------------------|---------------------------------------------------------------------------------------------------|
| Oscar       | Timer Game & Serial Comm    | Designed LED chase minigame with levels and serial UART feedback   |
| Marco       | Gyroscope Compass Lead      | Built IMU compass module with calibration, filtering, and directional feedback via STM32 LEDs     |
| James       | Riddle & Morse Code Lead    | Created interactive riddle and Morse code system using PWM buzzer output and GUI planning         |
| Ben         | Pressure Sensor Developer   | Developed analog pressure plate puzzle using voltage divider and ADC interpretation               |
| Johnathan   | Memory Game  | Implemented LED/button memory game with modular logic and physical user interface enhancements     |

---

## 🗂️ Challenge Overview

| Challenge         | Description                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| Memory Game       | Repeat LED/button sequences across levels with increasing difficulty        |
| Timer Game        | Stop a moving LED on a target; failure triggers buzzer |
| Pressure Plate    | Simulate idol-swap puzzle using a weighted sensor plate and motor feedback |
| Compass Navigation| Guide player toward North using gyroscope/magnetometer heading detection    |
| Morse Code Riddle | Decode a PWM buzzer message to unlock the final clue or instruction         |


---

## ⚙️ Software Architecture

- **Modular C** code (`.c/.h` files) with clear separation
- **Interrupt-driven UART** and **timers**
- **Function pointer callbacks** for modularity
- **Peripheral setup** (GPIO, ADC, PWM, I2C/SPI as needed)
- **Command parsing** via serial interface
- **3D-printed props** and physical interfaces for interactivity

---

## 🧪 Testing Plan

| Module             | Test Description                              | Method                                 | Expected Result                        | Status |
|--------------------|-----------------------------------------------|----------------------------------------|----------------------------------------|--------|
| Memory        | LED/button sequence validation                | Repeat correct pattern                 | Player progresses/fails appropriately  | ✅     |
| Timer Game         | LED stop timing and feedback                  | Button press at correct moment         | UART and LED indicate result           | ✅     |
| Pressure Plate     | Weight change detection                       | Swap object on plate                   | LED/buzzer response triggered          | ✅     |
| Compass            | Heading accuracy and stability                | Rotate board, track direction          | Accurate feedback toward North         | ✅     |
| Morse Code         | Buzzer playback and decode timing             | Listen and interpret Morse code        | Player identifies correct answer       | ✅     |

---

## 🖥️ Getting Started

### Requirements

- STM32F3 Discovery Boards (x6)
- STM32CubeIDE
- USB cables
- Serial terminal (Cutecom / PuTTY)
- Breadboards, resistors, buttons, buzzer, sensors, LEDs, transistors

### Build Instructions

1. Clone this repository.
2. Import challenge folders into STM32CubeIDE (`File > Import > Existing Projects`).
3. Build and flash code to STM32 boards.
4. Open a serial terminal (115200 baud) and interact with each challenge.

---

## 🏁 Integration and Flow

Players must **complete each puzzle** to progress through a sequence of five “doors.”  
Each STM32 board acts as one level. Upon solving a puzzle, the system:
- Provides success/failure feedback (LED, buzzer, motor)
- Sends or triggers the next challenge (via UART or external signal)

The storyline culminates in a **final pressure plate challenge**.

---

## ✅ Assessment Checklist

- [x] Modular `.c/.h` code structure
- [x] Fully working challenge boards
- [x] Interrupt-driven UART and timers
- [x] Physical puzzle interfaces
- [x] 3D-printed components used
- [x] Clean version-controlled repo
- [x] All features tested and documented
- [x] README includes updated roles and structure

---


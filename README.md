# MTRX2700 Treasure Hunt Training – Group 3

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
| Ben         | Pressure Sensor Developer   | Developed analog pressure plate using voltage divider and ADC interpretation, implement motor     |
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

## Challenge 1 and 2

### UART Messaging Protocol

This project uses UART to communicate between the STM32 microcontroller and a Python-based GUI which is ran locally on the device using vscode. The controller sends  messages to the GUI, which the GUI then displays in a organised and themed manner. The GUI can respond to user interaction either through the microntroller and UART or inputs directly into the GUI in the form of answers.

#### Message Structure

- **Each message** is an array of strings, sent line by line
- **Lines** are terminated with standard line endings (`\r`, `\r\n`)
- **Blank lines** are transmitted as `\r\n` to create visual spacing in the GUI
- **End of message** is  (`~`) at the end of the final line

#### Example C Implementation

```c
// Example function to send the intro message over UART
void print_intro(void) {
    const char *message_lines[] = {
        "Welcome to Treasure Hunter Training!",
        // ... lines omitted for brevity ...
        "Press the blue button to begin your journey!~"  // End marker
    };

    for (int i = 0; i < sizeof(message_lines) / sizeof(message_lines[0]); i++) {
        const char *line = message_lines[i];
        const char *to_send = (line[0] == '\0') ? "\r\n" : line;

        for (int j = 0; j < strlen(to_send); j++) {
            while (!(USART1->ISR & USART_ISR_TXE)); // Wait until TX buffer is empty
            USART1->TDR = to_send[j];
        }
        while (!(USART1->ISR & USART_ISR_TC));  // Wait until transmission complete
    }
```
#### How Button Presses Are Checked

- At the start, the code selects a random target LED (between 1 and 7) and flashes it briefly. The player needs to remember which LED was the target.
- The game then cycles the white LEDs, lighting them up one at a time from left to right and back again.
- When the player presses the button, the `stop_chase` flag is set. This is usually done inside a button interrupt.
- The program checks which LED was last lit before the button press (this value is stored as `final_led`).
    - If `final_led` matches the target LED chosen at the beginning, the player wins the round. The game continues to the next stage or resets as needed.
    - If `final_led` does not match the target, the red LED (or possibly a buzzer, depending on configuration) flashes on and off three times to indicate failure.
- After either outcome, the game resets so another attempt can be made.
- Missing the correct timing or remembering the wrong LED will result in the failure indication.

The logic is straightforward but can be challenging depending on the speed and attention of the player.

#### How the Morse Code Works
Any word or string can be picked by changing the input in the function playmorse(). Each letter is converted to its Morse code, then the output (LED or sound) turns on and off with correct timing for dots, dashes, letters, and word spaces—making the message readable in Morse. In the end we used an LED as the piezo buzzer we have is too quiet to hear properly in a loud classroom setting. We thought about implementing both but the PWM that the buzzer requires is not compatible with the LED.


- There is a  `morse_table` array holds the mapping of each letter and symbol. Each entry has the character and its Morse code, like `'A', ".-"` and so on.
- When the function needs to play a message, it looks up every letter using the `lookup_morse()` function, which is case-insensitive.
- The code then loops through every symbol (dot or dash) in that string:
    - It turns on a signal (could be an LED or a buzzer) with `pwm_on()`.
    - How long it stays on depends on if it’s a dot or a dash, each with its own individual timing
    - After each dot or dash, the output is turned off again with `pwm_off()`.

### Breadboard Setup

The breadboard configuration uses 7 white LEDs and 1 red LED, each controlled via separate GPIO pins on the STM32 microcontroller:

- **White LEDs (PA1–PA7):**
  - Each white LED is connected to an individual STM32 GPIO pin (PA1 through PA7).
  - Each LED is wired in series with its own resistor and NPN transistor.
  - The PAx pin (where x = 1–7) provides a small current to the transistor base, allowing a larger current to flow from the breadboard’s VCC rail, through the resistor and LED, then to ground.
  - This configuration offloads LED current from the microcontroller, preventing excessive draw on the STM32’s pins.

- **Red LED (PA15 – Morse Code Output):**
  - The red LED is set up similarly, using a resistor and NPN transistor.
  - It is connected to PA15, which is mapped to the TIM2 timer peripheral for PWM or precise Morse code signaling in the code.
  - Again, the transistor switches current from the VCC supply through the LED and resistor to ground, triggered by the PA15 pin.

**Why transistors?**  
The STM32 can’t safely supply enough current to power multiple LEDs directly. Transistors act as switches, letting the board’s power rail supply the required current while only a small control signal is needed from the microcontroller.

**Wiring summary:**
- Each PAx pin → 100Ω resistor (base) → NPN transistor base.
- Collector: VCC (after passing through ~100Ω current-limiting resistor and LED).
- Emitter: Ground.
- 
#### Example Breadboard Setup

Below is a photo of the actual breadboard wiring for this project:

<img src="IMG_2079.jpg" alt="Breadboard Setup" width="400"/>

---
# COMPASS IMPLEMENTATION
### IMU Compass using STM32F3 Discovery Board

This project implements a digital compass using the STM32F3 Discovery Board's built-in **gyroscope**, **accelerometer**, and **magnetometer** (IMU sensors).

---

#### 📌 Project Workflow

**1. Reading Sensor Data**  
- The STM32F3 Discovery Board provides BSP (Board Support Package) functions to read raw data directly from the IMU sensors via I2C.

**2. Calibration**  
- Even at rest, sensors produce small bias values.
- Calibration involves:
  - Keeping the board still
  - Collecting multiple raw samples
  - Averaging to calculate bias
  - Subtracting the bias from future readings

**3. Filtering (Sensor Fusion)**  
- Sensor values fluctuate even after calibration.
- We apply the **Madgwick filter** to fuse 9-axis IMU data and compute orientation.
- Orientation is defined in the **ENU (East-North-Up)** frame.

**ENU Direction Cosine Matrix**  
_Converts vector from body frame to navigation frame_:

<img src="Screenshot%202025-05-29%20142000.png" width="1000" height="400" alt="ENU Direction Cosine Matrix" />

**4. Displaying Compass Direction**  
- With pitch and roll computation, we compute the **heading angle**.
- The heading is mapped to cardinal directions and displayed using onboard LEDs.

---

#### 🔧 Example Code Snippet

```c
// Gyro: from mdps to rad/s
	  float gx = corrected_gyro_x * (M_PI / 180000.0f);
	  float gy = corrected_gyro_y * (M_PI / 180000.0f);
	  float gz = corrected_gyro_z * (M_PI / 180000.0f);

	  // Accel: milli-g => convert to g
	  float ax = corrected_acc_x / 1000.0f;
	  float ay = corrected_acc_y / 1000.0f;
	  float az = corrected_acc_z / 1000.0f;

	  // Madgwick Filter
	  float mx = corrected_mag_x;
	  float my = corrected_mag_y;
	  float mz = corrected_mag_z;

	  MadgwickAHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz);

	  // roll and pitch formula from Madgwick Filter formula
	  float roll  = atan2f(q0 * q1 + q2 * q3, 0.5f - q1 * q1 - q2 * q2);
	  float pitch = asinf(-2.0f * (q1 * q3 - q0 * q2));

	  // Projectile value
	  float Xh = mx * cosf(pitch) + mz * sinf(pitch);
	  float Yh = mx * sinf(roll) * sinf(pitch) + my * cosf(roll) - mz * sinf(roll) * cosf(pitch);

	  // Calculate heading of magnetometer in degree
	  float mag_heading = atan2f(-Yh, Xh) * (180.0f / M_PI);
	  if (mag_heading < 0) mag_heading += 360.0f;


---

# Challenges 4 and 5
## MEMORY CHALLENGE
The player must repeat LED/button sequences across levels with increasing difficulty.

There are 3 levels in total, with varying sequences and light-up frequencies. The module is capable of running on a single STM32 timer and has a completion callback that can be set by other modules. The completion callback will run when the player is successful. Skipping has been implemented for debugging purposes. The module can be skipped by clicking the blue button 3 times when the debug (yellow) LED is flashing. The module is started by pressing the blue button and only requires `init_buttons()` to be called prior to entering an infinite loop.

To modify the existing patterns more `Level`s can be added to the beats.c file. These levels can be configured according to the members of the `Level` struct in `beats.h`, to change the delay/number of levels.

Additional `finally*` functions can be added and integrated to change the functionality at the end of levels.

## BOULDER RUN

Equipment:
1. STM32 x1
2. male-female wires x6
3. 1k resistor x1
4. Op-amp x1
5. Breadboard wires
6. Pressure resistor
7. Micro serveo
8. tooth pick x2
9. Boulder run side x2
10. Ramp insert x2
11. Motor holder x1
12. Pressureplate stable x1
13. PressurePlateMTRX2700 x2

Construction:
1. Break the toothpicks in half and insert into the holes of one of the pressure plates.
2. Glue the toothpicks up straight in the holes.
3. Insert the stabilizers in the toothpicks.
4. Place he other pressure plate ontop of the toothpicks.
5. Glue the pressure resistor in the middle of the two boards and solder wires to extend the pins.
6. Using a breadboard make a op-amp buffer with the + input connected to a voltage divider with the 1k resistor connected to power and the pressure resistor to ground.
7. Connect PA0 to the output of the buffer, 3V from the board to the breadboard and ground to the breadboard.
8. Connect the ramp sides, motor holder and ramp inserts together to make the boulder ramp.
9. Place the motor in the motor holder and connect the power to 5V, ground to ground and the PWM to PA15.

How it works:

The code sets pin PA0 to an ADC and PA15 to a PWM with TIM2. The code starts by taking the current analog signal [set_voltage = ReadADC()] from the pin and uses it as a base range that must be kept within with a buffer variable. Then the code constantly reads the analog input from PA0 and compares it to the base voltage with a buffer. Such as [if (current_voltage - buffer > set_voltage || current_voltage + buffer < set_voltage)]. If a change is detected that is outside of the scope one LED will turn on with [set_led_register(1)] to indicate to the user that they have one second [delay(1000)] to replace a similar mass on the pressure plate. After one second if the analog voltage read is within the scope than nothing happens and the LED turns off. However if the read voltage is still out of the scope than the motor will turn by 45 degrees counter-clockwise [TIM2->CCR1 = 2000] then [delay(250); TIM2->CCR1 = 0;] (to simulate 45 degrees) and release the boulder. 

Modifications:
- Buffer can be increased to make the game easier or decreased to make it harder.
- The delay time can be increased/decreased to make the grace period between switching the masses easier/harder.  

Test:

Run the code with an object on the pressure plate, slowly add weight to the board and observe if an LED turns on, if it does, do the opposite and remove a little bit of weight slowly and check if an LED turns on. If it does the pressure plate works and the board is reading the pins correctly. If not check that the buffer isn't too big also remove the pressure plate cover and test just the resistor. Can also use a multimeter to check that the voltage divider is working.

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
| Pressure Plate     | Weight change detection                       | Swap object on plate                   | LED/motor response triggered           | ✅     |
| Compass            | Heading accuracy and stability                | Rotate board, track direction          | Accurate feedback toward North         | ✅     |
| Morse Code         | Buzzer playback and decode timing             | Listen and interpret Morse code        | Player identifies correct answer       | ✅     |

---

## 🖥️ Getting Started

### Requirements

- STM32F3 Discovery Boards (x6)
- STM32CubeIDE
- USB cables
- Serial terminal (Cutecom / PuTTY)
- Breadboards, resistors, buttons, buzzer, sensors, LEDs, transistors, op-amp

### Build Instructions

1. Clone this repository.
2. Import challenge folders into STM32CubeIDE (`File > Import > Existing Projects`).
3. Build and flash code to STM32 boards.
4. Open a serial terminal (115200 baud) and interact with each challenge.

---

## 🏁 Integration and Flow

Players must **complete each puzzle** to progress through a sequence of five “doors.”  
Each module acts as one level. Upon solving a puzzle, the system:
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


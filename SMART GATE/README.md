# Smart gate

This Arduino-based security system uses an IR sensor to detect the presence of a person, prompts for an RFID card or a keypad PIN, and controls a servo motor (e.g., door or barrier). It displays instructions and feedback on a 16x2 I2C LCD screen and logs all system events to the Serial Monitor.

---

## Features

-  IR Sensor detects when a person approaches
-  RFID (RC522) reads user card
-  Keypad (4x4) allows PIN input if RFID is invalid
-  Access granted with either:
  - Valid RFID card (`UID: 34 69 14 4C`)
  - Correct PIN code: `12345`
-  Servo motor rotates 120° for access, then resets to 0° after user leaves
-  LCD I2C (16x2) displays system messages
-  Serial Monitor logs all actions and results

---

##  Components Required

| Component            | Quantity |
|----------------------|----------|
| Arduino UNO          | 1        |
| MFRC522 RFID Module  | 1        |
| IR Obstacle Sensor   | 1        |
| Servo Motor (e.g., SG90 or MG996R) | 1 |
| Keypad 4x4           | 1        |
| LCD 16x2 I2C         | 1        |
| Jumper Wires         | many     |
| Breadboard           | 1        |

---

##  Pin Connections

### RFID RC522

| RC522 Pin | Arduino UNO |
|-----------|-------------|
| SDA       | D10         |
| SCK       | D13         |
| MOSI      | D11         |
| MISO      | D12         |
| RST       | D9          |
| 3.3V/GND  | 3.3V / GND  |

### IR Sensor
- **OUT → D2**

### Keypad 4x4

| Row/Col | Arduino UNO |
|---------|-------------|
| R1–R4   | D3, D4, D5, D6 |
| C1–C4   | A0, A1, A2, A3 |

### Servo
- **Signal → D8**

### LCD I2C
- **SDA → A4**, **SCL → A5**

---

##  How It Works

1. **Idle State** – Displays “System Ready...”
2. **Detection** – IR sensor detects object → LCD says "Scan your card"
3. **Card Check** – Waits 5 seconds for RFID scan
   - If valid → Access granted, servo rotates
   - If invalid/missing → Prompt for 5-digit PIN
4. **PIN Check**
   - If correct → Access granted, servo rotates
   - If incorrect → Access denied
5. **Exit** – Waits for person to leave (IR HIGH) → Wait 5 seconds → Servo returns to closed

---

##  Libraries Used

- `LiquidCrystal_I2C`
- `Servo`
- `SPI`
- `MFRC522`
- `Keypad`

Install them via **Library Manager** in Arduino IDE.

---

##  Example

```text
System initialized and ready.
IR sensor detected object.
Waiting for RFID card...
Card UID: 34 69 14 4C
Card is valid.
Access granted by RFID.
Waiting for object to leave...
Object left. Waiting 5 seconds before closing servo.
Servo returned to 0°.
Ready for next user.

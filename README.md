#  Conditional Monitoring System using ESP32

##  Mission
To build an intelligent monitoring system capable of detecting environmental changes (temperature, gas concentration, and object proximity) and respond in real time with automated controls and web-based visualization.

##  Objective
- To **monitor** environmental parameters (temperature and gas).
- To **trigger** alerts (buzzer/bell, fan, LED) based on predefined thresholds.
- To **log** the system behavior and status changes with timestamps.
- To **display** sensor values and system status on a web interface hosted by ESP32.

##  Goal
Create a **low-cost**, **efficient**, and **scalable** embedded system for environmental condition monitoring with **real-time feedback** and **remote access** through a web dashboard.

---

## Project Overview

This project uses an **ESP32** microcontroller to monitor temperature (via DHT11), gas levels (via MQ-2), and object proximity (via ultrasonic sensor). Depending on sensor readings, the system activates:
- a **fan** if the temperature is too high,
- a **buzzer/bell** if gas concentration is too high,
- and toggles an **LED** when an object is detected nearby.

All actions are logged with timestamps, and a web interface is provided for real-time monitoring and control.

---

##  Components Used

| Component             | Quantity | Function                                       |
|----------------------|----------|------------------------------------------------|
| ESP32 Dev Board      | 1        | Main controller with built-in Wi-Fi           |
| DHT11 Sensor         | 1        | Temperature and Humidity sensing              |
| MQ-2 Gas Sensor      | 1        | Detects gas levels in the environment         |
| Ultrasonic Sensor    | 1        | Measures object proximity                     |
| Relay Module / Buzzer| 1        | Rings when gas level exceeds safe limit       |
| Fan (or LED)         | 1        | Cools the area if temperature is high         |
| LED                  | 1        | Toggles when object detected                  |
| Resistors, Wires     | -        | Connections and protection                    |

---

##  Functional Features

-  **Temperature Monitoring**: Activates fan above 33°C.
-  **Gas Monitoring**: Rings bell when MQ-2 analog value > 500.
-  **Proximity Detection**: Toggles LED when object is < 50cm.
-  **Real-Time Dashboard**: View current status via Wi-Fi.
-  **Manual Control**: Toggle fan and bell from browser.
-  **State Logging**: View all changes in fan/bell states with timestamps.
-  **Serial Debugging**: Outputs live sensor data to serial monitor.

---

##  Web Dashboard

- Access the web interface at `http://192.168.4.1` after connecting to the ESP32 AP.
- Dashboard Pages:
  - `/` → Home Page
  - `/temperature` → Displays real-time temperature & humidity
  - `/gas` → Displays real-time gas level & buzzer status
  - `/state_log` → Logs all ON/OFF state changes of fan and bell

---

## System Logic

###  Temperature:
- **Condition:** > 33°C
- **Action:** Turns **fan ON**
- **Reset:** Fan OFF below threshold

###  Gas:
- **Condition:** MQ-2 analog value > 500
- **Action:** Rings **buzzer**
- **Reset:** Buzzer OFF below threshold

###  Proximity:
- **Condition:** Distance < 50 cm
- **Action:** Toggles **LED** ON/OFF

###  State Logging:
- Each fan or bell activation is saved with a **timestamp**
- Unchanged states are logged every 30 seconds

---

##  How to Use

1. Upload the project code to the ESP32.
2. Power on the ESP32 — it starts an access point `ESP32-Monitor`.
3. Connect your PC/Phone to the Wi-Fi network:
   - **SSID**: `ESP32-Monitor`
   - **Password**: `12345678`
4. Open `http://192.168.4.1` in your browser.
5. Explore the dashboard and view system logs.

---

##  Future Enhancements

- Export state logs to CSV
- Add email/SMS alerts for gas leaks
- Live charts with Chart.js
- Store logs in NVS or SD card
- Connect to external database or cloud server

---

##  Contributing

Want to contribute ideas or features? Fork this repo and send a pull request.

---

##  Author

**Munyaneza El-Chadai**  
Student – ACEJ/Karama  
Passionate about Embedded Systems & IoT  
Year: 2023–2026
---

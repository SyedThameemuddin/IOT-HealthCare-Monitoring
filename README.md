
# IoT Health Monitoring System

### Overview
This project aims to create a health monitoring system using an **ESP32**, **Heart Rate Pulse Sensor**, **DHT11 Temperature Sensor**, and IoT platforms such as **ThingSpeak** and **IFTTT**. It measures a patient's heart rate and body temperature and uploads this data to ThingSpeak for real-time monitoring. Additionally, it triggers an alert using IFTTT if the heart rate exceeds 120 BPM or the temperature goes above 35°C.

### Features
- **Real-time Health Monitoring**: Monitors heart rate (BPM) and body temperature.
- **Data Logging**: Sends the heart rate and temperature data to the **ThingSpeak** cloud for storage and analysis.
- **Automated Alerts**: Triggers an IFTTT alert if any critical condition is detected, like heart rate > 120 BPM or temperature > 35°C.
- **Wi-Fi Connectivity**: Uses ESP32 to connect to the internet for data transmission.

---

### Components Used
- **ESP32 Microcontroller**
- **Heart Rate Pulse Monitor Sensor**
- **DHT11 Temperature Sensor**
- **Jumper Wires**
- **Breadboard**

### Libraries Required
Ensure you have these libraries installed in the Arduino IDE:
1. **PulseSensorPlayground** – for managing the Pulse Sensor.
2. **DHT** – for interfacing with the DHT11 temperature sensor.
3. **WiFi** – to connect the ESP32 to a Wi-Fi network.
4. **HTTPClient** – to send data to ThingSpeak and trigger IFTTT alerts.

To install libraries, go to **Sketch** -> **Include Library** -> **Manage Libraries**, then search and install the required libraries.

---

### Circuit Diagram
| **Component**           | **ESP32 Pin**  | **Description**                 |
|-------------------------|----------------|---------------------------------|
| Heart Rate Sensor VCC    | 3.3V           | Power for the sensor            |
| Heart Rate Sensor GND    | GND            | Ground                          |
| Heart Rate Sensor Signal | GPIO34         | Reads heart rate signal         |
| DHT11 VCC                | 3.3V           | Power for DHT11                 |
| DHT11 GND                | GND            | Ground                          |
| DHT11 Data Pin           | GPIO23         | Reads temperature data          |
| On-board LED             | GPIO2          | Blinks when heart rate detected |

---

### ThingSpeak and IFTTT Setup
1. **ThingSpeak**: Create a ThingSpeak account and set up a new channel. Get the **API Key** from ThingSpeak and replace it in the code (`apiKey` variable).
2. **IFTTT**: Set up an IFTTT Webhook to trigger alerts for heart rate or temperature thresholds. Replace the **IFTTT Webhook key** and **event name** in the code with your own (`ifttt_key` and `ifttt_event_name` variables).

---

### Code Explanation
- **Heart Rate Monitoring**: 
  - The `PulseSensorPlayground` library reads the heart rate signal from the Pulse Monitor Sensor on GPIO 34.
  - The `sawStartOfBeat()` method detects a heartbeat and calculates the BPM.
- **Temperature Monitoring**:
  - The **DHT11** sensor measures the temperature using the `readTemperature()` method.
- **ThingSpeak Integration**:
  - Every 15 seconds, the ESP32 sends the latest heart rate and temperature data to ThingSpeak using **HTTPClient**.
- **IFTTT Alert**:
  - If the heart rate exceeds 120 BPM or the temperature is higher than 35°C, an alert is sent via IFTTT using a POST request.

---

### How to Use
1. **Connect** the components as described in the circuit diagram.
2. **Upload** the code to your ESP32 via the Arduino IDE.
3. **Monitor the data**: 
   - Open the Serial Monitor in the Arduino IDE to see the heart rate and temperature readings.
   - View the data logged in **ThingSpeak**.
4. **Receive alerts**:
   - If the heart rate or temperature crosses the threshold, receive an alert via **IFTTT**.

---

### Contributing
Feel free to open issues or submit pull requests if you'd like to improve the project.

---

### License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

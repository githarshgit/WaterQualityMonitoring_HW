# Water Quality Monitoring System (RP2040)

This project implements an IoT-based water quality monitoring system using an **Arduino Nano RP2040 Connect** (or compatible board). It continuously measures key water parameters such as **pH**, **Turbidity**, **TDS** (Total Dissolved Solids), and **Water Level** (via Ultrasonic sensor).

The data is processed and sent to an **MQTT Broker** in JSON format for real-time monitoring and is simultaneously logged to a local **SD Card** for offline analysis.

## 🚀 Features
- **Real-time Monitoring**: Measures pH, Turbidity, TDS, and Distance every 2 seconds.
- **IoT Connectivity**: Publishes sensor data to an MQTT broker over WiFi.
- **Offline Logging**: Saves all data records to an SD card key (`data.txt`).
- **Data Filtering**: Implements median filtering for stable readings (especially for TDS).

## 🛠 Hardware Requirements
- **Microcontroller**: Arduino Nano RP2040 Connect (or any board compatible with `WiFiNINA` library).
- **Sensors**:
  - **pH Sensor** (Analog)
  - **Turbidity Sensor** (Analog)
  - **TDS Sensor** (Analog)
  - **Ultrasonic Sensor** (HC-SR04 or compatible)
- **Storage**: Micro SD Card Module + Micro SD Card.
- **Power Supply**: Appropriate power source for the board and sensors.

## 🔌 Pin Configuration

| Component | Pin Type | RP2040 Pin |
|-----------|----------|------------|
| Turbidity | Analog   | `A1`       |
| TDS       | Analog   | `A2`       |
| pH        | Analog   | `A3`       |
| Ultrasonic Trigger | Digital | `3` |
| Ultrasonic Echo | Digital | `2`   |
| SD Card CS | SPI/Digital | `10` |

*Note: Ensure the SD card module is connected to the default SPI pins (MISO, MOSI, SCK) of your board.*

## 💻 Software Dependencies
You need the **Arduino IDE** to compile and upload this code. Install the following libraries via the Arduino Library Manager:

1.  **WiFiNINA** (for WiFi connectivity)
2.  **PubSubClient** (for MQTT)
3.  **SD** (Built-in, for SD Card)
4.  **SPI** (Built-in)

## ⚙️ Configuration
Before uploading, open `WaterQualityMqttRP2040.ino` and modify the following lines with your network and MQTT details:

```cpp
// WiFi Credentials
const char* ssid = "your_ssid";        // Enter your WiFi Name
const char* password = "your_password"; // Enter your WiFi Password

// MQTT Settings
const char* mqtt_server = "broker.hivemq.com"; // Your MQTT Broker URL
const char* mqtt_topic = "your_topic";         // Unique Topic to publish data to
```

## 📊 Data Output Format
The system publishes data to the MQTT topic in the following **JSON format**:

```json
{
  "ph": 7.05,
  "turbidity": 4.50,
  "tds": 120,
  "distance": 15
}
```

*   **ph**: Calculated pH value.
*   **turbidity**: Turbidity in NTU (Nephelometric Turbidity Units) or approx voltage mapping.
*   **tds**: Total Dissolved Solids in ppm.
*   **distance**: Distance measured by ultrasonic sensor in cm.

## 📝 Usage
1.  Connect all sensors to the specified pins.
2.  Insert a formatted SD card into the module.
3.  Update the code with your WiFi and MQTT credentials.
4.  Upload the code to your board.
5.  Open the **Serial Monitor** (Baud: 115200) to view debug logs.
6.  The board will attempt to connect to WiFi and MQTT.
7.  Once connected, data will stream to the serial monitor, MQTT broker, and SD card.

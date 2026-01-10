#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <SD.h>
#include <SPI.h>

// =================== WiFi Credentials ===================
const char* ssid = "your_ssid";
const char* password = "your_password";

// =================== MQTT Broker ===================
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "your_topic";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// =================== Sensor Pins ===================
#define TURB_PIN A1
#define TDS_PIN  A2
#define PH_PIN   A3
#define TRIG_PIN 3
#define ECHO_PIN 2

// SD CARD
#define SD_CS 10

// =================== TDS Variables ===================
#define VREF 5.0
#define SCOUNT 30

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogIndex = 0;

float temperature = 25;

// FUNCTION DECLARATIONS
float readPH();
float readTurbidity();
float readTDS();
int   readDistance();
int   getMedianNum(int bArray[], int iFilterLen);

// =================== MQTT Reconnect ===================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT... ");
    if (client.connect("ArduinoPublisher")) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed. rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 3 sec...");
      delay(3000);
    }
  }
}

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // MQTT
  client.setServer(mqtt_server, mqtt_port);
  reconnect();

  // SD CARD
  Serial.print("Initializing SD card... ");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Init Failed!");
  } else {
    Serial.println("SD Ready.");
  }

  Serial.println("Sensors + MQTT Ready...");
}

// =================== LOOP ===================
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float phValue    = readPH();
  float ntuValue   = readTurbidity();
  float tdsValue   = readTDS();
  int distanceCM   = readDistance();

  String payload = "{";
  payload += "\"ph\":" + String(phValue, 2) + ",";
  payload += "\"turbidity\":" + String(ntuValue, 2) + ",";
  payload += "\"tds\":" + String(tdsValue, 0) + ",";
  payload += "\"distance\":" + String(distanceCM);
  payload += "}";

  Serial.println("Publishing: " + payload);
  client.publish(mqtt_topic, payload.c_str());

  // LOG TO SD
  File file = SD.open("data.txt", FILE_WRITE);
  if (file) {
    file.println(payload);
    file.close();
    Serial.println("Logged to SD.");
  } else {
    Serial.println("SD Write Failed!");
  }

  delay(2000);
}

// =================== PH SENSOR ===================
float readPH() {
  int buf[10];
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(PH_PIN);
    delay(10);
  }

  for (int i = 0; i < 9; i++)
    for (int j = i + 1; j < 10; j++)
      if (buf[i] > buf[j]) {
        int t = buf[i]; buf[i] = buf[j]; buf[j] = t;
      }

  unsigned long avgValue = 0;
  for (int i = 2; i < 8; i++) avgValue += buf[i];

  float voltage = avgValue * 5.0 / 1024 / 6;
  Serial.println(voltage);
  return -5.70 * voltage + 32.225;
}

// =================== TURBIDITY ===================
float readTurbidity() {
  float turbVolt = 0;

  for (int i = 0; i < 800; i++)
    turbVolt += analogRead(TURB_PIN) * 5.0 / 1023.0;

  turbVolt /= 800.0;

  if (turbVolt < 2.5) return 3000;
  return -1120.4 * turbVolt * turbVolt + 5742.3 * turbVolt - 4353.8;
}

// =================== ULTRASONIC ===================
int readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

// =================== TDS ===================
float readTDS() {
  static unsigned long sampleTiming = millis();
  if (millis() - sampleTiming > 40U) {
    sampleTiming = millis();
    analogBuffer[analogIndex] = analogRead(TDS_PIN);
    analogIndex++;
    if (analogIndex == SCOUNT) analogIndex = 0;
  }

  for (int i = 0; i < SCOUNT; i++)
    analogBufferTemp[i] = analogBuffer[i];

  float avgV = getMedianNum(analogBufferTemp, SCOUNT) * VREF / 1024.0;

  float compensation = 1 + 0.02 * (temperature - 25.0);
  float compV = avgV / compensation;

  return (133.42 * compV * compV * compV
        - 255.86 * compV * compV
        + 857.39 * compV) * 0.5;
}

// =================== MEDIAN FILTER ===================
int getMedianNum(int bArray[], int len) {
  int bTab[len];
  for (int i = 0; i < len; i++) bTab[i] = bArray[i];

  for (int j = 0; j < len - 1; j++)
    for (int i = 0; i < len - j - 1; i++)
      if (bTab[i] > bTab[i + 1]) {
        int t = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = t;
      }

  return bTab[(len - 1) / 2];
}

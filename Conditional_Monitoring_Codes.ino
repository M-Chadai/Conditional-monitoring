#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// === DHT11 ===
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === Pins ===
#define GAS_SENSOR_PIN 34
#define FAN_PIN 12
#define BELL_PIN 14
#define TRIG_PIN 5
#define ECHO_PIN 18
#define DETECT_LED 27

// === States ===
bool fanState = false;
bool bellState = false;
bool ledState = false;
bool objectPreviouslyDetected = false;

const char* ssid = "ESP32-Monitor";
const char* password = "12345678";

WebServer server(80);

// === Time Client ===
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC+1

// === Logs ===
String stateLog = "";

unsigned long lastConditionCheck = 0;
unsigned long lastUltrasonicCheck = 0;
unsigned long lastPeriodicLog = 0;

// === Logging Function ===
void logState(const String& device, bool state, bool changed = true) {
  String timestamp = timeClient.getFormattedTime();
  String status = changed ? (state ? "ON" : "OFF") : (state ? "ON (UNCHANGED)" : "OFF (UNCHANGED)");
  stateLog += "<tr><td>" + timestamp + "</td><td>" + device + "</td><td>" + status + "</td></tr>";
  Serial.println(timestamp + " - " + device + " " + (changed ? "changed to " : "is still ") + (state ? "ON" : "OFF"));
}

// === Distance Sensor ===
long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  long distance = duration * 0.034 / 2;
  return distance;
}

// === Web Handlers ===
void handleRoot() {
  String html = R"rawliteral(
  <html><head><title>Conditional Monitoring</title>
  <style>body{font-family:Arial;text-align:center}a{padding:15px;background:#3498db;color:#fff;text-decoration:none;margin:10px;display:inline-block;border-radius:10px}</style>
  </head><body><h1>Conditional Monitoring Dashboard</h1>
  <a href="/temperature">Temperature Page</a>
  <a href="/gas">Gas Page</a>
  <a href="/state_log">State Log</a>
  </body></html>)rawliteral";
  server.send(200, "text/html", html);
}

void handleTempPage() {
  String html = R"rawliteral(
  <html><head><title>Temperature Monitoring</title>
  <style>body{font-family:Arial;text-align:center;background:#e0f7fa}
  table{margin:auto;border-collapse:collapse;width:80%}th,td{padding:10px;border:1px solid #00796b}th{background:#b2dfdb}
  a,button{margin:10px;padding:10px 20px;border:none;border-radius:5px;background:#00796b;color:white;text-decoration:none;cursor:pointer}
  </style></head>
  <body><h1>Temperature Monitoring</h1>
  <table id="log"><tr><th>Time</th><th>Temperature (°C)</th><th>Humidity (%)</th></tr></table>
  <button onclick="toggleFan()">Toggle FAN (Current: <span id='fanStatus'>OFF</span>)</button><br>
  <a href="/">Back</a>
  <script>
    let logData = document.getElementById("log");
    setInterval(() => {
      fetch('/temp_data').then(r => r.json()).then(data => {
        document.getElementById("fanStatus").textContent = data.fan ? "ON" : "OFF";
        let row = logData.insertRow(-1);
        row.innerHTML = `<td>${data.time}</td><td>${data.temp}</td><td>${data.humidity}</td>`;
      });
    }, 5000);
    function toggleFan() { fetch('/toggle_fan'); }
  </script></body></html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleGasPage() {
  String html = R"rawliteral(
  <html><head><title>Gas Monitoring</title>
  <style>body{font-family:Arial;text-align:center;background:#fffbe6}
  table{margin:auto;border-collapse:collapse;width:60%}th,td{padding:10px;border:1px solid #e67e22}th{background:#ffe0b2}
  a,button{margin:10px;padding:10px 20px;border:none;border-radius:5px;background:#e67e22;color:white;text-decoration:none;cursor:pointer}
  </style></head>
  <body><h1>Gas Monitoring</h1>
  <table id="log"><tr><th>Time</th><th>Gas (Raw)</th></tr></table>
  <button onclick="toggleBell()">Toggle BELL (Current: <span id='bellStatus'>OFF</span>)</button><br>
  <a href="/">Back</a>
  <script>
    let logData = document.getElementById("log");
    setInterval(() => {
      fetch('/gas_data').then(r => r.json()).then(data => {
        document.getElementById("bellStatus").textContent = data.bell ? "ON" : "OFF";
        let row = logData.insertRow(-1);
        row.innerHTML = `<td>${data.time}</td><td>${data.gas}</td>`;
      });
    }, 5000);
    function toggleBell() { fetch('/toggle_bell'); }
  </script></body></html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleStateLog() {
  String html = "<html><head><title>State Log</title><style>body{font-family:Arial;text-align:center;background:#f9f9f9}table{margin:auto;border-collapse:collapse;width:80%}th,td{border:1px solid #333;padding:10px;}th{background:#ccc}</style></head><body><h1>Fan/Bell State Changes</h1><table><tr><th>Time</th><th>Device</th><th>State</th></tr>" + stateLog + "</table><br><a href='/'>Back</a></body></html>";
  server.send(200, "text/html", html);
}

void handleTempData() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  String now = timeClient.getFormattedTime();

  StaticJsonDocument<200> doc;
  doc["temp"] = isnan(temp) ? 0 : temp;
  doc["humidity"] = isnan(humidity) ? 0 : humidity;
  doc["fan"] = fanState;
  doc["time"] = now;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleGasData() {
  int raw = analogRead(GAS_SENSOR_PIN);
  String now = timeClient.getFormattedTime();

  StaticJsonDocument<150> doc;
  doc["gas"] = raw;
  doc["bell"] = bellState;
  doc["time"] = now;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleToggleFan() {
  fanState = !fanState;
  digitalWrite(FAN_PIN, fanState);
  logState("Fan", fanState);
  server.send(200, "text/plain", "OK");
}

void handleToggleBell() {
  bellState = !bellState;
  digitalWrite(BELL_PIN, bellState);
  logState("Bell", bellState);
  server.send(200, "text/plain", "OK");
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(FAN_PIN, OUTPUT);
  pinMode(BELL_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(DETECT_LED, OUTPUT);
  digitalWrite(DETECT_LED, LOW);

  WiFi.softAP(ssid, password);
  timeClient.begin();
  timeClient.update();

  server.on("/", handleRoot);
  server.on("/temperature", handleTempPage);
  server.on("/gas", handleGasPage);
  server.on("/state_log", handleStateLog);
  server.on("/temp_data", handleTempData);
  server.on("/gas_data", handleGasData);
  server.on("/toggle_fan", handleToggleFan);
  server.on("/toggle_bell", handleToggleBell);

  server.begin();
  Serial.println("System started. Web server running.");
}

// === Main Loop ===
void loop() {
  server.handleClient();
  timeClient.update();

  unsigned long now = millis();

  // Every 2 seconds: Check conditions
  if (now - lastConditionCheck >= 2000) {
    float temp = dht.readTemperature();
    int gasRaw = analogRead(GAS_SENSOR_PIN);

    if (temp > 33 && !fanState) {
      fanState = true;
      digitalWrite(FAN_PIN, HIGH);
      logState("Fan", true);
    } else if (temp <= 33 && fanState) {
      fanState = false;
      digitalWrite(FAN_PIN, LOW);
      logState("Fan", false);
    }

    if (gasRaw > 55 && !bellState) {
      bellState = true;
      digitalWrite(BELL_PIN, HIGH);
      logState("Bell", true);
    } else if (gasRaw <= 55 && bellState) {
      bellState = false;
      digitalWrite(BELL_PIN, LOW);
      logState("Bell", false);
    }

    lastConditionCheck = now;
  }

  // Every 2 seconds: Ultrasonic detection + serial output
  if (now - lastUltrasonicCheck >= 2000) {
    long distance = getDistanceCM();
    Serial.print("Ultrasonic Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance <= 50) {
      Serial.println("Object detected within 50cm.");
      if (!objectPreviouslyDetected) {
        ledState = !ledState;
        digitalWrite(DETECT_LED, ledState);
        objectPreviouslyDetected = true;
        Serial.print("LED toggled ");
        Serial.println(ledState ? "ON" : "OFF");
      }
    } else {
      if (objectPreviouslyDetected) {
        Serial.println("Object moved away. Ready to detect again.");
      }
      objectPreviouslyDetected = false;
    }

    lastUltrasonicCheck = now;
  }

  // Every 30 seconds: log current state (unchanged)
  if (now - lastPeriodicLog >= 30000) {
    logState("Fan", fanState, false);
    logState("Bell", bellState, false);
    lastPeriodicLog = now;
  }
}

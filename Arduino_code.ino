#include <WiFi.h>                 // ESP32 Wi-Fi support
#include <Firebase_ESP_Client.h>  // Firebase library (RTDB, Auth, Config)
#include <Adafruit_BMP280.h>      // BMP280 temperature & pressure sensor
#include <Adafruit_Sensor.h>      // Required base class for Adafruit sensors
#include <WiFiUdp.h>              // UDP client for NTP time sync
#include <NTPClient.h>            // To get network time

const firebaseConfig = {
        apiKey: "AIzaSyCcQZ2ULN_aJ7ywjnMchEmJnyBKV2bkPv0",
        authDomain: "health-monitoring-e3c18.firebaseapp.com",
        databaseURL:"https://health-monitoring-e3c18-default-rtdb.firebaseio.com/",
        projectId: "health-monitoring-e3c18",
        storageBucket: "health-monitoring-e3c18.appspot.com", 
        messagingSenderId: "545283997480",
        appId: "1:545283997480:web:113d658e61c34fbc5ada1d"
};
firebase.initializeApp(firebaseConfig);
const db = firebase.database();
const heartRateEl = document.getElementById("heartRate");
const pressureEl = document.getElementById("pressure");
const temperatureEl = document.getElementById("temperature");
const timestampEl = document.getElementById("timestamp");
const vitalsRef = db.ref("patients/patient1/vitals");
let latestKey = "";
// Format timestamp to readable date & time
function formatTimestamp(unixTime) {
  if (!unixTime) return "--";
  const date = new Date(unixTime);
  return date.toLocaleString();  // You can customize format if needed
}
// Show vitals and check for abnormalities
vitalsRef.orderByKey().limitToLast(1).on("child_added", (snapshot) => {
  const data = snapshot.val();
  if (data) {
    heartRateEl.textContent = `${data.heartRate ?? "--"} bpm`;
    pressureEl.textContent = `${data.pressure_mmHg?.toFixed(2) ?? "--"} mmHg`;
    temperatureEl.textContent = `${data.temperature_F?.toFixed(1) ?? "--"} °F`;
    timestampEl.textContent = data.timestamp ?? "--";
    checkAbnormal(data);
  }
});
const alertBox = document.getElementById("alertBox");
function checkAbnormal(data) {
  const alerts = [];
  if (data.heartRate < 60 || data.heartRate > 120) {
    alerts.push(`❤️ Heart rate abnormal: ${data.heartRate} bpm`);
  }
  if (data.temperature_F < 97 || data.temperature_F > 100.4) {
  alerts.push(`🌡 Temperature abnormal: ${data.temperature_F} °F`);
}
  if (data.pressure_mmHg < 735 || data.pressure_mmHg > 790) {
    alerts.push(`🌬 Pressure abnormal: ${data.pressure_mmHg} mmHg`);
  }
  if (alerts.length > 0) {
    alertBox.innerHTML = alerts.join("<br>");
    alertBox.classList.remove("hidden");
    alertBox.classList.add("critical");
  } else {
    alertBox.innerHTML = "";
    alertBox.classList.add("hidden");
    alertBox.classList.remove("critical");
  }
}#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_BMP280.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi & Firebase Configuration
const char* WIFI_SSID = "ACTFIBERNET";
const char* WIFI_PASS = "act12345";
const char* FIREBASE_KEY = "AIzaSyCcQZ2ULN_aJ7ywjnMchEmJnyBKV2bkPv0";
const char* FIREBASE_URL = "https://health-monitoring-e3c18-default-rtdb.firebaseio.com/";
const char* FIREBASE_EMAIL = "arduino@example.com";
const char* FIREBASE_PASS = "arduino123";

// Sensor Configuration
#define BMP280_ADDR 0x76
#define HR_PIN 32

// Objects
Adafruit_BMP280 bmp;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST timezone offset: +5:30 = 19800 seconds

unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize BMP280
  if (!bmp.begin(BMP280_ADDR)) {
    Serial.println("BMP280 initialization failed!");
    while (1);
  }

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed!");
    ESP.restart();
  }
  Serial.println("\nWiFi connected!");

  // Time Sync
  timeClient.begin();
  timeClient.update();

  // Firebase setup
  config.api_key = FIREBASE_KEY;
  config.database_url = FIREBASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASS;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(2048);
}

void loop() {
  timeClient.update();

  if (Firebase.ready() && millis() - lastSendTime > 5000) {
    lastSendTime = millis();

    // Sensor readings
    float tempC = bmp.readTemperature();
    float tempF = (tempC * 9 / 5) + 32;
    float pressure_hPa = bmp.readPressure() / 100.0;
    float pressure_mmHg = pressure_hPa * 0.750062;

    int heartRate = map(analogRead(HR_PIN), 0, 4095, 50, 100);

    // Time string (formatted)
    String formattedTime = timeClient.getFormattedTime();
    time_t rawTime = timeClient.getEpochTime();
    struct tm* timeInfo = localtime(&rawTime);
    char dateStr[30];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", timeInfo);

    // JSON Payload
    FirebaseJson json;
    json.set("temperature_F", tempF);
    json.set("pressure_mmHg", pressure_mmHg);
    json.set("heartRate", heartRate);
    json.set("timestamp", dateStr);

    // Send to Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, "/patients/patient1/vitals", &json)) {
      Serial.println("✅ Data sent successfully");
    } else {
      Serial.println("❌ Firebase Error: " + fbdo.errorReason());
    }
  }

  // Maintain WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(5000);
  }
} 


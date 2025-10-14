#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://192.168.1.100:5000/predict"; // Replace with your PC IP


Adafruit_MPU6050 mpu;

#define IR_PIN 34  // IR sensor OUT pin
#define LED_RED 25 // Red LED GPIO

// Drowsiness threshold (tune this value based on testing)
const int IR_THRESHOLD = 500;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ESP32 + MPU6050 + IR Sensor + LED ===");

  // Initialize I2C for MPU6050
  Wire.begin(21, 22); //SDA, SCL
  if (!mpu.begin())
  {
    Serial.println("MPU6050 not found! Check wiring.");
    while (1)
      delay(1000);
  }
  Serial.println("MPU6050 initialized.");

  /*WiFi.begin(ssid, password);
Serial.print("Connecting to WiFi");
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}
Serial.println("\nWiFi connected.");*/

  pinMode(IR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);

  // CSV Header
  Serial.println("time_ms,ax,ay,az,gx,gy,gz,ir_value,drowsy_state");
}

void loop()
{
  // Read MPU
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Read IR
  int irVal = analogRead(IR_PIN);

  // Determine drowsiness (IR threshold for blink closure)
  int drowsy = (irVal < IR_THRESHOLD) ? 1 : 0;

  // Control LED
  if (drowsy)
  {
    digitalWrite(LED_RED, HIGH);
  }
  else
  {
    digitalWrite(LED_RED, LOW);
  }

  // Print CSV row
  Serial.print(millis());
  Serial.print(",");
  Serial.print(accel.acceleration.x);
  Serial.print(",");
  Serial.print(accel.acceleration.y);
  Serial.print(",");
  Serial.print(accel.acceleration.z);
  Serial.print(",");
  Serial.print(gyro.gyro.x);
  Serial.print(",");
  Serial.print(gyro.gyro.y);
  Serial.print(",");
  Serial.print(gyro.gyro.z);
  Serial.print(",");
  Serial.print(irVal);
  Serial.print(",");
  Serial.println(drowsy);

  delay(100); // ~10 Hz sampling

 if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"ax\":" + String(accel.acceleration.x) + ",";
  json += "\"ay\":" + String(accel.acceleration.y) + ",";
  json += "\"az\":" + String(accel.acceleration.z) + ",";
  json += "\"gx\":" + String(gyro.gyro.x) + ",";
  json += "\"gy\":" + String(gyro.gyro.y) + ",";
  json += "\"gz\":" + String(gyro.gyro.z) + ",";
  json += "\"ir_value\":" + String(irVal);
  json += "}";

  int httpCode = http.POST(json);
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Server response: " + payload);
  } else {
    Serial.println("Error sending POST: " + String(httpCode));
  }
  http.end();
}


}

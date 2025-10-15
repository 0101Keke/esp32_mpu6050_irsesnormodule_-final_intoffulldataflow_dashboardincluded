#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <HTTPClient.h>

//Writing to csv files
//=============================================================================
// #include <fstream>
// #include <iostream>
// #include <string>
// #include <LittleFS.h> // this a little file system mean't to run on the esp32

//=============================================================================

Adafruit_MPU6050 mpu;

const char *ssid = "YOUR_WIFI_NAME";
const char *password = "YOUR_WIFI_PASSWORD";
const char *serverUrl = "http://172.16.3.83:5000/predict"; // Replace with your PC IP e.g 192.168.1.100

#define SDA_PIN 21
#define SCL_PIN 22
#define IR_PIN 34  // IR sensor OUT pin
#define LED_RED 25 // Red LED GPIO
#define BUZZER_BASE_PIN 26

// Drowsiness threshold (tune this value based on testing)
const int IR_THRESHOLD = 1800; // higher -> closed (depending on the module (500 is the normal one))
const float NOD_THRESHOLD = 0.20;
const int SAMPLE_MS = 200;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ESP32 + MPU6050 + IR Sensor + RED LED + Buzzer ===");

  pinMode(IR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_BASE_PIN, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(BUZZER_BASE_PIN, LOW);

  // Initialize I2C for MPU6050
  Wire.begin(SDA_PIN, SCL_PIN); // SDA, SCL from MPU6050
  if (!mpu.begin())
  {
    Serial.println("MPU6050 not found! Check wiring.");
    while (1)
      delay(1000);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 initialized.");

  // Connect Wifi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
  }
  else
  {
    Serial.println("\nWiFi not connected (continuing in offline mode)");
  }

  /*pinMode(IR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  //digitalWrite(LED_RED, LOW);*/

  // CSV Header for recording
  Serial.println("time_ms,ax,ay,az,gx,gy,gz,ir_value,drowsy_state");

  //Printing to the csv file
  //=============================================================================
  
  // Serial.begin(115200); // Or your preferred baud rate
  // if (!LittleFS.begin()) {
  //   Serial.println("An Error has occurred while mounting LittleFS");
  //   return; // Or halt execution depending on your needs
  // }

  // Serial.println("LittleFS mounted successfully.");

  //   // The rest of your setup code...
  //   // Optional: write the CSV header only if the file is new
  //   if (!LittleFS.exists("/mpu_ir_log.csv")) {
  //       Serial.println("mpu_ir_log.csv");
  //   }
  //=============================================================================


}

void soundBuzzer(int msOn = 200, int msOff = 200, int repeats = 3)
{
  for (int i = 0; i < repeats; i++)
  {
    digitalWrite(BUZZER_BASE_PIN, HIGH);
    digitalWrite(LED_RED, HIGH);
    delay(msOn);
    digitalWrite(BUZZER_BASE_PIN, LOW);
    digitalWrite(LED_RED, LOW);
    delay(msOff);
  }
}
void loop()
{

  digitalWrite(LED_RED, HIGH);
  delay(1000);
  digitalWrite(LED_RED, LOW);
  delay(1000);
  // Read MPU
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Read IR
  int irVal = analogRead(IR_PIN);

  // Heuristics (instantaneous)
  bool eyesClosed = (irVal > IR_THRESHOLD);
  bool nod = (fabs(gyro.gyro.x) > NOD_THRESHOLD || fabs(gyro.gyro.y) > NOD_THRESHOLD);

  // Determine drowsiness (IR threshold for blink closure)
  // int drowsy = (irVal < IR_THRESHOLD) ? 1 : 0;
  int drowsy = (eyesClosed || nod) ? 1 : 0;

  // LED alert
  // digitalWrite(LED_RED, drowsy ? HIGH : LOW);

  // print CSV line (for logging / ML dataset)
  Serial.print(millis());
  Serial.print(",");
  Serial.print(accel.acceleration.x, 3);
  Serial.print(",");
  Serial.print(accel.acceleration.y, 3);
  Serial.print(",");
  Serial.print(accel.acceleration.z, 3);
  Serial.print(",");
  Serial.print(gyro.gyro.x, 4);
  Serial.print(",");
  Serial.print(gyro.gyro.y, 4);
  Serial.print(",");
  Serial.print(gyro.gyro.z, 4);
  Serial.print(",");
  Serial.print(irVal);
  Serial.print(",");
  Serial.println(drowsy);

  //Printing to the csv file
  //=============================================================================
  
  // std::ofstream outputFile("/mpu_ir_log.csv", std::ios::app);
  //  if (!outputFile.is_open()) {
  //       std::cerr << "Error opening file!" << std::endl;
  //      return ; // Indicate an error
  //  }    
  // String csvLine = String(millis()) + ", " +
  //                  String(accel.acceleration.x, 3) + ", " +
  //                  String(accel.acceleration.y, 3) + ", " +
  //                  String(accel.acceleration.z, 3) + ", " +
  //                  String(gyro.gyro.x, 4) + ", " +
  //                  String(gyro.gyro.y, 4) + ", " +
  //                  String(gyro.gyro.z, 4) + ", " +
  //                  String(irVal) + ", " +
  //                  String(drowsy);
    
  //  // Write the formatted string to the file
  //  outputFile << csvLine << std::endl;
  //  Serial.println("Data logged.");
  //  outputFile.close();
  //=============================================================================


  // Alert hardware
  if (drowsy)
  {
    // immediate sensory alert
    soundBuzzer(150, 150, 4);
  }
  else
  {
    // digitalWrite(LED_RED, LOW);
    digitalWrite(BUZZER_BASE_PIN, LOW);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"accel_x\":" + String(accel.acceleration.x, 3) + ",";
    json += "\"accel_y\":" + String(accel.acceleration.y, 3) + ",";
    json += "\"accel_z\":" + String(accel.acceleration.z, 3) + ",";
    json += "\"gyro_x\":" + String(gyro.gyro.x, 4) + ",";
    json += "\"gyro_y\":" + String(gyro.gyro.y, 4) + ",";
    json += "\"gyro_z\":" + String(gyro.gyro.z, 4) + ",";
    json += "\"ir_value\":" + String(irVal) + ",";
    json += "\"drowsy\":" + String(drowsy);
    json += "}";

    int code = http.POST(json);
    if (code > 0)
    {
      String reply = http.getString();
      Serial.println("Server reply: " + String(code) + " " + reply);
    }
    else
    {
      Serial.println("POST failed, code: " + String(code));
    }
    http.end();
  }

  delay(SAMPLE_MS);
}

#include <WiFi.h>
#include<time.h>
#include <Firebase_ESP_Client.h>

const int voltagePin = 34;  // ZMPT101B connected to GPIO 34
const int currentPin = 35;  // ACS712 connected to GPIO 35
const int relayPin = 32;  

const float referenceVoltage = 3.3;  // ESP32 ADC reference voltage
const int adcResolution = 4095;      // ESP32 ADC resolution
const float voltageDividerRatio = 220.0; // Calibration factor for ZMPT101B
const float acs712Sensitivity = 0.185;   // Sensitivity for ACS712-5A

int sampleCount = 500;  // Number of samples for RMS calculation

// NTP server details
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;

// Initializing variables
char timeStr[9], dateStr[11], buffer[100], loc[100];
struct tm timeinfo;
int hour, minute, second, day, month, year;

// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Wi-Fi credentials
const char* ssid = "hello";
const char* password = "12345678";

// Firebase settings
#define DATABASE_URL "https://smart-meter-67683-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyCzMuda6ipHiD0q6zcyBFQavxxryGWItrU"

#define USER_EMAIL "miniprojecttist@gmail.com"
#define USER_PASSWORD "hello@234"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(9600);  // Debugging
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;

  // Firebase setup
  Firebase.reconnectWiFi(true);

  // Initialize and configure time
  configTime(gmtOffset_sec, 0, ntpServer);
}

void getTimeAndDate(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Store time components
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;

  // Store date components
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;  // Months are 0-based
  year = timeinfo.tm_year + 1900;  // Years since 1900
}

float readVoltageRMS() {
  long sum = 0;
  for (int i = 0; i < sampleCount; i++) {
    int rawValue = analogRead(voltagePin);
    float voltage = (rawValue * referenceVoltage) / adcResolution;
    sum += voltage * voltage;
    delayMicroseconds(200);  // Small delay between samples
  }
  return sqrt(sum / sampleCount) * voltageDividerRatio;
}

float readCurrentRMS() {
  long sum = 0;
  for (int i = 0; i < sampleCount; i++) {
    int rawValue = analogRead(currentPin);
    float voltage = (rawValue * referenceVoltage) / adcResolution;
    float current = (voltage - referenceVoltage / 2) / acs712Sensitivity;
    sum += current * current;
    delayMicroseconds(200);
  }
  return sqrt(sum / sampleCount);
}

void loop() {
  digitalWrite(relayPin, LOW);
  // Format time and date strings
  getTimeAndDate();
  sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
  sprintf(dateStr, "%02d-%02d-%04d", day, month, year);

  sprintf(loc, "/s3/%s_%s", timeStr, dateStr);

  //  // Read voltage from ZMPT101B
  // float voltage = readVoltageRMS();
  // // Read current from ACS712
  // float current = readCurrentRMS();
  sprintf(buffer, "V: %f A: %f", 10, 30);

  // Send data to Firebase
  Firebase.RTDB.setString(&fbdo, F(loc), buffer);

  delay(2000);  // 2000 milliseconds = 2 seconds
  
}




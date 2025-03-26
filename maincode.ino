#include <WiFi.h>
#include <time.h>
#include <Firebase_ESP_Client.h>

#define RXp2 16  // ESP32 GPIO16 -> Connect to Mega TX1 (18)
#define TXp2 17  // ESP32 GPIO17 -> Connect to Mega RX1 (19)
HardwareSerial mySerial(2);  // Use Serial2

// NTP server details
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;

// Initializing variables
char timeStr[9], dateStr[11], buffer[100], loc1[100];
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
  Serial.begin(115200);   // Serial Monitor

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

  mySerial.begin(9600, SERIAL_8N1, RXp2, TXp2); // Serial2 with Mega
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

void loop() {
  String data, st;
  // Format time and date strings
  getTimeAndDate();
  sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
  sprintf(dateStr, "%02d-%02d-%04d", day, month, year);

  sprintf(loc1, "/s1/%s_%s", timeStr, dateStr);

  if (mySerial.available()) {
    data = mySerial.readStringUntil('\n');  // Read incoming data
    data.trim();

    Firebase.RTDB.setString(&fbdo, F(loc1), data);
  }

  if (Firebase.RTDB.getString(&fbdo, "/status/s1")) {
    st = fbdo.to<String>();
    mySerial.println(st);
  }

  delay(200);
}

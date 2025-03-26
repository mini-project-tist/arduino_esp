#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
ZMPT101B voltageSensor(A3, 60.0);

#include <iarduino_ACS712.h> 
iarduino_ACS712 sensor(A4);   // ACS712 connected to A1

#define relay 47

unsigned long previousMillis = 0;
const long interval = 10000;  // 5 minutes (300,000 ms)

String st, pw;

int stringToDigital(String str) {
    return (str == "HIGH") ? HIGH : LOW;
}

void setup() {
  Serial.begin(115200);    // Serial Monitor
  Serial1.begin(9600);     // Serial Communication with ESP32 (TX1/RX1)
  voltageSensor.setSensitivity(SENSITIVITY);

  float v=sensor.getZeroVAC(); 
  sensor.setZeroVAC(v);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  if(Serial1.available()){
    st = Serial1.readStringUntil('\n');  
    st.trim();  // Clean unwanted space

    digitalWrite(relay, stringToDigital(st));
  }
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update time
       
    float current = sensor.readAC(5); 
       
    float voltage = voltageSensor.getRmsVoltage();
    Serial.print("Current: "); Serial.print(current); Serial.print(" A");
    Serial.print(" Voltage: "); Serial.print(voltage); Serial.println(" V");

    pw = String(voltage*current);
    Serial1.println(pw);
    Serial.print("Power: "); Serial.println(pw);
  }

  delay(100);
}

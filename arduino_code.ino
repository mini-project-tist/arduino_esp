#define CURRENT_SENSOR A2  // ACS712 connected to A1
#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
#define relay1 31

ZMPT101B voltageSensor(A3, 60.0);

unsigned long previousMillis = 0;
const long interval = 10000;  // 5 minutes (300,000 ms)

String st;

void setup() {
  Serial.begin(115200);    // Serial Monitor
  Serial1.begin(9600);     // Serial Communication with ESP32 (TX1/RX1)
  voltageSensor.setSensitivity(SENSITIVITY);

  pinMode(relay1, OUTPUT);
  digitalWrite(relay1, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  if (Serial1.available()) {
    st = Serial1.readStringUntil('\n');  
    st.trim();  // Clean unwanted spaces
    
    if (st=="false") {
      digitalWrite(relay1, LOW);
    }else {
      digitalWrite(relay1, HIGH);
    }
  }
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update time

    int rawValue = analogRead(CURRENT_SENSOR);
    double voltage = rawValue * (5.0 / 1023.0);  // Convert ADC value to voltage
    double current = (voltage - 2.5) / 1.85;  // Convert to Amps (For ACS712-5A)


    double sen_voltage = voltageSensor.getRmsVoltage();

    Serial.print("Current: "); Serial.print(current); Serial.print(" A");
    Serial.print(" Voltage: "); Serial.print(sen_voltage); Serial.println(" V");

    if (st=="false") {
      Serial1.println(0);
    }else {
      Serial1.println(sen_voltage*current);
    }
  }

  delay(100);
}



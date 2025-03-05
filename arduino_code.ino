#define CURRENT_SENSOR A2  // ACS712 connected to A1
#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
ZMPT101B voltageSensor(A3, 60.0);

void setup() {
  Serial.begin(115200);    // Serial Monitor
  Serial1.begin(9600);     // Serial Communication with ESP32 (TX1/RX1)
  voltageSensor.setSensitivity(SENSITIVITY);
}

void loop() {
  int rawValue = analogRead(CURRENT_SENSOR);
  float voltage = rawValue * (5.0 / 1023.0);  // Convert ADC value to voltage
  float current = (voltage - 2.5) / 0.185;  // Convert to Amps (For ACS712-5A)

  // Serial.print("Current: "); Serial.print(current); Serial.println(" A");

  float sen_voltage = voltageSensor.getRmsVoltage();
  // Serial.println(sen_voltage);

  String data = String(current)+", "+String(sen_voltage);
  Serial1.println(sen_voltage*current);

  Serial.println(sen_voltage*current);

  delay(1000);
}



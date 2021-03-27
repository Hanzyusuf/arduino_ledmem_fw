/**
 * 
 * --- ARDUINO LED MEMORY GAME ---
 * INITIAL CONFIGURATION FILE FOR BLE MODULE
 * 
 * after uploading, make sure to type these without quotes!
 * type those into any serial com interface (eg: Arduino IDE's Serial Monitor)
 * 
 * COMMANDS:
 * "AT+NAMELed Memory" // optional
 * "AT+UUID0xFC20" // IMPORTANT!
 * "AT+CHAR0xFC2E" // IMPORTANT!
 * 
 * NOTE: this is specific to HM10 module, may work for other modules too,
 * for other modules, look up their AT commands in their respective manuals
 * 
 * to test whether initial config was successful, type these:
 * 
 * COMMANDS:
 * "AT+NAME?" // whatever you set above
 * "AT+UUID?" // must return 0xFC20
 * "AT+CHAR?" // must return 0xFC2E
 * 
 **/

#include <SoftwareSerial.h>

SoftwareSerial HM10(2, 3); // board RX & ble TX = 2, board TX & ble RX = 3

void setup() {
  Serial.begin(9600);
  HM10.begin(9600); // set HM10 serial at 9600 baud rate
}

void loop() {
  
  if (Serial.available()) {
    char data = Serial.read();
    HM10.write(data);
  }

  while (HM10.available()) {
    char data = HM10.read();
    Serial.write(data);
  }

}

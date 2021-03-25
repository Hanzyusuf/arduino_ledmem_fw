#include "EasySerialCom.h"

void EasySerialCom::setup(int serialBaudRate) {
    // since dynamic cast is not allowed on arduino, we will call serial.begin from main.cpp
    // if using fno rtti enabled board, uncomment lines below
    // if (HardwareSerial& hs = dynamic_cast<HardwareSerial&>(mySerial))
    //     hs.begin(serialBaudRate);
    // else if (SoftwareSerial& ss = dynamic_cast<SoftwareSerial&>(mySerial))
    //     ss.begin(serialBaudRate);
}

void EasySerialCom::loop() {
    char rc;
 
    while (mySerial.available() > 0) {
        rc = mySerial.read();

        if (bDataInProgress == true) {
            if (rc != endTag) {
                addToData(rc);
            }
            else {
                sendData();
                resetData();
            }
        }
        else if (rc == startTag) {
            bDataInProgress = true;
        }
    }
}

void EasySerialCom::addToData(char character) {
    receivedChars[index] = character;
    index++;
    if (index >= maxDataLength) {
        index = maxDataLength - 1;
        bDataExceededMaxBuffer = true;
    }
}

void EasySerialCom::sendData() {
    // terminate data and send processed data as command
    receivedChars[index] = '\0'; // terminate data
    if(onCommandReceived) {
        // create and send copy to prevent user from manipulating the original array
        char cmdcpy[maxDataLength];
        memcpy(cmdcpy, receivedChars, maxDataLength);
        onCommandReceived(mySerial ,cmdcpy, bDataExceededMaxBuffer ? Error::DataExceededMaxBuffer : Error::None);
    }
}

void EasySerialCom::resetData() {
    // reset data info
    bDataInProgress = false;
    bDataExceededMaxBuffer = false;
    index = 0; // now data will be written from start index (i.e. 0)
}
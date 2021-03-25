/*
  SerialCom.h - Library for reading incoming/outgoing serial data.
  Created by Hanzyusuf, January 10, 2021.
  Allows to send or receive data over serial ports in a more orgaized manner,
  with configurable settings for data handling such as start and end tags, errors.
*/

#ifndef EasySerialCom_h
#define EasySerialCom_h

#include "Arduino.h"
#include "SoftwareSerial.h"

class EasySerialCom
{

  public:
    enum Error {
      None,
      DataExceededMaxBuffer
    };
  
    typedef void (*onCommandReceivedFP)(Stream&, char*, Error);

  private:
    Stream& mySerial;
    char* receivedChars;
    const byte maxDataLength;
    char startTag = '<';
    char endTag = '>';
    onCommandReceivedFP onCommandReceived;
    bool bDataInProgress = false;
    byte index = 0;
    bool bDataExceededMaxBuffer = false;

  public:

    EasySerialCom(Stream& mySerial, onCommandReceivedFP onCommandReceived, const byte maxDataLength = 64) : mySerial(mySerial), onCommandReceived(onCommandReceived), maxDataLength(maxDataLength) {
      receivedChars = new char[maxDataLength];
    };

    ~EasySerialCom(){
      delete[] receivedChars;
    };

    void setup(int serialBaudRate); // to be called from main
    void loop(); // to be called from main
    void addToData(char character); // called when a character is received and data is in progress
    void sendData(); // called when a data within a start and end tag is transferred
    void resetData(); // called after sending data as command

    inline void setStartTag(char tag) {
      startTag = tag;
    };
    inline void setEndTag(char tag) {
      endTag = tag;
    };
    inline const char& getStartTag() const {
      return startTag;
    };
    inline const char& getEndTag() const {
      return endTag;
    };

};

#endif
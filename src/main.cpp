#include <Arduino.h>
#include <EasySerialCom.h>
#include <SoftwareSerial.h>

// CHANGE VARIABLE VALUES AS REQUIRED BELOW
const byte maxDataLength = 64; // max serial command length
// variables for game setup
const byte led_R = 2;
const byte led_G = 3;
const byte led_B = 4;
const byte buzzer = 6;
// enum to map led to numbers (DO NOT ASSIGN VALUES TO ANY OF THEM!)
enum Led_Index {
  LED_R,
  LED_G,
  LED_B,
  LED_LAST, // leave this here
  LED_UNDEFINED = 99 // leave this here too
};
// enum used to set interval of led lights during problem
enum Difficulty {
  EASY = 750,
  MEDIUM = 500,
  HARD = 300
};

// variables for game difficulty
const unsigned int maxLedMemoryLength = 16; // change this as desired
unsigned int ledMemory[maxLedMemoryLength];
const unsigned int startLedMemoryLength = 2;
const unsigned int increaseLedMemoryLengthEveryXLevel = 1;
const unsigned int increaseLedMemoryLengthBy = 1;
const unsigned int failCountGameOver = 3; // game over if user inputs wrong answer this many times
Difficulty currentDifficulty = Difficulty::EASY;

// DO NOT CHANGE VARIABLE VALUES BELOW!
// enum used to verify current game state
enum GameState {
  None,
  Just_Started,
  Alert_Leds,
  Wait_For_Answer_Input,
  Show_Problem,
  Game_Over
};
enum BuzzFreq {
  Buzz_MCU_Ready,
  Buzz_Start_Game,
  Buzz_End_Game,
  Buzz_Game_Over,
  Buzz_Alert_Led_Glowed,
  Buzz_Show_Problem_Led_Glowed,
  Buzz_AnswerInput_Successful,
  Buzz_AnswerInput_Correct,
  Buzz_AnswerInput_Incorrect
};
// variables for game when running
bool bGameRunning = false;
unsigned long timeStamp_previousMillis = 0;
unsigned long timeStamp_AnswerInput_LedTurnedOn = 0;
unsigned long timeElapsed_GameRunning = 0;
unsigned long timeElapsed_LastToggleAllLeds = 0; // used for alert/taunt
unsigned long timeElapsed_ShowProblem_Interval = 0;
unsigned long millisToGlowLedOnAnswerInput = 250; // keep this value under the current set difficulty value
bool bAllLedsOn = false;
unsigned int tempAlertLedToggleCount = 0; // to keep track of number of times leds have been toggled
unsigned int level = 1;
unsigned int currentLedMemoryLength = startLedMemoryLength;
unsigned int tempLedMemoryCount = 0; // to keep track of how many leds have been glowed
unsigned int failCount = 0;
bool bGameOver = false;
// set to false when the current command has not been processed
// the further incoming data will be stored in the arduino buffer and not be read until set to true
// this may be used as per firmware requirements
bool bListenForNewCommands = true;
unsigned int answerInput[maxLedMemoryLength]; // no init needed
unsigned int answerInputCount = 0;
bool bLastAttemptFailed = false; // true when user fails to guess, to repeat same memory pattern
GameState gameState = GameState::None;
bool bGodMode = false; // don't use this you cheater! :)

// function declaration for the arduino build environment to create prototypes of these functions.
// The prototypes of the main functions are already created by the arduino build environment.
void gameLoop();
void turnOnLed(unsigned int ledIndex);
void turnOffLed(unsigned int ledIndex);
void toggleAllLeds();
void turnOnAllLeds();
void turnOffAllLeds();
void incrementLevel();
bool answerInputReceived(unsigned int ledIndex);
void sendAnswerResponse(bool success);
void startGame();
void stopGame();
void endGame();
void gameOver();
void serial1_onCommandReceived(char* command, EasySerialCom::Error error);
void resetAllPins();
void initVariables();
void setGameState(GameState state);
void sendLevelUpdateCommand();
byte getLedPinFromIndex(unsigned int ledIndex);
void writeToSerial(char* msg);
void makeBuzz(BuzzFreq buzzFreq);

// --- SERIAL COMMUNICATION ---

// - uncomment if using software serial, comment hardware serial declaration below
SoftwareSerial mySerial(8, 9); // board RX & ble TX = 8, board TX & ble RX = 9
// - uncomment if using hardware serial, comment software serial declaration above
// HardwareSerial& mySerial = Serial;

// setup 'EasySerialCom' library
EasySerialCom easySerialCom1(mySerial, serial1_onCommandReceived, maxDataLength);

// --- FUNCTIONS ---

void setup() {
  // init/reset pins
  resetAllPins();
  initVariables();

  // begin serial
  mySerial.begin(9600);

  // buzz to indicate power on and ready
  makeBuzz(BuzzFreq::Buzz_Start_Game);
}

void loop() {// IMPORTANT - call loop on EasySerialCom object
  if(bListenForNewCommands)
    easySerialCom1.loop();

  if(bGameRunning)
    gameLoop();
}

void gameLoop() {
  // store current millis and update elapsed time since game running
  unsigned long currentMillis = millis();
  timeElapsed_GameRunning += currentMillis - timeStamp_previousMillis;

  // if game just started, flash alert leds
  if(gameState == GameState::Just_Started)
    setGameState(GameState::Alert_Leds);

  if(gameState == GameState::Alert_Leds) {
    timeElapsed_LastToggleAllLeds += currentMillis - timeStamp_previousMillis;

    if (timeElapsed_LastToggleAllLeds >= 250 && tempAlertLedToggleCount <= 5) {
      toggleAllLeds();
      if(bAllLedsOn)
        makeBuzz(BuzzFreq::Buzz_Alert_Led_Glowed);
      tempAlertLedToggleCount++;
      timeElapsed_LastToggleAllLeds = 0;
    }
    else if (timeElapsed_LastToggleAllLeds >= 500 && tempAlertLedToggleCount > 5) {
      tempAlertLedToggleCount = 0;
      timeElapsed_LastToggleAllLeds = 0;
      setGameState(GameState::Show_Problem);
    }
  }
  else if(gameState == GameState::Show_Problem) {
    timeElapsed_ShowProblem_Interval += currentMillis - timeStamp_previousMillis;

    if(timeElapsed_ShowProblem_Interval >= currentDifficulty) {
      // turn off all leds
      turnOffAllLeds();

      if(tempLedMemoryCount < currentLedMemoryLength 
        && timeElapsed_ShowProblem_Interval >= currentDifficulty + 150) {
        
        // decide whether to generate new random pattern or use the previous one
        unsigned int ledIndex = 0;
        if(!bLastAttemptFailed) {
          ledIndex = random(0, Led_Index::LED_LAST); // exclusive of max
          ledMemory[tempLedMemoryCount] = ledIndex;
        }
        else
          ledIndex = ledMemory[tempLedMemoryCount];
        
        turnOnLed(ledIndex);
        
        tempLedMemoryCount++;
        timeElapsed_ShowProblem_Interval = 0;

        makeBuzz(BuzzFreq::Buzz_Show_Problem_Led_Glowed);

        mySerial.print("led index: ");
        mySerial.print(ledIndex);
        mySerial.print("\n");
      }
      else if(tempLedMemoryCount >= currentLedMemoryLength) {
        tempLedMemoryCount = 0;
        timeElapsed_ShowProblem_Interval = 0;
        answerInputCount = 0;
        bLastAttemptFailed = false;
        setGameState(GameState::Wait_For_Answer_Input);
      }
    }
  }
  else if(gameState == GameState::Wait_For_Answer_Input) {
    if(timeStamp_AnswerInput_LedTurnedOn > 0 
      && timeElapsed_GameRunning - timeStamp_AnswerInput_LedTurnedOn > millisToGlowLedOnAnswerInput) {
      turnOffAllLeds();
      timeStamp_AnswerInput_LedTurnedOn = 0;
    }
  }

  // update previous millis to current millis
  timeStamp_previousMillis = currentMillis;
}

void turnOnLed(unsigned int ledIndex) {
  digitalWrite(getLedPinFromIndex(ledIndex), HIGH);
}

void turnOffLed(unsigned int ledIndex) {
  digitalWrite(getLedPinFromIndex(ledIndex), LOW);
}

void toggleAllLeds(){
  if(bAllLedsOn)
    turnOffAllLeds();
  else
    turnOnAllLeds();
}

void turnOnAllLeds() {
  for(byte i = 0; i < LED_LAST; i++) {
    turnOnLed(i);
  }
  bAllLedsOn = true;
}

void turnOffAllLeds() {  
  for(byte i = 0; i < LED_LAST; i++) {
    turnOffLed(i);
  }
  bAllLedsOn = false;
}

void incrementLevel() {
  level++;
  if(currentLedMemoryLength < maxLedMemoryLength)
    currentLedMemoryLength = startLedMemoryLength + (increaseLedMemoryLengthBy * (level-1) / increaseLedMemoryLengthEveryXLevel);

  if(currentLedMemoryLength >= maxLedMemoryLength)
    currentLedMemoryLength = maxLedMemoryLength;
}

bool answerInputReceived(unsigned int ledIndex) {
  if(gameState != GameState::Wait_For_Answer_Input)
    return false;

  if(bGodMode)
    ledMemory[answerInputCount] = ledIndex; // use this if lit color should match input answer, this one will fail if out of bound led index is provided
    //ledIndex = ledMemory[answerInputCount]; // use this if lit color should match actual answer
  
  // invalid led index
  if(getLedPinFromIndex(ledIndex) == Led_Index::LED_UNDEFINED)
    return false;

  answerInput[answerInputCount] = ledIndex;

  timeStamp_AnswerInput_LedTurnedOn = timeElapsed_GameRunning;
  turnOffAllLeds();
  turnOnLed(ledIndex);

  // check if answer input value is incorrect
  if(answerInput[answerInputCount] != ledMemory[answerInputCount]) {
    
    failCount++;
    
    if(failCount >= failCountGameOver) {
      makeBuzz(BuzzFreq::Buzz_Game_Over);
      gameOver();
    }
    else {
      makeBuzz(BuzzFreq::Buzz_AnswerInput_Incorrect);
      bLastAttemptFailed = true; // to repeat same pattern next time
      turnOffAllLeds();
      //sendAnswerResponse(true);
      setGameState(GameState::Alert_Leds);
    }

    return false;
  }

  // increment answer input count
  answerInputCount++;

  //sendAnswerResponse(true);

  // if number of required inputs have been received, it means all inputs were correct
  if(answerInputCount >= currentLedMemoryLength) {
    makeBuzz(BuzzFreq::Buzz_AnswerInput_Successful);
    delay(250);
    turnOffAllLeds();
    incrementLevel();
    setGameState(GameState::Alert_Leds);
    sendLevelUpdateCommand();
  }
  else{
    makeBuzz(BuzzFreq::Buzz_AnswerInput_Correct);
  }

  return true;
}

void sendAnswerResponse(bool success) {
  if(success)
    writeToSerial("<Answer_Response:Success>");
  else
    writeToSerial("<Answer_Response:Fail>");
}

void startGame() {
  initVariables();

  timeStamp_previousMillis = millis();
  setGameState(GameState::Just_Started);
  sendLevelUpdateCommand();
  bGameRunning = true;
  makeBuzz(BuzzFreq::Buzz_Start_Game);
}

void stopGame() {
  bGameRunning = false;
  turnOffAllLeds();
  makeBuzz(BuzzFreq::Buzz_End_Game);
}

void endGame() {
  setGameState(GameState::None);
  stopGame();
  initVariables();
}

void gameOver() {
  bGameOver = true;
  setGameState(GameState::Game_Over);
  stopGame();
}

void serial1_onCommandReceived(char* command, EasySerialCom::Error error) {
  bListenForNewCommands = false;

  //mySerial.print("\nreceived command: ");
  //mySerial.println(command);

  if(error == EasySerialCom::Error::None) {
    char* responseChar = "<CMD_Response:Success>";

    if(strcmp(command, "Request_State:Start_Game") == 0)
      startGame();
    else if(strstr(command, "Answer_Input:")) {
      if(gameState != GameState::Wait_For_Answer_Input) {
        responseChar = "<CMD_Response:Fail>";
        writeToSerial("<Error:not taking answer input yet!>");
        sendAnswerResponse(false);
      }
      else {
        // split array on copied array because strtok modifies original
        char cmdcpy[maxDataLength];
        memcpy(&cmdcpy, command, maxDataLength);
        char* commandElem = strtok(cmdcpy, ":"); // gets the 1st element at index 0
        commandElem = strtok(NULL, ":"); // gets the 2nd element at index 1
        if(commandElem != NULL) {
          unsigned int ledIndex = atoi(commandElem); // convert char* to int
          if(getLedPinFromIndex(ledIndex) == Led_Index::LED_UNDEFINED && !bGodMode) {
            responseChar = "<CMD_Response:Fail>";
            writeToSerial("<Error:undefined led index!>");
            sendAnswerResponse(false);
          }
          else
            sendAnswerResponse(answerInputReceived(ledIndex));
        }
        else {
          responseChar = "<CMD_Response:Fail>";
          writeToSerial("<Error:null or empty answer input>");
          sendAnswerResponse(false);
        }
      }
    }
    else if(strcmp(command, "Request_State:End_Game") == 0)
      endGame();
    else if(strstr(command, "Set_Difficulty:")) {
      char cmdcpy[maxDataLength];
      memcpy(&cmdcpy, command, maxDataLength);
      char* commandElem = strtok(cmdcpy, ":"); // gets the 1st element at index 0
      commandElem = strtok(NULL, ":"); // gets the 2nd element at index 1

      if(strncmp(commandElem, "EASY", 4) == 0)
        currentDifficulty = Difficulty::EASY;
      else if(strncmp(commandElem, "MEDIUM", 6) == 0)
        currentDifficulty = Difficulty::MEDIUM;
      else if(strncmp(commandElem, "HARD", 4) == 0)
        currentDifficulty = Difficulty::HARD;
      else {
        responseChar = "<CMD_Response:Fail>";
        writeToSerial("<Error:invalid set difficulty argument>");
      }
    }
    else if(strcmp(command, "IAmALoser") == 0)
      bGodMode = true;
    else if(strcmp(command, "IAmNotALoser") == 0)
      bGodMode = false;
    else {
      responseChar = "<CMD_Response:Fail>";
      writeToSerial("<Error:unrecognized command>");
    }

    writeToSerial(responseChar);
  }
  else{
    writeToSerial("<CMD_Response:Fail>");
    
    if(error == EasySerialCom::Error::DataExceededMaxBuffer) {
      writeToSerial("Error:command exceeded max data length");
    }
  }

  bListenForNewCommands = true;
}

// resets all pins
void resetAllPins() {
  // set pinmode to output on all leds and buzzer
  for(byte i = 0; i < LED_LAST; i++) {
    pinMode(getLedPinFromIndex(i), OUTPUT);
  }
  pinMode(buzzer, OUTPUT);

  // turn off all leds and buzzer
  turnOffAllLeds();
  noTone(buzzer);
}

void initVariables(){
  bGameRunning = false;
  timeStamp_previousMillis = 0;
  timeStamp_AnswerInput_LedTurnedOn = 0;
  timeElapsed_GameRunning = 0;
  timeElapsed_LastToggleAllLeds = 0;
  timeElapsed_ShowProblem_Interval = 0;
  millisToGlowLedOnAnswerInput = 250;
  bAllLedsOn = false;
  tempAlertLedToggleCount = 0;
  level = 1;
  currentLedMemoryLength = startLedMemoryLength;
  tempLedMemoryCount = 0;
  failCount = 0;
  bGameOver = false;
  bListenForNewCommands = true;
  answerInputCount = 0;
  bLastAttemptFailed = false;
  gameState = GameState::None;
}

void setGameState(GameState state) {
  gameState = state;
  switch (state) {
  case GameState::None :
    writeToSerial("<GameState_Update:None>");
    return;
    case GameState::Just_Started :
    writeToSerial("<GameState_Update:Just_Started>");
    return;
  case GameState::Alert_Leds :
    writeToSerial("<GameState_Update:Alert_Leds>");
    return;
  case GameState::Show_Problem :
    writeToSerial("<GameState_Update:Show_Problem>");
    return;
  case GameState::Wait_For_Answer_Input :
    writeToSerial("<GameState_Update:Wait_For_Answer_Input>");
    return;
    case GameState::Game_Over :
    writeToSerial("<GameState_Update:Game_Over>");
    return;
  default:
    return;
  }
  
}

void sendLevelUpdateCommand() {
  char levelChar[8];
  sprintf(levelChar, "%d>", level);
  char cmdLevelUpdate[] = "<Level_Update:";
  strcat(cmdLevelUpdate, levelChar);
  writeToSerial(cmdLevelUpdate);
}

// MUST UPDATE if adding or removing leds
byte getLedPinFromIndex(unsigned int ledIndex) {
  switch(ledIndex) {
    case Led_Index::LED_R: // 0
      return led_R;
    case Led_Index::LED_G: // 1
      return led_G;
    case Led_Index::LED_B: // 2
      return led_B;
    default:
      return Led_Index::LED_UNDEFINED;
  }
}

void writeToSerial(char* msg) {
  mySerial.write(msg);
  mySerial.write("\n");
}

void makeBuzz(BuzzFreq buzzFreq) {
  switch(buzzFreq) {
    case BuzzFreq::Buzz_MCU_Ready:
      tone(buzzer, 1400);
      delay(100);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_Start_Game:
      delay(100);
      tone(buzzer, 1200);
      delay(100);
      tone(buzzer, 1000);
      delay(100);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_End_Game:
      delay(100);
      tone(buzzer, 800);
      delay(100);
      tone(buzzer, 1200);
      delay(100);
      noTone(buzzer);     
      break;
    case BuzzFreq::Buzz_Game_Over:
      tone(buzzer, 600);
      delay(800);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_Alert_Led_Glowed:
      tone(buzzer, 450);
      delay(50);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_Show_Problem_Led_Glowed:
      tone(buzzer, 1800);
      delay(100);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_AnswerInput_Successful:
      tone(buzzer, 1400);
      delay(200);
      tone(buzzer, 1000);
      delay(200);
      tone(buzzer, 1200);
      delay(200);
      tone(buzzer, 1400);
      delay(300);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_AnswerInput_Correct:
      tone(buzzer, 1200);
      delay(200);
      noTone(buzzer);
      break;
    case BuzzFreq::Buzz_AnswerInput_Incorrect:
      tone(buzzer, 800);
      delay(400);
      noTone(buzzer);
      break;
  }
  
}
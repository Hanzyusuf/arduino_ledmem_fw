# Arduino Led Memory Game (Firmware)

Arduino Led Memory Game is a basic memory game built for arduino made with the intention of helping newbies (like me) to get started with microcontrollers and serial communication over USB and Bluetooth Low Energy.

##### Note: To work with the basic USB over serial setup with PC, BLE setup is not required, but to use the [Android app](https://github.com/Hanzyusuf/arduino_ledmem_android) with this, [BLE setup](#ble_setup) is required.

## Requirements

### Hardware
* Arduino board
* 3 leds (any color, prefer different colors)
* 3 resistors for leds (around 270 ohm should be fine)
* 1 resistor for buzzer (around 220 ohm should be fine)
* 1 Piezo Buzzer
* A breadboard and wires for easy connection

##### Note: I have used 100 ohm resistors for the leds and the buzzer both, bright and loud! :)

### Software

* Visual Studio Code (VS Code)
  -  see [Install VS Code for Windows](https://code.visualstudio.com/docs/setup/windows)
* PlatformIO extension for VS Code
  -  see [Install PlatformIO extension for VS Code](https://platformio.org/install/ide?install=vscode)
* Arduino IDE (for easy serial communication)


## Installation (Basic)

### Arduino Hardware Setup:
Photo goes here

### Arduino Firmware Setup:

1. Clone the project (or download and extract).
![git download](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/git_download.png)

2. Open VS Code and open PlatformIO Home page
![pio home](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/pio_home.png)

3. Open the cloned (or extracted) folder with PlatformIO extension in VS Code.
![pio open](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/pio_open.png)

4. Connect arduino to PC and upload project to arduino!
![pio upload](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/pio_upload.png)

## Usage

### Commands:
#### - To Send Commands:
  ##### These commands are to be sent to the arduino via serial communication, these drive the game.
  ```
  <Request_State:*reqst_arg>	--- to set game state
  <Answer_Input:*led_index>	--- to give answer input
  <Set_Difficulty:*diff>		--- to set difficulty
  <IAmALoser>			--- to turn god mode on
  <IAmNotALoser>			--- to turn god mode off
  ```
##### Note: All commands are enclosed between '<' and '>' because the library [Arduino Easy Serial Com](https://github.com/Hanzyusuf/arduino_easy_serial_com) requires it.
  
#### - To Receive Commands:
  ##### These commands are to be received from the arduino via serial communication, you may choose to use these as you want, or ignore.
  ```
  CMD_Response:*result		--- received whenever a valid command is sent successfully
  Error:*error_msg		--- received whenever an invalid command is sent or transmission error occurs
  GameState_Update:*game_state	--- received whenever the game state changes
  Answer_Response:*result		--- received whenever answer input is given
  Level_Update:*level		--- received whenever level changes and also at game start
  ```

#### Legend:

  ```
  *reqst_arg: 	Start_Game | End_Game | Pause_Game | Resume_Game
  *result: 	Fail | Success
  *diff:		EASY | MEDIUM | HARD
  *game_state:	None | Alert_Leds | Show_Problem | Wait_For_Answer_Input | Game_Over
  *level: 	any whole number >= 1
  *led_index: 	any whole number >= 0  (by default, led red = 0 & led green = 1 & led blue = 2)
  *error_msg: 	any string
  ```
  
##### Note that almost all of these commands could have used single digit integers instead of long strings to achieve the same results but I wanted to test the limitations of data transmission through  BLE, which limits data to around 20kb.

### Basic Usage Example:

##### After [uploading the firmware](#Installation) to Arduino, follow these steps for a basic gameplay example.

- Connect Arduino to PC via USB.
- Open *Arduino IDE*.
- Open *Serial Monitor* (*Shift + Ctrl + M*).
- *Optional* - type ```<Set_Difficulty:MEDIUM>```  (*defaults to EASY*).
- Type ```<Request_State:Start_Game>```
- Wait for Arduino to show a pattern problem.
- Type ```<Answer_Input:*led_index>``` for each led glowed and in the same order.
  - Replace ***led_index** with index of the led glowed.
  - By default, *led red* = 0 & *led green* = 1 & *led blue* = 2.

##### Additional Notes:

- Inputting the correct answer sequence will increase the level by one.
- As level increases, the pattern length increases.
- Inputting a wrong answer will make the game show the problem again.
- Failing to input the right answer pattern 3 times will end the game.
- Send command ```<IAmALoser>``` to turn on god mode ;)

## Installation (BLE)

### Arduino Hardware Setup:
#### Note: Same as [basic setup](#Installation (Basic)) but with a BLE module attached on the other side of the breadboard.
Photo goes here

### Arduino Firmware Setup:
#### This will also be the same as the basic setup, but two additional steps are required beforehand.

1. Upload [this sketch](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/ble_setup_(hm10).ino) to Arduino using Arduino IDE.

2. Open Serial Monitor in Arduino IDE (Ctrl+Shift+M).

3. Type these one by one in the Serial Monitor:
	- *optional*
	```AT+NAMELed Memory```
    
	- *important*
	```AT+UUID0xFC20```
	```AT+CHAR0xFC2E```

4. Perform the [basic setup](#Installation (Basic)).

5. Modify main.cpp:
	- comment this [line](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/7b76227bffd7afd2498d52670107fee54d4023fe/src/main.cpp#L109):
	```// SoftwareSerial mySerial(8, 9);```
	- uncomment this [line](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/7b76227bffd7afd2498d52670107fee54d4023fe/src/main.cpp#L111):
	```HardwareSerial& mySerial = Serial;```

6. Upload firmware to Arduino!
<br />

#### To play over bluetooth, you can:
- use my [android app](https://github.com/Hanzyusuf/arduino_ledmem_android)
- use any app which allows BLE serial communication ([like this](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal))

## Demo

### USB:
Video or photo could go here

### BLE (using the [android app](https://github.com/Hanzyusuf/arduino_ledmem_android)):
Video or photo could go here

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)
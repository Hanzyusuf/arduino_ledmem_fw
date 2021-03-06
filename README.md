# Arduino Led Memory Game (Firmware)

Arduino Led Memory Game is a basic memory game built for arduino made with the intention of helping newbies (like me) to get started with microcontrollers and serial communication over USB and Bluetooth Low Energy.

##### Note: To work with the basic USB over serial setup with PC, BLE setup is not required, but to use the [Android app](https://github.com/Hanzyusuf/arduino_ledmem_android) with this, [BLE setup](#installation-ble) is required.

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

![Basic Setup](https://github.com/Hanzyusuf/extra_files_01/blob/main/hw_setup_ble.gif?raw=true)

### Arduino Firmware Setup:

1. Clone the project (or download and extract).<br/><br/>
![git download](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/git_download.png)

2. Open VS Code and open PlatformIO Home page.<br/><br/>
![pio home](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/pio_home.png)

3. Open the cloned (or extracted) folder with PlatformIO extension in VS Code.<br/><br/>
![pio open](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/master/extra_resources/pio_open.png)

4. Connect arduino to PC and upload project to arduino!<br/><br/>
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

##### After [uploading the firmware](#installation-basic) to Arduino, follow these steps for a basic gameplay example.

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
#### Note: Same as [basic setup](#installation-basic) but with a BLE module attached on the other side of the breadboard.

![BLE Setup](https://github.com/Hanzyusuf/extra_files_01/blob/main/hw_setup_ble.gif?raw=true)

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

4. Perform the [basic setup](#installation-basic).

5. Modify main.cpp:
	- uncomment this [line](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L109) to:
	```SoftwareSerial mySerial(2, 3);```
	- comment this [line](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L111) to:
	```// HardwareSerial& mySerial = Serial;```

6. Upload firmware to Arduino!
<br />

#### To play over bluetooth, you can:
- use my [android app](https://github.com/Hanzyusuf/arduino_ledmem_android)
- use any app which allows BLE serial communication ([like this](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal))

## Demo

### USB:
[Youtube Link](https://youtu.be/yDiGy3ULO8g)

[![LedMem USB Mode](http://i3.ytimg.com/vi/yDiGy3ULO8g/hqdefault.jpg)](https://youtu.be/yDiGy3ULO8g "LedMem USB Mode")

### BLE (using the [android app](https://github.com/Hanzyusuf/arduino_ledmem_android)):
[Youtube Link](https://youtu.be/YGHFWe7IwpE)

[![LedMem BLE Mode](http://i3.ytimg.com/vi/YGHFWe7IwpE/hqdefault.jpg)](https://youtu.be/YGHFWe7IwpE "LedMem BLE Mode")

## Extra

#### You can also modify the game as required by editing the main.cpp file. Some basic modifications can be done as following:

#### To add more leds to the board:
- for example, to add a yellow led on pin 11, add a line like this [here](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L10):
    ```cpp
    const byte led_R = 8;
    ...
    ...
    const byte led_Y = 11; // THIS IS ADDED!
    ```
- then in [this enum](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L13), add an entry for the new led:
    ```cpp
    enum Led_Index {
        LED_R,
        LED_G,
        LED_B,
        LED_Y, // THIS IS ADDED!
        LED_LAST,
        LED_UNDEFINED = 99
    };
    ```
- finally, modify [this function](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L511):
    ```cpp
    byte getLedPinFromIndex(unsigned int ledIndex) {
        switch(ledIndex) {
            ...
            case Led_Index::LED_Y: // THIS IS ADDED!
                return led_Y; // THIS IS ADDED!
            ...
        }
    }
    ```

##### Note that the [android app](https://github.com/Hanzyusuf/arduino_ledmem_android) is built according to the default 3 led setup. Adding leds will render the android app useless.

#### To adjust difficulty speeds:
- simply modify the enum at [this line](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L21):
	```cpp
	enum Difficulty {
	    EASY = 750,
	    MEDIUM = 500,
	    HARD = 300
	};
	```

#### To change more game settings:
- Modify [any of these lines](https://github.com/Hanzyusuf/arduino_ledmem_fw/blob/d6530ab27b8a233a0fa6bc2648747d31026ddb82/src/main.cpp#L28-L34) as required.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)
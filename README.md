# RC522 RFID Reader and Writer for STM32L152RB

This project utilizes the RC522 RFID reader and writer module in conjunction with the 
STM32L152RB microcontroller to read and write data to RFID cards. 
The project is written in C and compiled using Keil uVision 5.

### [CODE HERE!](https://github.com/CoMiKx/2565-CPE311-CENexy/blob/master/Project/src/main.c) & [LIBRARY HERE!](https://github.com/CoMiKx/2565-CPE311-CENexy/blob/master/Drivers/STM32L1xx_HAL_Driver/Src/RC522.c)

## Getting Started
### Hardware
* STM32L152RB microcontroller
* RC522 RFID reader and writer module
* RFID cards for testing
* Jumper wires for connecting the RC522 module to the STM32L152RB

### Software
* Keil uVision 5 for writing and compiling the code

### Connections
* Connect the MISO pin of the RC522 module to PB3 of the STM32L152RB
* Connect the SCK pin of the RC522 module to PB4 of the STM32L152RB
* Connect the SDA pin of the RC522 module to PB5 of the STM32L152RB
* Connect the MOSI pin of the RC522 module to PB6 of the STM32L152RB
* Connect the RST pin of the RC522 module to PB7 of the STM32L152RB
* Connect the GND pin of the RC522 module to GND of the STM32L152RB
* Connect the 3.3V pin of the RC522 module to 3.3V of the STM32L152RB
* Connect the LED YELLOW to PC1 of the STM32L152RB
* Connect the LED RED to PC2 of the STM32L152RB

## Pin Allocation
  ![Pin Allocation](https://github.com/CoMiKx/2565-CPE311-CENexy/blob/master/pin_allocation.jpg?raw=true)

## Usage
1. Open the project in Keil uVision 5 and build the code.
2. Flash the code to the STM32L152RB microcontroller.
3. Place an RFID card on the RC522 module to read the data from the card.
4. Use the appropriate function to write data to the card.

## Features
* Read and write data to RFID cards using the RC522 module
* Uses the SPI2 peripheral of the STM32L152RB for communication with the RC522 module
* Basic error handling and debugging functions
* Support for multiple card types such as Mifare Classic, Ultralight, and ProX

## Resources
* [STM32L152RB datasheet](https://www.st.com/resource/en/datasheet/stm32l152rb.pdf)
* [RC522 datasheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)
* [Keil uVision 5 user manual](https://developer.arm.com/documentation/101407/0538)

## Acknowledgments
* The RC522 library used in this project is based on the [**MFRC522 library**](https://github.com/miguelbalboa/rfid) by Miguel Balboa.
* Special thanks to the STMicroelectronics for providing the STM32CubeMX software, 
  which was used to generate the initial code for the STM32L152RB.
* A big thank you to the Keil uVision 5 team for their powerful and user-friendly 
  development environment.
* We would also like to thank the community for providing valuable resources and 
  guidance throughout the development of this project.
  
## Gantt chart
  ![chart](https://i.imgur.com/Ra7kCo7.png)
  [Full Gantt Chart here open with file .gantt in project.](https://www.onlinegantt.com/#/gantt)

## Waterfall Model
  ![model](https://github.com/CoMiKx/2565-CPE311-CENexy/blob/master/model.png?raw=true)

## Flow Chart
  ![flowchart](https://github.com/CoMiKx/2565-CPE311-CENexy/blob/master/flowchart.png?raw=true)
 
## Conclusion
  This project provides a basic framework for utilizing the RC522 RFID reader and writer module 
  with the STM32L152RB microcontroller. With the provided code, you can read and write data to RFID cards 
  using the SPI2 peripheral of the STM32L152RB. This project can be further expanded upon to add more advanced functionality 
  such as card authentication and data encryption.

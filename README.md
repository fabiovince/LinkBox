# MQTT, MODBUS and ALEXA DIDATIC AND PROTOTYPE KIT

[![NPM](https://img.shields.io/npm/l/react)](https://github.com/fabiovince/LinkBox/blob/main/LICENSE) 

# About Project

LinkBox is a WiFi multiprotocol tool designed for internet of things (IOT) and industrial internet of things (IIOT) didactic and development purposes. It is implemented with three ESP32-WROOM-32D. One ESP32 is a MQTT Broker and the remaining two ESP32 are configurable devices. They can be configured to communicate using MODBUS, MQTT or Alexa smart device. 
                 

## Front View

Each device can be used as a MODBUS server or MQTT client or a Alexa smart device. The protocol menu is  acessed when push buttom is pressed for 10 seconds. Menu display the current protocol and SSID. In order to change between protocols press push buttom for 1 second and release it for 2 seconds. If push buttom remain release for more than 6 senconds ESP32 will reboot to load the chosen protocol.

![01-frontal_image_new_version](https://github.com/fabiovince/LinkBox/assets/7406537/5c18a51f-e126-49a9-9951-c4c1aa9b8d32)

![02-frontal_image_modbus_registers_details](https://github.com/fabiovince/LinkBox/assets/7406537/d1be1f94-91bf-4617-9232-b16866825962)


## Inside View

On boot, MQTT broker and both configurable devices connects to MW301R Mercusys ROUTER SSID. 

![03-inside_view](https://github.com/fabiovince/LinkBox/assets/7406537/09e91706-f85e-4aea-a2c3-1d6274c72695)


## Schematic Diagram

Each device and the MQTT Broker are independent circuits, so they can be implemented in individual boxes, if necessary.

![04-schematic_diagram](https://github.com/fabiovince/LinkBox/assets/7406537/4dc0ea5f-349e-4053-9a60-0d387e44a560)


# Used Technologies
## Device Hardware
- One ESP32-WROOM-32D
- One 4pin 0.96 Inch Yellow/Blue I2C Serial 128X64 OLED Display
- Two 5k potentiometer
- Two 10k 1/4W 5% resistor
- Five 220 ohms 1/4W 5% resistor
- One push buttom
- One retentive buttom
- One green LED 
- One red LED 
- One common anode RGB LED
  
## Integrated Development Environment
- Visual Studio Code
- PlataformIO:
  - Board: uPesy ESP32 Wroom Devkit
  - Framework: Arduino
  
## Firmware
- C++

## MQTT Broker 
- Hardware
  - One ESP32-WROOM-32D
  - One 4pin 0.96 Inch Yellow/Blue I2C Serial 128X64 OLED Display
- Arduino IDE Firmware:
  - Link Dropbox: https://www.dropbox.com/scl/fo/3s1xd0g3zrd0lvtknvdgb/h?rlkey=5u9olarl8w7rnul6cxagvwvpd&dl=0
- Default IP and Topic:
  - IP: 192.168.1.233
  - Topic: /LB
    
# Synoptic And Dashboard To LinkBox's Supervisory Control

## ScadaBR Synoptic Panel with MODBUS Data Source

ScadaBR is free, open-source software for developing Automation, Data Acquisition and Supervisory Control applications. 

ScadaBR official website link: https://www.scadabr.com.br/

Next figure shows a ScadaBR's Synoptic Panel implemented with a MODBUS data source to LinkBox's supervisory control. This ScadaBR's Synoptic Panel example can be found inside ScadaBR installation folder provided by the Dropbox link below.

VagaModbus Analyzer is a free software from Microsoft Store. It can be used as MQTT client to test READ/WRITE commands to LinkBox. Link from Microsoft Store: https://www.microsoft.com/store/productId/9PG8QTRLP62X?ocid=pdpshare 

![05-ScadaBR_Synoptic_Panel](https://github.com/fabiovince/LinkBox/assets/7406537/34b989dc-8f46-4098-adca-e2db0eb5b7ed)


## Node-RED Dashboard with MQTT Connection

Node-RED is a programming tool for wiring together hardware devices, APIs and online services in new and interesting ways. (https://nodered.org/)  Node-RED flows are becoming popular on IOT and IIOT applications. New Industry 4.0 PLCs can also be programed by Node-RED flows.

The figure below shows the flow implemented in the Node-RED dashboard for the LinkBox project running  on Windows and Android operating system. Node-RED can run also on Linux, macOS and Raspberry Pi OS. This flow was implemented to behave like a SCADA. 
That is, the dashboard is constantly updated with the current state of the hardware.  

- Dropbox download link for flow implemented on Node-RED: https://www.dropbox.com/scl/fo/cqh4to197a80hadz3esbo/h?rlkey=f3pqaxwa7h5j014ujrmqn2tlo&dl=0

To run Node-RED on Android it is necessary to install Termux. https://github.com/termux/termux-app/releases <br />
Youtube - Como Instalar O Termux No Android: https://www.youtube.com/watch?v=rln46ulRFEw

![06 - Node-RED Dashboard with MQTT Connection_01](https://github.com/fabiovince/LinkBox/assets/7406537/c710bdfc-3eca-4f50-a47a-d544ed878615)

MQTTBox is a free software from Microsoft Store. It can be used to test READ/WRITE commands to LinkBox. MQTTBox enables you to create MQTT clients to publish or subscribe to topics, create MQTT virtual device, load test MQTT devices or brokers and much more. Link from Microsoft Store: https://www.microsoft.com/store/productId/9NBLGGH55JZG?ocid=pdpshare

Implemented Node-RED flow sends and receives JSON-type objects similar to MODBUS frames, as detailed below:

-> WRITE COMMANDS: <br />
  - Turn on Green LED:  { "value": 1, "fc": 5, "unitid": 1, "address": 0, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Turn off Green LED: { "value": 0, "fc": 5, "unitid": 1, "address": 0, "quantity": 1, "origin": "client", "fn": 0 }

  - Turn on Red LED:  { "value": 1, "fc": 5, "unitid": 1, "address": 1, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Turn off Red LED: { "value": 0, "fc": 5, "unitid": 1, "address": 1, "quantity": 1, "origin": "client", "fn": 0 }

  - Set Red RGB to 50%:    { "value": 20, "fc": 6, "unitid": 1, "address": 0, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Set Green RGB to 30%:  { "value": 50, "fc": 6, "unitid": 1, "address": 1, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Set Blue RGB to 70%:   { "value": 70, "fc": 6, "unitid": 1, "address": 2, "quantity": 1, "origin": "client", "fn": 0 }

-> READ COMMANDS: <br />
  - Read Retentive Button:  { "fc": 2, "unitid": 1, "address": 0, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Read Push Button:       { "fc": 2, "unitid": 1, "address": 1, "quantity": 1, "origin": "client", "fn": 0 }

  - Read Red RGB:    { "fc": 3, "unitid": 1, "address": 0, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Read Green RGB:  { "fc": 3, "unitid": 1, "address": 1, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Read Blue RGB:   { "fc": 3, "unitid": 1, "address": 2, "quantity": 1, "origin": "client", "fn": 0 }

  - Read Left Potentiometer:  { "fc": 4, "unitid": 1, "address": 0, "quantity": 1, "origin": "client", "fn": 0 } <br />
  - Read Right Potentiometer: { "fc": 4, "unitid": 1, "address": 1, "quantity": 1, "origin": "client", "fn": 0 }

-> JSON Keys:
  - "value": value (0-65235) Range defined by firmware implementation.
  - "unitid": device's identification (1-255) Range defined by firmware implementation.
  - "fc": MODBUS like function code
  - "address": register address offset (0-65235) Range defined by firmware implementation.
  - "origin": defines if JSON object is from client or server
  - "fn": frame number (0-65235) Range defined by firmware implementation. When a write command is gererated it stores the current frame number and just considers response with frame number >= stored frame
          
## Build Instructions and Related Softwares:

LinkBox pictures and assembly instructions. 
Link from Dropbox: https://www.dropbox.com/scl/fo/tbs9np715aqork23sw76q/h?rlkey=d7amn1jst6973gzth82ytolqe&dl=0

ScadaBR for Windows Installation download.
Link from Dropbox: https://www.dropbox.com/scl/fo/xynmp47ol7sotwd9gwftd/h?rlkey=ucapo0msubysi3el0jxzka6xw&dl=0

ScadaBR for Linux Installation download.
Link from Dropbox: https://www.dropbox.com/scl/fo/tbormiz7tivm6j3hyt6f6/h?rlkey=o1a42fnvvhi7lunzpy7kgiube&dl=0

It was developed to attend the following subjects: Industrial Informatic II, Home Automation and Industrial Networks I of                    |
Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)   

# Autors

Prof. Fábio Vincenzi and Prof. Renato Carrijo


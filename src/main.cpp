//+---------------------------------------------------------------------------------------------+
//|   Didatic KIT - LinkBox - Developed to to support learning the following subjects:          |
//    Industrial Informatics II, Home Automation and Industrial Networks I of                   |
//    Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)  |
//    by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                          |
//                                                                 Updated: 2023/10/28          |         
//+---------------------------------------------------------------------------------------------+

#include <WiFi.h>
#include <Alexa.h>
#include <Adafruit_AHTX0.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "SSID_PASSWORD.h"
#include "global_vars.h"

//#define Enable_ArduinoRelay  //Inverse Logic, because when input signal of Arduino Relay is 0 (Zero) the outpup is NO

//+--- Alexa ---+
fauxmoESP fauxmo;


//+--- OLED ---+
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//+--- Sensor AHT10 ---+
//#define Enable_AHT10

#ifdef Enable_AHT10
  #include "AHT10.h"
  int ahtValue;  //to store T/RH result
  AHTxx aht10(AHTXX_ADDRESS_X38, AHT1x_SENSOR); //sensor address, sensor type
#endif

//+--- CONFIG ---+/
#include "CONFIG.h"

//#define Enable_DEBUG

//PROTOCOLS
String s_client_id="";

enum 
{
    MODBUS_PROTOCOL_LinkBox= 0, 
    MODBUS_PROTOCOL_1C205= 1,
    MQTT_PROTOCOL= 2, 
    OPC_UA_PROTOCOL= 3,    
    Alexa_AWS= 4,
};

//+--- MQTT LIBs ---+
#include <PubSubClient.h>
#include "MQTT.h"
//#define MQTT_MAX_PACKET_SIZE 2048
//#include "SENSOR.h"

//+--- ModbusIP object ---+
LbModbusIP mb;

/***************  MODBUS ********************************/
// OBS: A biblioteca Modbus já soma os offsets 10001, 30001 e 400001 para os diferentes tipos de registradores
// desse modo, cada tipo de registrador (Coil, Discrete Inputs, Input Register e Holding Register)
// podem ser acrescentados a partir de 0 (zero) para o ScadaBR
//
//  https://github.com/andresarmento/modbus-arduino/blob/master/README.md
//The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in ScadaBR (http://www.scadabr.com.br) offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the CAS Modbus Scanner (http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software. 
//
//+--- Registrador de Offsets ---+
//Register Type   	Register Number  	Register Size 	Permission
//    Coil			    1-9999			    1 bit		   R/W
//Discrete Inputs     10001-19999			1 bit		   R/W
//Input Register	  30001-39999			16 bit		    R
//Holding Register	  40001-49999		    16 bit 	       R/W

//+---------------------------+                    //+----------------------------+
//+--- Function Code READ: ---+                    //+--- Function Code WRITE: ---+
//+---------------------------+                    //+----------------------------+                             
//01: Coils (FC=01)                                05: Single Coil (FC=05)
//02: Discrete Inputs (FC=02)                      06: Single Holding Register (FC=06)
//03: Multiple Holding Registers (FC=03)           0F: Multiple Coils (FC=15)
//04: Input Registers (FC=04)                      10: Multiple Holding Registers (FC=16)

//
//Modbus Jargon
//In this library was decided to use the terms used in Modbus to the methods names, then is important clarify the names of register types:
//------------------------------------------------------------------------+
//Register type	   |   Use as	         |  Access	    | Library methods
//------------------------------------------------------------------------+
//Coil	           |   Digital Output	 | Read/Write	  | addCoil(), Coil()
//Holding Register |	 Analog Output	 | Read/Write	  | addHreg(), Hreg()
//Discrete Inputs  |   Digital Input	 | Read Only	  | addIsts(), Ists()
//Input Register	 |   Analog Input	   | Read Only	  | addIreg(), Ireg()
//------------------------------------------------------------------------+

uint8_t u_thisDeviceId=0; //Client/Slave Device Number (usado na comunicação MQTT para diferenciar dispositivos)


 /***************  Telnet ********************************/
WiFiServer TelnetServer(23); // Telnet Server Declaration port 23
WiFiClient SerialOTA;        // Telnet Client Declaration 
bool haveClient = false;     //client detection flag
//+------------------------------------------------------+/


//+----------------------------+
//+--- Temporização -----------+
//+----------------------------+

//+--- MQTT POLLING TIME ---+
#define def_mqtt_polling_ms 1  // MQTT polling time (milliseconds)
unsigned long ul_MqttPoll_PrevTime = 0;  // will store last time sensors was read

//+--- SENSORS POLLING TIME ---+
#define def_sensors_polling_ms 5000  // sensors polling time (milliseconds)
unsigned long ul_SensorRead_PrevTime = 0;  // will store last time sensors was read

//+--- BUTTOM PRESS/RELEASE TIME ---+
unsigned long ul_PreviousMsButtonRead = 0;  // will store last time sensors was read

//+--- One Second Timming ---+
unsigned long ul_OneSecond_PrevTime=0;

//+--- PUSH BUTTON ---+
unsigned long ul_PushButton_PrevTime;
bool b_Pressed_Block=false;
bool b_Released_Block=false;

//+--- Menu Variables ---+
bool b_GoToMenu=false;
bool b_MenuHold=false; //Keeps the menu activated until a choice is made or detects inactivity for 5 or more seconds
bool b_MenuFirstCall=false;
bool b_GoToLoop=false;
char by_menu_option=0;
char by_stored_protocol_option=0;
unsigned int ui_menu_protocol_option=0;
unsigned int ui_ms_button_pressed=0;
unsigned int ui_ms_button_released=0;


//+--- variables for blinking an ci_led with milisseconds ---+
const int ci_led = 2; // ESP32 Pin to which onboard ci_led is connected
unsigned long ul_Blink_CurrTime = 0;  // will store last time ci_led was updated
const long cl_IntervalBlink = 1000;  // interval at which to blink (milliseconds)
int ci_ledState = LOW;  // ci_ledState used to set the ci_led



//+----------------------------+
//+--- MQTT -------------------+
//+----------------------------+
String s_mqtt_broker_IP = ""; 
bool b_MqttStatus = 0;
//
bool b_mqtt_reconnect=false;              
bool b_mqtt_reconnected_message=false;    
int i_mqtt_reconnected_display_seconds=0; 


//+----------------------------+
//+-------- STRINGS -----------+
//+----------------------------+
unsigned int i=0; //início de uma substring
unsigned int f=0; //fim de uma substring


//+----------------------------+
//+--------- OLED -------------+
//+----------------------------+
char s_oled_header[40];


void ButtomMenuCheck();

String s_mac=""; //MAC Address

//+----------------------------------------------------------------------------------+

void setup() 
{
    Serial.begin(115200);

    //+----------------------------------------------------------------------------------+
    //+ Configura os pinos dos LEDs Verde e Vermelho
    #ifdef Enable_ArduinoRelay
        pinMode(def_pin_LED_GREEN, OUTPUT);
        digitalWrite(def_pin_LED_GREEN, HIGH);  //Inverse Logic, because when input signal of Arduino Relay is 0 (Zero) the outpup is NO
        //
        pinMode(def_pin_LED_RED, OUTPUT);
        digitalWrite(def_pin_LED_RED, HIGH);    //Inverse Logic, because when input signal of Arduino Relay is 0 (Zero) the outpup is NO
    #else
        pinMode(def_pin_LED_GREEN, OUTPUT);
        digitalWrite(def_pin_LED_GREEN, LOW);
        //
        pinMode(def_pin_LED_RED, OUTPUT);
        digitalWrite(def_pin_LED_RED, LOW);
    #endif
    //+----------------------------------------------------------------------------------+
    //+ Configures retentive and push buttons pins as input
    pinMode(def_pin_RETN_BUTTON, INPUT);
    pinMode(def_pin_PUSH_BUTTON, INPUT);
    //+----------------------------------------------------------------------------------+

    /*************** OLED *********************/
    mb.OledInitialize(); // initialize OLED display with I2C address 0x3C


    /*************** initialize EEPROM with predefined size *********************/
    EEPROM.begin(EEPROM_SIZE);

    //+--- EEPROM.write(address, value)
    //EEPROM.write(0, 1);  EEPROM.commit();  //Força uma opção no MENU        

    by_stored_protocol_option = EEPROM.read(0);  //protocolo selecionado

    Serial.println("WiFi Connecting...");
    SerialOTA.println("WiFi Connecting...");
    //WiFi.mode(WIFI_STA);
    
    
    if(by_stored_protocol_option==MODBUS_PROTOCOL_LinkBox)
    {
        /*************** OLED *********************/
        mb.OledLine1("MODBUS > SSID:LinkBox ");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" CONNECT...  ");
        mb.OledUpdate_Static(1,2,2);
        delay(3000);
        //*****************************************/
        WiFi.begin(LinkBox_ssid, LinkBox_password); //MODBUS and MQTT LinkBox SSID and Password
    }
    else if(by_stored_protocol_option==MODBUS_PROTOCOL_1C205)
    {
        /*************** OLED *********************/
        mb.OledLine1("MODBUS>SSID:INDUSTRIA");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" CONNECT...  ");
        mb.OledUpdate_Static(1,2,2);
        delay(3000);
        //*****************************************/
        WiFi.begin(Sala_1C205_ssid, Sala_1C205_password); //Connects to the classroom router 
    }
    else if(by_stored_protocol_option==MQTT_PROTOCOL)
    {
        /*************** OLED *********************/
        mb.OledLine1(" MQTT > SSID:LinkBox ");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" CONNECT...  ");
        mb.OledUpdate_Static(1,2,2);
        delay(3000);
        //*****************************************/
        WiFi.begin(LinkBox_ssid, LinkBox_password); //MODBUS and MQTT LinkBox SSID and Password
    }
    else if(by_stored_protocol_option==Alexa_AWS)
    {
        /*************** OLED *********************/
        mb.OledLine1("Alexa > SSID:DOMOTICA");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" CONNECT...  ");
        mb.OledUpdate_Static(1,2,2);
        delay(3000);
        //*****************************************/
        WiFi.begin(Alexa_ssid, Alexa_password); //Connects to the Home Automation  classroom router
    }
    else  //When the ESP32 is being recorded for the first time it is necessary to choose an option in the MENU 
    {
        EEPROM.write(0, 0); EEPROM.commit();  // ("MODBUS > SSID:LinkBox ") option
        delay(300);
        by_stored_protocol_option = EEPROM.read(0);  //selected protocol

        /*************** OLED *********************/
        mb.OledLine1("MODBUS > SSID:LinkBox ");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" CONNECT...  ");
        mb.OledUpdate_Static(1,2,2);
        delay(3000);
        //*****************************************/
        WiFi.begin(LinkBox_ssid, LinkBox_password); 
    }

  
    while (WiFi.waitForConnectResult() != WL_CONNECTED) 
    {

        //+--- If the PUSH Button is pressed go to MENU ---+
        if(digitalRead(def_pin_PUSH_BUTTON) == 1)
        {
            b_GoToMenu=true;
            b_MenuHold=true;
            b_MenuFirstCall=true; 
            b_GoToLoop=true;
            break;
        }

        Serial.println("WiFi Connection Failed! Rebooting...");
        SerialOTA.println("WiFi Connection Failed! Rebooting..."); //+--Putty PORT 23
        //
        mb.OledLine1("             ");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" FAILED!!!   ");        
        mb.OledUpdate();
        delay(2000);

        //+--- If the PUSH Button is pressed go to MENU---+
        if(digitalRead(def_pin_PUSH_BUTTON) == 1)
        {
            b_GoToMenu=true;
            b_MenuHold=true;
            b_MenuFirstCall=true; 
            b_GoToLoop=true;
            break;
        }

        //
        mb.OledLine1("             ");
        mb.OledLine2("   WiFi     ");
        mb.OledLine3(" RESTART...  ");
        mb.OledUpdate();
        delay(2000);

        //+--- If the PUSH Button is pressed go to MENU---+
        if(digitalRead(def_pin_PUSH_BUTTON) == 1)
        {
            b_GoToMenu=true;
            b_MenuHold=true;
            b_MenuFirstCall=true; 
            b_GoToLoop=true;
            break;
        }

        ESP.restart();
    }
    

    if(b_GoToLoop==false)
    {
        if(by_stored_protocol_option==MODBUS_PROTOCOL_LinkBox)
        {
            //*****************************************/
            mb.OledLine1("MODBUS > SSID:LinkBox");
            mb.OledLine2("  WiFi OK    ");
            mb.OledLine3("             ");
            mb.OledUpdate_Static(1,2,2);
            delay(3000);
            //*****************************************/
        }
        else if(by_stored_protocol_option==MODBUS_PROTOCOL_1C205)
        {
            //*****************************************/
            mb.OledLine1("MODBUS>SSID:INDUSTRIA");
            mb.OledLine2("  WiFi OK    ");
            mb.OledLine3("             ");
            mb.OledUpdate_Static(1,2,2);
            delay(3000);
            //*****************************************/
        }
        else if(by_stored_protocol_option==MQTT_PROTOCOL)
        {
            //*****************************************/
            mb.OledLine1(" MQTT > SSID:LinkBox");
            mb.OledLine2("  WiFi OK    ");
            mb.OledLine3("             ");
            mb.OledUpdate_Static(1,2,2);
            delay(3000);
            //*****************************************/
        }
        else if(by_stored_protocol_option==Alexa_AWS)
        {
            //*****************************************/
            mb.OledLine1("Alexa > SSID:Domotica");
            mb.OledLine2("  WiFi OK    ");
            mb.OledLine3("             ");
            mb.OledUpdate_Static(1,2,2);
            delay(3000);
            //*****************************************/
        }
 

        //Modbus IP Config 
        if(by_stored_protocol_option==MODBUS_PROTOCOL_LinkBox)
        {
            mb.config();  //MODBUS Only
            //*****************************************/
            mb.OledLine1("MODBUS>SSID:LinkBox");
            mb.OledLine2(" MODBUS OK   ");
            mb.OledLine3("             ");
            mb.OledUpdate_Static(1,2,2);
            delay(3000);
            //*****************************************/
        }
        
        if(by_stored_protocol_option==MODBUS_PROTOCOL_1C205)
        {
            mb.config();  //MODBUS Only
            //*****************************************/
            mb.OledLine1("MODBUS>SSID:INDUSTRIA");
            mb.OledLine2(" MODBUS OK   ");
            mb.OledLine3("             ");
            mb.OledUpdate_Static(1,2,2);
            delay(3000);
            //*****************************************/
        }
    }


    /* Permite definir porta para conexão
        Padrão: ESP8266 - 8266
                ESP32   - 3232              */
    //ArduinoOTA.setPort(port);

    /* Permite definir nome do host
      Padrão: ESP8266 - esp8266-[ChipID]
              ESP32   - esp32-[MAC]       */


    s_mac += String(WiFi.macAddress());
    
    Serial.print("MAC: ");   Serial.println(s_mac);  
    //SerialOTA.print("MAC: "); SerialOTA.println(s_mac);
    //const char mac[] = (String(WiFi.macAddress())).c_str();

    if(s_mac==LB0_Id1_MAC){ s_client_id += LB0_Id1_client; s_mqtt_broker_IP += LB0_mqtt_broker_IP; u_thisDeviceId= LB0_Id1_MQTT; }
    else if(s_mac==LB0_Id2_MAC){ s_client_id += LB0_Id2_client; s_mqtt_broker_IP += LB0_mqtt_broker_IP; u_thisDeviceId= LB0_Id2_MQTT; }
    //
    else if(s_mac==LB1_Id1_MAC){ s_client_id += LB1_Id1_client; s_mqtt_broker_IP += LB1_mqtt_broker_IP; u_thisDeviceId= LB1_Id1_MQTT; }
    else if(s_mac==LB1_Id2_MAC){ s_client_id += LB1_Id2_client; s_mqtt_broker_IP += LB1_mqtt_broker_IP; u_thisDeviceId= LB1_Id2_MQTT; }
    //
    else if(s_mac==LB2_Id1_MAC){ s_client_id += LB2_Id1_client; s_mqtt_broker_IP += LB2_mqtt_broker_IP; u_thisDeviceId= LB2_Id1_MQTT; }
    else if(s_mac==LB2_Id2_MAC){ s_client_id += LB2_Id2_client; s_mqtt_broker_IP += LB2_mqtt_broker_IP; u_thisDeviceId= LB2_Id2_MQTT; }
    //
    else if(s_mac==LB3_Id1_MAC){ s_client_id += LB3_Id1_client; s_mqtt_broker_IP += LB3_mqtt_broker_IP; u_thisDeviceId= LB3_Id1_MQTT; }
    else if(s_mac==LB3_Id2_MAC){ s_client_id += LB3_Id2_client; s_mqtt_broker_IP += LB3_mqtt_broker_IP; u_thisDeviceId= LB3_Id2_MQTT; }
    //
    else if(s_mac==LB4_Id1_MAC){ s_client_id += LB4_Id1_client; s_mqtt_broker_IP += LB4_mqtt_broker_IP; u_thisDeviceId= LB4_Id1_MQTT; }
    else if(s_mac==LB4_Id2_MAC){ s_client_id += LB4_Id2_client; s_mqtt_broker_IP += LB4_mqtt_broker_IP; u_thisDeviceId= LB4_Id2_MQTT; }
    //
    else if(s_mac==LB5_Id1_MAC){ s_client_id += LB5_Id1_client; s_mqtt_broker_IP += LB5_mqtt_broker_IP; u_thisDeviceId= LB5_Id1_MQTT; }
    else if(s_mac==LB5_Id2_MAC){ s_client_id += LB5_Id2_client; s_mqtt_broker_IP += LB5_mqtt_broker_IP; u_thisDeviceId= LB5_Id2_MQTT; }
    //
    else if(s_mac==LB6_Id1_MAC){ s_client_id += LB6_Id1_client; s_mqtt_broker_IP += LB6_mqtt_broker_IP; u_thisDeviceId= LB6_Id1_MQTT; }
    else if(s_mac==LB6_Id2_MAC){ s_client_id += LB6_Id2_client; s_mqtt_broker_IP += LB6_mqtt_broker_IP; u_thisDeviceId= LB6_Id2_MQTT; }
    //
    else if(s_mac==LB7_Id1_MAC){ s_client_id += LB7_Id1_client; s_mqtt_broker_IP += LB7_mqtt_broker_IP; u_thisDeviceId= LB7_Id1_MQTT; }
    else if(s_mac==LB7_Id2_MAC){ s_client_id += LB7_Id2_client; s_mqtt_broker_IP += LB7_mqtt_broker_IP; u_thisDeviceId= LB7_Id2_MQTT; }
    //
    else if(s_mac==LB8_Id1_MAC){ s_client_id += LB8_Id1_client; s_mqtt_broker_IP += LB8_mqtt_broker_IP; u_thisDeviceId= LB8_Id1_MQTT; }
    else if(s_mac==LB8_Id2_MAC){ s_client_id += LB8_Id2_client; s_mqtt_broker_IP += LB8_mqtt_broker_IP; u_thisDeviceId= LB8_Id2_MQTT; }
    //
    else if(s_mac==LB9_Id1_MAC){ s_client_id += LB9_Id1_client; s_mqtt_broker_IP += LB9_mqtt_broker_IP; u_thisDeviceId= LB9_Id1_MQTT; }
    else if(s_mac==LB9_Id2_MAC){ s_client_id += LB9_Id2_client; s_mqtt_broker_IP += LB9_mqtt_broker_IP; u_thisDeviceId= LB9_Id2_MQTT; }
    //
    else if(s_mac==REN_Id1_MAC){ s_client_id += REN_Id1_client; s_mqtt_broker_IP += REN_mqtt_broker_IP; u_thisDeviceId= REN_Id1_MQTT; }
    else if(s_mac==REN_Id2_MAC){ s_client_id += REN_Id2_client; s_mqtt_broker_IP += REN_mqtt_broker_IP; u_thisDeviceId= REN_Id2_MQTT; }
    //
    else if(s_mac==FAB_Id1_MAC){ s_client_id += FAB_Id1_client; s_mqtt_broker_IP += FAB_mqtt_broker_IP; u_thisDeviceId= FAB_Id1_MQTT; }
    else if(s_mac==FAB_Id2_MAC){ s_client_id += FAB_Id2_client; s_mqtt_broker_IP += FAB_mqtt_broker_IP; u_thisDeviceId= FAB_Id2_MQTT; }
    //
    else { s_client_id += "ESP32_NOVA-"; s_mqtt_broker_IP += "192.168.1.250"; }

    if(b_GoToLoop==false)
    {

        if(by_stored_protocol_option==MQTT_PROTOCOL)
        {
                    //bool ConnectMQTT(bool reconnect) 
            b_MqttStatus = ConnectMQTT(false); //MQTT only - Connects to broker and subscribes to topic
        }


        if(by_stored_protocol_option!=Alexa_AWS)
        {
            #ifdef Enable_AHT10
                //+-----------------------------------------------------------------------------+
                //+--- Sensor AHT10 ------------------------------------------------------------+
                //+-----------------------------------------------------------------------------+
                //+--- MODBUS Atualiza os registradores (Input Registers) dos potenciômetros ---+
                //+--- Register Type   	Register Number  	Register Size 	Permission            +
                //+--- Input Register	  30001-39999		   16 bit	        R                       +
                
                AHT10_Config();
                
                if(b_AHT10_INIT_OK == true) //Read temperature and humidity if the AHT10 sensor has been initialized correctly
                {
                  ahtValue = aht10.readTemperature();   //read 6-bytes via I2C, takes 80 milliseconds
                  mb.Ireg(ir_TEMPTURE_OFFSET, ahtValue);  //+--- reads from the AHT10 sensor and copy the value to the Modbus ir_TEMPTURE_OFFSET offset register
                  //Serial.printf("\nTEMPTURE=%d",ahtValue);
                  ahtValue = aht10.readHumidity();      //read 6-bytes from I2C, takes 80 milliseconds
                  mb.Ireg(ir_HUMIDITY_OFFSET, ahtValue); //+--- reads from AHT10 sensor and copy the value to the Modbus ir_HUMIDITY_OFFSET offset register
                  //Serial.printf("\nHUMIDITY=%d",ahtValue);
                }
                //+-----------------------------------------------------------------------------+
            #endif
        }

    }

    s_client_id += String(WiFi.macAddress());
    ArduinoOTA.setHostname(s_client_id.c_str());

  
    // No authentication by default
    //ArduinoOTA.setPassword("admin");
    //ArduinoOTA.setPassword("");
    //ArduinoOTA.setPassword("domus3312");
    //
    //
    // cc_password can be set with it's md5 value as well
    // https://md5.gromweb.com/
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3"); //Codified with MD5 Hash Generator
    //ArduinoOTA.setPasswordHash("0ac4fb5f0c170b08e9a7d2f4c07536be"); //Codified with MD5 Hash Generator


    ArduinoOTA
    .onStart([]() 
    {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    })
    
    .onEnd([]() 
    {
        Serial.println("\nEnd");
    })
      
    .onProgress([](unsigned int progress, unsigned int total) 
    {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })

    .onError([](ota_error_t error) 
    {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();

    //+----------------------------------------------------------------------------------+
    //OBS: ADD CODE BELOW THIS NOTE!!!
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

     
    /*************** Telnet  *********************/
    TelnetServer.begin();
    TelnetServer.setNoDelay(true);

    /*************** MODBUS  *********************/
    //+-- Set_XXXXX(int device, int offset) to Modbus.h lib
    mb.Set_CoilsOffset(0, co_LED_GREEN_OFFSET);
    mb.Set_CoilsOffset(1, co_LED_RED_OFFSET);
    //
    mb.Set_DigitalInputOffset(0, di_RETN_BUTTON_OFFSET);
    mb.Set_DigitalInputOffset(1, di_PUSH_BUTTON_OFFSET);
    //
    #ifdef Enable_AHT10
        mb.Set_InputRegsOffset(0, ir_TEMPTURE_OFFSET);
        mb.Set_InputRegsOffset(1, ir_HUMIDITY_OFFSET);
    #else
        mb.Set_InputRegsOffset(0, ir_POT_LEFT_OFFSET);
        mb.Set_InputRegsOffset(1, ir_POT_RIGHT_OFFSET);
    #endif
    //
    mb.Set_HoldingRegsOffset(0, hr_RGB_R_OFFSET);
    mb.Set_HoldingRegsOffset(1, hr_RGB_G_OFFSET);
    mb.Set_HoldingRegsOffset(2, hr_RGB_B_OFFSET);

    /*************** MODBUS  *********************/
    //+-- Add binary Modbus registers (bit=COIL) ---+
    //+--- Coils ---+
    mb.addCoil(co_LED_GREEN_OFFSET);
    mb.Coil(co_LED_GREEN_OFFSET, false); 
    mb.addCoil(co_LED_RED_OFFSET);
    mb.Coil(co_LED_RED_OFFSET, false);  

    //+--- Digital Inputs ---+
    mb.addIsts(di_RETN_BUTTON_OFFSET);
    mb.addIsts(di_PUSH_BUTTON_OFFSET);
  
    #ifdef Enable_AHT10
        //+-- Adds analog Modbus registers (2 bytes)
        //+--- Imput Registers ---+
        mb.addIreg(ir_TEMPTURE_OFFSET);
        mb.addIreg(ir_HUMIDITY_OFFSET);
    #else
        //+-- Adds analog Modbus registers (2 bytes)
        //+--- Imput Registers ---+
        mb.addIreg(ir_POT_LEFT_OFFSET);
        mb.addIreg(ir_POT_RIGHT_OFFSET);
    #endif

    //+--- Holding Registers ---+
    mb.addHreg(hr_RGB_R_OFFSET);
    mb.addHreg(hr_RGB_G_OFFSET);
    mb.addHreg(hr_RGB_B_OFFSET);

    /*************** PWM  config *********************/
    // PWM Config
    ledcSetup(def_rgb_red_channel, def_pwm_frequency, def_pwm_resolution);
    ledcSetup(def_rgb_blue_channel, def_pwm_frequency, def_pwm_resolution);
    ledcSetup(def_rgb_green_channel, def_pwm_frequency, def_pwm_resolution);
      
    //Associating the GPIO with the chosen channel
    ledcAttachPin(def_rgb_pin_red, def_rgb_red_channel);
    ledcAttachPin(def_rgb_pin_blue, def_rgb_blue_channel);
    ledcAttachPin(def_rgb_pin_green, def_rgb_green_channel);

    //Turn de LEDs off
    ledcWrite(def_rgb_red_channel, 1023);
    ledcWrite(def_rgb_blue_channel, 1023);
    ledcWrite(def_rgb_green_channel, 1023);


    /******************** OLED ****************************/
    //***      Header (first line of the display)       ***/
    /******************************************************/
    if(by_stored_protocol_option==MODBUS_PROTOCOL_LinkBox){ strcpy(s_oled_header,"MODBUS IP:"); strcat(s_oled_header, (WiFi.localIP().toString().c_str()));  strcat(s_oled_header, " (SSID:LinkBox)"); }
    else if(by_stored_protocol_option==MODBUS_PROTOCOL_1C205){ strcpy(s_oled_header,"MODBUS IP:"); strcat(s_oled_header, (WiFi.localIP().toString().c_str())); strcat(s_oled_header, " (SSID:INDUSTRIA)"); }
    else if(by_stored_protocol_option==MQTT_PROTOCOL){ strcpy(s_oled_header,"MQTT CLIENT IP:"); strcat(s_oled_header, (WiFi.localIP().toString().c_str())); strcat(s_oled_header, " (SSID:LinkBox)"); }
    else if(by_stored_protocol_option==OPC_UA_PROTOCOL){ strcpy(s_oled_header,"OPC UA:"); strcat(s_oled_header, (WiFi.localIP().toString().c_str())); }
    else if(by_stored_protocol_option==Alexa_AWS){ strcpy(s_oled_header,"Alexa:"); strcat(s_oled_header, (WiFi.localIP().toString().c_str())); }
    else { strcpy(s_oled_header,"Nenhuma Opção do MENU"); }
    //
    if(b_GoToLoop==false)
    {
        mb.OledLine1(s_oled_header);
        mb.OledLine2("             ");
        mb.OledLine3("             ");
        mb.OledUpdate();


        if(by_stored_protocol_option==Alexa_AWS)
        {
              Alexa_Config();
        } // FIM - if(by_stored_protocol_option==ALEXA)
     
    }

    ul_OneSecond_PrevTime=millis();    
    ul_SensorRead_PrevTime=millis();
    ul_PushButton_PrevTime=millis();

}


//+----------------------------------------------------------------------------------+

void loop() 
{
    ArduinoOTA.handle();  

    //SerialOTA.print("MAC: "); SerialOTA.println(s_mac);
    
    //+--- TELNET ------------------------------------+
    // Handle new/disconnecting clients.
    if (!haveClient) 
    {
        // Check for new client connections.
        SerialOTA = TelnetServer.available();
        if (SerialOTA) 
        {
          haveClient = true;
        }
    } 
    else if (!SerialOTA.connected()) 
    {
      // The current client has been disconnected.
      SerialOTA.stop();
      SerialOTA = WiFiClient();
      haveClient = false;
    }

    #ifdef def_Show_SizeOf_DataTypes
      Show_SizeOf_DataTypes();
    #endif

    //+--- MODBUS ------------------------------------+
    if((by_stored_protocol_option==MODBUS_PROTOCOL_LinkBox)||(by_stored_protocol_option==MODBUS_PROTOCOL_1C205))
    {
        mb.task();
    }

    //+--- MQTT CONNECT ------------------------------+
    if((by_stored_protocol_option==MQTT_PROTOCOL)&&(client.state()!=0)&&(b_GoToLoop==false))
    {
                //bool ConnectMQTT(bool reconnect) 
        b_mqtt_reconnect=true;
        b_MqttStatus = ConnectMQTT(b_mqtt_reconnect); //Restart MQTT - Connect to broker and subscribe to topic
    }

    //+--- ALEXA ------------------------------+
    if(by_stored_protocol_option==Alexa_AWS)
    {
        fauxmo.handle();
    }
 

    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+---    Coil				      1-9999				   1 bit			     R/W     ---+
    //+-- Copies the Modbus register value (co_LED_GREEN_OFFSET) to the GPIO16=RX2 pin (LED_GREEN)
    #ifdef Enable_ArduinoRelay  
        digitalWrite(def_pin_LED_GREEN, !(mb.Coil(co_LED_GREEN_OFFSET))); // Inverte a lógica de acionamento porque o contato NO do relé Arduino fecha quando o sinal na entrada é 0(Zero))
    #else
        digitalWrite(def_pin_LED_GREEN, (mb.Coil(co_LED_GREEN_OFFSET))); 
    #endif
    //
    //
    // 

    //+-- Copies the value from the Modbus register (co_LED_RED_OFFSET) to the GPIO4=D4 pin (LED_RED)
    digitalWrite(def_pin_LED_RED, mb.Coil(co_LED_RED_OFFSET));
    //
    //
    //
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+-- Discrete Inputs     10001-19999		     1 bit		       R/W     ---+
    mb.Ists(di_RETN_BUTTON_OFFSET, digitalRead(def_pin_RETN_BUTTON));
    mb.Ists(di_PUSH_BUTTON_OFFSET, digitalRead(def_pin_PUSH_BUTTON));
    //

    //
    //
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+- Holding Register	   40001-49999			   16 bit          R     ---+
    //+-- Configures the PWM value to activate the red LED of the RGB LED 
    //+-- LEDs are common anode, so when the cyclic ratio is 0 the LED emits maximum brightness
    ledcWrite(def_rgb_red_channel, (1023-((mb.Hreg(hr_RGB_R_OFFSET)*1023)/100)));

    //+-- Configures the PWM value to activate the green LED of the RGB LED ---+
    //+-- LEDs are common anode, so when the cyclic ratio is 0 the LED emits maximum brightness
    ledcWrite(def_rgb_green_channel, (1023-((mb.Hreg(hr_RGB_G_OFFSET)*1023)/100)));

    //+-- Configures the PWM value to activate the blue LED of the RGB LED ---+
    //+-- LEDs are common anode, so when the cyclic ratio is 0 the LED emits maximum brightness
    ledcWrite(def_rgb_blue_channel, (1023-((mb.Hreg(hr_RGB_B_OFFSET)*1023)/100)));

    /*
    //+--- loop to blink without delay ---+
    if(millis()<ul_Blink_CurrTime){ ul_Blink_CurrTime=millis(); }
    if(millis()-ul_Blink_CurrTime>=1)     
    {
        //SerialOTA.print("time: ");
        //SerialOTA.println(millis());

        // if the ci_led is off turn it on and vice-versa:
        ci_ledState = not(ci_ledState);
        // set the ci_led with the ci_ledState of the variable:
        digitalWrite(ci_led,  ci_ledState);
    }
  */

    //+--- Loop - Checks if there is any publication in MQTT BROKER 
    if(millis()<ul_MqttPoll_PrevTime){ ul_MqttPoll_PrevTime=millis(); } //millis() will overflow (go back to zero), after approximately 50 days. So, when it happens ul_MqttPoll_PrevTime must be reseted
    if(millis()-ul_MqttPoll_PrevTime>=def_mqtt_polling_ms) 
    {
        ul_MqttPoll_PrevTime=millis();
  
        //+--- Checks if MQTT is the selected protocol ---+
        if(by_stored_protocol_option==MQTT_PROTOCOL)
        {
            client.loop();  //+--- Check if there is any publication in MQTT BROKER 
                            //+--- If there is any data, it will be received by the routine Callback(char *topic, byte *payload, unsigned int length)
        }
    }

  //+--- loop send sensor values ---+
    if(millis()<ul_SensorRead_PrevTime){ ul_SensorRead_PrevTime=millis(); } //millis() will overflow (go back to zero), after approximately 50 days. So, when it happens ul_SensorRead_PrevTime must be reseted
    if(millis()-ul_SensorRead_PrevTime>=def_sensors_polling_ms) 
    {
        ul_SensorRead_PrevTime=millis();

        if(by_stored_protocol_option!=Alexa_AWS)
        {
            #ifdef Enable_AHT10
                if(b_AHT10_INIT_OK == true) //Read temperature and humidity if the AHT10 sensor has been initialized correctly
                {
                    //+--- MODBUS Atualiza os registradores (Input Registers) dos potenciômetros ---+
                    //+--- Register Type   	Register Number  	Register Size 	Permission            +
                    //+--- Input Register	  30001-39999		       16 bit	         Read               +
                    ahtValue = aht10.readTemperature();   //read 6-bytes via I2C, takes 80 milliseconds
                    mb.Ireg(ir_TEMPTURE_OFFSET, ahtValue);  //+--- Copia o valor da temperatura lida no sensor AHT10 para o registrador Modbus ir_TEMPTURE_OFFSET
                    //Serial.printf("\nTEMPTURE=%d",ahtValue);
                    ahtValue = aht10.readHumidity();      //read 6-bytes via I2C, takes 80 milliseconds
                    mb.Ireg(ir_HUMIDITY_OFFSET, ahtValue); //+--- Copia o valor da umidade lida no sensor AHT10 para o registrador Modbus ir_HUMIDITY_OFFSET  
                    //Serial.printf("\nHUMIDITY=%d",ahtValue);
                }
            #else
                //+--- MODBUS Updates the potentiometer Input Registers ---+
                //+--- Register Type   	Register Number  	Register Size 	Permission            +
                //+--- Input Register	  30001-39999		   16 bit	        R                       +
                mb.Ireg(ir_POT_LEFT_OFFSET,  analogRead(def_pin_POT_LEFT));  //+--- Copies the value from ADC1_3 (GPIO39) to the Modbus register ir_POT_LEFT_OFFSET
                mb.Ireg(ir_POT_RIGHT_OFFSET, analogRead(def_pin_POT_RIGHT)); //+--- Copies the value from ADC1_6 (GPIO34) to the Modbus register ir_POT_RIGHT_OFFSET 
                //
            #endif
        }


        /*
        //+--- MQTT (Sends sensor readings to the BROKER without a request) -----------+
        if(by_stored_protocol_option==MQTT_PROTOCOL)
        {   
            //MqttSend_ModbusFrame(b_MqttStatus, mb.Ireg(ir_TEMPTURE_OFFSET), 4, 1, 0, 1);
            //MqttSend_ModbusFrame(b_MqttStatus, ca_topic, mb.Ireg(ir_TEMPTURE_OFFSET), 4, 1, 0, 1);
            //MqttSend_ModbusFrame(b_MqttStatus, topic_Id1, mb.Ireg(ir_HUMIDITY_OFFSET), 4, 1, 1, 1);
            //SerialOTA.println("Passou por aqui!"); //+--Putty PORT 23
        } 
        */
    }


    //+--- One Second Timming ---+
    if(millis()<ul_OneSecond_PrevTime){ ul_OneSecond_PrevTime=millis(); } //millis() will overflow (go back to zero), after approximately 50 days. So, when it happens ul_OneSecond_PrevTime must be reseted
    if(millis()-ul_OneSecond_PrevTime>=1000)
    {
        ul_OneSecond_PrevTime=millis();
        if(b_mqtt_reconnected_message==true)
        {
            i_mqtt_reconnected_display_seconds++;
            if(i_mqtt_reconnected_display_seconds > 6)
            {
              mb.OledLine2("             ");
              mb.OledLine3("             ");
              mb.OledUpdate();
              b_mqtt_reconnected_message=false;
            }
        }
    }

    ButtomMenuCheck();
    
    if(b_GoToMenu==false){ mb.OledUpdate(); }
    if(b_GoToMenu==true){ Protocol_Menu(); }

} // FIM - void loop()


//+----------------------------------------------------------------------------------+
void ButtomMenuCheck()
{
    //+--- loop button pressed/released timming ---+
    if(millis()<ul_PushButton_PrevTime){ ul_PushButton_PrevTime=millis(); } //millis() will overflow (go back to zero), after approximately 50 days. So, when it happens ul_PushButton_PrevTime must be reseted
    if(millis()-ul_PushButton_PrevTime>=1) 
    {
      ul_PushButton_PrevTime=millis();
      
      //+--- Lê o PUSH Button ---+
      if(digitalRead(def_pin_PUSH_BUTTON) == 1)
      {
          ui_ms_button_pressed++; 
          ui_ms_button_released=0; 
          //char str1[8]; // DEBUG
          //itoa(ui_ms_button_pressed, str1, 10); // DEBUG
          //mb.OledLine3(str1); // DEBUG
          //mb.OledLine2("Step 1 OK     "); // DEBUG
      }    
      
      if(digitalRead(def_pin_PUSH_BUTTON) == 0)
      {
          ui_ms_button_pressed=0; 
          ui_ms_button_released++; 
          //char str1[8]; // DEBUG
          //itoa(ui_ms_button_released, str1, 10); // DEBUG
          //mb.OledLine3(str1); // DEBUG
          //mb.OledLine2("            "); // DEBUG
      }        
      
      //+--- Checks if the push button is pressed for a while to access the menu
      if((IsPushButtonPressed(200))&&(b_MenuHold==false))
      {
        //mb.OledLine2("Step 2 OK     "); // DEBUG
        b_GoToMenu=true;
        b_MenuHold=true;
        b_MenuFirstCall=true; 
      }

    } // END - button pressed/released timing loop ---+
   
}

//+----------------------------------------------------------------------------------+
bool IsPushButtonPressed(unsigned int ms)
{
  if((ui_ms_button_pressed>ms)&&(b_Pressed_Block==false))
  { 
      b_Pressed_Block=true; 
      b_Released_Block=false; 
      return true; 
  }
  return false;
}

//+---------------------------------------------------------------+

bool IsPushButtonReleased(unsigned int ms)
{
  if((ui_ms_button_released>ms)&&(b_Released_Block==false))
  { 
      b_Pressed_Block=false; 
      b_Released_Block=true; 
      return true; 
  }
  return false;
}




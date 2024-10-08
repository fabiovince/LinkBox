//+---------------------------------------------------------------------------------------------+
//|   Didatic KIT - LinkBox - Developed to support learning the following subjects:             |
//    Industrial Informatics II, Home Automation and Industrial Networks I of                   |
//    Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)  |
//    by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                          |
//                                                                 Updated: 2024/05/03          |         
//                                                                                              |
// # Updates                                                                                    |
// may/02/24 : Renato F.                                                                        |
// Now the Modbus TCP connection is more stable. Limit of 8 Clients Modbus TCP.                 |
// Include the emelianov/modbus-esp8266 code to Modbus TCP handling, because the modbus         |
// connections were closed every time.                                                          |
// New Function: Disable the OLED scan using the Modbus Address 9000.                           |
//+---------------------------------------------------------------------------------------------+

#include <WiFi.h>
#include <WiFi_CONN.h>
#include <Alexa.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "global_vars.h"
#include <ModbusIP_ESP8266.h>

#define OTA_ENABLED 1

//+--- Alexa ---+
fauxmoESP fauxmo;

//+--- OLED ---+
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//+--- Sensor AHT10 ---+
#ifdef Enable_AHT10
  #include <Adafruit_AHTX0.h>
  #include <AHT10.h>
  bool b_AHT10_INIT_OK = false;
  int ahtValue;  //to store T/RH result
  Adafruit_AHTX0 aht10; //sensor address, sensor type
#endif


//+--- CONFIG ---+/
#include "CONFIG.h"

//PROTOCOLS
String s_client_id="";


//+--- MQTT LIBs ---+
#include <PubSubClient.h>
#include "MQTT.h"
//#define MQTT_MAX_PACKET_SIZE 2048
//#include "SENSOR.h"

//+--- Modbus objects ---+
LbBoard lbmb;
ModbusIP mb1;

/***************  MODBUS ********************************/
// OBS: A biblioteca Modbus utiliza o padrao modbus novo (modbus.org) 
// desse modo, cada tipo de registrador (Coil, Discrete Inputs, Input Register e Holding Register)
// podem ser acrescentados a partir de 0 (zero) para o ScadaBR
// Todo a base modus é feita pelo projeto "Modbus Library for Arduino" de Andre Sarmento https://github.com/andresarmento/modbus-esp8266 
// The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in ScadaBR (http://www.scadabr.com.br) offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the CAS Modbus Scanner (http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software. 
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
//
// New Features: 24/may/05 
// In the Modbus Protocol, to disable scan display of variables - write in the Coil address 9000 the value 1. This feature is to increase the response from the board to Modbus Commands. 



uint8_t u_thisDeviceId=0; //Client/Slave Device Number (usado na comunicação MQTT para diferenciar dispositivos)


 /***************  Telnet ********************************/
WiFiServer TelnetServer(23); // Telnet Server Declaration port 23
#if 1
WiFiClient SerialOTA;        // Telnet Client Declaration 
#endif
bool haveClient = false;     //client detection flag
//+------------------------------------------------------+/


//+----------------------------+
//+--- Temporização -----------+
//+----------------------------+

//+--- MQTT POLLING TIME ---+
#define def_mqtt_polling_ms 1  // MQTT polling time (milliseconds)
unsigned long ul_MqttPoll_PrevTime = 0;  // will store last time sensors was read

//+--- SENSORS POLLING TIME ---+
#define def_sensors_polling_ms 1000  // sensors polling time (milliseconds)
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

unsigned int by_stored_protocol_option=0;
unsigned int ui_menu_protocol_option=0;
unsigned int ui_ms_button_pressed=0;
unsigned int ui_ms_button_released=0;

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
//+--------- OLED -------------+
//+----------------------------+
char s_oled_header[40];
char s_oled_hdrcmp[15];

String s_mac=""; //MAC Address

//+----------------------------------------------------------------------------------+

void setup() 
{
    Serial.begin(115200);

    //+----------------------------------------------------------------------------------+
    //+ Configure the Green and Red LED pins
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
    lbmb.OledInitialize(); // initialize OLED display with I2C address 0x3C


    /*************** initialize EEPROM with predefined size *********************/
    EEPROM.begin(EEPROM_SIZE);

    //+--- EEPROM.write(address, value)
    //EEPROM.write(0, 1);  EEPROM.commit();  //Initializes the menu with the zero option       

    //by_stored_protocol_option = EEPROM.read(0);  //protocolo selecionado
    by_stored_protocol_option = EEPROM.read(0);  //protocolo selecionado

    /*************** WiFi CONNECT *********************/
    WiFi_CONNECT();

 #if OTA_ENABLED   
 
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
#endif

    /*************** MODBUS  *********************/
    //+-- Initiate the Modbus Points - Because MQTT use the Modbus Structure, needs to became available to the others 
    //if ((by_stored_protocol_option==MODBUS_PROTOCOL_LinkBox)||(by_stored_protocol_option==MODBUS_PROTOCOL_1C205))
       lbmb.ModbusInit();
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
    else if(by_stored_protocol_option==ALEXA_PROTOCOL){ strcpy(s_oled_header,"Alexa:"); strcat(s_oled_header, (WiFi.localIP().toString().c_str())); }
    else { strcpy(s_oled_header,"Nenhuma Opção do MENU"); }
    //
    if(b_GoToLoop==false)
    {
        lbmb.OledLine1(s_oled_header);
        lbmb.OledLine2("             ");
        lbmb.OledLine3("             ");
        lbmb.OledUpdate();


        if(by_stored_protocol_option==ALEXA_PROTOCOL)
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
        mb1.task();
    }

    lbmb.Task();

    //+--- MQTT CONNECT ------------------------------+
    if((by_stored_protocol_option==MQTT_PROTOCOL)&&(client.state()!=0)&&(b_GoToLoop==false))
    {
                //bool ConnectMQTT(bool reconnect) 
        b_mqtt_reconnect=true;
        b_MqttStatus = ConnectMQTT(b_mqtt_reconnect); //Restart MQTT - Connect to broker and subscribe to topic
    }

    //+--- ALEXA ------------------------------+
    if(by_stored_protocol_option==ALEXA_PROTOCOL)
    {
        fauxmo.handle();
    }
 

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

        if(by_stored_protocol_option!=ALEXA_PROTOCOL)
        {
            #ifdef Enable_AHT10
                if(b_AHT10_INIT_OK == true) //Read temperature and humidity if the AHT10 sensor has been initialized correctly
                {
                    //+--- MODBUS Atualiza os registradores (Input Registers) dos potenciômetros ---+
                    //+--- Register Type   	Register Number  	Register Size 	Permission            +
                    //+--- Input Register	  30001-39999		       16 bit	         Read               +
                    sensors_event_t humidity, temp;
                    aht10.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                    ahtValue = temp.temperature;   //read 6-bytes via I2C, takes 80 milliseconds
                    mb1.Ireg(ir_TEMPTURE_OFFSET, ahtValue);  //+--- reads from the AHT10 sensor and copy the value to the Modbus ir_TEMPTURE_OFFSET offset register
                    //Serial.printf("\nTEMPTURE=%d",ahtValue);
                    ahtValue = humidity.relative_humidity; //read 6-bytes from I2C, takes 80 milliseconds
                    mb1.Ireg(ir_HUMIDITY_OFFSET, ahtValue); //+--- reads from AHT10 sensor and copy the value to the Modbus ir_HUMIDITY_OFFSET offset register
                    //Serial.printf("\nHUMIDITY=%d",ahtValue);
                }
            #else
                //+--- MODBUS Updates the potentiometer Input Registers ---+
                //+--- Register Type   	Register Number  	Register Size 	Permission            +
                //+--- Input Register	  30001-39999		   16 bit	        R                       +
                mb1.Ireg(ir_POT_LEFT_OFFSET,  analogRead(def_pin_POT_LEFT));  //+--- Copies the value from ADC1_3 (GPIO39) to the Modbus register ir_POT_LEFT_OFFSET
                mb1.Ireg(ir_POT_RIGHT_OFFSET, analogRead(def_pin_POT_RIGHT)); //+--- Copies the value from ADC1_6 (GPIO34) to the Modbus register ir_POT_RIGHT_OFFSET 
                //
            #endif
        }

    }


    //+--- One Second Timming ---+
    if (!lbmb.isOledDisable()){
        if(millis()<ul_OneSecond_PrevTime){ ul_OneSecond_PrevTime=millis(); } //millis() will overflow (go back to zero), after approximately 50 days. So, when it happens ul_OneSecond_PrevTime must be reseted
        if(millis()-ul_OneSecond_PrevTime>=1000)
        {
            ul_OneSecond_PrevTime=millis();
            if(b_mqtt_reconnected_message==true)
            {
                i_mqtt_reconnected_display_seconds++;
                if(i_mqtt_reconnected_display_seconds > 6)
                {
                    lbmb.OledLine2("             ");
                    lbmb.OledLine3("             ");
                    lbmb.OledUpdate();
                    b_mqtt_reconnected_message=false;
                }
            }
        }

    }

    ButtomMenuCheck();
    
    if (!lbmb.isOledDisable()){
       if(b_GoToMenu==false) { 
        lbmb.OledUpdate(); 
       }
    }

    if(b_GoToMenu==true){ Protocol_Menu(); }

} // FIM - void loop()


//+----------------------------------------------------------------------------------+

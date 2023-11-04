#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

        //+----------------------------------------------------------------+
        //+--- To enable MOTOR KIT project uncomment directives below:
        //#define Enable_OLED_MOTOR_messages
        //#define Enable_ArduinoRelay  //Inverse Logic, because when input signal of Arduino Relay is 0 (Zero) the outpup is NO
        //#define Enable_AHT10 
        //+----------------------------------------------------------------+


        /* types for global vars */
        //+--- MODBUS -------+
        #include <LbModbus.h>  //Adicionar essa lib: Rascunho->Incluir Biblioteca->Adicionar Biblioteca.ZIP...  Modbus.zip
        #include <LbModbusIP_ESP32.h>  //Adicionar essa lib: Rascunho->Incluir Biblioteca->Adicionar Biblioteca.ZIP...  ModbusIP_ESP32-master.zip

        //+--- Global ModbusIP object ---+
        extern LbModbusIP mb;

        extern char ca_line3[18]; //OLED display line 3

        //+--- Register Type   	Register Number  	Register Size 	Permission
        //+---    Coil				      1-9999				   1 bit			     R/W     ---+
        #define co_LED_GREEN_OFFSET  0  // Offset do Coil Modbus para acionar co_led GREEN (bit=COIL) 
        #define co_LED_RED_OFFSET    1  // Offset do Coil Modbus para acionar co_led RED (bit=COIL)

        //+--- Register Type   	Register Number  	Register Size 	Permission
        //+-- Discrete Inputs     10001-19999			   1 bit			     R/W     ---+
        #define di_RETN_BUTTON_OFFSET  0  // Offset do Discrete Input para acionar o botão retentivo vermelho (bit=Discrete Input) 
        #define di_PUSH_BUTTON_OFFSET  1  // Offset do Discrete Input para acionar o botão não retentivo preto (bit=Discrete Input)

        //+--- Register Type   	Register Number  	Register Size 	Permission
        //+--- Input Register		  30001-39999			   16 bit	          R     ---+
        #define ir_POT_LEFT_OFFSET   0    // Offset do Input Register Modbus (2 bytes) ADC1_3 GPIO39
        #define ir_POT_RIGHT_OFFSET  1    // Offset do Input Register Modbus (2 bytes) ADC1_6 GPIO34

        //+--- Register Type   	Register Number  	Register Size 	Permission
        //+--- Input Register		  30001-39999			   16 bit	          R     ---+
        #define ir_TEMPTURE_OFFSET  0    // Offset do Input Register Modbus (2 bytes) AHT10 TEMPTURE
        #define ir_HUMIDITY_OFFSET  1    // Offset do Input Register Modbus (2 bytes) AHT10 HUMIDITY

        //+--- Register Type   	Register Number  	Register Size 	Permission
        //+- Holding Register		  40001-49999			  16 bit	          R     ---+
        #define hr_RGB_R_OFFSET  0    // Offset do Holding Register Modbus (2 bytes) PWM GPIO15
        #define hr_RGB_G_OFFSET  1    // Offset do Holding Register Modbus (2 bytes) PWM' GPIO2
        #define hr_RGB_B_OFFSET  2    // Offset do Holding Register Modbus (2 bytes) PWM GPIO0


        /*************** POTENTIOMETERS GPIO define ********************************/
        //+--- pino usado ---+
        #define def_pin_POT_LEFT  39 //GPIO39
        #define def_pin_POT_RIGHT 34 //GPIO34

        /*************** LEDs GPIO define ********************************/
        //+--- pino usado ---+
        #define def_pin_LED_GREEN 16 //GPIO16
        #define def_pin_LED_RED 4    //GPIO04

        /*************** PWM GPIO define ****************************************/
        #define def_rgb_pin_red 15
        #define def_rgb_red_channel 0
        //
        #define def_rgb_pin_blue 0
        #define def_rgb_blue_channel 1
        //
        #define def_rgb_pin_green 2
        #define def_rgb_green_channel 2
        //
        #define def_pwm_frequency 5000

        #define def_pwm_resolution 10

        /*************** BUTTONS GPIO define ****************************************/
        #define def_pin_RETN_BUTTON 18  //GPIO5
        #define def_pin_PUSH_BUTTON 5 //GPIO18




        //+--- Non-Volatile Memory Enable --------------+
        #include <EEPROM.h>
        #define EEPROM_SIZE 1  // Armazena a opção de protocolo atual - define the number of bytes you want to access

        //+--- Alexa ---+
        #include <fauxmoESP.h>
        //#define MOTOR "KitMotor"
        extern fauxmoESP fauxmo;


        //+--- Main.cpp Global Vars ---+
        //+- Menu Global Variables 
        extern bool b_GoToMenu;
        extern bool b_MenuFirstCall;
        extern char by_menu_option;
        extern char by_stored_protocol_option;
        extern bool b_MenuHold; //Mantem o menu ativado até que uma escolha seja realizada ou detecte inatividade por 5 ou mais segundos
        extern bool b_GoToLoop;
   
        //
        extern String s_client_id;
        extern bool b_mqtt_reconnected_message;
        extern unsigned int ui_ms_button_released;
        extern int i_mqtt_reconnected_display_seconds;
        extern bool IsPushButtonReleased(unsigned int ms);
        extern bool IsPushButtonPressed(unsigned int ms);

        //+--- Arduino Json ---+
        #include <ArduinoJson.h>
        extern StaticJsonDocument<200> frame;


        //+--- Menu.cpp Global Vars ---+
        extern void Protocol_Menu();

        /***************  Telnet ********************************/
        //extern WiFiServer TelnetServer(23); // Telnet Server Declaration port 23
        extern WiFiClient SerialOTA;        // Telnet Client Declaration 
        extern bool haveClient;     //client detection flag
        //+------------------------------------------------------+/

        //+----------------------------+
        //+--- MQTT -------------------+
        //+----------------------------+ 
        #include <PubSubClient.h>
        //Global Object
        extern PubSubClient client;

        //Global Variables
        extern bool b_MqttStatus;
        extern String s_mqtt_broker_IP; 
        extern uint8_t u_thisDeviceId; //Client/Slave Device Number (usado na comunicação MQTT para diferenciar dispositivos)

        // MQTT Broker
        //const char *mqtt_broker = "test.mosquitto.org";  //Host do broket
        //const char *mqtt_broker = "192.168.1.233";  //Host do broket
        //
        extern const char *topic;   //Tópico LinkBox Servidor 
        extern const char *mqtt_username;  //Usuario
        extern const char *mqtt_cc_password; //Senha
        extern const int mqtt_port; //Porta


 
        //+----------------------------+
        //+--------- OLED -------------+
        //+----------------------------+
        extern char s_oled_header[40];


        //+----------------------------+
        //+--------- AHT10 ------------+
        //+----------------------------+
        //+--- Sensor AHT10 ---+
        #include <Adafruit_AHTX0.h>

        //#ifdef Enable_AHT10
        //#include <AHT10.h>
        //int ahtValue;  //to store T/RH result
        //AHTxx aht10(AHTXX_ADDRESS_X38, AHT1x_SENSOR); //sensor address, sensor type
        //Adafruit_AHTX0 aht10; //sensor address, sensor type
        //#endif
        extern bool b_AHT10_INIT_OK;
        extern Adafruit_AHTX0 aht10; //sensor address, sensor type





#endif
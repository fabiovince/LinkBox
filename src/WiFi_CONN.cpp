#include "Config.h"
//#include <SSID_PASSWORD.h>
#include "WiFi_CONN.h"
#include "global_vars.h"
#include "MQTT.h"
#include <ArduinoOTA.h>

void WiFi_CONNECT()
{
    //+---------------------------------------------------+
    //+--- SSID and PASSWORD CONFIG ----------------------+
    //+---------------------------------------------------+
    const char* Alexa_ssid = "Domotica";
    const char* Alexa_password = "domotica1c203a";

    const char* LinkBox_ssid = "LinkBox";
    const char* LinkBox_password = "industria50";

    const char* Sala_1C205_ssid = "INDUSTRIA";
    const char* Sala_1C205_password = "industria50";
    //+---------------------------------------------------+

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
    else if(by_stored_protocol_option==ALEXA_PROTOCOL)
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
        else if(by_stored_protocol_option==ALEXA_PROTOCOL)
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
    else if(s_mac==MOTOR_Id1_MAC){ s_client_id += MOTOR_Id1_client; s_mqtt_broker_IP += MOTOR_mqtt_broker_IP; u_thisDeviceId= MOTOR_Id1_MQTT; }
    //
    else if(s_mac==FAB_Id1_MAC){ s_client_id += FAB_Id1_client; s_mqtt_broker_IP += FAB_mqtt_broker_IP; u_thisDeviceId= FAB_Id1_MQTT; }
    else if(s_mac==FAB_Id2_MAC){ s_client_id += FAB_Id2_client; s_mqtt_broker_IP += FAB_mqtt_broker_IP; u_thisDeviceId= FAB_Id2_MQTT; }
    //
    else { s_client_id += "ESP32_NOVA-"; s_mqtt_broker_IP += "192.168.1.233"; u_thisDeviceId= 1; }

    if(b_GoToLoop==false)
    {

        if(by_stored_protocol_option==MQTT_PROTOCOL)
        {
                    //bool ConnectMQTT(bool reconnect) 
            b_MqttStatus = ConnectMQTT(false); //MQTT only - Connects to broker and subscribes to topic
        }


        if(by_stored_protocol_option!=ALEXA_PROTOCOL)
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
                  sensors_event_t humidity, temp;
                  aht10.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  ahtValue = temp.temperature;   //read 6-bytes via I2C, takes 80 milliseconds
                  mb.Ireg(ir_TEMPTURE_OFFSET, ahtValue);  //+--- reads from the AHT10 sensor and copy the value to the Modbus ir_TEMPTURE_OFFSET offset register
                  //Serial.printf("\nTEMPTURE=%d",ahtValue);
                  ahtValue = humidity.relative_humidity; //read 6-bytes from I2C, takes 80 milliseconds
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
    //
    //
    // cc_password can be set with it's md5 value as well
    // https://md5.gromweb.com/
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3"); //Codified with MD5 Hash Generator
    //ArduinoOTA.setPasswordHash("0ac4fb5f0c170b08e9a7d2f4c07536be"); //Codified with MD5 Hash Generator

} //END - WiFi_Connect()
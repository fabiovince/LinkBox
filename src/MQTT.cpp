//+---------------------------------------------------------------------------------------------+
//|   Didatic KIT - LinkBox - Developed to to support learning the following subjects:          |
//    Industrial Informatics II, Home Automation and Industrial Networks I of                   |
//    Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)  |
//    by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                          |
//                                                                 Updated: 2023/10/28          |         
//+---------------------------------------------------------------------------------------------+

#include "MQTT.h"
#include "global_vars.h"
#include <Adafruit_SSD1306.h>

char ca_OLED_line3[18]="";  //

StaticJsonDocument<200> frame;

//#define Enable_Payload_Debug
//#define Enable_Display_OLED
#define Enable_TXT_Analisys
//#define Enable_CleanMQTT


// MQTT Broker CONFIG
//const char *mqtt_broker = "test.mosquitto.org";  //Host do broket
//const char *mqtt_broker = "192.168.1.233";  //Host do broket
//
const char *topic = "/LB";   //Tópico LinkBox Servidor 


//const char *topic_ledGreen = "/LB0/ledGreen";   //LED Green topic (ledGreen) 
//const char *topic_ledRed = "/LB0/ledRed";   //LED Ree topic (ledRed) 
//
const char *mqtt_username = "";         //User
const char *mqtt_cc_password = "";         //Password
const int mqtt_port = 1883;             //Port

//Variáveis  
//char timestamp[] = "2023061571844"; 

//Objects
WiFiClient espClient;
PubSubClient client(espClient);



//+------------------------------------------------------------------------------+
void Callback(char *topic, uint8_t *jsonChar, unsigned int length) 
{


    //
    #ifdef Enable_Payload_Debug
        SerialOTA.print(" Callback: "); //+--Putty PORT 23
        //
        for (int k = 0; k < length; k++) 
        {
            SerialOTA.print((char)jsonChar[k]); //+-- DEBUG - Putty PORT 23
        }
        
        //Serial.println(""); //+-- DEBUG - Arduino Serial Monitor
        SerialOTA.println(""); //+-- DEBUG - Putty PORT 23
        //Serial.println("-----------------------"); //+-- DEBUG - Arduino Serial Monitor
        SerialOTA.println("----------------------------------------------------"); //+-- DEBUG - Putty PORT 23
    #endif

    //+-----------------------------------------+
    //+--------- JSON MODBUS FRAME -------------+
    //+-----------------------------------------+
    
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(frame, jsonChar);

    // Test if parsing succeeds.
    if (error) 
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        //
        SerialOTA.print("deserializeJson() failed: "); //+--Putty PORT 23
        SerialOTA.println(error.f_str()); //+--Putty PORT 23
        return;
      }
      
    #ifdef Enable_CleanMQTT
          //void MqttSend_ModbusFrame(bool stat, const char *topic, uint16_t value, uint8_t fc, uint8_t unitid, uint16_t address, uint16_t quantity)
          String action = frame["action"];
          String origin = frame["origin"];
          uint8_t unitid = frame["unitid"];
          uint16_t currFrameNumber = frame["fn"];
         
          //uint16_t value = frame["value"];
          //uint8_t fc = frame["fc"];
          //uint16_t address = frame["address"];
          //uint16_t quantity = frame["quantity"];
         
          
    #else
          //void MqttSend_ModbusFrame(bool stat, const char *topic, uint16_t value, uint8_t fc, uint8_t unitid, uint16_t address, uint16_t quantity)
          uint16_t value = frame["value"];
          uint8_t fc = frame["fc"];
          uint8_t unitid = frame["unitid"];
          uint16_t address = frame["address"];
          uint16_t quantity = frame["quantity"];
          String origin = frame["origin"];
          uint16_t currFrameNumber = frame["fn"]; //fn = frame number
    #endif

    /*
    SerialOTA.print("value:");    SerialOTA.println(value);    //+-- DEBUG -Putty PORT 23
    SerialOTA.print("fc:");       SerialOTA.println(fc);       //+-- DEBUG -Putty PORT 23
    SerialOTA.print("unitid:");   SerialOTA.println(unitid);   //+-- DEBUG -Putty PORT 23
    SerialOTA.print("address:");  SerialOTA.println(address);  //+-- DEBUG -Putty PORT 23
    SerialOTA.print("quantity:"); SerialOTA.println(quantity); //+-- DEBUG -Putty PORT 23
    SerialOTA.print("origin:");     SerialOTA.println(origin);     //+-- DEBUG -Putty PORT 23
    //Serial.println(""); Serial.println("-----------------------"); //+-- DEBUG - Arduino Serial Monitor
    SerialOTA.println(""); SerialOTA.println("----------------------------------------------------"); //+-- DEBUG - Putty PORT 23
    */
    
    //Node-Red Modbus Payload Type: jsonChar[] = { value: x , 'fc': x, 'unitid': x, 'address': x , 'quantity': x };
    //jsonChar[] = {value,fc,unitid,address,quantity}

    if(b_MqttStatus && (origin=="client")&&(u_thisDeviceId == unitid))
    {
       

      #ifdef Enable_CleanMQTT

          if(action == "READ")
          { 

              //+-----------------------------------------+
              //+--------- JSON MODBUS FRAME -------------+
              //+-----------------------------------------+
              //+--- Início do PAYLOAD - Add values in the frame ---+
              frame.clear();
              frame["action"] = "READ";
              //
              frame["led_green"] = digitalRead(def_pin_LED_GREEN);
              frame["led_red"] = digitalRead(def_pin_LED_RED);
              //
              frame["retn"] = digitalRead(def_pin_RETN_BUTTON);
              frame["push"] = digitalRead(def_pin_PUSH_BUTTON);
              //
              frame["pot_left"] = analogRead(def_pin_POT_LEFT);
              frame["pot_right"] = analogRead(def_pin_POT_RIGHT);
              //
              frame["rgb_red"] = mb1.Hreg(hr_RGB_R_OFFSET);
              frame["rgb_green"] = mb1.Hreg(hr_RGB_G_OFFSET);
              frame["rgb_blue"] = mb1.Hreg(hr_RGB_B_OFFSET);
              //
              frame["unitid"] = unitid;
              frame["origin"] = "server";
              frame["origi2"] = "fabio";
              frame["fn"] = currFrameNumber;
              //
              //+--- FIM do PAYLOAD ---+
              //
              char jsonChar[300]="";
              serializeJson(frame, jsonChar);
              //
              client.publish(topic, jsonChar); //+--- Publica PAYLOAD ---+
          }
      #else
            //+---------------------------+                    
            //+--- Function Code READ: ---+                    
            //+---------------------------+ 
            //void MqttSend_ModbusFrame(bool b_MQTT_Status, value, fc, unitid, address, uint16_t quantity, uint16_t currFrameNumber)
            //+--------------------------------------------------------------------------------------------+                    
            //+--- Function Code READ: Coils (FC=01) ------------------------------------------------------+                    
            //+--------------------------------------------------------------------------------------------+ 
            if (fc == 1) 
            {
                bool auxval;
                lbmb.OledLine2("TT-FC:01(");
                auxval = lbmb.OledShowValueFc1(fc,address);            

                MqttSend_ModbusFrame(b_MqttStatus, auxval, fc, unitid, address, quantity, currFrameNumber);
            }    
            //

            //+--------------------------------------------------------------------------------------------+                    
            //+--- Function Code READ: Discrete Inputs (FC=02)  -------------------------------------------+                    
            //+--------------------------------------------------------------------------------------------+ 
            if (fc == 2) 
            {
                bool auxval;                
                lbmb.OledLine2("TT-FC:01(");
                auxval = lbmb.OledShowValueFc2(fc,address);            

                MqttSend_ModbusFrame(b_MqttStatus, auxval, fc, unitid, address, quantity, currFrameNumber);
            }               

            //+--------------------------------------------------------------------------------------------+                    
            //+--- Function Code READ: Multiple Holding Registers (FC=03) ---------------------------------+                    
            //+--------------------------------------------------------------------------------------------+ 
            if (fc == 3) 
            {
                uint16_t auxval;    

                lbmb.OledLine2("TT-FC:03(");
                auxval = lbmb.OledShowValueFc3(fc,address);            

                MqttSend_ModbusFrame(b_MqttStatus, auxval, fc, unitid, address, quantity, currFrameNumber);
            }               

            //+--------------------------------------------------------------------------------------------+                    
            //+--- Function Code READ: Input Registers (FC=04) --------------------------------------------+                    
            //+--------------------------------------------------------------------------------------------+ 
             if (fc == 4) 
            {
                uint16_t auxval;    

                lbmb.OledLine2("TT-FC:04(");
                auxval = lbmb.OledShowValueFc4(fc,address);            

                MqttSend_ModbusFrame(b_MqttStatus, auxval, fc, unitid, address, quantity, currFrameNumber);
            }            
 
            //+----------------------------+                    
            //+--- Function Code WRITE: ---+                    
            //+----------------------------+ 
            //+--------------------------------------------------------------------------------------------+                    
            //+--- Function Code WRITE: Coils (FC=05) -----------------------------------------------------+                    
            //+--------------------------------------------------------------------------------------------+
            if (fc == 5) 
            {
                uint16_t auxval;    

                switch (address){
                   case  co_LED_GREEN_OFFSET:
                       mb1.Coil(co_LED_GREEN_OFFSET, value);
                       break;
                   case  co_LED_RED_OFFSET:
                       mb1.Coil(co_LED_RED_OFFSET, value);
                       break;
                   default: 
                       Serial.println("Error: Fction 5 - Address is out of range!!!!");
                       break;
                }

                lbmb.OledLine2("TT-FC:05(");
                auxval = lbmb.OledShowValueFc5(fc,address);            

                MqttSend_ModbusFrame(b_MqttStatus, auxval, fc, unitid, address, quantity, currFrameNumber);
            } 
          
            //+--------------------------------------------------------------------------------------------+                    
            //+--- Function Code WRITE: Single Holding Register (FC=06) -----------------------------------+                    
            //+--------------------------------------------------------------------------------------------+ 
            if (fc == 6) 
            {
                uint16_t auxval;    

                switch (address){
                   case  hr_RGB_R_OFFSET:
                       mb1.Hreg(hr_RGB_R_OFFSET, value);
                       break;
                   case  hr_RGB_G_OFFSET:
                       mb1.Hreg(hr_RGB_G_OFFSET, value);
                       break;
                   case  hr_RGB_B_OFFSET:
                       mb1.Hreg(hr_RGB_B_OFFSET, value);
                       break;
                    default: 
                       Serial.println("Error: Fction 6 - Address is out of range!!!!");
                       break;
                }

                lbmb.OledLine2("TT-FC:06(");
                auxval = lbmb.OledShowValueFc6(fc,address);            

                MqttSend_ModbusFrame(b_MqttStatus, auxval, fc, unitid, address, quantity, currFrameNumber);
            } 
 

      #endif

        #ifdef Enable_Display_OLED
        if (mb.displayInfo_MqttModbus(fc, address, quantity)) 
            Serial.println("Error: displayInfo_MqttModbus - Modbus Function does not exist!!!"); 
        #endif
        
        //SerialOTA.println("Veio do client!"); //+-- DEBUG - Putty PORT 23

        #ifdef Enable_TXT_Analisys 
            TXT_Analisys(jsonChar, length); 
        #endif

    } // FIM - if(origin=="client")


      //
      //Node-Red Modbus Payload Type: payload[] = { value: x , 'fc': x, 'unitid': x, 'address': x , 'quantity': x };
      //payload[] = {value,fc,unitid,address,quantity}
      //+--- Início do PAYLOAD - Add values in the frame ---+
      /*
      frame["value"] = 0;
      frame["fc"] = 0;
      frame["unitid"] = 0;
      frame["address"] = 0;
      frame["quantity"] = 0;
      frame["from"] = "server";
      */
    //
    //+--- FIM do PAYLOAD ---+
    //

}

//+------------------------------------------------------------------------------+
void TXT_Analisys(uint8_t *charArr, unsigned int arrLength)
{

    for (uint cnt = 0; cnt <= arrLength; cnt++) 
    {
        charArr[cnt] = toupper(charArr[cnt]); //+--- converte para caixa alta ---+
    }

    //+--------------------------------+
    //+---------- ON/OFF --------------+
    //+--------------------------------+
    // Switch MOTOR ON -> on
    if (((char)charArr[0] == 'O')&&((char)charArr[1] == 'N')) 
    {
        //digitalWrite(def_pin_LED_GREEN, HIGH);   // Turn the ci_led ON
        mb1.Coil(0,1);  // Turn the MOTOR ON
        lbmb.OledLine2("TT->TXT(");
        lbmb.OledLine3("MOTOR ON ");
    } 
    // Switch  MOTOR OFF -> off
    if (((char)charArr[0] == 'O')&&((char)charArr[1] == 'F')&&((char)charArr[2] == 'F'))
    {
        //digitalWrite(def_pin_LED_GREEN, LOW);  // Turn the ci_led OFF
        mb1.Coil(0,0);  // Turn the MOTOR OFF
        lbmb.OledLine2("TT->TXT(");
        lbmb.OledLine3("MOTOR OFF ");
    }

    //+--------------------------------+
    //+-------- LIGAR/DESLIGAR ----------+
    //+--------------------------------+
    // Switch MOTOR ON -> LIGAR
    if (((char)charArr[0] == 'L')&&((char)charArr[1] == 'I')&&((char)charArr[2] == 'G')&&((char)charArr[3] == 'A')&&((char)charArr[4] == 'R')) 
    {
        //digitalWrite(def_pin_LED_GREEN, HIGH);   // Turn the ci_led ON
        mb1.Coil(0,1);  // Turn the MOTOR ON
        lbmb.OledLine2("TT->TXT(");
        lbmb.OledLine3("MOTOR ON ");
    } 
    // Switch  MOTOR OFF -> DESLIGAR
    if (((char)charArr[0] == 'D')&&((char)charArr[1] == 'E')&&((char)charArr[2] == 'S')&&((char)charArr[3] == 'L')&&((char)charArr[4] == 'I')&&((char)charArr[5] == 'G')&&((char)charArr[6] == 'A')&&((char)charArr[7] == 'R')) 
    {
        //digitalWrite(def_pin_LED_GREEN, LOW);  // Turn the ci_led OFF
        mb1.Coil(0,0);  // Turn the MOTOR OFF
        lbmb.OledLine2("TT->TXT(");
        lbmb.OledLine3("MOTOR OFF ");
    }

    //+---------------------------------------+
    //+-------- Temperatura/Umidade ----------+
    //+---------------------------------------+
    // GET Temperatura -> tmp, temperatura, tempture
    if ((((char)charArr[0] == 'T')&&((char)charArr[1] == 'M')&&((char)charArr[2] == 'P'))||
        (((char)charArr[0] == 'T')&&((char)charArr[1] == 'E')&&((char)charArr[2] == 'M')&&((char)charArr[3] == 'P'))||
        (((char)charArr[0] == 'T')&&((char)charArr[1] == 'E')&&((char)charArr[2] == 'M')&&((char)charArr[3] == 'P')&&((char)charArr[4] == 'E')&&((char)charArr[5] == 'R')&&((char)charArr[6] == 'A')&&((char)charArr[7] == 'T')&&((char)charArr[8] == 'U')&&((char)charArr[9] == 'R')&&((char)charArr[10] == 'A'))||
        (((char)charArr[0] == 'T')&&((char)charArr[1] == 'E')&&((char)charArr[2] == 'M')&&((char)charArr[3] == 'P')&&((char)charArr[4] == 'T')&&((char)charArr[5] == 'U')&&((char)charArr[6] == 'R')&&((char)charArr[7] == 'E'))) 
    {
        lbmb.OledLine2("TT->TXT(");
        //
        char str[] = "  TMP=";
        strcpy(ca_OLED_line3,str);
        char str1[8];
        itoa(mb1.Ireg(ir_TEMPTURE_OFFSET), str1, 10);
        strcat(ca_OLED_line3,str1); 
        strcat(ca_OLED_line3,"C");
        lbmb.OledLine3(ca_OLED_line3);
        MqttSend_SensorValue(b_MqttStatus, "TMP", mb1.Ireg(ir_TEMPTURE_OFFSET));
    } 

    // GET Umidade -> umidade, ur, umidity
    if ((((char)charArr[0] == 'U')&&((char)charArr[1] == 'M')&&((char)charArr[2] == 'I')&&((char)charArr[3] == 'D')&&((char)charArr[4] == 'A')&&((char)charArr[5] == 'D')&&((char)charArr[6] == 'E'))||
        (((char)charArr[0] == 'U')&&((char)charArr[1] == 'R'))||
        (((char)charArr[0] == 'H')&&((char)charArr[1] == 'U')&&((char)charArr[2] == 'M')&&((char)charArr[3] == 'I')&&((char)charArr[4] == 'D')&&((char)charArr[5] == 'I')&&((char)charArr[6] == 'T')&&((char)charArr[7] == 'Y'))) 
    {
        lbmb.OledLine2("TT->TXT(");
        //
        char str[] = "  UR=";
        strcpy(ca_OLED_line3,str);
        char str1[8];
        itoa(mb1.Ireg(ir_HUMIDITY_OFFSET), str1, 10);
        strcat(ca_OLED_line3,str1); 
        strcat(ca_OLED_line3,"%");
        lbmb.OledLine3(ca_OLED_line3);
        MqttSend_SensorValue(b_MqttStatus, "UR", mb1.Ireg(ir_HUMIDITY_OFFSET));
    }

    
    //+------------------------+
    // Switch LED RED ON -> gon
    if (((char)charArr[0] == 'r')&&((char)charArr[1] == 'o')&&((char)charArr[2] == 'n')) 
    {
        mb1.Coil(1,1);  // Turn the LED GREEN ON
        lbmb.OledLine2("TT->TXT(");
        lbmb.OledLine3("LED RED ON ");
    } 
    // Switch LED RED OFF -> goff
    if (((char)charArr[0] == 'r')&&((char)charArr[1] == 'o')&&((char)charArr[2] == 'f')&&((char)charArr[3] == 'f'))
    {
        mb1.Coil(1,0);  // Turn the LED GREEN OFF
        lbmb.OledLine2("TT->TXT(");
        lbmb.OledLine3("LED RED OFF ");
        //lbmb.OledLine3("             ");
    }




}

//+------------------------------------------------------------------------------+

void MqttSend_SensorValue(bool b_MQTT_Status, String sensor, uint16_t value)
{
    //const char *topic = ca_topic; 
    
    if(b_MQTT_Status)
    { 

        //+-----------------------------------------+
        //+--------- JSON MODBUS FRAME -------------+
        //+-----------------------------------------+
        
        //
        //Node-Red Modbus Payload Type: payload[] = { value: x , 'fc': x, 'unitid': x, 'address': x , 'quantity': x };
        //payload[] = {value,fc,unitid,address,quantity}
        //+--- Início do PAYLOAD - Add values in the frame ---+
        frame.clear();
        frame["sensor"] = sensor;
        frame["value"] = value;
    //
    //+--- FIM do PAYLOAD ---+
    //
  
    char jsonChar[100]="";
    serializeJson(frame, jsonChar);
    
    //
    client.publish(topic, jsonChar); //+--- Publica PAYLOAD ---+
    //client.loop();  //+--- Verifica se existe alguma publicação no  MQTT BROKER  e rotorna na função Callback(char *topic, byte * payload, unsigned int length); ---+

    /*
    for (int k = 0; k < sizeof(jsonChar); k++) 
    {
        SerialOTA.print(jsonChar[k]); //+--Putty PORT 23
    }
    */
    

    }

}

//+------------------------------------------------------------------------------+

void MqttSend_ModbusFrame(bool b_MQTT_Status, uint16_t value, uint8_t fc, uint8_t unitid, uint16_t  address, uint16_t quantity, uint16_t currFrameNumber)
{
    //const char *topic = ca_topic; 
    
    if(b_MQTT_Status)
    { 

        //+-----------------------------------------+
        //+--------- JSON MODBUS FRAME -------------+
        //+-----------------------------------------+
        
        //
        //Node-Red Modbus Payload Type: payload[] = { value: x , 'fc': x, 'unitid': x, 'address': x , 'quantity': x };
        //payload[] = {value,fc,unitid,address,quantity}
        //+--- Início do PAYLOAD - Add values in the frame ---+
        frame.clear();
        frame["value"] = value;
        frame["fc"] = fc;
        frame["unitid"] = unitid;
        frame["address"] = address;
        frame["quantity"] = quantity;
        //frame["disableMsgOutput"] = 0;        
        frame["origin"] = "server";
        frame["fn"] = currFrameNumber;
    //
    //+--- FIM do PAYLOAD ---+
    //
  
    char jsonChar[100]="";
    serializeJson(frame, jsonChar);
    
    //
    client.publish(topic, jsonChar); //+--- Publica PAYLOAD ---+
    //client.loop();  //+--- Verifica se existe alguma publicação no  MQTT BROKER  e rotorna na função Callback(char *topic, byte * payload, unsigned int length); ---+

    /*
    for (int k = 0; k < sizeof(jsonChar); k++) 
    {
        SerialOTA.print(jsonChar[k]); //+--Putty PORT 23
    }
    */
    

    }

}

//+------------------------------------------------------------------------------+
bool ConnectMQTT(bool reconnect) 
{
    byte by_nTries = 1;
    //client.setBufferSize(1024);
    client.setServer(s_mqtt_broker_IP.c_str(), mqtt_port);
    client.setCallback(Callback);

    do 
    {
        //String s_client_id = "LB0-";
        //s_client_id += String(WiFi.macAddress());

        /*************** MQTT *********************/
        //mb.OledLine1("              ");
        //mb.OledLine2("   MQTT       ");
        //mb.OledLine3(" CONNECT...   ");
        //mb.OledUpdate();
        //delay(2000);
        //*****************************************/

        if (client.connect(s_client_id.c_str(), mqtt_username, mqtt_cc_password)) 
        {
            Serial.println("Exito na conexão:");
            SerialOTA.println("Exito na conexão:");
            //
            Serial.printf("Cliente %s conectado ao broker\n", s_client_id.c_str());
            SerialOTA.printf("Cliente %s conectado ao broker\n", s_client_id.c_str());
            //
            Serial.printf("Estado do Cliente: %d\n",client.state());
            SerialOTA.printf("Estado do Cliente: %d\n",client.state());
            //
            if(reconnect == false)
            {
              lbmb.OledLine1("             ");
              lbmb.OledLine2("  MQTT OK    ");
              lbmb.OledLine3("             ");
              lbmb.OledUpdate();
              delay(3000);
            }
            else
            {
              //mb.OledLine1("             ");
              lbmb.OledLine2(" RECONNECT   ");
              lbmb.OledLine3("    MQTT     ");
              lbmb.OledUpdate();
              b_mqtt_reconnected_message=true;
              i_mqtt_reconnected_display_seconds=0;
            }
            
        } 
        else 
        {
            //
            lbmb.OledLine1("              ");
            lbmb.OledLine2("   MQTT       "); 
            //
            char line3[18]="(";
            char str1[8];
            itoa(by_nTries, str1, 10);
            strcat(line3,str1);
            //
            if(by_nTries==1)
            { 
                char str2[]=") FAIL"; 
                strcat(line3,str2); 
            } 
            else 
            { 
                char str2[]=") FAILS"; 
                strcat(line3,str2); 
            }
            
            //
            lbmb.OledLine3(line3);
            lbmb.OledUpdate();
            //
            Serial.println("MQTT - Falha ao conectar!");
            Serial.printf("Estado do Cliente: %d\n",client.state());
            //
            SerialOTA.println("MQTT - Falha ao conectar!");
            SerialOTA.printf("Estado do Cliente: %d\n",client.state());
            //
            Serial.print("Tentativas: ");
            Serial.println(String(by_nTries));
            //
            SerialOTA.print("Tentativas: ");
            SerialOTA.println(by_nTries);
            delay(2000);
        }

        //+--- Se o PUSH Button estiver pressionado ir para o MENU---+
        if(digitalRead(def_pin_PUSH_BUTTON) == 1)
        {
            b_GoToMenu=true;
            b_MenuHold=true;
            b_MenuFirstCall=true; 
            b_GoToLoop=true;
            break;
        }

        by_nTries++;
    } while (!client.connected() && by_nTries < 5);

    if(by_nTries>=5)
    {
      lbmb.OledLine1("             ");
      lbmb.OledLine2("   MQTT     ");
      lbmb.OledLine3(" FAILED!!!   ");
      lbmb.OledUpdate();
      delay(6000);
    }

    if (by_nTries < 5) 
    {
        // publish and subscribe
        // client.publish(topic, "{test,2023061571844}");
        
        client.subscribe(topic);

        /*
        client.subscribe(topic_ledGreen);
        client.subscribe(topic_ledRed);
        client.subscribe(topic_rgbR);
        client.subscribe(topic_rgbG);
        client.subscribe(topic_rgbB);
        client.subscribe(topic_buttonRetentive);
        client.subscribe(topic_buttonPush);
        client.subscribe(topic_potL);
        client.subscribe(topic_potR);
        */
        Serial.println("Teste de publicação e subscrição com êxito.");  
        SerialOTA.println("Teste de publicação e subscrição com êxito."); 

        return 1;
    } 
    else 
    {
        Serial.println("MQTT - Não conectado!!!");
        SerialOTA.println("MQTT - Não conectado!!!");
        return 0;
    }
}


//+------------------------------------------------------------------------------+

void show(const char * tag, int l) 
{
    Serial.print(tag); Serial.print("\t"); Serial.println(l);
    SerialOTA.print(tag); SerialOTA.print("\t"); SerialOTA.println(l);
}

void Show_SizeOf_DataTypes()
{

    Serial.begin(115200); delay(200); Serial.println();

    show("              bool",sizeof(bool));
    show("           boolean",sizeof(boolean));
    show("              byte",sizeof(byte));
    show("              char",sizeof(char));
    show("     unsigned char",sizeof(unsigned char));
    show("           uint8_t",sizeof(uint8_t));
    
    show("             short",sizeof(short));
    show("          uint16_t",sizeof(uint16_t));
    show("              word",sizeof(word));

    show("               int",sizeof(int));
    show("      unsigned int",sizeof(unsigned int));
    show("            size_t",sizeof(size_t));
    
    show("             float",sizeof(float));
    show("              long",sizeof(long));
    show("     unsigned long",sizeof(unsigned long));
    show("          uint32_t",sizeof(uint32_t));

    show("            double",sizeof(double));
    
    show("         long long",sizeof(long long));
    show("unsigned long long",sizeof(unsigned long long));
    show("          uint64_t",sizeof(uint64_t));

}



/*
//+--- O objetivo dessa rotina é enviar dados via MQTT com payload menor do que no formato JSON. 
void MqttSend_ModbusFrame(bool stat, const char *topic, uint16_t value, uint8_t fc, uint8_t id, uint16_t  address, uint16_t quantity)
{
    char payload[28] = "";
    char str[8];
    //Node-Red Modbus Payload Type: payload[] = { value: x , 'fc': x, 'unitid': x, 'address': x , 'quantity': x };
    //payload[] = {value,fc,unitid,address,quantity}
    //
    //+--- Início do PAYLOAD---+
    strcpy(payload,"(");
    //
    //+--- value ---+
    itoa(value, str, 10);
    strcat(payload,str);
    strcat(payload,",");
    //
    //+--- fc ---+
    itoa(fc, str, 10);
    strcat(payload,str);
    strcat(payload,",");
    //
    //+--- Id ---+
    itoa(id, str, 10);
    strcat(payload,str);
    strcat(payload,",");
    //
    //+--- address ---+
    itoa(address, str, 10);
    strcat(payload,str);
    strcat(payload,",");
    //
    //+--- quantity ---+
    itoa(quantity, str, 10);
    strcat(payload,str);
    //
    strcat(payload,")");
    //
    //+--- FIM do PAYLOAD ---+
    //
    
    //for (int k = 0; k < sizeof(payload); k++) 
    //{
    //    SerialOTA.print((char) payload[k]); //+--Putty PORT 23
    //}
    
    //
    client.publish(topic, payload); //+--- Publica PAYLOAD ---+
    client.loop();  //+--- Verifica se existe alguma publicação no  MQTT BROKER  e rotorna na função Callback(char *topic, byte * payload, unsigned int length); ---+
}
*/
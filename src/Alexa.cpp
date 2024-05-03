//+---------------------------------------------------------------------------------------------+
//|   Didatic KIT - LinkBox - Developed to to support learning the following subjects:          |
//    Industrial Informatics II, Home Automation and Industrial Networks I of                   |
//    Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)  |
//    Prof. Fábio Vincenzi and Prof. Renato Carrijo                                             |
//                                                                 Updated: 2023/10/28          |         
//+---------------------------------------------------------------------------------------------+

#include "global_vars.h"
#include <fauxmoESP.h>

#define MOTOR "KitMotor"
//fauxmoESP fauxmo;

void Alexa_Config() 
{

        // By default, fauxmoESP creates its own web server on the defined port
        // TCP port must be 80 for gen3 devices (default is 1901)
        // This must be done before the enable() call
        fauxmo.createServer(true); // não é necessário, este é o valor padrão
        fauxmo.setPort(80); // Isso é necessário para dispositivos gen3

        // You must call enable(true) as soon as you have a WiFi connection
        // You can enable or disable the library at any time
        // Disabling it will prevent devices from being discovered and switched
        fauxmo.enable(true);

        // It is possible to use different ways to call Alexa to modify the state of devices. Ex:
        // "Alexa, start ENGINE"

        // Add devices
        fauxmo.addDevice(MOTOR);


        fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) 
        {
              // Callback when an Alexa command is received.
              // Use device_id or device_name to choose the element on which to perform an action (relay, LED, ...)
              // state is a boolean (ON/OFF) and value is a number from 0 to 255 (if you say "set the kitchen light to 50%" you will get 128 here).

              //Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

              if ( (strcmp(device_name, MOTOR) == 0) ) 
              {
                    //Serial.println("ENGINE relay activated by Alexa");
                    
                    if (state) 
                    {
                        mb1.Coil(0,1);  // Turn the MOTOR ON
                    } 
                    else 
                    {
                        mb1.Coil(0,0);  // Turn the MOTOR OFF
                    }
              }
        });

}

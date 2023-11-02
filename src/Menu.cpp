//+---------------------------------------------------------------------------------------------+
//|   Didatic KIT - LinkBox - Developed to to support learning the following subjects:          |
//    Industrial Informatics II, Home Automation and Industrial Networks I of                   |
//    Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)  |
//    by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                          |
//                                                                 Updated: 2023/10/28          |         
//+---------------------------------------------------------------------------------------------+

#include "global_vars.h"


  //+--- PROTOCOL MENU OPTIONS ---+
  enum 
  {
      MODBUS_LinkBox = 0x00, // MODBUS PROTOCOL
      MODBUS_1C205 = 0x01, // MODBUS PROTOCOL
      MQTT   = 0x02, // MQTT PROTOCOL
      OPC_UA  = 0x03, // OPC UA PROTOCOL
      ALEXA  = 0X04, // ALEXA
  };

void Protocol_Menu()
{
  
    if(b_MenuFirstCall==true)
    { 

        by_stored_protocol_option = EEPROM.read(0); //selected protocol

        by_menu_option =  by_stored_protocol_option;
        
        if(by_stored_protocol_option==0){      mb.OledLine1(" =>MODBUS     "); mb.OledLine2("   SSID:      "); mb.OledLine3("  LinkBox     "); }
        else if(by_stored_protocol_option==1){ mb.OledLine1(" =>MODBUS     "); mb.OledLine2("   SSID:      "); mb.OledLine3(" INDUSTRIA    "); }
        else if(by_stored_protocol_option==2){ mb.OledLine1("  =>MQTT      "); mb.OledLine2("   SSID:      "); mb.OledLine3("  LinkBox     "); }
        else if(by_stored_protocol_option==3){ mb.OledLine1(" =>OPC UA     "); }
        else if(by_stored_protocol_option==4){ mb.OledLine1(" =>ALEXA      "); mb.OledLine2("   SSID:      "); mb.OledLine3(" Domotica     ");}
        mb.OledUpdate_Static(2,2,2);

        if(IsPushButtonReleased(5))
        { 
            b_MenuFirstCall=false; 
            //mb.OledLine2("Step 3 OK     "); // DEBUG
        }
    }
    
    if((IsPushButtonPressed(10)) && (b_MenuFirstCall==false))
    {  
        by_menu_option++;
        if(by_menu_option >= 5){ by_menu_option=0; }

        //mb.OledLine2("by_menu_option++  "); // DEBUG
    }

    if((ui_ms_button_released>400) && (b_MenuHold==true))
    {
        b_GoToMenu=false;
        b_MenuHold=false;
        b_GoToLoop=false;

        mb.OledLine1(s_oled_header);
        mb.OledLine2("             ");
        mb.OledLine3("             ");
        mb.OledUpdate();
        
        if(by_menu_option == OPC_UA)
        {
            by_menu_option=by_stored_protocol_option;
            mb.OledLine2("             ");
            mb.OledLine2("OPC UA NOT   ");
            mb.OledLine3("  FOUND!     ");
            mb.OledUpdate();
            delay(5000);
            mb.OledLine2("             ");
            mb.OledLine3("             ");
            mb.OledUpdate();
            return;
        }

        if(by_menu_option != by_stored_protocol_option)
        { 
            EEPROM.write(0, by_menu_option);
            EEPROM.commit();
            //
            ESP.restart(); 
        }

        if(WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            ESP.restart(); 
        }
    }

    
    switch(by_menu_option) 
    {

        case MODBUS_LinkBox:  
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              mb.OledLine1(" =>MODBUS     ");
              mb.OledLine2("   SSID:      ");
              mb.OledLine3("  LinkBox     ");
              mb.OledUpdate_Static(2,2,2);
        break;

        case MODBUS_1C205:  
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              mb.OledLine1(" =>MODBUS     ");
              mb.OledLine2("   SSID:      ");
              mb.OledLine3(" INDUSTRIA    ");
              mb.OledUpdate_Static(2,2,2);
        break;
      
        case MQTT:
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              mb.OledLine1("  =>MQTT     ");
              mb.OledLine2("   SSID:     ");
              mb.OledLine3("  LinkBox     ");
              mb.OledUpdate_Static(2,2,2);
        break;

        case OPC_UA:
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              mb.OledLine1("             ");
              mb.OledLine2(" =>OPC UA    ");
              mb.OledLine3("             ");
              mb.OledUpdate_Static(2,2,2);
        break;

        case ALEXA:
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              mb.OledLine1(" =>ALEXA      ");
              mb.OledLine2("   SSID:      ");
              mb.OledLine3(" Domotica     ");
              mb.OledUpdate_Static(2,2,2);
        break;

        default:    
               IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
               mb.OledLine1(" =>MODBUS     ");
               mb.OledLine2(" DEFALUT      ");
               mb.OledLine3("              ");
               mb.OledUpdate_Static(2,2,2);
        break;
    }
  
}


//+----------------------------------------------------------------------------------+
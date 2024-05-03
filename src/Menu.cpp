//+---------------------------------------------------------------------------------------------+
//|   Didatic KIT - LinkBox - Developed to to support learning the following subjects:          |
//    Industrial Informatics II, Home Automation and Industrial Networks I of                   |
//    Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)  |
//    by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                          |
//                                                                 Updated: 2023/10/28          |         
//+---------------------------------------------------------------------------------------------+

#include "global_vars.h"


//+----------------------------------------------------------------------------------+

void Protocol_Menu()
{
  
    if(b_MenuFirstCall==true)
    { 

        by_stored_protocol_option = EEPROM.read(0); //selected protocol

        by_menu_option =  by_stored_protocol_option;
        
        if(by_stored_protocol_option==0){      lbmb.OledLine1(" =>MODBUS     "); lbmb.OledLine2("   SSID:      "); lbmb.OledLine3("  LinkBox     "); }
        else if(by_stored_protocol_option==1){ lbmb.OledLine1(" =>MODBUS     "); lbmb.OledLine2("   SSID:      "); lbmb.OledLine3(" INDUSTRIA    "); }
        else if(by_stored_protocol_option==2){ lbmb.OledLine1("  =>MQTT      "); lbmb.OledLine2("   SSID:      "); lbmb.OledLine3("  LinkBox     "); }
        else if(by_stored_protocol_option==3){ lbmb.OledLine1(" =>OPC UA     "); }
        else if(by_stored_protocol_option==4){ lbmb.OledLine1(" =>ALEXA      "); lbmb.OledLine2("   SSID:      "); lbmb.OledLine3(" Domotica     ");}
        lbmb.OledUpdate_Static(2,2,2);

        if(IsPushButtonReleased(5))
        { 
            b_MenuFirstCall=false; 
            //lbmb.OledLine2("Step 3 OK     "); // DEBUG
        }
    }
    
    if((IsPushButtonPressed(10)) && (b_MenuFirstCall==false))
    {  
        by_menu_option++;
        if(by_menu_option >= 5){ by_menu_option=0; }

        //lbmb.OledLine2("by_menu_option++  "); // DEBUG
    }

    if((ui_ms_button_released>400) && (b_MenuHold==true))
    {
        b_GoToMenu=false;
        b_MenuHold=false;
        b_GoToLoop=false;

        lbmb.OledLine1(s_oled_header);
        lbmb.OledLine2("             ");
        lbmb.OledLine3("             ");
        lbmb.OledUpdate();
        
        if(by_menu_option == OPC_UA_PROTOCOL)
        {
            by_menu_option=by_stored_protocol_option;
            lbmb.OledLine2("             ");
            lbmb.OledLine2("OPC UA NOT   ");
            lbmb.OledLine3("  FOUND!     ");
            lbmb.OledUpdate();
            delay(5000);
            lbmb.OledLine2("             ");
            lbmb.OledLine3("             ");
            lbmb.OledUpdate();
            return;
        }

        if(by_menu_option != by_stored_protocol_option)
        { 
            EEPROM.write(0, by_menu_option);
            EEPROM.commit();
            //
            ESP.restart(); 
        }

        if(by_menu_option == by_stored_protocol_option) //Restore Current Menu
        { 
            lbmb.OledLine1(s_oled_header);
            lbmb.OledLine2("             ");
            lbmb.OledLine3("             ");
            lbmb.OledUpdate();
            by_menu_option=100;
        }

        if(WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            ESP.restart(); 
        }
    }

    
    switch(by_menu_option) 
    {

        case MODBUS_PROTOCOL_LinkBox:  
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              lbmb.OledLine1(" =>MODBUS     ");
              lbmb.OledLine2("   SSID:      ");
              lbmb.OledLine3("  LinkBox     ");
              lbmb.OledUpdate_Static(2,2,2);
        break;

        case MODBUS_PROTOCOL_1C205:  
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              lbmb.OledLine1(" =>MODBUS     ");
              lbmb.OledLine2("   SSID:      ");
              lbmb.OledLine3(" INDUSTRIA    ");
              lbmb.OledUpdate_Static(2,2,2);
        break;
      
        case MQTT_PROTOCOL:
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              lbmb.OledLine1("  =>MQTT     ");
              lbmb.OledLine2("   SSID:     ");
              lbmb.OledLine3("  LinkBox     ");
              lbmb.OledUpdate_Static(2,2,2);
        break;

        case OPC_UA_PROTOCOL:
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              lbmb.OledLine1("             ");
              lbmb.OledLine2(" =>OPC UA    ");
              lbmb.OledLine3("             ");
              lbmb.OledUpdate_Static(2,2,2);
        break;

        case ALEXA_PROTOCOL:
              IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
              lbmb.OledLine1(" =>ALEXA      ");
              lbmb.OledLine2("   SSID:      ");
              lbmb.OledLine3(" Domotica     ");
              lbmb.OledUpdate_Static(2,2,2);
        break;

        default:    
               IsPushButtonReleased(10); //Unlocks the action of pressing the push button (b_Pressed_Block=false) for a new choice
               //lbmb.OledLine1(" =>MODBUS     ");
               //lbmb.OledLine2(" DEFALUT      ");
               //lbmb.OledLine3("              ");
               //lbmb.OledUpdate_Static(2,2,2);
        break;
    }
  
}


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
          //lbmb.OledLine3(str1); // DEBUG
          //lbmb.OledLine2("Step 1 OK     "); // DEBUG
      }    
      
      if(digitalRead(def_pin_PUSH_BUTTON) == 0)
      {
          ui_ms_button_pressed=0; 
          ui_ms_button_released++; 
          //char str1[8]; // DEBUG
          //itoa(ui_ms_button_released, str1, 10); // DEBUG
          //lbmb.OledLine3(str1); // DEBUG
          //lbmb.OledLine2("            "); // DEBUG
      }        
      
      //+--- Checks if the push button is pressed for a while to access the menu
      if((IsPushButtonPressed(200))&&(b_MenuHold==false))
      {
        //lbmb.OledLine2("Step 2 OK     "); // DEBUG
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

//+----------------------------------------------------------------------------------+

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

//+----------------------------------------------------------------------------------+
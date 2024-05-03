//+--------------------------------------------------------------------------------------------+
//  LbBoard.cpp -  
//  This file has the main treatment for the LinkBox Board (Oled,Process Value)            
//  
//  by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                           |
//                                                                 Updated: 2024/05/02         |         
//+--------------------------------------------------------------------------------------------+
#include "global_vars.h"

#include <Arduino.h>
//#include "LbBoard.h"
#include <WiFi.h>

//#include <ModbusIP_ESP8266.h> //teste rff

//+--- OLED ---+
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#define MOTOR

char ca_line1[40]="";  //"IP:xxx.xxx.xxx.xxx"
char ca_line2[18]="";  //
char ca_line3[18]="";  //

int  x=0;
int  minX=0;
int  i_nModbusMessages=0;   // Number of Modbus messages

word coils_offset[10];
word digital_input_offset[10];
word input_regs_offset[10];
word holding_regs_offset[10];



bool b_disable_oled=false;  //Enable or disable the OLED scan using the Modbus Address 9000  

extern byte gucframe[10];  //Modbus Frame Header received for the Modbus Routine. Only to Animate the OLED display
extern uint32_t reqcount;    
uint32_t lastreqcount=0;     

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

LbBoard::LbBoard() {
    _regs_head = 0;
    _regs_last = 0;
}

LBTRegister* LbBoard::searchRegister(word address) {
    LBTRegister *reg = _regs_head;
    //if there is no register configured, bail
    if(reg == 0) return(0);
    //scan through the linked list until the end of the list or the register is found.
    //return the pointer.
    do {
        if (reg->address == address) return(reg);
        reg = reg->next;
	} while(reg);
	return(0);
}



//+--- dos #defines dos dispositivos Modbus do arquivo principal LinkBox.ino para essa biblioteca Modbus.h
//+--- OBS: Criar um arquivo denominado LinkBox.ino.globals.h não funcionou
void LbBoard::Set_CoilsOffset(int device, word offset)
{   // device = 0 Led Verde; = 1 Led Vermelho;  
    coils_offset[device]=offset;
}

void LbBoard::Set_DigitalInputOffset(int device, word offset)
{   // device = 0 Retentive Buttom Red; = 1 Push Buttom Black 
    digital_input_offset[device]=offset;
}

void LbBoard::Set_InputRegsOffset(int device, word offset)
{   // device = 0 Potenciômetro Esquerdo; = 1 Potenciômetro Direito;  
    input_regs_offset[device]=offset;
}

void LbBoard::Set_HoldingRegsOffset(int device, word offset)
{   // device = 0 Led RGB Red; = 1 Led RGB Green; = 2 Led RGB Blue;  
    holding_regs_offset[device]=offset;
}

//+---  LbBoard::ModbusInit()
//+---  Initiate the Modbus Points in the respective area modbus. 
//+---  Important!!!! The modbus area and scan is used ether to MQTT and Alexa    

void LbBoard::ModbusInit(){
   
    Set_CoilsOffset(0, co_LED_GREEN_OFFSET);
    Set_CoilsOffset(1, co_LED_RED_OFFSET);
    Set_CoilsOffset(2, co_DISABLE_OLED_OFFSET);   
    //
    Set_DigitalInputOffset(0, di_RETN_BUTTON_OFFSET);
    Set_DigitalInputOffset(1, di_PUSH_BUTTON_OFFSET);
    //
    #ifdef Enable_AHT10Enable_AHT10
        Set_InputRegsOffset(0, ir_TEMPTURE_OFFSET);
        Set_InputRegsOffset(1, ir_HUMIDITY_OFFSET);
    #else
        Set_InputRegsOffset(0, ir_POT_LEFT_OFFSET);
        Set_InputRegsOffset(1, ir_POT_RIGHT_OFFSET);
    #endif
    //
    Set_HoldingRegsOffset(0, hr_RGB_R_OFFSET);
    Set_HoldingRegsOffset(1, hr_RGB_G_OFFSET);
    Set_HoldingRegsOffset(2, hr_RGB_B_OFFSET);

    /*************** MODBUS  *********************/
    //+-- Add binary Modbus registers (bit=COIL) ---+
    //+--- Coils ---+
    mb1.addCoil(co_LED_GREEN_OFFSET);
    mb1.Coil(co_LED_GREEN_OFFSET, false); 
    mb1.addCoil(co_LED_RED_OFFSET);
    mb1.Coil(co_LED_RED_OFFSET, false);  
    mb1.addCoil(co_DISABLE_OLED_OFFSET);
    mb1.Coil(co_DISABLE_OLED_OFFSET, false);  

    //+--- Digital Inputs ---+
    mb1.addIsts(di_RETN_BUTTON_OFFSET);
    mb1.addIsts(di_PUSH_BUTTON_OFFSET);
  
    #ifdef Enable_AHT10
        //+-- Adds analog Modbus registers (2 bytes)
        //+--- Imput Registers ---+
        mb1.addIreg(ir_TEMPTURE_OFFSET);
        mb1.addIreg(ir_HUMIDITY_OFFSET);
    #else
        //+-- Adds analog Modbus registers (2 bytes)
        //+--- Imput Registers ---+
        mb1.addIreg(ir_POT_LEFT_OFFSET);
        mb1.addIreg(ir_POT_RIGHT_OFFSET);
    #endif

    //+--- Holding Registers ---+
    mb1.addHreg(hr_RGB_R_OFFSET);
    mb1.addHreg(hr_RGB_G_OFFSET);
    mb1.addHreg(hr_RGB_B_OFFSET);
   
    //initialize modbus connection
    mb1.server();
}


//+---  LbBoard::Task()
//+---  Scan the modbus commands (Application layer) and update the hardware values 
 
void LbBoard::Task()
{
    bool bAux;
    
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+---    Coil				      1-9999				   1 bit			     R/W     ---+
    //+-- Copies the Modbus register value (co_LED_GREEN_OFFSET) to the GPIO16=RX2 pin (LED_GREEN)
    #ifdef Enable_ArduinoRelay  
        digitalWrite(def_pin_LED_GREEN, !(mb1.Coil(co_LED_GREEN_OFFSET))); // Inverte a lógica de acionamento porque o contato NO do relé Arduino fecha quando o sinal na entrada é 0(Zero))
    #else
        digitalWrite(def_pin_LED_GREEN, (mb1.Coil(co_LED_GREEN_OFFSET))); 
    #endif

    //+-- Copies the value from the Modbus register (co_LED_RED_OFFSET) to the GPIO4=D4 pin (LED_RED)
    digitalWrite(def_pin_LED_RED, mb1.Coil(co_LED_RED_OFFSET));

    //+--- Enable/Disable Display using modbus (Fct=1; Addr=9000; Value=1 - disable display OLED, 0 - Enable display OLED )
    //digitalWrite(def_pin_LED_RED, mb1.Coil(co_DISABLE_OLED_OFFSET)); 
    bAux = mb1.Coil(co_DISABLE_OLED_OFFSET);
    if ((b_disable_oled == 0) && (bAux==1)) {
        //user has just change the oled display to off - clear the display 
        //if (by_stored_protocol_option == MODBUS_PROTOCOL_LinkBox){ 
           //strcpy(s_oled_hdrcmp,"MB:");
           //strcat(s_oled_hdrcmp, (WiFi.localIP().toString().c_str()));  
           //strcat(s_oled_hdrcmp, " (SSID:LinkBox)"); }
        //} 
        //OledLine1(s_oled_hdrcmp);
        OledLine2("             ");
        OledLine3("             ");
        OledUpdate();
    }
    b_disable_oled = bAux; 

    //When the OLED is enabled, show the function code received in the display  
    if (b_disable_oled == 0) {
       if (reqcount != lastreqcount){
          if (receivePDU(gucframe)){
            Serial.println("Error: Function does not exist!!!!");
          }
       } 
        lastreqcount = reqcount;  
    }

    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+-- Discrete Inputs     10001-19999		     1 bit		       R/W     ---+
    mb1.Ists(di_RETN_BUTTON_OFFSET, digitalRead(def_pin_RETN_BUTTON));
    mb1.Ists(di_PUSH_BUTTON_OFFSET, digitalRead(def_pin_PUSH_BUTTON));
    

    //+--- MODBUS Updates the potentiometer Input Registers ---+
    //+--- Register Type   	Register Number  	Register Size 	Permission            +
    //+--- Input Register	  30001-39999		   16 bit	        R                       +
    mb1.Ireg(ir_POT_LEFT_OFFSET,  analogRead(def_pin_POT_LEFT));  //+--- Copies the value from ADC1_3 (GPIO39) to the Modbus register ir_POT_LEFT_OFFSET
    mb1.Ireg(ir_POT_RIGHT_OFFSET, analogRead(def_pin_POT_RIGHT)); //+--- Copies the value from ADC1_6 (GPIO34) to the Modbus register ir_POT_RIGHT_OFFSET 

    //
    //
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+- Holding Register	   40001-49999			   16 bit          R     ---+
    //+-- Configures the PWM value to activate the red LED of the RGB LED 

    #ifdef Enable_COMMON_ANODE_RGB
        //+---
        //+-- LEDs are common anode, so when the cyclic ratio is 0 the LED emits maximum brightness
        //+---
        //+-- Configures the PWM value to activate the red LED of the RGB LED ---+
        ledcWrite(def_rgb_red_channel, (1023-((mb1.Hreg(hr_RGB_R_OFFSET)*1023)/100)));

        //+-- Configures the PWM value to activate the green LED of the RGB LED ---+
        ledcWrite(def_rgb_green_channel, (1023-((mb1.Hreg(hr_RGB_G_OFFSET)*1023)/100)));

        //+-- Configures the PWM value to activate the blue LED of the RGB LED ---+
        ledcWrite(def_rgb_blue_channel, (1023-((mb1.Hreg(hr_RGB_B_OFFSET)*1023)/100)));
    #else
        //+---
        //+--  //+-- LEDs are common cathode, so when the cyclic ratio is 100 the LED emits maximum brightness
        //+---
        //+--- Configures the PWM value to activate the red LED of the RGB LED ---+
        ledcWrite(def_rgb_red_channel, ((mb1.Hreg(hr_RGB_R_OFFSET)*1023)/100));

        //+--- Configures the PWM value to activate the green LED of the RGB LED ---+
        ledcWrite(def_rgb_green_channel, ((mb1.Hreg(hr_RGB_G_OFFSET)*1023)/100));

        //+--- Configures the PWM value to activate the blue LED of the RGB LED ---+
        ledcWrite(def_rgb_blue_channel, ((mb1.Hreg(hr_RGB_B_OFFSET)*1023)/100));

    #endif

}

//+---------------------------------------------------------------------------+
// DISPLAY - OLED - Treatment

void LbBoard::OledInitialize()
{    
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
      Serial.println(F("failed to start SSD1306 OLED"));
      while (1);
    }

    x    = oled.width(); //Usado no scroll do IP
    //minX = -12 * strlen(ca_line1); // 12 = 6 pixels/character * text size 2 - Usado no scroll do IP
}

//+--- Check status OLED - Enable or Disable
//+--- Return 1 is disabled. 
bool LbBoard::isOledDisable() {
    return b_disable_oled;
}

void LbBoard::OledLine1(const char txt[])
{
    //const char ip[] = "IP:";
    //strcpy(ca_line1,ip);
    //strcat(ca_line1,txt);
    strcpy(ca_line1,txt);
    //ca_line1[6]='.'; //ca_line1[10]='.'; ca_line1[14]='.';
    minX = -12 * strlen(ca_line1); // 12 = 6 pixels/character * text size 2 - Usado no scroll do IP
}


void LbBoard::OledLine2(const char txt[])
{
    if(i_nModbusMessages>=9)
    { 
        i_nModbusMessages=1; 
    }
    else
    {
        i_nModbusMessages++;
    }
    strcpy(ca_line2,txt);
    char str1[8];
    char str2[]=")";
    itoa(i_nModbusMessages, str1, 10);
    strcat(ca_line2,str1);
    strcat(ca_line2,str2);
}

void LbBoard::OledLine3(const char txt[])
{
    strcpy(ca_line3,txt);
}


//+-- LbBoard::OledShowValueFc1
//+-- Show the Hardware value in the display if the function is Read Coils (Fction = 01)
//+-- Return the same value for other functions 

bool LbBoard::OledShowValueFc1 (uint8_t fc, uint16_t address)
{
    bool value=0;

    if ((fc==1) && (address==(coils_offset[0])))
    { 
        value = mb1.Coil(coils_offset[0]);
        if(value == 1) { 
            char str[] = " LED GRN=1     ";
            strcpy(ca_line3,str);  
        } 
        else { 
            char str[] = " LED GRN=0     ";
            strcpy(ca_line3,str);  
        } 
    }
    else if ((fc==1) && (address == coils_offset[1]))
    { 
        value = mb1.Coil(coils_offset[1]);
        if(value == 1) { 
            char str[] = " LED RED=1     ";
            strcpy(ca_line3,str);  
        } 
        else { 
            char str[] = " LED RED=0     ";
            strcpy(ca_line3,str);  
        } 
    }    

    lbmb.OledUpdate();

    return value;
}

//+-- LbBoard::OledShowValueFc1
//+-- Show the Hardware value in the display if the function is Read Inputs (Fction = 02)
//+-- Return the same value for other functions 

bool LbBoard::OledShowValueFc2 (uint8_t fc, uint16_t address)
{
    bool value=0;

    if( (fc==2) && (address == digital_input_offset[0]))
    {
       value = mb1.Ists(digital_input_offset[0]);
       if(value==1) {           
            char str[] = "BT. RETN=1   ";
            strcpy(ca_line3,str); 
        } 
        else { 
            char str[] = "BT. RETN=0   ";
            strcpy(ca_line3,str); 
        } 
    }
    else if ((fc==2) && (address == digital_input_offset[1]))
    {
       value = mb1.Ists(digital_input_offset[1]); 
       if(value==1) {           
            char str[] = "BT. PUSH=1   ";
            strcpy(ca_line3,str); 
        } 
        else { 
            char str[] = "BT. PUSH=0   ";
            strcpy(ca_line3,str); 
        } 
    }
    lbmb.OledUpdate();

    return value;
}

//+-- LbBoard::OledShowValueFc3
//+-- Show the Hardware value in the display if the function is Holding Registers (Fction = 03)
//+-- Return the same value for other functions 

uint16_t LbBoard::OledShowValueFc3 (uint8_t fc, uint16_t address)
{
    uint16_t value=0;

    if((fc==3) && (address==(holding_regs_offset[0])))
    {
        value = mb1.Hreg(holding_regs_offset[0]); 

        char str[] = "RGB R=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(value, str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    else if((fc==3)&&(address==(holding_regs_offset[1])))
    {
        value = mb1.Hreg(holding_regs_offset[1]); 

        char str[] = "RGB G=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(value, str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    else if((fc==3)&&(address==(holding_regs_offset[2])))
    {
        value = mb1.Hreg(holding_regs_offset[2]); 

        char str[] = "RGB B=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(value, str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    lbmb.OledUpdate();

    return value;
}

//+-- LbBoard::OledShowValueFc4
//+-- Show the Hardware value in the display if the function is Write Coils (Fction = 04)
//+-- Return the same value for other functions 

uint16_t LbBoard::OledShowValueFc4 (uint8_t fc, uint16_t address)
{
    uint16_t value=0;

    if((fc==4)&&(address==(input_regs_offset[0])))
    {
        value = mb1.Ireg(input_regs_offset[0]); 

        #ifdef Enable_OLED_MOTOR_messages
            char str[] = "  TMP=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(value, str1, 10);
            strcat(ca_line3,str1); 
            strcat(ca_line3,"C");
        #else
            char str[] = "PO. L=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(value, str1, 10);
            strcat(ca_line3,str1); 
        #endif
    }
    else if((fc==4)&&(address==(input_regs_offset[1])))
    {
        value = mb1.Ireg(input_regs_offset[1]); 

        #ifdef Enable_OLED_MOTOR_messages
            char str[] = "  UR=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(value, str1, 10);
            strcat(ca_line3,str1); 
            strcat(ca_line3,"%");
        #else
            char str[] = "PO. R=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(value, str1, 10);
            strcat(ca_line3,str1); 
        #endif
    }
    lbmb.OledUpdate();

    return value;
}

//+-- LbBoard::OledShowValueFc5
//+-- Show the Hardware value in the display if the function is Write Coils (Fction = 05)
//+-- Return the same value for other functions 
bool LbBoard::OledShowValueFc5 (uint8_t fc, uint16_t address)
{
    bool value=0;

    if ((fc==1) && (address==(coils_offset[0])))
    { 
        value = mb1.Coil(coils_offset[0]);
        if(value == 1) { 
            char str[] = " LED GRN=1     ";
            strcpy(ca_line3,str);  
        } 
        else { 
            char str[] = " LED GRN=0     ";
            strcpy(ca_line3,str);  
        } 
    }
    else if ((fc==1) && (address == coils_offset[1]))
    { 
        value = mb1.Coil(coils_offset[1]);
        if(value == 1) { 
            char str[] = " LED RED=1     ";
            strcpy(ca_line3,str);  
        } 
        else { 
            char str[] = " LED RED=0     ";
            strcpy(ca_line3,str);  
        } 
    }    

    lbmb.OledUpdate();

    return value;
}

//+-- LbBoard::OledShowValueFc6
//+-- Show the Hardware value in the display if the function is Write Coils (Fction = 06)
//+-- Return the same value for other functions 

uint16_t LbBoard::OledShowValueFc6 (uint8_t fc, uint16_t address)
{
    uint16_t value=0;

   if((fc==6) && (address==(holding_regs_offset[0])))
    {
        value = mb1.Hreg(holding_regs_offset[0]); 

        char str[] = "RGB R=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(value, str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    else if((fc==6)&&(address==(holding_regs_offset[1])))
    {
        value = mb1.Hreg(holding_regs_offset[1]); 

        char str[] = "RGB G=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(value, str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    else if((fc==6)&&(address==(holding_regs_offset[2])))
    {
        value = mb1.Hreg(holding_regs_offset[2]); 

        char str[] = "RGB B=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(value, str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    lbmb.OledUpdate();

    return value;
}


void LbBoard::OledUpdate_Static(int txtSizeLine1, int txtSizeLine2, int txtSizeLine3) //Mostra a primeira linha (AMARELA) estática e de tamanho menor, igual a 1
{
    //+--- Scroll IP ---+
    oled.clearDisplay();
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE); // Draw white text
    
    oled.setTextSize(txtSizeLine1);      // Normal 1:1 pixel scale

    //+--- Print OLED Line1 ---+
    oled.setCursor(0, 0); 
    oled.println(ca_line1);

    oled.setTextSize(txtSizeLine2);      // Normal 1:1 pixel scale

    //+--- Print OLED Line2 ---+
    oled.setCursor(0, 20);  
    oled.println(ca_line2);

    oled.setTextSize(txtSizeLine3);      // Normal 1:1 pixel scale

    //+--- Print OLED Line2 ---+
    oled.setCursor(0, 40); 
    oled.println(ca_line3);
    oled.display();
}


void LbBoard::OledUpdate() //Primeira linha (AMARELA) do display com scroll
{
    //+--- Scroll IP ---+
    oled.clearDisplay();
    oled.setTextWrap(false);
    oled.setTextSize(2);      // Normal 1:1 pixel scale
    oled.setTextColor(SSD1306_WHITE); // Draw white text
    oled.setCursor(x, 0);
    oled.cp437(true);  // Use full 256 char 'Code Page 437' font
    oled.print(ca_line1);
    //oled.display();
    if(--x < minX){ x = oled.width(); }
    
    //+--- Print OLED Line2 ---+
    oled.setCursor(0, 20);   
    oled.println(ca_line2);

    //+--- Print OLED Line2 ---+
    oled.setCursor(0, 40);   
    oled.println(ca_line3);
    oled.display();
}

bool LbBoard::receivePDU(byte* frame) {
    bool ret=0; 
 
    byte fcode  = frame[0];
    word field1 = (word)frame[1] << 8 | (word)frame[2];
    word field2 = (word)frame[3] << 8 | (word)frame[4];

    //+-- Function Codes
    //+-- MB_FC_READ_COILS       = 0x01, // Read Coils (Output) Status 0xxxx
    //+-- MB_FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs) 1xxxx
    //+-- MB_FC_READ_REGS        = 0x03, // Read Holding Registers 4xxxx
    //+-- MB_FC_READ_INPUT_REGS  = 0x04, // Read Input Registers 3xxxx
    //+-- MB_FC_WRITE_COIL       = 0x05, // Write Single Coil (Output) 0xxxx
    //+-- MB_FC_WRITE_REG        = 0x06, // Preset Single Register 4xxxx
    //+-- MB_FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs) 0xxxx
    //+-- MB_FC_WRITE_REGS       = 0x10, // Write block of contiguous registers 4xxxx

    switch (fcode) {

        case MB_FC_WRITE_REG:  
            OledLine2("MD-FC:06(");
            lbmb.OledShowValueFc6(fcode,field1);    
        break;
        case MB_FC_READ_REGS:
            OledLine2("MD-FC:03(");
            lbmb.OledShowValueFc3(fcode,field1);    
        break;
        case MB_FC_WRITE_REGS:
            OledLine2("MD-FC:16(");
            lbmb.OledShowValueFc6(fcode,field1);    
        break;

        #ifndef USE_HOLDING_REGISTERS_ONLY
            case MB_FC_READ_COILS:
                OledLine2("MD-FC:01(");
                lbmb.OledShowValueFc1(fcode,field1);    
            break;
            case MB_FC_READ_INPUT_STAT:
                OledLine2("MD-FC:02(");
                lbmb.OledShowValueFc2(fcode,field1);    
            break;
            case MB_FC_READ_INPUT_REGS:
                OledLine2("MD-FC:04(");
                lbmb.OledShowValueFc4(fcode,field1);    
            break;
            case MB_FC_WRITE_COIL:
                OledLine2("MD-FC:05(");  
                lbmb.OledShowValueFc5(fcode,field1);    
            break;
            case MB_FC_WRITE_COILS:
                OledLine2("MD-FC:15("); 
                lbmb.OledShowValueFc5(fcode,field1);    
            break;
        #endif
        default:
            ret = 1;
    }
    return ret;
}


bool LbBoard::displayInfo_MqttModbus(uint8_t fcode, uint16_t field1, uint16_t field2) {
    bool ret=0;

    //+-- Function Codes
    //+-- MB_FC_READ_COILS       = 0x01, // Read Coils (Output) Status 0xxxx
    //+-- MB_FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs) 1xxxx
    //+-- MB_FC_READ_REGS        = 0x03, // Read Holding Registers 4xxxx
    //+-- MB_FC_READ_INPUT_REGS  = 0x04, // Read Input Registers 3xxxx
    //+-- MB_FC_WRITE_COIL       = 0x05, // Write Single Coil (Output) 0xxxx
    //+-- MB_FC_WRITE_REG        = 0x06, // Preset Single Register 4xxxx
    //+-- MB_FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs) 0xxxx
    //+-- MB_FC_WRITE_REGS       = 0x10, // Write block of contiguous registers 4xxxx

    switch (fcode) {

        case MB_FC_WRITE_REG:  
            OledLine2("TT-FC:06(");
            lbmb.OledShowValueFc6(fcode,field1);   
        break;

        case MB_FC_READ_REGS:
            OledLine2("TT-FC:03(");
            lbmb.OledShowValueFc3(fcode,field1);   
        break;

        //case MB_FC_WRITE_REGS:
        //field1 = startreg, field2 = status
        //    OledLine2("TT-FC:16(");
        //    lbmb.OledShowValueFc6(fcode,field1);               
        //break;

        #ifndef USE_HOLDING_REGISTERS_ONLY
        case MB_FC_READ_COILS:
            OledLine2("TT-FC:01(");
            lbmb.OledShowValueFc1(fcode,field1);               
        break;

        case MB_FC_READ_INPUT_STAT:
            OledLine2("TT-FC:02(");
            lbmb.OledShowValueFc2(fcode,field1);               
        break;

        case MB_FC_READ_INPUT_REGS:
            OledLine2("TT-FC:04(");
            lbmb.OledShowValueFc4(fcode,field1);               
        break;

        case MB_FC_WRITE_COIL:
            OledLine2("TT-FC:05(");  
            lbmb.OledShowValueFc5(fcode,field1);               
        break;

        //case MB_FC_WRITE_COILS:
            //field1 = startreg, field2 = numoutputs
        //   OledLine2("TT-FC:15("); 
        //   lbmb.OledShowValueFc5(fcode,field1);               
        //break;

        #endif
        default:
            ret = 1;
    }
    return ret;
}


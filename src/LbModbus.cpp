//+--------------------------------------------------------------------------------------------+
//  Modbus.cpp - Source for Modbus Base Library                                                |
//	Copyright (C) 2014 André Sarmento Barbosa                                                  |
//	-----                                                                                      |
//  Library modified to support learning the following subjects:                                                 |
//  Industrial Informatic II, Home Automation and Industrial Networks I of                     |
//  Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)   |
//  by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                           |
//                                                                 Updated: 2023/11/02         |         
//+--------------------------------------------------------------------------------------------+

#include <Arduino.h>
#include "LbModbus.h"
#include <WiFi.h>

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


Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

LbModbus::LbModbus() {
    _regs_head = 0;
    _regs_last = 0;
}

TRegister* LbModbus::searchRegister(word address) {
    TRegister *reg = _regs_head;
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
void LbModbus::Set_CoilsOffset(int device, word offset)
{   // device = 0 Led Verde; = 1 Led Vermelho;  
    coils_offset[device]=offset;
}

void LbModbus::Set_DigitalInputOffset(int device, word offset)
{   // device = 0 Retentive Buttom Red; = 1 Push Buttom Black 
    digital_input_offset[device]=offset;
}

void LbModbus::Set_InputRegsOffset(int device, word offset)
{   // device = 0 Potenciômetro Esquerdo; = 1 Potenciômetro Direito;  
    input_regs_offset[device]=offset;
}

void LbModbus::Set_HoldingRegsOffset(int device, word offset)
{   // device = 0 Led RGB Red; = 1 Led RGB Green; = 2 Led RGB Blue;  
    holding_regs_offset[device]=offset;
}


//+---------------------------------------------------------------------------+

void LbModbus::OledInitialize()
{    
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
      Serial.println(F("failed to start SSD1306 OLED"));
      while (1);
    }

    x    = oled.width(); //Usado no scroll do IP
    //minX = -12 * strlen(ca_line1); // 12 = 6 pixels/character * text size 2 - Usado no scroll do IP
}

void LbModbus::OledLine1(const char txt[])
{
    //const char ip[] = "IP:";
    //strcpy(ca_line1,ip);
    //strcat(ca_line1,txt);
    strcpy(ca_line1,txt);
    //ca_line1[6]='.'; //ca_line1[10]='.'; ca_line1[14]='.';
    minX = -12 * strlen(ca_line1); // 12 = 6 pixels/character * text size 2 - Usado no scroll do IP
}


void LbModbus::OledLine2(const char txt[])
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

void LbModbus::OledLine3(const char txt[])
{
    strcpy(ca_line3,txt);
}

void LbModbus::ModbusOledLine3(byte fc, word reg, word numregs )
{
            
    /*   
            char str[8];  //+-- Para Debug ---+
            itoa(reg, str, 10); //+-- Para Debug ---+
            strcpy(ca_line3,str); //+-- Para Debug ---+
    */  
    
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+---    Coil				      1-9999				   1 bit			     R/W     ---+
    if(((fc==1)||(fc==5))&&(reg==(coils_offset[0])))
    { 
        #ifdef MOTOR
            if(Coil(coils_offset[0])==1)
            { 
                char str[] = " MOTOR ON   ";
                strcpy(ca_line3,str); 
            } 
            else 
            { 
                char str[] = " MOTOR OFF    ";
                strcpy(ca_line3,str); 
            } 
        #else
            if(Coil(coils_offset[0])==1)
            { 
                char str[] = " LED GRN=1     ";
                strcpy(ca_line3,str);  
            } 
            else 
            { 
                char str[] = " LED GRN=0     ";
                strcpy(ca_line3,str);  
            } 
        #endif


    }
    //+---
    if(((fc==1)||(fc==5))&&(reg==(coils_offset[1])))
    { 
        if(Coil(coils_offset[1])==1)
        { 
            char str[] = " LED RED=1     ";
            strcpy(ca_line3,str);  
        } 
        else 
        { 
            char str[] = " LED RED=0     ";
            strcpy(ca_line3,str);  
        } 
    }
    
    //
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+-- Discrete Inputs     10001-19999		   1 bit		   R/W     ---+
    if((fc==2)&&(reg==(digital_input_offset[0])))
    {
       if(Ists(digital_input_offset[0])==1)
        {           
            char str[] = "BT. RETN=1   ";
            strcpy(ca_line3,str); 
        } 
        else
        { 
            char str[] = "BT. RETN=0   ";
            strcpy(ca_line3,str); 
        } 
    }
    //
    if((fc==2)&&(reg==(digital_input_offset[1])))
    {
       if(Ists(digital_input_offset[1])==1)
        {           
            char str[] = "BT. PUSH=1   ";
            strcpy(ca_line3,str); 
        } 
        else
        { 
            char str[] = "BT. PUSH=0   ";
            strcpy(ca_line3,str); 
        } 
    }
    
    //
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+--- Input Register	  30001-39999		   16 bit	        R     ---+
    if((fc==4)&&(reg==(input_regs_offset[0])))
    {
        #ifdef MOTOR
            char str[] = "  TMP=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(Ireg(input_regs_offset[0]), str1, 10);
            strcat(ca_line3,str1); 
            strcat(ca_line3,"C");
        #else
            char str[] = "PO. L=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(Ireg(input_regs_offset[0]), str1, 10);
            strcat(ca_line3,str1); 
        #endif
    }
    //
    if((fc==4)&&(reg==(input_regs_offset[1])))
    {
        #ifdef MOTOR
            char str[] = "  UR=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(Ireg(input_regs_offset[1]), str1, 10);
            strcat(ca_line3,str1); 
            strcat(ca_line3,"%");
        #else
            char str[] = "PO. R=";
            strcpy(ca_line3,str);
            char str1[8];
            itoa(Ireg(input_regs_offset[1]), str1, 10);
            strcat(ca_line3,str1); 
        #endif
    }
    
    //
    //+--- Register Type   	Register Number  	Register Size 	Permission
    //+- Holding Register	  40001-49999			16 bit          R     ---+
    
    if((fc==3)&&(reg==(holding_regs_offset[0])))
    {
        char str[] = "RGB R=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(Hreg(holding_regs_offset[0]), str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }

    if((fc==6)&&(reg==(holding_regs_offset[0])))
    {
        char str[] = "RGB R=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(Hreg(holding_regs_offset[0]), str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }
    
    //
    
    if((fc==3)&&(reg==(holding_regs_offset[1])))
    {
        char str[] = "RGB G=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(Hreg(holding_regs_offset[1]), str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }

    if((fc==6)&&(reg==(holding_regs_offset[1])))
    {
        char str[] = "RGB G=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(Hreg(holding_regs_offset[1]), str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }

    //

    if((fc==3)&&(reg==(holding_regs_offset[2])))
    {
        char str[] = "RGB B=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(Hreg(holding_regs_offset[2]), str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }

    if((fc==6)&&(reg==(holding_regs_offset[2])))
    {
        char str[] = "RGB B=";
        strcpy(ca_line3,str);
        char str1[8];
        itoa(Hreg(holding_regs_offset[2]), str1, 10);
        strcat(ca_line3,str1); 
        strcat(ca_line3,"%");
    }

    //
}


void LbModbus::OledUpdate_Static(int txtSizeLine1, int txtSizeLine2, int txtSizeLine3) //Mostra a primeira linha (AMARELA) estática e de tamanho menor, igual a 1
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


void LbModbus::OledUpdate() //Primeira linha (AMARELA) do display com scroll
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


void LbModbus::addReg(word address, word value) {
    TRegister *newreg;

	newreg = (TRegister *) malloc(sizeof(TRegister));
	newreg->address = address;
	newreg->value		= value;
	newreg->next		= 0;

	if(_regs_head == 0) {
        _regs_head = newreg;
        _regs_last = _regs_head;
    } else {
        //Assign the last register's next pointer to newreg.
        _regs_last->next = newreg;
        //then make temp the last register in the list.
        _regs_last = newreg;
    }
}

bool LbModbus::Reg(word address, word value) {
    TRegister *reg;
    //search for the register address
    reg = this->searchRegister(address);
    //if found then assign the register value to the new value.
    if (reg) {
        reg->value = value;
        return true;
    } else
        return false;
}

word LbModbus::Reg(word address) {
    TRegister *reg;
    reg = this->searchRegister(address);
    if(reg)
        return(reg->value);
    else
        return(0);
}

void LbModbus::addHreg(word offset, word value) {
    this->addReg(offset + 40001, value);
}

bool LbModbus::Hreg(word offset, word value) {
    return Reg(offset + 40001, value);
}

word LbModbus::Hreg(word offset) {
    return Reg(offset + 40001);
}

#ifndef USE_HOLDING_REGISTERS_ONLY
    void LbModbus::addCoil(word offset, bool value) {
        this->addReg(offset + 1, value?0xFF00:0x0000);
    }

    void LbModbus::addIsts(word offset, bool value) {
        this->addReg(offset + 10001, value?0xFF00:0x0000);
    }

    void LbModbus::addIreg(word offset, word value) {
        this->addReg(offset + 30001, value);
    }

    bool LbModbus::Coil(word offset, bool value) {
        return Reg(offset + 1, value?0xFF00:0x0000);
    }

    bool LbModbus::Ists(word offset, bool value) {
        return Reg(offset + 10001, value?0xFF00:0x0000);
    }

    bool LbModbus::Ireg(word offset, word value) {
        return Reg(offset + 30001, value);
    }

    bool LbModbus::Coil(word offset) {
        if (Reg(offset + 1) == 0xFF00) {
            return true;
        } else return false;
    }

    bool LbModbus::Ists(word offset) {
        if (Reg(offset + 10001) == 0xFF00) {
            return true;
        } else return false;
    }

    word LbModbus::Ireg(word offset) {
        return Reg(offset + 30001);
    }
#endif


void LbModbus::receivePDU(byte* frame) {
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
            //field1 = reg, field2 = value
            OledLine2("MD-FC:06(");
            this->writeSingleRegister(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_READ_REGS:
            //field1 = startreg, field2 = numregs
            OledLine2("MD-FC:03(");
            this->readRegisters(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_WRITE_REGS:
            //field1 = startreg, field2 = status
            OledLine2("MD-FC:16(");
            this->writeMultipleRegisters(frame,field1, field2, frame[5]);
            ModbusOledLine3(fcode,field1,field2);
        break;

        #ifndef USE_HOLDING_REGISTERS_ONLY
        case MB_FC_READ_COILS:
            //field1 = startreg, field2 = numregs
            OledLine2("MD-FC:01(");
            this->readCoils(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs
            OledLine2("MD-FC:02(");
            this->readInputStatus(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = numregs
            OledLine2("MD-FC:04(");
            this->readInputRegisters(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_WRITE_COIL:
            //field1 = reg, field2 = status
            OledLine2("MD-FC:05(");  
            this->writeSingleCoil(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_WRITE_COILS:
            //field1 = startreg, field2 = numoutputs
            OledLine2("MD-FC:15("); 
            this->writeMultipleCoils(frame,field1, field2, frame[5]);
            ModbusOledLine3(fcode,field1,field2);
        break;

        #endif
        default:
            this->exceptionResponse(fcode, MB_EX_ILLEGAL_FUNCTION);
    }
}


void LbModbus::displayInfo_MqttModbus(uint8_t fcode, uint16_t field1, uint16_t field2) {


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
            //field1 = reg, field2 = value
            OledLine2("TT-FC:06(");
            //this->writeSingleRegister(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_READ_REGS:
            //field1 = startreg, field2 = numregs
            OledLine2("TT-FC:03(");
            //this->readRegisters(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        //case MB_FC_WRITE_REGS:
            //field1 = startreg, field2 = status
        //    OledLine2("TT-FC:16(");
        //    this->writeMultipleRegisters(frame,field1, field2, frame[5]);
        //    ModbusOledLine3(fcode,field1,field2);
        //break;

        #ifndef USE_HOLDING_REGISTERS_ONLY
        case MB_FC_READ_COILS:
            //field1 = startreg, field2 = numregs
            OledLine2("TT-FC:01(");
            //this->readCoils(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs
            OledLine2("TT-FC:02(");
            //this->readInputStatus(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = numregs
            OledLine2("TT-FC:04(");
            //this->readInputRegisters(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        case MB_FC_WRITE_COIL:
            //field1 = reg, field2 = status
            OledLine2("TT-FC:05(");  
            //this->writeSingleCoil(field1, field2);
            ModbusOledLine3(fcode,field1,field2);
        break;

        //case MB_FC_WRITE_COILS:
            //field1 = startreg, field2 = numoutputs
        //    OledLine2("TT-FC:15("); 
        //    this->writeMultipleCoils(frame,field1, field2, frame[5]);
        //    ModbusOledLine3(fcode,field1,field2);
        //break;

        #endif
        default:
            this->exceptionResponse(fcode, MB_EX_ILLEGAL_FUNCTION);
    }
}


void LbModbus::exceptionResponse(byte fcode, byte excode) {
    //Clean frame buffer
    free(_frame);
    _len = 2;
    _frame = (byte *) malloc(_len);
    _frame[0] = fcode + 0x80;
    _frame[1] = excode;

    _reply = MB_REPLY_NORMAL;
}

void LbModbus::readRegisters(word startreg, word numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x007D) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //*** See comments on readCoils method.
    if (!this->searchRegister(startreg + 40001)) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }


    //Clean frame buffer
    free(_frame);
	_len = 0;

	//calculate the query reply message length
	//for each register queried add 2 bytes
	_len = 2 + numregs * 2;

    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_REGS;
    _frame[1] = _len - 2;   //byte count

    word val;
    word i = 0;
	while(numregs--) {
        //retrieve the value from the register bank for the current register
        val = this->Hreg(startreg + i);
        //write the high byte of the register value
        _frame[2 + i * 2]  = val >> 8;
        //write the low byte of the register value
        _frame[3 + i * 2] = val & 0xFF;
        i++;
	}

    _reply = MB_REPLY_NORMAL;
}

void LbModbus::writeSingleRegister(word reg, word value) {
    //No necessary verify illegal value (EX_ILLEGAL_VALUE) - because using word (0x0000 - 0x0FFFF)
    //Check Address and execute (reg exists?)
    if (!this->Hreg(reg, value)) {
        this->exceptionResponse(MB_FC_WRITE_REG, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Check for failure
    if (this->Hreg(reg) != value) {
        this->exceptionResponse(MB_FC_WRITE_REG, MB_EX_SLAVE_FAILURE);
        return;
    }

    _reply = MB_REPLY_ECHO;
}

void LbModbus::writeMultipleRegisters(byte* frame,word startreg, word numoutputs, byte bytecount) {
    //Check value
    if (numoutputs < 0x0001 || numoutputs > 0x007B || bytecount != 2 * numoutputs) {
        this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address (startreg...startreg + numregs)
    for (int k = 0; k < numoutputs; k++) {
        if (!this->searchRegister(startreg + 40001 + k)) {
            this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_ADDRESS);
            return;
        }
    }

    //Clean frame buffer
    free(_frame);
	_len = 5;
    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_WRITE_REGS;
    _frame[1] = startreg >> 8;
    _frame[2] = startreg & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;

    word val;
    word i = 0;
	while(numoutputs--) {
        val = (word)frame[6+i*2] << 8 | (word)frame[7+i*2];
        this->Hreg(startreg + i, val);
        i++;
	}

    _reply = MB_REPLY_NORMAL;
}

#ifndef USE_HOLDING_REGISTERS_ONLY
void LbModbus::readCoils(word startreg, word numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x07D0) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //Check only startreg. Is this correct?
    //When I check all registers in range I got errors in ScadaBR
    //I think that ScadaBR request more than one in the single request
    //when you have more then one datapoint configured from same type.
    if (!this->searchRegister(startreg + 1)) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs%8) _len++; //Add 1 to the message length for the partial byte.

    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_COILS;
    _frame[1] = _len - 2; //byte count (_len - function code and byte count)

    byte bitn = 0;
    word totregs = numregs;
    word i;
	while (numregs--) {
        i = (totregs - numregs) / 8;
		if (this->Coil(startreg))
			bitSet(_frame[2+i], bitn);
		else
			bitClear(_frame[2+i], bitn);
		//increment the bit index
		bitn++;
		if (bitn == 8) bitn = 0;
		//increment the register
		startreg++;
	}

    _reply = MB_REPLY_NORMAL;
}

void LbModbus::readInputStatus(word startreg, word numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x07D0) {
        this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //*** See comments on readCoils method.
    if (!this->searchRegister(startreg + 10001)) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs%8) _len++; //Add 1 to the message length for the partial byte.

    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_INPUT_STAT;
    _frame[1] = _len - 2;

    byte bitn = 0;
    word totregs = numregs;
    word i;
	while (numregs--) {
        i = (totregs - numregs) / 8;
		if (this->Ists(startreg))
			bitSet(_frame[2+i], bitn);
		else
			bitClear(_frame[2+i], bitn);
		//increment the bit index
		bitn++;
		if (bitn == 8) bitn = 0;
		//increment the register
		startreg++;
	}

    _reply = MB_REPLY_NORMAL;
}

void LbModbus::readInputRegisters(word startreg, word numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x007D) {
        this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //*** See comments on readCoils method.
    if (!this->searchRegister(startreg + 30001)) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

	//calculate the query reply message length
	//for each register queried add 2 bytes
	_len = 2 + numregs * 2;

    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_INPUT_REGS;
    _frame[1] = _len - 2;

    word val;
    word i = 0;
	while(numregs--) {
        //retrieve the value from the register bank for the current register
        val = this->Ireg(startreg + i);
        //write the high byte of the register value
        _frame[2 + i * 2]  = val >> 8;
        //write the low byte of the register value
        _frame[3 + i * 2] = val & 0xFF;
        i++;
	}

    _reply = MB_REPLY_NORMAL;
}

void LbModbus::writeSingleCoil(word reg, word status) {
    //Check value (status)
    if (status != 0xFF00 && status != 0x0000) {
        this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address and execute (reg exists?)
    if (!this->Coil(reg, (bool)status)) {
        this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Check for failure
    if (this->Coil(reg) != (bool)status) {
        this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_SLAVE_FAILURE);
        return;
    }

    _reply = MB_REPLY_ECHO;
}

void LbModbus::writeMultipleCoils(byte* frame,word startreg, word numoutputs, byte bytecount) {
    //Check value
    word bytecount_calc = numoutputs / 8;
    if (numoutputs%8) bytecount_calc++;
    if (numoutputs < 0x0001 || numoutputs > 0x07B0 || bytecount != bytecount_calc) {
        this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address (startreg...startreg + numregs)
    for (int k = 0; k < numoutputs; k++) {
        if (!this->searchRegister(startreg + 1 + k)) {
            this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_ADDRESS);
            return;
        }
    }

    //Clean frame buffer
    free(_frame);
	_len = 5;
    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_WRITE_COILS;
    _frame[1] = startreg >> 8;
    _frame[2] = startreg & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;

    byte bitn = 0;
    word totoutputs = numoutputs;
    word i;
	while (numoutputs--) {
        i = (totoutputs - numoutputs) / 8;
        this->Coil(startreg, bitRead(frame[6+i], bitn));
        //increment the bit index
        bitn++;
        if (bitn == 8) bitn = 0;
        //increment the register
        startreg++;
	}

    _reply = MB_REPLY_NORMAL;
}
#endif


/*
void ModbusUpdateOLED(String ca_line1, String ca_line2)
{

//+--- Scroll IP ---+
  oled.clearDisplay();
  oled.setTextWrap(false);
  oled.setTextSize(2);      // Normal 1:1 pixel scale
  oled.setTextColor(SSD1306_WHITE); // Draw white text
  oled.setCursor(x, 0);
  oled.print(message);
  //oled.display();
  if(--x < minX) x = oled.width();
  //+---  

//+--- Print TXT 01 ---+
  oled.setCursor(0, 20);   
  //oled.println("0123456789");
  oled.println(ca_line1);
  //oled.setCursor(0, 40); 
  //oled.println("MQTT:6512345");
  oled.display();
  
}
*/
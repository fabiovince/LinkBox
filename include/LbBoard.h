//+--------------------------------------------------------------------------------------------+
//  Modbus.cpp - Source for Modbus Base Library                                                |
//	Copyright (C) 2014 André Sarmento Barbosa                                                  |
//	-----                                                                                      |
//  Modified to attend the following subjects:                                                 |
//  Industrial Informatic II, Home Automation and Industrial Networks I of                     |
//  Automation and Control Engineering Course - Federal University of Uberlândia (FEELT/UFU)   |
//  by Prof. Fábio Vincenzi and Prof. Renato Carrijo                                           |
//                                                                 Updated: 2023/10/28         |         
//+--------------------------------------------------------------------------------------------+
#ifndef LBBOARD_H
#define LBBOARD_H

#include "Arduino.h"


#define MAX_REGS     32
#define MAX_FRAME   128
//#define USE_HOLDING_REGISTERS_ONLY


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


typedef unsigned int u_int;

//Function Codes
enum {
    MB_FC_READ_COILS       = 0x01, // Read Coils (Output) Status 0xxxx
    MB_FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs) 1xxxx
    MB_FC_READ_REGS        = 0x03, // Read Holding Registers 4xxxx
    MB_FC_READ_INPUT_REGS  = 0x04, // Read Input Registers 3xxxx
    MB_FC_WRITE_COIL       = 0x05, // Write Single Coil (Output) 0xxxx
    MB_FC_WRITE_REG        = 0x06, // Preset Single Register 4xxxx
    MB_FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs) 0xxxx
    MB_FC_WRITE_REGS       = 0x10, // Write block of contiguous registers 4xxxx
};

//Exception Codes
enum {
    MB_EX_ILLEGAL_FUNCTION = 0x01, // Function Code not Supported
    MB_EX_ILLEGAL_ADDRESS  = 0x02, // Output Address not exists
    MB_EX_ILLEGAL_VALUE    = 0x03, // Output Value not in Range
    MB_EX_SLAVE_FAILURE    = 0x04, // Slave Deive Fails to process request
};

//Reply Types
enum {
    MB_REPLY_OFF    = 0x01,
    MB_REPLY_ECHO   = 0x02,
    MB_REPLY_NORMAL = 0x03,
};


typedef struct LBTRegister {
    word address;
    word value;
    struct LBTRegister* next;
} LBTRegister;


class LbBoard {
    private:
        LBTRegister *_regs_head;
        LBTRegister *_regs_last;

#if 0
        void exceptionResponse(byte fcode, byte excode);
        void readRegisters(word startreg, word numregs);
        void writeSingleRegister(word reg, word value);
        void writeMultipleRegisters(byte* frame,word startreg, word numoutputs, byte bytecount);
        #ifndef USE_HOLDING_REGISTERS_ONLY
            void readCoils(word startreg, word numregs);
            void readInputStatus(word startreg, word numregs);
            void readInputRegisters(word startreg, word numregs);
            void writeSingleCoil(word reg, word status);
            void writeMultipleCoils(byte* frame,word startreg, word numoutputs, byte bytecount);
        #endif

        void addReg(word address, word value = 0);
        bool Reg(word address, word value);
        word Reg(word address);
#endif

        LBTRegister* searchRegister(word addr);

    protected:
        byte *_frame;
        byte  _len;
        byte  _reply;
        bool receivePDU(byte* frame); 


    public:
        LbBoard();
        void Task(void);
        bool isOledDisable(void);
        void ModbusInit(void);
        
        bool OledShowValueFc1(uint8_t fc, uint16_t address);
        bool OledShowValueFc2(uint8_t fc, uint16_t address);
        uint16_t OledShowValueFc3(uint8_t fc, uint16_t address);
        uint16_t OledShowValueFc4(uint8_t fc, uint16_t address);
        bool OledShowValueFc5(uint8_t fc, uint16_t address);
        uint16_t OledShowValueFc6(uint8_t fc, uint16_t address);


        void addHreg(word offset, word value = 0);
        bool Hreg(word offset, word value);
        word Hreg(word offset);

        #ifndef USE_HOLDING_REGISTERS_ONLY
            void addCoil(word offset, bool value = false);
            void addIsts(word offset, bool value = false);
            void addIreg(word offset, word value = 0);

            bool Coil(word offset, bool value);
            bool Ists(word offset, bool value);
            bool Ireg(word offset, word value);

            bool Coil(word offset);
            bool Ists(word offset);
            word Ireg(word offset);

            //+--- LinkBox ---+
            //
            void Set_CoilsOffset(int device, word offset);
            void Set_DigitalInputOffset(int device, word offset);
            void Set_InputRegsOffset(int device, word offset);
            void Set_HoldingRegsOffset(int device, word offset);
            //
            void OledInitialize();
            void OledLine1(const char txt[]);
            void OledLine2(const char txt[]);
            void OledLine3(const char txt[]);

            void OledUpdate();  
            void OledUpdate_Static(int txtSizeLine1, int txtSizeLine2, int txtSizeLine3);
            
            bool displayInfo_MqttModbus(uint8_t fcode, uint16_t field1, uint16_t field2); 
        #endif
};



#endif //LBMODBUS_H

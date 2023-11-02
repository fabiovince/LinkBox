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

#include <LbModbus.h>
#include <WiFi.h>

#ifndef LBMODBUSIP_ESP32_H
#define LBMODBUSIP_ESP32_H

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10

class LbModbusIP : public LbModbus {
    private:
        byte _MBAP[7];
    public:
        LbModbusIP();
        //void config(const char* ssid, const char* password);
        void config();
        void task();
};

#endif //LBMODBUSIP_ESP32_H

//+--------------------------------------------------------------------------------------+
//|   KIT didático LinkBox (AHT10.ino) - Desenvolvido para as disciplinas:               |
//    Informática Industrial II, Domótica e Redes Industriais I da                       |
//    Faculdade de Engenharia Elétrica da Universidade Federal de Uberlândia (FEELT/UFU) |
//    Prof. Fábio Vincenzi e Prof. Renato Carrijo                                        |
//                                                      Última Atualização: 04/09/2023   |         
//+--------------------------------------------------------------------------------------+


#include "AHT10.h"
#include "global_vars.h"



//#ifdef Enable_AHT10
    void AHT10_Config() 
    {  
        int by_nTries = 1;


        while ((aht10.begin() != true) && by_nTries < 5) //for ESP-01 use aht10.begin(0, 2);
        {
            Serial.println(F("AHT1x not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
            SerialOTA.println("AHT1x not connected or fail to load calibration coefficient");
            
            lbmb.OledLine1("              ");
            lbmb.OledLine2("   AHT10      "); 
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
            delay(3000);
            by_nTries++;
        }

        if(by_nTries>=5)
        {
          lbmb.OledLine1("             ");
          lbmb.OledLine2("   AHT10     ");
          lbmb.OledLine3(" FAILED!!!   ");
          lbmb.OledUpdate();

          b_AHT10_INIT_OK = false;

          delay(6000);
        }
        else
        {
          lbmb.OledLine1("             ");
          lbmb.OledLine2(" AHT10 OK    ");
          lbmb.OledLine3("             ");
          lbmb.OledUpdate();

          b_AHT10_INIT_OK = true;

          delay(3000);
        }
    }
//#endif


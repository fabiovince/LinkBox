#ifndef __MQTT_H
  #define __MQTT_H
  #include <WiFi.h>
  
  
  //Prototipos
  bool ConnectMQTT(bool reconnect);
  void Callback(char *topic, uint8_t * payload, unsigned int length);
  //void MqttSend_ModbusFrame(bool stat, const char *topic, uint16_t value, uint8_t fc, uint8_t id, uint16_t  address, uint16_t quantity);
  void MqttSend_SensorValue(bool b_MQTT_Status, String sensor, uint16_t value);
  void MqttSend_ModbusFrame(bool stat, uint16_t value, uint8_t fc, uint8_t id, uint16_t  address, uint16_t quantity, uint16_t currFrameNumber);
  void TXT_Analisys(uint8_t *charArr, unsigned int arrLength);

  #ifdef def_Show_SizeOf_DataTypes
    void show(const char * tag, int l);
    void Show_SizeOf_DataTypes();
  #endif
  
#endif

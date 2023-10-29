#ifndef BourbonWifi_h
#define BourbonWifi_h

#include <Arduino.h>
#include "SPIFFS.h"

// #define MSGPACK_DEBUGLOG_ENABLE
// #include <MsgPack.h>

struct WifiConfig{
  String wsid;
  String pass;
  // MSGPACK_DEFINE(wsid, pass);
};


class BourbonWifi{
  private:
    static constexpr const char* CONFIG_FILE_NAME = "/.wificonfig";
    static BourbonWifi* instance;
    WifiConfig config;
    BourbonWifi() {};

  public:
    static BourbonWifi* init(){
      if (instance==nullptr){
        instance = new BourbonWifi();
      }
      return instance;
    }

    void readConfig(){
      if (!SPIFFS.begin(true)) {
        Serial.println("Can't read WiFi configuration.");
        return;
      }
      if (!SPIFFS.exists(BourbonWifi::CONFIG_FILE_NAME)){
        Serial.println("Config does not exist");
        return;
      }
      File configFile = SPIFFS.open(BourbonWifi::CONFIG_FILE_NAME, FILE_READ);
      configFile.readBytes((char*)&this->config, sizeof(WifiConfig));
      Serial.println("Config:");
      Serial.print("WSID: ");
      Serial.println(this->config.wsid);
      Serial.print("PASS: ");
      Serial.println(this->config.pass);
      configFile.close();
    }
    
    void createConfig(){
      if (!SPIFFS.begin(true)) {
        Serial.println("Can't read WiFi configuration.");
        return;
      }
      File configFile = SPIFFS.open(BourbonWifi::CONFIG_FILE_NAME, FILE_WRITE);

      WifiConfig newConfig = {"MyWifi", "abcd"};
      configFile.write((unsigned char*)&newConfig, sizeof(WifiConfig));
      configFile.close();
      Serial.println("File created");
    }

    void deleteConfig(){
      if (!SPIFFS.begin(true)) {
        Serial.println("Can't read WiFi configuration.");
        return;
      }
      SPIFFS.remove(BourbonWifi::CONFIG_FILE_NAME);
      Serial.println("Arquivo removido");
    }

};

BourbonWifi* BourbonWifi::instance = nullptr;

#endif

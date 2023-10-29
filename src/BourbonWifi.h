#ifndef BourbonWifi_h
#define BourbonWifi_h

#include <Arduino.h>
#include "SPIFFS.h"
#include <DNSServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

struct WifiConfig{
  String wsid;
  String pass;
};

const char indexHtml[] PROGMEM = R"HTML(
      <!DOCTYPE HTML>
      <html>
        <head>
        <title>Config</title>
          <meta name="viewport" content="width=device-width, initial-scale=1">
        </head>
        <body>
          <h1>Configuration</h1>
        </body>
      </html>)HTML";


class BourbonWifi{
  private:
    static constexpr const char* CONFIG_FILE_NAME = "/.wificonfig";
        
    static BourbonWifi* instance;
    WifiConfig config;
    DNSServer *dnsServer;
    AsyncWebServer *server;

    BourbonWifi() {};

  public:
    static BourbonWifi* init(){
      if (instance==nullptr){
        instance = new BourbonWifi();
      }
      return instance;
    }

    void setupWebServer(){
      class CaptiveRequestHandler : public AsyncWebHandler {
        public:
          CaptiveRequestHandler() {}
          virtual ~CaptiveRequestHandler() {}

          bool canHandle(AsyncWebServerRequest *request){
            return true;
          }

          void handleRequest(AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", indexHtml); 
          }
      };
      this->server = new AsyncWebServer(80);
      server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", indexHtml); 
        Serial.println("Client Connected");
      });
      server->addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
      server->begin();
    }

    static void processDNS(void *params){
      BourbonWifi *self = (BourbonWifi *) params;
      for(;;){
        self->dnsServer->processNextRequest();
        vTaskDelay(1 / portTICK_PERIOD_MS);
      }
    }

    void initConfigAccessPoint(){
      WiFi.mode(WIFI_AP); 
      WiFi.softAP("ESP-WifiConfig");
      dnsServer = new DNSServer();
      dnsServer->start(53, "*", WiFi.softAPIP());
      xTaskCreate(BourbonWifi::processDNS,
        "Process DNS",
        (10 * 1024),
        (void *) this,
        1,
        NULL);
      this->setupWebServer();
      Serial.println("Webserver running!");
      Serial.print("AP IP address: ");
      Serial.println(WiFi.softAPIP());
    }

    void readConfig(){
      if (!SPIFFS.begin(true)) {
        Serial.println("Can't read WiFi configuration.");
        return;
      }
      if (!SPIFFS.exists(BourbonWifi::CONFIG_FILE_NAME)){
        Serial.println("Config does not exist");
        this->initConfigAccessPoint();
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

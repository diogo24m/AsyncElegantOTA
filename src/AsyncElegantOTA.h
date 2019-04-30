#ifndef ElegantOTA_h
#define ElegantOTA_h

#include "Arduino.h"
#include "stdlib_noniso.h"

#if defined(ESP8266)
    #include "ESP8266WiFi.h"
    #include <Hash.h>
    #include <ESPAsyncTCP.h>
#elif defined(ESP32)
    #include "WiFi.h"
    #include <Hash.h>
    #include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>

#include "elegantWebpage.h"


class AsyncElegantOtaClass{
    public:

        void begin(AsyncWebServer &server){
            server.on("/update", HTTP_GET, [&](AsyncWebServerRequest *request){
                AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", ELEGANT_HTML, ELEGANT_HTML_SIZE);
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
            });

            server.on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
                // the request handler is triggered after the upload has finished... 
                // create the response, add header, and send response
                AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
                response->addHeader("Connection", "close");
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
                restartRequired = true;
            }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                //Upload handler chunks in data
                if (!index) {
                
                    #if defined(ESP8266)
                        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;      
                        if (!Update.begin(maxSketchSpace)){ // Start with max available size
                    #endif
                    
                    #if defined(ESP32)
                        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with max available size
                    #endif
                            Update.printError(Serial);   
                        }

                    #if defined(ESP8266)
                        Update.runAsync(true); // Tell the updaterClass to run in async mode
                    #endif
                
                }

                // Write chunked data to the free sketch space
                if (Update.write(data, len) != len) {
                    Update.printError(Serial); 
                }
                    
                if (final) { // if the final flag is set then this is the last frame of data
                    if (Update.end(true)) { //true to set the size to the current progress

                    }
                }
            });
        }

        void loop(){
            if(restartRequired){
                ESP.restart();
            }
        }

    private:
        bool restartRequired = false;

};

AsyncElegantOtaClass AsyncElegantOTA;
#endif
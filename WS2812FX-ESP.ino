/*
  WS2812FX Webinterface.

  https://github.com/Miauto/WS2812FX-ESP


  FEATURES
      Webinterface with mode, color, speed and brightness selectors
      Amélioration pour interface avec une domotique
      ajout de wifimanager : gestion de la conexion wifi simplifier

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG
  2017-03-07 Amélioration interface domotique, wifimanager, update for OTA
  2016-11-26 initial version

*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WS2812FX.h>

/* Add Update for OTA with the binarie File 20-12-2016 */
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
MDNSResponder mdns;

/* add wifimanager */
//#include <DNSServer.h>
#include <WiFiManager.h>

#include "index.html.h"
#include "main.js.h"


// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define LED_PIN D3                       // 0 = GPIO0, 2=GPIO2
#define MAX_LED_COUNT 24
int LED_COUNT = 5;

#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.
#define HTTP_PORT 80

#define DEFAULT_COLOR 0xFF5900
#define DEFAULT_BRIGHTNESS 100
#define DEFAULT_SPEED 10
#define DEFAULT_MODE FX_MODE_RAINBOW

#define BRIGHTNESS_STEP 15              // in/decrease brightness by this amount per click
#define SPEED_STEP 10                   // in/decrease brightness by this amount per click

unsigned long last_wifi_check_time = 0;
String modes = "";

WS2812FX ws2812fx = WS2812FX(MAX_LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server = ESP8266WebServer(HTTP_PORT);


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Starting...");

  modes.reserve(5000);
  modes_setup();

  Serial.println("WS2812FX setup");
  ws2812fx.init();
  ws2812fx.setMode(DEFAULT_MODE);
  ws2812fx.setColor(DEFAULT_COLOR);
  ws2812fx.setSpeed(DEFAULT_SPEED);
  ws2812fx.setBrightness(DEFAULT_BRIGHTNESS);
  ws2812fx.start();

  Serial.println("Wifi setup");
  WiFiManager wifiManager;
//  wifiManager.resetSettings();
  wifiManager.autoConnect("ESP_Light");

  Serial.println("HTTP server setup");
  server.on("/", srv_handle_index_html);
  server.on("/main.js", srv_handle_main_js);
  server.on("/modes", srv_handle_modes);
  server.on("/etat", srv_handle_etat);
  server.on("/set", srv_handle_set);
  server.onNotFound(srv_handle_not_found);
  server.begin();
  httpUpdater.setup(&server);
  Serial.println("HTTP server started.");

  Serial.println("ready!");
}


void loop() {
  unsigned long now = millis();

  mdns.update();
  server.handleClient();
  ws2812fx.service();

}


/*
   Build <li> string for all modes.
*/
void modes_setup() {
  modes = "";
  for (uint8_t i = 0; i < ws2812fx.getModeCount(); i++) {
    modes += "<li><a href='#' class='m' id='";
    modes += i;
    modes += "'>";
    modes += ws2812fx.getModeName(i);
    modes += "</a></li>";
  }
}

/* #####################################################
  #  Webserver Functions
  ##################################################### */

void srv_handle_not_found() {
  server.send(404, "text/plain", "File Not Found");
}

void srv_handle_index_html() {
  server.send_P(200, "text/html", index_html);
}

void srv_handle_main_js() {
  server.send_P(200, "application/javascript", main_js);
}

void srv_handle_modes() {
  server.send(200, "text/plain", modes);
}

void srv_handle_etat() {
  server.send(200, "text/plain", String(ws2812fx.getMode()));
}

void srv_handle_set() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "c") {
      uint32_t tmp = (uint32_t) strtol(&server.arg(i)[0], NULL, 16);
      if (tmp >= 0x000000 && tmp <= 0xFFFFFF) {
        ws2812fx.setColor(tmp);
      }
    }

    if (server.argName(i) == "m") {
      uint8_t tmp = (uint8_t) strtol(&server.arg(i)[0], NULL, 10);
      ws2812fx.setMode(tmp % ws2812fx.getModeCount());
    }

    if (server.argName(i) == "b") {
      if (server.arg(i) == "m") {
        ws2812fx.decreaseBrightness(BRIGHTNESS_STEP);
      } else if (server.arg(i) == "p") {
        ws2812fx.increaseBrightness(BRIGHTNESS_STEP);
      } else if (server.arg(i).toInt() >= 0 && server.arg(i).toInt() <= 255 && server.arg(i) != "") {
        ws2812fx.setBrightness(server.arg(i).toInt());
      }
    }

    if (server.argName(i) == "s") {
      if (server.arg(i) == "m") {
        ws2812fx.decreaseSpeed(SPEED_STEP);
      } else if (server.arg(i) == "p") {
        ws2812fx.increaseSpeed(SPEED_STEP);
      } else  {
        ws2812fx.setSpeed(server.arg(i).toInt());
      }
    }

    if (server.argName(i) == "d") {
      if (server.arg(i) > "0" && server.arg(i) <= "24") {
        LED_COUNT = server.arg(i).toInt();
      }
    }
  }
  server.send(200, "text/plain", "OK");
}

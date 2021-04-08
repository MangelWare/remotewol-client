#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WakeOnLan.h>
#include <ESP8266HTTPClient.h>
#include <bearssl/bearssl.h>
#include <Base64.h>

#include "secrets.h"

#ifndef STASSID
#define STASSID "<your-ssid>"
#define STAPSK  "<your-password>"
#endif

#ifndef POLLING_RATE_SEC
#define POLLING_RATE_SEC 20
#endif

#ifndef POLLING_URL
#define POLLING_URL "http://remotewol.simonmangel.de:8080/poll"
#endif

#ifndef WAKEUP_MSG
#define WAKEUP_MSG "Wake up!"
#endif

WiFiUDP UDP;
IPAddress computer_ip(255, 255, 255, 255); 

const char* ssid     = STASSID;
const char* password = STAPSK;

const int polling_rate_seconds = POLLING_RATE_SEC;
const char* polling_url = POLLING_URL;

const char* wakeup_msg = WAKEUP_MSG;

char hmac_key[32];
int parse_hmac_key_successful = 0;


void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  Serial.print("Waiting for WiFi...");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    tries++;
    Serial.print(".");
  }
  Serial.println();
  if(tries>=20) {
    Serial.println("Error: WiFi could not be connected!");
    ESP.deepSleep(polling_rate_seconds*1000000);
  }

  //Decode HMAC key
  Serial.println("Decoding HMAC key...");
  int dec_len = Base64.decodedLength((char *)hmac_key_encoded, strlen(hmac_key_encoded));
  Serial.printf("Size of decoded: %d\n",dec_len);
  if (dec_len == 32) {
    Base64.decode(hmac_key, (char *)hmac_key_encoded, strlen(hmac_key_encoded));
    Serial.printf("Decoded key: ");
    for (int i = 0; i<32; i++) {
      Serial.printf("%02x",hmac_key[i]);
    }
    Serial.println();
    parse_hmac_key_successful = 1;
  }
  
}

void loop() {  
  if(!parse_hmac_key_successful) {
    Serial.println("Error: HMAC key could not be parsed correctly!");
    ESP.deepSleep(polling_rate_seconds*1000000);
    return;
  }

  Serial.println("Starting poll!");

  Serial.println("WiFi connection established!");
  WiFiClient wifi_c;
  HTTPClient http;

  String auth_challenge;
  Serial.println("Attempting first request...");
  if(http.begin(wifi_c, polling_url)) {
    int httpCode = http.GET();
    if(httpCode > 0) {
      Serial.printf("HTTP GET: Received code %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        auth_challenge = http.getString();
        Serial.println(auth_challenge);
      }
    } else {
      Serial.printf("HTTP GET request failed, error: %s\n",http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("Unable to connect to the URL via HTTP!\n");
  }

  if (auth_challenge.length() > 0) {
    Serial.println("Auth challenge received! Sending response request...");
    Serial.printf("Auth random: %s\n",auth_challenge.c_str());
    // Decode auth_challenge
    //Serial.println("Init random buf...");
    unsigned char auth_random[64];
    //Serial.println("Decode base64...");
    int decodedLength = Base64.decodedLength((char *)auth_challenge.c_str(), auth_challenge.length());
    if(decodedLength != 64) {
      Serial.printf("Invalid decoded random length: %d\n",decodedLength);
    } else {
      char auth_random[decodedLength];
      Base64.decode(auth_random, (char *)auth_challenge.c_str(), auth_challenge.length());
      Serial.printf("Decoded random: ");
      for (int i = 0; i<64; i++) {
        Serial.printf("%02x",(int) auth_random[i]);
      }
      Serial.println();
      //Serial.println("Define contexts...");
      br_hmac_key_context key_context;
      br_hmac_context hmac_context;
      //Serial.println("Setting key...");
      //Serial.println("Initializing key context...");
      br_hmac_key_init(&key_context, &br_sha256_vtable, hmac_key, (size_t) 32);
      //Serial.println("Initializing hmac context...");
      br_hmac_init(&hmac_context, &key_context,0);
      //Serial.println("Creating hmac output...");
      br_hmac_update(&hmac_context, auth_random, 64);
      char hmac_out[32];
      br_hmac_out(&hmac_context, hmac_out);
      Serial.printf("HMAC: ");
      for (int i=0; i<sizeof(hmac_out); i++) {
        Serial.printf("%02x",(int) hmac_out[i]);
      }
      Serial.println();

      int encodedLength = Base64.encodedLength(32);
      char hmac_out_encoded[encodedLength];
      Base64.encode(hmac_out_encoded, (char *) hmac_out, 32);
      Serial.printf("Encoded HMAC: %s\n",hmac_out_encoded);

      // Assemble auth string
      char auth_token[256] = {0};
      int size_random = auth_challenge.length();
      memcpy(auth_token, auth_challenge.c_str(), size_random);
      auth_token[size_random]=':';
      memcpy(auth_token+size_random+1, hmac_out_encoded, encodedLength);
      Serial.printf("Auth token: %s\n",auth_token);

      if(http.begin(wifi_c, polling_url)) {
        http.addHeader("Authorization",auth_token);
        int httpCode = http.GET();
        if(httpCode > 0) {
          Serial.printf("HTTP GET: Received code %d\n", httpCode);
          if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String server_response = http.getString();
            if (strcmp(wakeup_msg, server_response.c_str())==0) {
              Serial.println("Wakeup call received! Sending magic packet...");
              WakeOnLan::sendWOL(computer_ip, UDP, (byte *)mac, sizeof mac);
              delay(200);
            }
          }
        } else {
          Serial.printf("HTTP GET request failed, error: %s\n",http.errorToString(httpCode).c_str());
        }
        http.end();
      } else {
        Serial.printf("Unable to connect to the URL via HTTP!\n");
      }
      
    }
  } else {
    Serial.printf("Challenge was invalid!\n");
  }
  //String polling_result = poll_response(http,wifi_c,polling_url,auth_challenge);


  ESP.deepSleep(polling_rate_seconds*1000000);
}

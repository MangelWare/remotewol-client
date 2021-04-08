#ifndef STASSID
#define STASSID "<Your WiFi SSID>"
#define STAPSK  "<Your WiFi Password>"
#endif


const char* hmac_key_encoded = "///////////////////////////////////////////="; // Replace by your base64-encoded 32-bit HMAC key
const byte mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Replace by your MAC address to send the WOL packet to

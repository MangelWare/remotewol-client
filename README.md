# Remote WOL: Client

Client implementation of a simple Remote WOL (Wake-on-LAN) setup based on an ESP8266.
The polling mechanism consists of two HTTP GET requests with a custom HMAC-based challenge-response authentication.

See [here](https://github.com/MangelWare/remotewol-server) for a server implementation using Node.js, Express and a Telegram Bot.

### Arduino Libraries

- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [WakeOnLan-ESP8266](https://github.com/koenieee/WakeOnLan-ESP8266)
- [Base 64 Arduino Library](https://github.com/agdl/Base64)

# Remote Wake-on-LAN: Client

Client implementation of a simple Remote WOL (Wake-on-LAN) setup based on an ESP8266.
The polling mechanism consists of two HTTP GET requests with a custom HMAC-based challenge-response authentication.

See [here](https://github.com/MangelWare/remotewol-server) for a server implementation using Node.js, Express and a Telegram Bot.

### Arduino Libraries

- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [WakeOnLan-ESP8266](https://github.com/koenieee/WakeOnLan-ESP8266)
- [Base 64 Arduino Library](https://github.com/agdl/Base64)

### Deployment

To deploy the client firmware, replace the placeholder secrets in `remotewol/secrets.h`. 
Furthermore, replace the polling URL in `remotewol/remotewol.ino`.
You can also configure the polling rate here (see `POLLING_RATE_SEC`).
The firmware can be flashed to your ESP8266 via the Arduino IDE. Note that the implementation uses the deep sleep feature of the ESP8266, so the respective GPIO pin has to be connected to RST for this to work (tutorials w.r.t. specific boards can be found online).

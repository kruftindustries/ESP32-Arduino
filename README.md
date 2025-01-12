# ESP32 Arduino Examples
![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)  ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![HTML5](https://img.shields.io/badge/html5-%23E34F26.svg?style=for-the-badge&logo=html5&logoColor=white) ![JavaScript](https://img.shields.io/badge/javascript-%23323330.svg?style=for-the-badge&logo=javascript&logoColor=%23F7DF1E)

Tested on KiCony KC868-A16 | Pinout
:-------------------------:|:-------------------------:
![KC868-A16](https://github.com/user-attachments/assets/9053a999-cb6e-4c21-93d5-177785851ad7)|![ESP32-WROOM-32e Pinout](https://github.com/user-attachments/assets/a5f06755-1bcb-472d-89d0-6c821c5ebba6)

Tested using Arduino 2.3.4 **ESP32 Dev Module** board with the following settings:
![alt text](https://github.com/user-attachments/assets/11c2dcfe-1d54-4f9c-a2fb-3b92e104409e)

I had to modify pins_arduino.h for this board

```
%LocalAppData%\Arduino15\packages\esp32\hardware\esp32\3.0.7\variants\esp32\pins_arduino.h
```
```C++
static const uint8_t SDA = 4;
static const uint8_t SCL = 5;
```

# Example Files

## AP_PCF8574.ino
### Wifi AP Webpage - 16ch PCF8574 I/O - INA226


|Uses the following libraries:|
:-------------------------:
|[![arduino-library-badge](https://www.ardu-badge.com/badge/PCF8574.svg?)](https://github.com/RobTillaart/PCF8574)|
|[![arduino-library-badge](https://www.ardu-badge.com/badge/INA226.svg?)](https://github.com/RobTillaart/INA226)|




This reads and sets the status of 16 digital channels via PCF8574, and reads Voltage, Current, and Wattage via INA226 over I2C.

Desktop             |  Mobile
:-------------------------:|:-------------------------:
![Desktop](https://github.com/user-attachments/assets/03666ce1-3bb0-40e0-b69b-059ed6c98f5e)|![Mobile](https://github.com/user-attachments/assets/35dd440d-15e9-4c7d-805d-9d4a7f9a2313)

## ETH_LAN8720.ino
### Ethernet Client test - Serial port output

```HTML
ETH Started
ETH Connected
ETH MAC: DE:AD:BE:EF:13:37, IPv4: 192.168.100.248, FULL_DUPLEX, 100Mbps

connecting to google.com
HTTP/1.1 301 Moved Permanently
Location: http://www.google.com/
Content-Type: text/html; charset=UTF-8
Content-Security-Policy-Report-Only: object-src 'none';base-uri 'self';script-src 'nonce-00000000000000000000' 'strict-dynamic' 'report-sample' 'unsafe-eval' 'unsafe-inline' https: http:;report-uri https://csp.withgoogle.com/csp/gws/other-hp
Date: Sun, 12 Jan 2025 11:13:35 GMT
Expires: Tue, 11 Feb 2025 11:13:35 GMT
Cache-Control: public, max-age=2592000
Server: gws
Content-Length: 219
X-XSS-Protection: 0
X-Frame-Options: SAMEORIGIN

<HTML><HEAD><meta http-equiv="content-type" content="text/html;charset=utf-8">
<TITLE>301 Moved</TITLE></HEAD><BODY>
<H1>301 Moved</H1>
The document has moved
<A HREF="http://www.google.com/">here</A>.
</BODY></HTML>
closing connection
```

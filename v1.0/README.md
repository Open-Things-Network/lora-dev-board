# v1.0

![pcb overview](Media/pcb-overview.png)

# Opis płytki
Możliwe warianty konfiguracji:
* Arduino Pro Mini i RFM95W 
* Arduino Pro Mini i RN2483A
* tylko RN2483A

## Arduino Pro Mini
Platforma współpracuje z modułami [Arduino Pro Mini](https://github.com/sparkfun/Arduino_Pro_Mini_328) lub ich klonami w obu wariantach (3.3V/8MHz i 5V/16MHz).
> Do programowania potrzebny jest konwerter USB/Serial.

## Interfejs LoRaWAN
Płytka pozwala na realizację interfejsu LoRaWAN na dwa różne sposoby:
* wykorzystanie modułu nadajnika-odbiornika LoRa [RFM95W](https://www.hoperf.com/modules/lora/RFM95.html) i implementacja stosu LoRaWAN na mikrokontrolerze - komunikacja z modułem poprzez SPI (piny 11, 12 i 13 Arduino Pro Mini)
* zastosowanie modułu z wbudowaną obsługą stosu LoRaWAN [RN2483A](https://www.microchip.com/wwwproducts/en/RN2483) - komunikacja z modułem poprzez port szeregowy obsługiwany programowo (piny 6 (RX) i 7 (TX) Arduino Pro Mini).

## Układ zasilania
Zastosowany regulator napięcia LDO [TS2940CW33](https://www.taiwansemi.com/en/products/details/TS2940CW33).

## Układ ładowania
Zastosowany kontroler ładowania akumulatorów litowo-polimerowych (Li-Po) i litowo-jonowych (Li-Ion) [MCP73831](https://www.microchip.com/wwwproducts/en/en024903#additional-features).

## Diody LED
|Oznaczenie|Opis|
|----------|----|
|LED|dioda do ogólnego zastosowania w aplikacji użytkownika (`USER_LED`)|
|VCC|sygnalizuje obecność napięcia zasilania|
|CHR|sygnalizuje ładowanie podłączonej baterii Li-Po/Li-Ion|

## Interfejsy komunikacyjne
Do podłączenia zewnętrznych czujników dostępne są interfejsy:
* I2C wyprowadzony bezpośrednio z Arduino Pro Mini
* port szeregowy obsługiwany programowo (z wykorzystaniem biblioteki `SoftwareSerial`) na pinach 8 (RX) i 9 (TX) Arduino Pro Mini.

## Pomiar napięcia baterii
Wykorzystane wejście analogowe A0 i dzielnik napięcia R5, R6.

W kolumnach A,B,C ... zaznaczone zostały elementy wymagane (+) oraz opcjonalne (\*) dla specyficznej wersji urządzenia

* A - wersja minimalna [doc/A_minimal.md](doc/A_minimal.md)
* B - zasilanie bateryjne, czujniki analogowe
* C - nazwa wersji

# Lista elementów
| Nazwa | Element | Ilość | Uwagi |A|B|
|-------|---------|-------|-------|-|-|
|Moduł micro USB| |1| |*| |
|uchwyt baterii AA|KEYSTONE 2915|2|| |+|
|BAT,SOLAR|JST-PH 2pin 2mm|2|| | | |
|C1,C4|4,7μF MLCC 1206|2|| | |
|C2|100nF MLCC 1206|1|| |+|
|C3|22uF 6,3V Tantalum CASE:A|1|| |+|
|D1,D2,D3|SMD 1206|3|diody LED (LED,VCC,CHR)| | |
|J1|Wurth Elektronik 629105136821 Micro Usb|1|| |*|
|J2|U.FL 50R|1|gniazdo antenowe| |*|
|J3|WR-PHD 2.54 mm SMT Pin Header 61000418221|1|interfejs szeregowy| | |
|J4|WR-PHD 2.54 mm SMT Dual Pin Header 61000821121|1|interfejs I2C| | |
|Q1|BC817 SOT-23|1|| | | |
|Q2|YJL2301F SOT-23|1|| | |
|R11|51R SMD 1206|1|| | |
|R10|470R SMD 1206|1|| | |
|R4,R8,R9|1k SMD 1206|3|| |
|R1|4k SMD 1206|1|| | |
|R2,R3,R7,R12|10k SMD 1206|4|| | |
|R6|33k SMD 1206|1|| |+|
|R5|100k SMD 1206|1|| |+|
|SW1|Slide Switch 1P2T os102011ma1qn1|1|włącznik zasilania| | |
|U1|RFM95W|1|**dotyczy tylko wersji z RFM95W**|+|+|
|U2|TS2940-3.3 SOT-223|1|regulator napięcia LDO| |+|
|U3|MCP73831-2-OT|1|kontroler ładowania baterii Li-Po/Li-Ion| | |
|U4|RN2483A|1|**dotyczy tylko wersji z RN2483A**| | |
|U5|Arduino Pro Mini|1||+|+|

# Uwagi
W schemacie została wprowadzona zmiana elementu **Q2** na **YJL2301F** oraz jego połączeniu. 

![mosfet update](Media/mosfet.jpg)

Należy uwzględnić poprawkę w montażu płytki, mosfet należy odwrócić "do góry nóżkami" i wlutować w następujący sposób:

![mosfet pcb](Media/mosfet2.jpg)

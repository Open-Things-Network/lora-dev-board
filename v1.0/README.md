# v1.0

![pcb overview](Media/pcb-overview.png)

# Spis treści:
1. [Opis płytki](#Opis płytki)
2. [Arduino Pro Mini](#Arduino Pro_Mini)
3. [Interfejs LoRaWAN](#Interfejs LoRaWAN)
4. [Układ zasilania](#Układ zasilania)
5. [Układ ładowania](#Układ ładowania)
6. [Diody LED](#Diody LED)
7. [Interfejsy komunikacyjne](#Interfejsy komunikacyjne)
8. [Pomiar napięcia baterii](#Pomiar napięcia_baterii)
9. [Sterowanie zasilaniem układów zewnętrznych](#Sterowanie zasilaniem układów zewnętrznych)
10. [Zworki](#Zworki)
11. [Lista elementów](#Lista elementów)

# Opis płytki
Możliwe warianty konfiguracji:
1. Arduino Pro Mini i RFM95W 
2. Arduino Pro Mini i RN2483A
3. RN2483A

Szczegółowe instrukcje dla wersji montażowych:
* Wersja minimalna [doc/A_minimal.md](doc/A_minimal.md)
* Zasilanie bateryjne, czujniki analogowe
* W przygotowaniu

## Arduino Pro Mini
Platforma współpracuje z modułami [Arduino Pro Mini](https://github.com/sparkfun/Arduino_Pro_Mini_328) lub ich klonami w obu wariantach (3.3V/8MHz i 5V/16MHz).
> **UWAGA!** Do programowania potrzebny jest konwerter USB/Serial.

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
* I2C wyprowadzony bezpośrednio z Arduino Pro Mini (istnieje możliwość zastosowania rezystorów podciągających R2 i R3)
* port szeregowy obsługiwany programowo (z wykorzystaniem biblioteki `SoftwareSerial`) na pinach 8 (RX) i 9 (TX) Arduino Pro Mini.

## Pomiar napięcia baterii
Wykorzystane wejście analogowe A0 i dzielnik napięcia R5, R6.

## Sterowanie zasilaniem układów zewnętrznych

## Zworki
|Symbol|Nazwa|Przeznaczenie|
|------|-----|-------------|
|JP4|PWR ON||
|JP2|VCC/VBAT||
|JP9|OUT VBAT/VCC||
|JP3|DIO2/D6||
|JP7|SCL||
|JP8|SDA||
|JP5|RX||
|JP6|TX||
|JP1|LED RN||

# Lista elementów
W Kolumnach A,B,C,D zaznaczone zostały elementy wymagane (+) i opcjonalne (\*) dla odpowiednich konfiguracji urządzenia:
* A - Zasilanie z portu USB
* B - Zasilanie bateriami o napięciu do **3V**
* C - Zasilanie bateriami litowymi nie ładowalnymi o napięciu **3,6V**
* D - Zasilanie akumulatorami li-Ion oraz li-Pol o napięciu **3,6V**

| Nazwa | Element |A|B|C|D| Sekcja |
|-------|---------|-|-|-|-|--------|
|U5|Arduino Pro Mini|+|+|+|+| |
|U1|RFM95W **dotyczy tylko wersji z RFM95W**||+|+|+|+|RFM9W|
|U4|RN2483A **dotyczy tylko wersji z RN2483A**|+|+|+|+|RN2483A|
|R11|51R SMD 1206 **dotyczy tylko wersji z RN2483A**|+|+|+|+|RN2483A|
|R12|10k SMD 1206 **dotyczy tylko wersji z RN2483A**|+|+|+|+|RN2483A|
|J2|U.FL gniazdo antenowe|\*|\*|\*|\*|RFM95W i RN2483A|
|U2|TS2940-3.3 SOT-223 regulator napięcia LDO|+| |\*|\*|Zasilanie|
|C2|100nF MLCC 1206|+| |\*|\*|Zasilanie|
|C3|22uF 6,3V Tantalum CASE:A|+| |\*|\*|Zasilanie|
|SW1|Switch 1P2T os102011ma1qn1 włącznik zasilania|\*|\*|\*|\*|Zasilanie|
|J1|Wurth Elektronik 629105136821 Micro Usb|+| | |\*|Zasilanie i Ładowarka|
|bateria AA|KEYSTONE 2915 (wymagne dwie sztuki na płytkę)|\*|\*|\*|\*|Zasilanie i Ładowarka|
|BAT|JST-PH 2pin 2mm|\*|\*|\*|\*|Zasilanie i Ładowarka|
|U3|MCP73831-2-OT kontroler ładowania|\*| | |\*|Ładowarka|
|C1|4,7μF MLCC 1206|\*| | |\*|Ładowarka|
|C4|4,7μF MLCC 1206|\*| | |\*|Ładowarka|
|R1|4k SMD 1206|\*| | |\*|Ładowarka|
|SOLAR|JST-PH 2pin 2mm|\*| | |\*|Ładowarka|
|R6|33k SMD 1206| | |\*|\*|Pomiar napięcia baterii|
|R5|100k SMD 1206| | |\*|\*|Pomiar napięcia baterii|
|D1|LED SMD 1206 LED|\*|\*|\*|\*|Diody LED|
|D2|LED SMD 1206 VCC|\*|\*|\*|\*|Diody LED|
|D3|LED SMD 1206 CHR|\*|\*|\*|\*|Diody LED|
|R8|1k SMD 1206|\*|\*|\*|\*|Diody LED|
|R9|1k SMD 1206|\*|\*|\*|\*|Diody LED|
|R10|470R SMD 1206|\*|\*|\*|\*|Diody LED|
|J3|WR-PHD 2.54 mm SMT Pin Header 61000418221 UART|\*|\*|\*|\*|Interfejsy komunikacyjne|
|J4|WR-PHD 2.54 mm SMT Dual Pin Header 61000821121 I2C|\*|\*|\*|\*|Interfejsy komunikacyjne|
|R2|10k SMD 1206|\*|\*|\*|\*|Interfejsy komunikacyjne|
|R3|10k SMD 1206|\*|\*|\*|\*|Interfejsy komunikacyjne|
|Q1|BC817 SOT-23 Tranzystor NPN|\*| |\*|\*|Zasilanie układów zew.|
|Q2|YJL2301F SOT-23 Tranzystor MOSFET|\*| |\*|\*|Zasilanie układów zew.|
|R7|10k SMD 1206|\*| |\*|\*|Zasilanie układów zew.|
|R4|1k SMD 1206|\*| |\*|\*|Zasilanie układów zew.|

# Uwagi
W schemacie została wprowadzona zmiana elementu **Q2** na **YJL2301F** oraz jego połączeniu. 

![mosfet update](Media/mosfet.jpg)

Należy uwzględnić poprawkę w montażu płytki, mosfet należy odwrócić "do góry nóżkami" i wlutować w następujący sposób:

![mosfet pcb](Media/mosfet2.jpg)

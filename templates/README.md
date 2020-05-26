# Szablony aplikacji

Zastosowanie szablonów ma na celu przyspieszenie procesu tworzenia własnych aplikacji. Wystarczy dokonać wstępnej parametryzacji i skupić się na implementacji docelowych funkcjonalności. Szablony aplikacji zapewniają realizację komunikacji LoRaWAN, wprowadzanie układu w stan uśpienia oraz ułatwiają wykorzystanie zasobów platformy w wybranym wariancie konfiguracji. 

Dostępne wersje szablonów:
* `lorawan-rfm-template.ino` - dla wariantu 1 konfiguracji (Arduino Pro Mini i RFM95W)
* `lorawan-rn-template.ino` - dla wariantu 2 konfiguracji (Arduino Pro Mini i RN2483A)

## Wymagania związane z oprogramowaniem
Wykorzystane biblioteki:
* [Arduino-LMIC Library](https://github.com/matthijskooijman/arduino-lmic)
* [Low Power Library for Arduino](https://github.com/rocketscream/Low-Power)
* [Cayenne LPP](https://github.com/ElectronicCats/CayenneLPP) lub [The Things Network Arduino Library](https://github.com/TheThingsNetwork/arduino-device-lib), która zawiera CayenneLPP i jest również używana do komunikacji z RN2483A

## Komunikacja LoRaWAN
### Zastosowanie RFM95W
Komunikacja mikrokontrolera z układem SX1276 (nadajnik-odbiornik LoRa), który znajduje się na płytce RFM95W jest realizowana poprzez interfejs SPI. Szczegółowe informacje dotyczące układu transceivera SX1276 można znaleźć w [dokumentacji](https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R0000001OKs/Bs97dmPXeatnbdoJNVMIDaKDlQz8q1N_gxDcgqi7g2o). Moduł RFM95W jest połączony z Arduino Pro Mini w następujący sposób:

|RFM95W|Arduino Pro Mini|
|:----:|:--------------:|
|MOSI|11 (MOSI)|
|MISO|12 (MISO)|
|SCK|13 (SCK)|
|NSS|10|
|RESET|4|
|DIO0|5|
|DIO1|7|
|DIO2|6 (opcja - patrz JP3)|

Za realizację stosu LoRaWAN odpowiada biblioteka [Arduino-LMIC Library](https://github.com/matthijskooijman/arduino-lmic).

Pierwszym krokiem jest poinformowanie biblioteki, które piny Arduino zostały użyte do realizacji komunikacji z transceiverem.
Mapowanie pinów zgodnie z przedstawioną powyżej listą połączeń:
```c
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = { 5, 7, LMIC_UNUSED_PIN }, // lub { 5, 7, 6 } jeśli ma być wykorzystany również DIO2
};
```
Kolejnym krokiem jest przekazanie informacji o sposobie aktywacji urządzenia w sieci.
W szablonie użyto aktywacji według personalizacji (ABP - *Activation By Personalization*). Jest to najprostszy sposób aktywacji wykorzystywany do realizacji urządzenia podłączonego na stałe do jednej wybranej sieci. Urządzenie jest gotowe do pracy zaraz po rejestracji w sieci. Proces rejestracji urządzenia w sieci The Things Network jest dobrze opisany w [dokumentacji](https://www.thethingsnetwork.org/docs/devices/registration.html). Podczas rejestracji należy pamiętać o wybraniu metody aktywacji ABP.

Urządzenie posiada stały adres (*Device Address*) i zestaw kluczy sesyjnych wykorzystywanych do sprawdzania integralności wiadomości w warstwie sieci (*Network Session Key*) oraz do szyfrowania danych w warstwie aplikacji (*Application Session Key*). Zarówno adres urządzenia, jak i klucze sesji nie zmieniają się przez cały czas pracy urządzenia.<br>
Informacje uzyskane podczas rejestracji urządzenia należy wstawić w miejsce właściwych stałych:
```c
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const PROGMEM u1_t APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const u4_t DEVADDR = 0x00000000;
```
Stosowanie ABP jest mniej bezpieczne i nie jest zlecane również ze względu na ograniczenia funkcjonalne, lecz brak konieczności nawiązywania i sprawdzania połączenia z siecią może być w pewnych zastosowaniach zaletą. 

Komunikacja bezprzewodowa urządzenia z bramkami LoRaWAN odbywa się w kanałach, których środek wyznaczają ściśle określone częstotliwości. Zwykle używa się ośmiu kanałów o szerokości 125kHz, z których trzy są obowiązkowe i niezależne od sieci, a pozostałe są opcjonalne i mogą być określone indywidualnie dla każdej sieci, przy zachowaniu właściwej separacji i zasad określonych w zaleceniach dotyczących pasma.<br>
W przypadku sieci TTN stosowane są następujące kanały:
```c
// obowiązkowe
LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
// dodatkowe
LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
```
Jest jeszcze jeden dedykowany kanał (869.525MHz) o szerokości 125kHz, który służy tylko do przesyłania informacji w kierunku do urządzenia (*downlink*) w tzw. drugim oknie odbiorczym RX2. 
Parametry transmisji w tym kanale są stałe i ustawione domyślnie na SF12, czyli DR0 (*Data Rate 0*). W przypadku sieci TTN parametr ten powinien być zmieniony na SF9 (DR2):
```c 
LMIC.dn2Dr = DR_SF9;
```
To wystarczy żeby rozpocząć komunikację z wykorzystaniem The Things Network - otwartej sieci LoRaWAN.

LMIC oferuje prosty model programowania oparty na zdarzeniach, w którym wszystkie zdarzenia protokołu są wysyłane do funkcji zwrotnej `onEvent()`.
W tym modelu cały kod aplikacji jest uruchamiany w tak zwanych zadaniach (funkcjach), które są wykonywane w głównym wątku za pomocą harmonogramu `os_runloop_once()`. Do zarządzania zadaniami wymagana jest dodatkowa struktura `osjob_t`, która identyfikuje zadanie i przechowuje informacje kontekstowe. Przykładem zadania w opisywanym szablonie jest funkcja `do_send()` odpowiadająca za przygotowanie i  wysyłanie danych. Zadanie to jest wykonywane cyklicznie co określony interwał (`TX_INTERVAL`). Wysyłanie danych z urządzenia do sieci jest inicjowane wywołaniem funkcji `LMIC_setTxData2()`. Zakończenie wysyłania danych jest sygnalizowane wystąpieniem zdarzenia `EV_TXCOMPLETE` obsługiwanego w `onEvent()`. W tym miejscu planowane jest kolejne wykonanie zadania poprzez wywołanie `os_setTimedCallback()`. Można również sprawdzić, czy nie zostały odebrane informacje zwrotne przesłane z sieci do urządzenia.    

Wywołanie poniższej funkcji powoduje kompensację błędów zegara i opóźnień wprowadzanych przez program, co w rezultacie zwiększa szansę odebrania informacji zwrotnej przesyłanej do urządzenia w wyznaczonych oknach czasowych (dokładny opis problemu znajduje się w [dokumentacji LMIC](https://github.com/matthijskooijman/arduino-lmic#timing)).
```c 
LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
```

Ustawianie parametru DR (*Data Rate*) oraz mocy nadajnika umożliwia funkcja `LMIC_setDrTxpow()`. Przykład użycia tej funkcji w celu ustawienia DR0 (SF7) i mocy nadajnika na poziomie 14dBm (czyli 25mW):
```c
LMIC_setDrTxpow(DR_SF7, 14);
```

Wywołanie tej funkcji powoduje wyłączenie trybu sprawdzania połączenia z siecią, które w przypadku ABP nie ma praktycznie zastosowania:
```c
LMIC_setLinkCheckMode(0);
```

Jeśli aplikacja wykorzystuje ABP, a urządzenie nie pracuje w klasie B, to warto zaoszczędzić trochę zasobów wyłączając część kodu odpowiedzialną za obsługę nieużywanych funkcjonalności. Można tego dokonać poprzez odkomentowanie następujących definicji w pliku `src/lmic/config.h` biblioteki LMIC:
```c
// Uncomment this to disable all code related to joining
#define DISABLE_JOIN
// Uncomment this to disable all code related to ping
#define DISABLE_PING
// Uncomment this to disable all code related to beacon tracking.
// Requires ping to be disabled too
#define DISABLE_BEACONS
```
W ten sposób można zyskać nawet kilka kilobajtów pamięci programu. Dzięki temu można również usunąć z aplikacji nieużywane funkcje (ich brak nie będzie już generował błędów podczas kompilacji):
```c
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }
```
## Interwał wysyłania danych
## Rodzaj i format danych
## Oszczędzanie energii
## Wykorzystanie zasobów platformy
### Pomiar napięcia baterii
### Obsługa portu szeregowego
### Wykorzystanie diody LED
Na płytce można zamontować dodatkową diodę LED podłączoną do pinu 2 Arduino Pro Mini.
```c
#define USER_LED 2 // additional LED
```
Dioda ta może być wykorzystana w aplikacji w dowolny sposób.
W prezentowanych szablonach służy jako wskaźnik komunikacji LoRaWAN - jest włączana przed zainicjowaniem wysyłania danych i wyłączana po zakończeniu cyklu transmisji i odbioru danych. 

Sterowanie diodą LED:
```c
pinMode(USER_LED, OUTPUT);
...
digitalWrite(USER_LED, HIGH);
digitalWrite(USER_LED, LOW);
```
### Sterowanie zasilaniem układów zewnętrznych

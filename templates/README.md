# Szablony aplikacji

Zastosowanie szablonów ma na celu przyspieszenie procesu tworzenia własnych aplikacji. Wystarczy dokonać wstępnej parametryzacji i skupić się na implementacji docelowych funkcjonalności. Szablony aplikacji zapewniają realizację komunikacji LoRaWAN, wprowadzanie układu w stan uśpienia oraz ułatwiają wykorzystanie zasobów platformy w wybranym wariancie konfiguracji. 

Dostępne wersje szablonów:
* `lorawan-rfm-template.ino` - dla wariantu 1 konfiguracji (Arduino Pro Mini i RFM95W)
* `lorawan-rn-template.ino` - dla wariantu 2 konfiguracji (Arduino Pro Mini i RN2483A)

## Wymagania związane z oprogramowaniem
Wykorzystane biblioteki:
* [Arduino-LMIC Library](https://github.com/matthijskooijman/arduino-lmic)
* [Low-Power](https://github.com/rocketscream/Low-Power)
* [Cayenne LPP](https://github.com/ElectronicCats/CayenneLPP) lub [The Things Network Arduino Library](https://github.com/TheThingsNetwork/arduino-device-lib), która zawiera CayenneLPP i jest również używana do komunikacji z RN2483A

## Komunikacja LoRaWAN
## Interwał wysyłania danych
## Rodzaj i format danych
## Oszczędzanie energii
## Wykorzystanie zasobów platformy
### Pomiar napięcia baterii
### Obsługa portu szeregowego
### Wykorzystanie diody LED 
### Sterowanie zasilaniem układów zewnętrznych

# lora-dev-board

Uniwersalna platforma sprzętowa ułatwiająca projektowanie i testowanie urządzeń IoT wykorzystujących do komunikacji sieć LoRaWAN.

## Cele projektu

* Udostępnienie gotowego do wykorzystania projektu płytki PCB
* Dostarczenie przykładowych szablonów aplikacji pozwalających na programowanie płytki przy użyciu Arduino SDK

## Założenia

Platforma została zaprojektowana tak, aby jej zbudowanie i uruchomienie było możliwe dla hobbystów posiadających podstawowe umiejętności w zakresie budowanie układów elektronicznych oraz programowania. Zgodnie z tym  przyjęto też dodatkowe założenia: 

* Zastosowanie popularnej płytki Arduino Pro Mini z mikrokontrolerem ATmega328P
* Współpraca z najczęściej stosowanymi układami nadawczo-odbiorczymi LoRa: RFM95W lub RN2483
* Wykorzystanie różnych źródeł zasilania, w tym standardowych baterii alkalicznych, baterii litowych oraz akumulatorów litowo-polimerowych lub litowo-jonowych
* Możliwość ładowania baterii litowo-polimerowych lub litowo-jonowych z wykorzystaniem paneli fotowoltaicznych małej mocy lub poprzez grniazdo USB

## Jak zacząć?

Poniższe punkty zostaną wkrótce uzupełnione opisami.

* Projekt płytki: [kompletacja elementów](https://github.com/Open-Things-Network/lora-dev-board/blob/master/v1.0/README.md#lista-elementów) i budowa urządzenia
* Uruchomienie bazowej aplikacji wysyłającej informacje o stanie urządzenia

#include <TheThingsNetwork.h>
#include <SoftwareSerial.h>
#include <CayenneLPP.h>
#include <LowPower.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_BME280.h>

#define LOW_POWER

#define USER_LED 2
#define ONE_WIRE_BUS 9

#define VBAT (analogRead(A0) * 0.013) // napięcie z baterii mierzone na wejściu A0 (dzielnik R4, R5 i VREF = 3.3V)

#define DS18_TEMPERATURE_CHANNEL 11
#define RN_RESET_PIN 4
#define debugSerial Serial
SoftwareSerial loraSerial(6, 7); // port do komunikacji z RN2483 (RX, TX)

// tryb OTAA - AppEUI i AppKey
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

TheThingsNetwork ttn(loraSerial, debugSerial, TTN_FP_EU868, 11);

// interwał wysyłania danych w ms
const unsigned long TX_INTERVAL = 900000;

// bufor danych (maksymalnie 51 bajtów)
CayenneLPP lppdata(51);

// czujnik BME280 (I2C)
Adafruit_BME280 bme;   
bool bmeFound;

// czujnik temperatury DS18B20 (1-Wire)
OneWire ow(ONE_WIRE_BUS);
DallasTemperature ds(&ow);
bool dsFound;

unsigned long start_time;

// skanowanie magistrali 1-Wire - wyszukiwanie podłączonych czujników DS18xxx
void owScan()
{
  DeviceAddress address;
  dsFound = false;
  
  ds.begin();
  if (ds.getDeviceCount() == 0)
    return;

  for (int i = 0; i < ds.getDeviceCount(); i++)
  {
    if (ds.getAddress(address, i))
    {
      Serial.print("Device found @ ");
      for (int j = 0; j < 8; j++)
        printHex(address[j]);
      Serial.println();
    }
  }
  if (ds.getDS18Count() > 0)
    dsFound = true;
}

// pobieranie danych z czujników i wrzucanie do bufora
void getData()
{
  // czyszczenie bufora
  lppdata.reset();

  float temperature;
  // odczyt danych z czujnika BME280
  if (bmeFound)
  {
    temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F; // hPa
    float humidity = bme.readHumidity();
    
    Serial.print("Temperatures (°C): ");
    Serial.println(temperature);
    Serial.print("Pressure (hPa): ");
    Serial.println(pressure);
    Serial.print("Humidity (%): ");
    Serial.println(humidity);
  
    lppdata.addTemperature(1, temperature);
    lppdata.addRelativeHumidity(2, humidity);
    lppdata.addBarometricPressure(3, pressure);
  }

  // napięcie baterii
  lppdata.addAnalogInput(4, VBAT);
  
  // odczyt danych z czujników DS18B20
  if (dsFound)
  {
    DeviceAddress address;
    int channel = DS18_TEMPERATURE_CHANNEL;
    for (int i = 0; i < ds.getDeviceCount(); i++)
    {
      if (!ds.getAddress(address, i))
        continue;
      
      if (!ds.validFamily(address))
        continue;

      ds.requestTemperaturesByAddress(address);
      temperature = ds.getTempC(address);
      if (temperature == DEVICE_DISCONNECTED_C)
        continue;
      
      Serial.print("DS18B20[");
      Serial.print(i);
      Serial.print("] temperature (°C): ");
      Serial.println(temperature);

      if (lppdata.addTemperature(channel++, temperature) == 0)
          break;
    }
  }
}

void setup()
{
  pinMode(USER_LED, OUTPUT);

  debugSerial.begin(9600);
  // oczekiwanie na Serial Monitor
  while (!debugSerial && millis() < 1000);

  Serial.println("--- START");
  
  // czujnika BME280
  if (bme.begin(0x76))
  { 
    bmeFound = true;
    Serial.println("BME280 found");
  }

  // czujniki temperatury DS18B20
  owScan();
   
  // uruchomienie komunikacji z RN2483
  loraSerial.begin(9600);
  ttn.wake();
  ttn.resetHard(RN_RESET_PIN);
  ttn.reset(false);
  ttn.onMessage(message);

  // odczyt i wyświetlanie informacji z RN2483
  Serial.println("--- STATUS");
  ttn.showStatus();

  // wyłączenie ADR
  ttn.setADR(false);
  
  // podłączenie do sieci 
  Serial.println("--- JOIN");
  ttn.join(appEui, appKey);
}

void loop() 
{
  // wysyłanie danych
  start_time = millis();
  Serial.println("--- SEND");
  digitalWrite(USER_LED, HIGH);
  getData();
  ttn.sendBytes(lppdata.getBuffer(), lppdata.getSize());
  digitalWrite(USER_LED, LOW);

#ifdef LOW_POWER
  // wprowadzenie w tryb uśpienia
  ttn.sleep((unsigned long)TX_INTERVAL);
  Serial.flush();
  for (int i = 0; i < int((TX_INTERVAL / 1000) / 8); i++) 
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  // wybudzenie RN2483
  ttn.wake();
#else
  while (millis() - start_time < TX_INTERVAL)
  {
    delay(1);
  }
#endif
}

static void printHex(uint8_t b) 
{
  if (b < 0x10)
    Serial.print('0');
  Serial.print(b, HEX);
}

void message(const uint8_t *payload, size_t size, port_t port)
{
  Serial.println("--- MESSAGE");
  Serial.print("Received " + String(size) + " bytes on port " + String(port) + ": ");
  for (int i = 0; i < size; i++)
  {
    printHex(payload[i]);
  }
  Serial.println();
}

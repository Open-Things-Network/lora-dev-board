/*
 * Application template for lora-dev-board
 * ---------------------------------------
 * Board version: 1.0
 * Board config: Arduino Pro Mini + RFM95W
 * ---------------------------------------
 */
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <CayenneLPP.h>
#include <LowPower.h>

#include <BH1750.h>
BH1750 lightSensor;

//#include <i2c_BMP280.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;

#include <sds.h>

// #define JUST_SEND_HELLO 
#define SAVE_ENERGY


// ------- inputs, outputs and interfaces (see lora-dev-board circuit diagram)

#define USER_LED 2 // additional LED
#define VOUT_ENABLE 3 // output voltage (TP5) control (VCC/VBAT - depends on JP9)

SoftwareSerial userSerial(8, 9); // GPSP available on J3 (RX, TX)
SDS sds(userSerial);
SDS::DATA data;

#define VBAT (analogRead(A0) * 0.00424568) // battery voltage measured on A0

// ------- LoRaWAN configuration

// ABP mode - set DevAddr and session keys
static const PROGMEM u1_t NWKSKEY[16] = { 0x20, 0x61, 0x4F, 0x03, 0xD9, 0x6F, 0x7C, 0x7E, 0xAA, 0x31, 0xBC, 0x81, 0x94, 0xBA, 0xFE, 0xAD };
static const PROGMEM u1_t APPSKEY[16] = { 0x75, 0x02, 0xED, 0x8D, 0xF2, 0xA6, 0x20, 0xB8, 0xA2, 0x2D, 0x63, 0x8C, 0xBA, 0xBA, 0x94, 0x29 };
static const u4_t DEVADDR = 0x26011FAA;

// leave empty callbacks (only used in OTAA) 
//void os_getArtEui (u1_t* buf) { }
//void os_getDevEui (u1_t* buf) { }
//void os_getDevKey (u1_t* buf) { }

// pin mapping for SX1276 (see lora-dev-board circuit diagram)
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = { 5, 7, LMIC_UNUSED_PIN }, // or { 5, 7, 6 } if JP3 is in DIO2 position
};

// ------- tasks, intervals, buffers, etc.

// data send task
static osjob_t sendjob;

// data send interval in seconds
const unsigned TX_INTERVAL = 900;

// data buffer
#ifdef JUST_SEND_HELLO
  static uint8_t hello[] = "Hello!";
#else
  CayenneLPP lppdata(51);
#endif

// -------  

void getData()
{
#ifndef JUST_SEND_HELLO  

  // Light sensor
  uint16_t lux = lightSensor.readLightLevel();    //odczytanie wartości z czujnika

  // odczyt czujnika BME280
  float temperature = bme280.readTemperature();
  float pressure = bme280.readPressure() / 100.0F; // hPa
  float humidity = bme280.readHumidity();

  digitalWrite(VOUT_ENABLE, HIGH);
  // SDS wake up
  sds.wakeUp();
  delay(30000); 

  // read from SDS011
  bool sds_ok = false;
  sds.requestRead();
  if (sds.readUntil(data))
  {
    sds_ok = true;
  }
  
//  // SDS sleep mode
  sds.sleep();
  digitalWrite(VOUT_ENABLE, LOW);
  
  lppdata.reset();
  // here you can read data from sensors and prepare buffer to send
  lppdata.addAnalogInput(1, VBAT); // add battery voltage
  lppdata.addTemperature(2, temperature);
  lppdata.addRelativeHumidity(3, humidity);
  lppdata.addBarometricPressure(4, pressure);
  if (sds_ok) 
  {
    lppdata.addAnalogInput(5, data.pm_25 / 10.0);
    lppdata.addAnalogInput(6, data.pm_100 / 10.0);
  }
  lppdata.addLuminosity(7, lux);

#endif
}

void do_send(osjob_t* j)
{ 
  Serial.println("--- SEND");
  if (LMIC.opmode & OP_TXRXPEND) 
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } 
  else 
  {
    digitalWrite(USER_LED, HIGH);
#ifdef JUST_SEND_HELLO
    LMIC_setTxData2(1, hello, sizeof(hello) - 1, 0);
#else 
    getData();
    LMIC_setTxData2(1, lppdata.getBuffer(), lppdata.getSize(), 0);
#endif
    Serial.println(F("Packet queued"));
  }
}

void onEvent(ev_t ev) 
{
//  Serial.print(os_getTime());
//  Serial.print(": ");
  switch(ev) 
  {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
      {
        Serial.println(F("Received ack"));
      }
      if (LMIC.dataLen) 
      {
        Serial.println("--- MESSAGE");
        uint8_t port = LMIC.frame[LMIC.dataBeg - 1];
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.print(F(" bytes on port "));
        Serial.print(port);
        Serial.print(": ");
        for (int i = 0; i < LMIC.dataLen; i++)
        {
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
        }
        Serial.println();
      }
      digitalWrite(USER_LED, LOW);
#ifdef SAVE_ENERGY
      Serial.flush();
      for (int i = 0; i < int(TX_INTERVAL/8); i++) 
      {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      }
      do_send(&sendjob);
#else
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
#endif
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      Serial.println((unsigned) ev);
      break;
  }
}

void setup() 
{
  Serial.begin(9600);
  userSerial.begin(9600);
  
  pinMode(USER_LED, OUTPUT);
  pinMode(VOUT_ENABLE, OUTPUT);
    
  lightSensor.begin();

  bme280.begin(0x76);

  sds.passiveMode();

  Serial.println("--- PERSONALIZE");
  os_init();
  LMIC_reset();
#ifdef PROGMEM
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI); 
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setLinkCheckMode(0);
  LMIC.dn2Dr = DR_SF9;
  LMIC_setDrTxpow(DR_SF7, 14);

  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
  
  do_send(&sendjob);
}

void loop() 
{  
  os_runloop_once();

  // user serial port test
  if (userSerial.available())
  {
    Serial.write(userSerial.read());
  }
  if (Serial.available())
  {
    userSerial.write(Serial.read());
  }
}

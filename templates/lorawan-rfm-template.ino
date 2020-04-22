/*
Application template for lora-dev-board with Arduino Pro Mini
-------------------------------------------------------------
Board version:       1.0
Board configuration: RFM95W
*/
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <CayenneLPP.h>
#include <LowPower.h>

#define JUST_SEND_HELLO
#define SAVE_ENERGY


// ------- outputs and interfaces (see lora-dev-board scheme)
#define USER_LED 2 
#define VOUT 3 
SoftwareSerial userSerial(8, 9); // RX, TX

// ------- LoRaWAN configuration

// ABP mode - set DevAddr and session keys
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const PROGMEM u1_t APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const u4_t DEVADDR = 0x00000000;

// leave empty callbacks (only used in OTAA) 
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// pin mapping for SX1276 (see lora-dev-board scheme)
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = { 5, 7, LMIC_UNUSED_PIN },
};

// ------- tasks, intervals, buffers, etc.

// data send task
static osjob_t sendjob;

// data send interval in seconds
const unsigned TX_INTERVAL = 60;

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
  lppdata.reset();
  // here you can read data from sensors and prepare buffer to send
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
  Serial.print(os_getTime());
  Serial.print(": ");
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
  pinMode(VOUT, OUTPUT);

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

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <CayenneLPP.h>
#include <LowPower.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SAVE_ENERGY

// ------- inputs, outputs and interfaces (see lora-dev-board circuit diagram)

#define USER_LED 2 // additional LED
#define VOUT_ENABLE 3 // output voltage (TP5) control (VCC/VBAT - depends on JP9)

// battery voltage measured on A0
#define VBAT (analogRead(A0) * 0.00424568)

// 1-wire bus pin
#define ONE_WIRE_BUS 9 

// ------- LoRaWAN configuration

// Application EUI (LSB)
static const u1_t PROGMEM APPEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// Device EUI (LSB) 
static const u1_t PROGMEM DEVEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// Application Key (MSB)
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

// pin mapping
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
const unsigned TX_INTERVAL = 60;

// temperature sensors DS18xxx (1-Wire)
OneWire ow(ONE_WIRE_BUS);
DallasTemperature ts(&ow);
bool tsFound;

// data buffer
CayenneLPP lppdata(16);

// -------  

// 1-wire bus scanning for temperature sensors
void owScan()
{
  DeviceAddress address;
  tsFound = false;
  
  ts.begin();
  
  if (ts.getDeviceCount() == 0)
    return;
  
  if (ts.getDS18Count() > 0) 
    tsFound = true;
}

void getData()
{
  lppdata.reset();
  
  // here you can read data from sensors and prepare buffer to send

  // add battery voltage
  lppdata.addAnalogInput(1, VBAT); 

  // add temperatures
  if (tsFound)
  {
    DeviceAddress address;
    float temperature;
    uint8_t channel = 2;
    for (int i = 0; i < ts.getDeviceCount(); i++)
    {
      if (!ts.getAddress(address, i))
        continue;
      
      if (!ts.validFamily(address))
        continue;

      ts.requestTemperaturesByAddress(address);
      temperature = ts.getTempC(address);
      if (temperature == DEVICE_DISCONNECTED_C)
        continue;

      if (lppdata.addTemperature(channel, temperature) == 0)
          break;
    }
  }
}

void do_send(osjob_t* j)
{
  if (LMIC.opmode & OP_TXRXPEND) 
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } 
  else 
  {
    digitalWrite(USER_LED, HIGH);
    getData();
    LMIC_setTxData2(1, lppdata.getBuffer(), lppdata.getSize(), 0);
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
      u4_t netid = 0;
      devaddr_t devaddr = 0;
      u1_t nwkKey[16];
      u1_t artKey[16];
      LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
      Serial.print("netid: ");
      Serial.println(netid, DEC);
      Serial.print("devaddr: ");
      Serial.println(devaddr, HEX);
      Serial.println();
      LMIC_setLinkCheckMode(0);
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
        Serial.println(F("Received ack"));
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
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
        Serial.println();
      }
      digitalWrite(USER_LED, LOW);
#ifdef SAVE_ENERGY
      Serial.flush();
      for (int i = 0; i < int(TX_INTERVAL/8); i++) 
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
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
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      break;
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
}

void setup() 
{
  Serial.begin(9600);

  pinMode(USER_LED, OUTPUT);
  pinMode(VOUT_ENABLE, OUTPUT);
  
  owScan();

  os_init();
  LMIC_reset();

  LMIC_setAdrMode(false);
  LMIC.dn2Dr = DR_SF9;        
  LMIC_setDrTxpow(DR_SF7, 14);

  LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
  
  do_send(&sendjob);
}

void loop() 
{
  os_runloop_once();
}

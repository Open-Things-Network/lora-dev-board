/*
 * Application template for lora-dev-board
 * ---------------------------------------
 * Board version: 1.0
 * Board config: Arduino Pro Mini + RN2483
 * ---------------------------------------
 */
#include <TheThingsNetwork.h>
#include <SoftwareSerial.h>
#include <CayenneLPP.h>
#include <LowPower.h>

#define JUST_SEND_HELLO
#define SAVE_ENERGY
#define JOIN_MODE_OTAA


// ------- inputs, outputs and interfaces (see lora-dev-board circuit diagram)

#define USER_LED 2 // additional LED
#define VOUT_ENABLE 3 // output voltage (TP5) control (VCC/VBAT - depends on JP9)

SoftwareSerial userSerial(8, 9); // GPSP available on J3 (RX, TX)
SoftwareSerial rnSerial(6, 7); // SP for RN2483 (RX, TX)

#define VBAT (analogRead(A0) * 0.00424568) // battery voltage measured on A0

// ------- LoRaWAN configuration

#define freqPlan TTN_FP_EU868

#ifdef JOIN_MODE_OTAA
  // OTAA mode - set AppEUI and AppKey
  const char *appEUI = "0000000000000000";
  const char *appKey = "00000000000000000000000000000000";
#else
  // ABP mode - set DevAddr and session keys
  const char *devAddr = "00000000";
  const char *nwkSKey = "00000000000000000000000000000000";
  const char *appSKey = "00000000000000000000000000000000";
#endif

// RN2483
TheThingsNetwork ttn(rnSerial, Serial, freqPlan);

// ------- intervals, buffers, etc.

// data send interval in seconds
const unsigned int TX_INTERVAL = 60;

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
  lppdata.addAnalogInput(1, VBAT); // add battery voltage
#endif
}

void setup() 
{
  Serial.begin(9600);
  userSerial.begin(9600);
  
  pinMode(USER_LED, OUTPUT);
  pinMode(VOUT_ENABLE, OUTPUT);
  
  rnSerial.begin(9600);
  ttn.wake();

  ttn.onMessage(message);

#ifdef JOIN_MODE_OTAA
  Serial.println("--- STATUS");
  ttn.showStatus();
  Serial.println("--- JOIN");
  ttn.join(appEUI, appKey);
#else
  Serial.println("--- PERSONALIZE");
  ttn.personalize(devAddr, nwkSKey, appSKey);
  Serial.println("--- STATUS");
  ttn.showStatus();
#endif
}

void loop() 
{
  Serial.println("--- SEND");
  digitalWrite(USER_LED, HIGH);
#ifdef JUST_SEND_HELLO
  ttn.sendBytes(hello, sizeof(hello) - 1);
#else
  getData();
  ttn.sendBytes(lppdata.getBuffer(), lppdata.getSize());
#endif
  digitalWrite(USER_LED, LOW);

#ifdef SAVE_ENERGY
  ttn.sleep((unsigned long)TX_INTERVAL * 1000);
  Serial.flush();
  for (int i = 0; i < int(TX_INTERVAL / 8); i++) 
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  ttn.wake();
#else
  delay(TX_INTERVAL);
#endif
}

void message(const uint8_t *payload, size_t size, port_t port)
{
  Serial.println("--- MESSAGE");
  Serial.print("Received " + String(size) + " bytes on port " + String(port) + ": ");
  for (int i = 0; i < size; i++)
  {
    Serial.print(payload[i], HEX);
  }
  Serial.println();
}

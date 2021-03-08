/*

Module:  catena_simple_host.ino

Function:
        simple_host.ino example converted to use Catena-Arduino-Platform
        framework, and pollable interface.

Copyright notice and License:
        See LICENSE file accompanying this project.

Author:
        Dhinesh Kumar Pitchai, MCCI Corporation   January 2021

*/

// to compile this, you must install the Catena-Arduino-Platform library:
// see https://github.com/mcci-catena/Catena-Arduino-Platform

#include <Catena.h>
#include <MCCI_Catena_SM70.h>

using namespace McciCatena;
using namespace McciCatenaSm70;

// the framework object.
Catena gCatena;

cSerial<decltype(Serial2)> sm70Serial(&Serial2);
cSM70 gSm70(&sm70Serial, D12, D12); // this is host and RS-232 or USB-FTDI

cSM70::HRequest_t hRequestType;
cSM70::Error errorType;

constexpr uint8_t kFramPowerOn = D10;
constexpr uint8_t kRs485PowerOn = D11;
constexpr uint8_t kBoosterPowerOn = D5;

static inline void powerOn(void)
	{
	pinMode(kRs485PowerOn, OUTPUT);
	digitalWrite(kRs485PowerOn, HIGH);
	}

static inline void powerOff(void)
	{
	pinMode(kRs485PowerOn, INPUT);
	digitalWrite(kRs485PowerOn, LOW);
	}

static inline void fRAMpowerOn(void)
	{
	pinMode(kFramPowerOn, OUTPUT);
	digitalWrite(kFramPowerOn, HIGH);
	}

static inline void fRAMpowerOff(void)
	{
	pinMode(kFramPowerOn, INPUT);
	digitalWrite(kFramPowerOn, LOW);
	}

static inline void boostpowerOn(void)
	{
	pinMode(kBoosterPowerOn, OUTPUT);
	digitalWrite(kBoosterPowerOn, HIGH);
	}

static inline void boostpowerOff(void)
	{
	pinMode(kBoosterPowerOn, INPUT);
	digitalWrite(kBoosterPowerOn, LOW);
	}

unsigned long u32wait;

void setup() {
	fRAMpowerOn();		// FRAM/Flash Power On
	powerOn();		// turn on the transceiver.
	boostpowerOff();	// turn off the boost regulator
	gCatena.begin();	// set up the framework
	gSm70.begin();	// baud-rate at 4800
}

void loop() {
	gCatena.poll(); // check incoming messages & drive things along.
  gSm70.poll();
}

/*void startReadData_cmpltn(hRequestType hRequest, void *pUserData, errorType errcode){

}

void startReadInfo_cmpltn(hRequestType hRequest, void *pUserData, errorType errcode){
	
}*/

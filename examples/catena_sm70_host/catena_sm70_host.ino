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
// this is host and RS-232 or USB-FTDI
cSM70 gSm70(&sm70Serial, D12, D12); // for 4801 pin D12 is Tx and Rx

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
	gSm70.begin(gCatena);	// start and initialize SM70
}

void startReadData_cmpltn(cSM70::HRequest_t hRequest, void *pUserData, cSM70::Error errcode)
	{
	if (errcode == cSM70::Error::kOk)
		{
		gCatena.SafePrintf("Data is valid");
		gCatena.SafePrintf("Data: %u", pUserData);
		}
	else
		gCatena.SafePrintf("Data is not valid");
	}

void startReadInfo_cmpltn(cSM70::HRequest_t hRequest, void *pUserData, cSM70::Error errcode)
	{
	if (errcode == cSM70::Error::kOk)
		{
		gCatena.SafePrintf("Sensor info is valid");
		gCatena.SafePrintf("Sensor info: %u", pUserData);
		}
	else
		gCatena.SafePrintf("Sensor info is not valid");
	}

void loop() {
	gCatena.poll(); // check incoming messages & drive things along.

	gCatena.SafePrintf("Start Read Data to be called\n");
	gSm70.startReadData(startReadData_cmpltn, nullptr);
	delay(5000);
	
	gCatena.SafePrintf("Start Read Info to be called\n");
	gSm70.startReadInfo(startReadInfo_cmpltn, nullptr);
	delay(5000);
}

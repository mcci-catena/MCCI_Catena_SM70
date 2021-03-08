/*

Module:  MCCI_Catena_SM70.cpp

Function:
	Header file for MCCI Catena library for Aeroqual SM70 Ozone sensor

Copyright and License:
	This file copyright (C) 2021 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	See accompanying LICENSE file for copyright and license information.

Author:
	Dhinesh Kumar Pitchai, MCCI Corporation	January 2021

*/

#include "MCCI_Catena_SM70.h"

using namespace McciCatenaSm70;

bool cSM70::begin(){
    if (! this->m_flags.b.Registered)
        {
        this->m_pSerial->registerPollableObject(this);
        this->m_flags.b.Registered = true;
        }

    if (! this->m_flags.b.Running)
        {
        // start the FSM
        this->m_flags.b.TxEnabled = false;
        this->m_flags.b.RxEnabled = false;
        this->m_flags.b.Exit = false;
        this->m_fsm.init(*this, &cSM70::fsmDispatch);
        }

    return true;
    }

void cSM70::end(){
    if (this->m_flags.b.Running)
        {
        this->m_flags.b.Exit = true;
        while (this->m_flags.b.Running)
            this->m_fsm.eval();
        }
}

void cSM70::poll(){
    if(this->m_rxEnPin){
		int current = m_pSerial->available();

		if (!current)
			this->fsmEval();
		}
	}

void cSM70::cancel(HRequest_t hRequest){

}

cSM70::HRequest_t cSM70::startReadData(CompletionFn *pDoneFn, void *pUserData){
	this->m_pSerial->write(this->m_DataRequest.getPointer(), 4);
	this->readData();
    // return
}

cSM70::HRequest_t cSM70::startReadInfo(CompletionFn *pDoneFn, void *pUserData){
	this->m_pSerial->write(this->m_SensorInfoRequest.getPointer(), 4);
	this->readInfo();
	// return
}

cSM70::Error cSM70::readData(){
	uint8_t * pDataReport = this->m_DataReport.getPointer();
	uint8_t dataReportIndex = 0;

	while(dataReportIndex < 15){
		*(pDataReport + dataReportIndex) =  this->m_pSerial->read();
		++dataReportIndex;
		}

	return this->m_DataReport.isValid();
}

cSM70::Error cSM70::readInfo(){
	uint8_t * pSensorInfo = this->m_SensorInfo.getPointer();
	uint8_t sensorInfoIndex = 0;

	while(sensorInfoIndex < 15){
		*(pSensorInfo + sensorInfoIndex) =  this->m_pSerial->read();
		++sensorInfoIndex;
		}

	return this->m_SensorInfo.isValid();
}

/*

Name:   cSM70::fsmDispatch()

Function:
    Handle the FSM updates for SM70

Definition:
    cSM70::State
			cSM70::fsmDispatch(
				cSM70::State curState,
				bool fEntry
				);

Description:
    The lower SM70 engine is polled, and if an event
    occurs, we call the SM70 FSM to move things along.

Returns:
    No result is returned.

*/

cSM70::State
cSM70::fsmDispatch(
    cSM70::State curState,
    bool fEntry
    )
    {
    State newState;

    newState = State::stNoChange;

    switch (curState)
        {
    case State::stInitial:
        newState = State::stNormal;
        break;

    case State::stNormal:
        {
        if (fEntry)
            {
            /* state entry */;
            }

        this->m_pSerial->begin(this->kBaud);
        newState = State::stDataRequest;
        }
        break;

    case State::stDataRequest:
        {
        if (fEntry)
            {
            /* nothing */
            }

        newState = State::stSensorInfoRequest;
        }
        break;

    case State::stSensorInfoRequest:
        {
        if (fEntry)
            {
            /* nothing */
            }

        newState = State::stFinal;
        }
        break;

    case State::stFinal:
        {
		this->m_pSerial->end();
		this->m_flags.b.TxEnabled = false;
		this->m_flags.b.RxEnabled = false;
        this->m_flags.b.Running = false;
        }
        break;

    default:
        newState = State::stInitial;
        break;
        }

    return newState;
    }
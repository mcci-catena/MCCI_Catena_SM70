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
#include <CatenaBase.h>

using namespace McciCatenaSm70;

bool cSM70::begin(McciCatena::CatenaBase &rCatena){
    if (! this->m_flags.b.Registered)
        {
        rCatena.registerObject(this);
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

    if (! this->m_flags.b.RequestsInitialized)
        {
        this->m_RqPool.init();
        this->m_flags.b.RequestsInitialized = true;
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
    cSM70::Request request_t;

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
        newState = State::stCheckPendingRequest;
        }
        break;

    case State::stCheckPendingRequest:
        {
        if (fEntry)
            {
            /* nothing */
            }

        if(request_t.pNext != nullptr)
            {
            if(request_t.requestCode == cSM70::RequestCode_t::kReadData)
                newState = State::stDataRequest;

            else if(request_t.requestCode == cSM70::RequestCode_t::kReadData)
                newState = State::stSensorInfoRequest;

            else
                newState = State::stCheckPendingRequest;
            }

        else
            newState = State::stFinal;
        }
        break;

    case State::stDataRequest:
        {
        if (fEntry)
            {
            /* nothing */
            }

        newState = State::stCheckPendingRequest;
        }
        break;

    case State::stSensorInfoRequest:
        {
        if (fEntry)
            {
            /* nothing */
            }

        newState = State::stCheckPendingRequest;
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


void cSM70::RqPool_t::init()
    {
    for (auto p = &this->m_Requests[0]; p < this->m_Requests + this->knRequests - 1; ++p)
        {
        this->free(p);
        }
    }


void cSM70::RqPool_t::free(Request *pRequest)
    {
    Request *pTemp;

    if (pRequest != NULL)
        {
        pRequest->pLast = NULL;
        pRequest->pNext = NULL;
        pRequest->pDoneFn = NULL;
        pRequest->pUserData = NULL;
        }
    else
        return;

    if(this->m_pFree == NULL)
        {
        this->m_pFree = pRequest;
        }
    else
        {
        pTemp = this->m_pFree;
        while(pTemp->pNext != NULL)
            {
            pTemp = pTemp->pNext;
            }

        pRequest->pLast = pTemp;
        pTemp->pNext = pRequest;
        }
    }

cSM70::Request* cSM70::RqPool_t::allocate()
        {
        Request *pTemp = this->m_pFree;
        Request *pLast;
        while(pTemp->pNext != NULL)
            {
            pLast = pTemp;
            pTemp = pTemp->pNext;
            }
        pLast->pNext = NULL;
        return pTemp;
        }

bool cSM70::RqPool_t::addPending(Request *pRequest)
    {
    Request *pTemp;

    if(this->m_pPending == NULL)
        {
        this->m_pPending = pRequest;
        this->m_pCurrent = pRequest;
        return true;
        }
    else
        {
        pTemp = this->m_pPending;
        while(pTemp->pNext != NULL)
            {
            pTemp = pTemp->pNext;
            }

        pRequest->pLast = pTemp;
        pTemp->pNext = pRequest;
        }
    }

bool cSM70::RqPool_t::freeCurrent()
    {
    Request *pCurrent;

    if(this->m_pCurrent == NULL)
        {
        return false;
        }
    pCurrent = this->m_pCurrent;

    /*No Next pending Request */
    if(this->m_pPending->pNext == NULL)
        {
        this->m_pPending = NULL;
        this->m_pCurrent = NULL;
        }
    else /*make next pending as pending head and current */
        {
        this->m_pPending = this->m_pPending->pNext;
        this->m_pCurrent = this->m_pPending->pNext;
        }

    this->free(pCurrent);
    return true;
    }
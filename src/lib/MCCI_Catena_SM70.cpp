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
        pinMode(this->m_txEnPin, OUTPUT);
        pinMode(this->m_rxEnPin, OUTPUT);
        digitalWrite(this->m_txEnPin, 1);
        digitalWrite(this->m_rxEnPin, 1);
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
			this->m_fsm.eval();
		}
	}

void cSM70::cancel(HRequest_t hRequest){

}

cSM70::HRequest_t cSM70::startReadData(CompletionFn *pDoneFn, void *pUserData){
    Request *pReadRequest = this->m_RqPool.allocate();
    pReadRequest->pDoneFn = pDoneFn;
    pReadRequest->pUserData = pUserData;
    pReadRequest->requestCode = RequestCode_t::kReadData;
    this->m_RqPool.addPending(pReadRequest);

    return pReadRequest;
    }

cSM70::HRequest_t cSM70::startReadInfo(CompletionFn *pDoneFn, void *pUserData){
    Request *pReadRequest = this->m_RqPool.allocate();
    pReadRequest->pDoneFn = pDoneFn;
    pReadRequest->pUserData = pUserData;
    pReadRequest->requestCode = RequestCode_t::kReadInfo;
    this->m_RqPool.addPending(pReadRequest);

    return pReadRequest;
    }

/*
cSM70::Error cSM70::readData(){
    auto * pDataReport = this->m_DataReport.getPointer();
    uint8_t dataReportIndex = 0;

    while(dataReportIndex < 15){
        *(pDataReport + dataReportIndex) =  this->m_pSerial->read();
        ++dataReportIndex;
        }

    return this->m_DataReport.isValid();
    }

cSM70::Error cSM70::readInfo(){
    auto * pSensorInfo = this->m_SensorInfo.getPointer();
    uint8_t sensorInfoIndex = 0;

    while(sensorInfoIndex < 15){
        *(pSensorInfo + sensorInfoIndex) =  this->m_pSerial->read();
        ++sensorInfoIndex;
        }

    return this->m_SensorInfo.isValid();
    } */

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
            newState = State::stCheckPendingRequest;
            }
            break;

        case State::stCheckPendingRequest:
            {
            if (fEntry)
                {
                /* nothing */
                }

            if (this->m_RqPool.m_pCurrent != nullptr)
                {
                Request * const pCurrent = this->m_RqPool.m_pCurrent;

                if (pCurrent->requestCode == cSM70::RequestCode_t::kReadData)
                    {
                    // send the data request. Call a function that will:
                    // 1. turn on the transceiver
                    // 2. send the pre-canned data reqeust
                    this->m_txEmptyBytes = this->m_pSerial->availableForWrite();

                    startTransaction(this->m_DataRequest);
                    newState = State::stSendingRequest;
                    }

                else if (pCurrent->requestCode == cSM70::RequestCode_t::kReadInfo)
                    {
                    // send the info request. Call a function that will:
                    // 1. turn on the transceiver
                    // 2. send the pre-canned info request
                    this->m_txEmptyBytes = this->m_pSerial->availableForWrite();

                    startTransaction(this->m_SensorInfoRequest);
                    newState = State::stSendingRequest;
                    }

                else

                    newState = State::stCheckPendingRequest;
                }

            else
                newState = State::stFinal;
            }
            break;

        case State::stSendingRequest:
            {
            if (fEntry)
                {
                }

            if (this->m_pSerial->availableForWrite() == this->m_txEmptyBytes)
                {
                // tx is empty. Wait for UART to drain
                newState = State::stDrainTx;
                }
            }
            break;

        case State::stDrainTx:
            {
            if (fEntry)
                {
                // wait 20 bit times
                this->m_txEmptyMicros = micros();
                }

            if (micros() - this->m_txEmptyMicros > 1000000 / this->kBaud * 20)
                newState = State::stEnableRx;
            }
            break;

        case State::stEnableRx:
            {
            Request * const pCurrent = this->m_RqPool.m_pCurrent;

            if (fEntry)
                {
                digitalWrite(this->m_txEnPin, 0);
                digitalWrite(this->m_rxEnPin, 0);
                this->m_rxEnableMicros = micros();
                this->m_pSerial->drainRead();
                }

            // consume characters
            while (this->m_pSerial->available() > 0 && pCurrent->nActual < pCurrent->nBuffer)
                {
                pCurrent->pBuffer[pCurrent->nActual] = this->m_pSerial->read();
                ++pCurrent->nActual;
                }

            // otherwise check for timeout
            if (pCurrent->nActual >= pCurrent->nBuffer)
                newState = State::stRequestDone;
            else if (millis() - this->m_rxEnableMicros > 100000 / this->kBaud * 20)
                newState = State::stRequestDone;
            // otherwise no change.
            break;
            }

        case State::stRequestDone:
            {
            Request * const pCurrent = this->m_RqPool.m_pCurrent;

            if (fEntry)
                {
                digitalWrite(this->m_txEnPin, 1);
                digitalWrite(this->m_rxEnPin, 1);
                }

            // based on type of request, validate response
            // complete client request
            // go on to next request
            newState = State::stValidate;
            }
            break;

        case State::stValidate:
            {
            if (fEntry)
                {
                /* nothing */
                }

            Request * const pCurrent = this->m_RqPool.m_pCurrent;

            if (pCurrent->requestCode == cSM70::RequestCode_t::kReadData)
                {
                pCurrent->pDoneFn(pCurrent, pCurrent->pUserData, this->m_DataReport.isValid());
                }

            else if (pCurrent->requestCode == cSM70::RequestCode_t::kReadInfo)
                {
                pCurrent->pDoneFn(pCurrent, pCurrent->pUserData, this->m_SensorInfo.isValid());
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

void cSM70::startTransferRequest(const void *pRequestBytes, size_t nRequestBytess)
    {
	auto * pDataRequest = this->m_DataRequest.getPointer();
	auto * pSensorInfoRequest = this->m_SensorInfoRequest.getPointer();

    if (pRequestBytes == pDataRequest)
        {
        this->m_pSerial->write(pDataRequest, 4);
        }

    else if (pRequestBytes == pSensorInfoRequest)
        {
        this->m_pSerial->write(pSensorInfoRequest, 4);
        }
    return;
    }

bool cSM70::listAppend(Request *&pHead, Request *pRequest)
    {
    if (pRequest == nullptr)
        return false;

    if( pHead == NULL)
        {
        pRequest->pNext = pRequest;
        pRequest->pLast = pRequest;
        pHead = pRequest;
        return true;
        }
    else
        {
        // the predecessor of head is tail.
        auto const pOldTail = pHead->pLast;

        // set back and forward links for pRequest;
        // it's going between pOldTail and pHead.
        pRequest->pNext = pHead;
        pRequest->pLast = pOldTail;

        // insert it in the list. This is not atomic, but
        // since we're not using preemption, it doesn't
        // need to be atomic.
        pOldTail->pNext = pRequest;
        pHead->pLast = pRequest;
        return false;
        }
    }

void cSM70::RqPool_t::free(Request *pRequest)
    {
    cSM70::listAppend(this->m_pFree, pRequest);
    }

void cSM70::listRemove(Request *&pHead, Request *pRequest)
    {
    if (pRequest == nullptr)
        return;

    // return node, after detaching from list.
    // if list contains only one node -- i.e. if
    // pRequest is the only  element of the list,
    // result is pRequest, but head gets set to null.
    auto const pNext = pRequest->pNext;
    if (pNext == pRequest)
        {
        if (pHead == pRequest)
            pHead = nullptr;
        }
    else
        {
        // list has two or more elements

        // if removing head, advance head pointer
        if (pHead == pRequest)
            pHead = pNext;

        // get predecessor of pRequest.
        auto const pPrevious = pRequest->pLast;

        // remove pRequest from list; this works even if pPrevious == pNext.
        pPrevious->pNext = pNext;
        pNext->pLast = pPrevious;

        // some backtracking algorithms depend on leaving the links
        // unchanged in the allocated node, but in this case we
        // are not backtracking and it's a lot easier to understand
        // if every allocated node is in itself a valid list.
        pRequest->pNext = pRequest;
        pRequest->pLast = pRequest;
        }

    }

cSM70::Request* cSM70::RqPool_t::allocate()
    {
    Request * const pHead = this->m_pFree;

    // listRemove() will check for empty list
    listRemove(this->m_pFree, pHead);

    return pHead;
    }

bool cSM70::RqPool_t::addPending(Request *pRequest)
    {
    return cSM70::listAppend(this->m_pPending, pRequest);
    }

bool cSM70::RqPool_t::freeCurrent()
    {
    Request *pCurrent;

    if (this->m_pCurrent == nullptr)
        {
        return false;
        }
    pCurrent = this->m_pCurrent;
    listRemove(this->m_pCurrent, pCurrent);
    this->free(pCurrent);
    return this->m_pPending != nullptr;
    }
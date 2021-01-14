/*

Module:	MCCI_Catena_SM70.h

Function:
	Header file for MCCI Catena library for Aeroqual SM70 Ozone sensor

Copyright and License:
	This file copyright (C) 2021 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	See accompanying LICENSE file for copyright and license information.

Author:
	Terry Moore, MCCI Corporation	January 2021

*/

#ifndef _MCCI_Catena_SM70_h_
#define _MCCI_Catena_SM70_h_	/* prevent multiple includes */

#pragma once

#include <Arduino.h>
#include <cstdint>

namespace McciCatenaSm70 {

/// create a version number for comparison
static constexpr std::uint32_t
makeVersion(
    std::uint8_t major, std::uint8_t minor, std::uint8_t patch, std::uint8_t local = 0
    )
    {
    return ((std::uint32_t)major << 24) | ((std::uint32_t)minor << 16) | ((std::uint32_t)patch << 8) | (std::uint32_t)local;
    }

/// extract major number from version
static constexpr std::uint8_t
getMajor(std::uint32_t v)
    {
    return std::uint8_t(v >> 24u);
    }

/// extract minor number from version
static constexpr std::uint8_t
getMinor(std::uint32_t v)
    {
    return std::uint8_t(v >> 16u);
    }

/// extract patch number from version
static constexpr std::uint8_t
getPatch(std::uint32_t v)
    {
    return std::uint8_t(v >> 8u);
    }

/// extract local number from version
static constexpr std::uint8_t
getLocal(std::uint32_t v)
    {
    return std::uint8_t(v);
    }

/// version of library, for use by clients in static_asserts
static constexpr std::uint32_t kVersion = makeVersion(0,1,0);

/// internal port class, used for providing useful, constant behavior to the template class,
/// and also useful for removing templating from the main class so we can separate header
/// from implementation
class cSerialAbstract
        {
public:
        /// constructor
        cSerialAbstract() {};

        // neither copyable nor movable.
        cSerialAbstract(const cSerialAbstract&) = delete;
        cSerialAbstract& operator=(const cSerialAbstract&) = delete;
        cSerialAbstract(const cSerialAbstract&&) = delete;
        cSerialAbstract& operator=(const cSerialAbstract&&) = delete;

	/// initialze and set baud rate. Must be provided by concrete class
        virtual void begin(unsigned long baudrate) const = 0;
	/// initialize, set baud rate and configuration. Must be provided by concrete class.
        virtual void begin(unsigned long baudrate, uint16_t config) const = 0;
	/// return count of bytes available in rx buffer. Must be provided by concrete class.
        virtual int available() const = 0;
	/// read a byte from the rx buffer. Must be provided by concrete class.
        virtual int read() const = 0;
	/// write a buffer; uses output buffer, but blocks if buffer is full.
        virtual size_t write(const uint8_t *buffer, size_t size) const = 0;
	/// shut down (e.g. for system sleep). Must be provided by concrete class.
        virtual void end() const = 0;

	/// drain the read buffer. May be provided by concrete class; if not, a default
	/// implementation is provided.
        virtual void drainRead() const
                {
                while (this->read() >= 0)
                        /* discard */;
                }
	/// drain the write buffer. Must be provided by concrete class.
        virtual void drainWrite() const = 0;

        /// provided as a synonym for drainWrite, so that we
        /// have the complete Arduino::Uart interface. May be overridded
	/// by concrete class.
        virtual void flush() const
                {
                this->drainWrite();
                }
        };

/// create a class that is abstract but uses a physical Arduino serial port of type T.
template <class T> class cSerial : public cSerialAbstract
        {
public:
        /// constructor. \param pPort is a pointer to an Arduino Serial port of some kind.
        cSerial(T *pPort) : m_pPort(pPort) {};

        // neither copyable nor movable.
        cSerial(const cSerial&) = delete;
        cSerial& operator=(const cSerial&) = delete;
        cSerial(const cSerial&&) = delete;
        cSerial& operator=(const cSerial&&) = delete;

	/// map virtual method onto Arduino port's method.
        virtual int available() const override
                {
                return this->m_pPort->available();
                }

	/// map virtual method onto Arduino port's method.
        virtual void begin(unsigned long ulBaudRate) const override
                {
                this->m_pPort->begin(ulBaudRate);
                }

	/// map virtual method onto Arduino port's method.
        virtual void begin(unsigned long ulBaudRate, uint16_t config) const override
                {
                this->m_pPort->begin(ulBaudRate, config);
                }

	/// map virtual method onto Arduino port's method.
        virtual void drainWrite() const override
                {
                this->m_pPort->flush();
                }

	/// map virtual method onto Arduino port's method.
        virtual int read() const override
                {
                return this->m_pPort->read();
                }

	/// map virtual method onto Arduino port's method.
        virtual size_t write(const uint8_t *buffer, size_t size) const override
                {
                return this->m_pPort->write(buffer, size);
                }

	/// map virtual method onto Arduino port's method.
        virtual void end() const override
                {
                this->m_pPort->end();
                }

private:
        T *m_pPort;	/// pointer to physical port that this class wraps.
        };

class cSM70
	{
private:
	static std::uint8_t checksum(const std::uint8_t *pBuf, size_t nBuf, std::uint8_t cs = 0)
		{
		for (; nBuf > 0; ++pBuf, --nBuf)
			cs += *pBuf;
		}

	static std::uint32_t get32le(const std::uint8_t (&a)[4])
		{
		return (std::uint32_t(a[3]) << 24) | 
		       (std::uint32_t(a[2]) << 16) |
		       (std::uint32_t(a[1]) <<  8) |
		       (std::uint32_t(a[0]) <<  0);
		}

public:
	enum class Header : std::uint8_t
		{
		BASE = 0x55,		/// indicates message from computer to SM70
		SENSOR = 0xAA,		/// indicates messsage from SM70 to computer
		};

	enum class MessageType : std::uint8_t
		{
		DATA_REPORT2 = 0x0F,	/// a data report, but DATA2 is not valid.
		DATA_REPORT = 0x10,	/// a data report, sensor to computer
		DATA_REQUEST = 0x1A,	/// request a data report
		SENSOR_INFO = 0xFB,	/// request or indicate a sensor-info message
		};

	enum class Error : std::uint8_t
		{
		kOk = 0,		/// success
		kBadHeader,		/// message has a bad header byte (byte 0)
		kBadType,		/// message has a bad type byte (byte 1)
		kBadChecksum,		/// message has incorrect checksum
		kBadNameLength,		/// SensorInfoReport::m_nameLength is not legal
		};
	
	enum class SensorStatus : std::int8_t
		{
		kInvalid = -1,		/// invalid status in message
		kOk = 0,		/// sensor is ok
		kFailure = 1,		/// sensor has failed
		kAging = 2,		/// sensor is aging
		};

	enum class DisplayFormat : std::uint8_t
		{
		kInvalid = 0,		/// invalid result seen
		k1_3 = 1,		/// #.###
		k2_2 = 2,		/// ##.##
		k3_1 = 3,		/// ###.#
		k4_0 = 4,		/// ####.
		};

	class DataRequest
		{
	public:
		/// default constructor
		DataRequest()
			: m_hdr(Header::BASE)
			, m_type(MessageType::DATA_REQUEST)
			, m_reserved(0)
			, m_cs(0x1A)
			{
			}

	private:
		Header		m_hdr;
		MessageType	m_type;
		std::uint8_t	m_reserved;
		std::uint8_t	m_cs;
		};

	class DataReport
		{
	public:
		DataReport() {};

		/// check DataReport for validity. \return Error::kOk for
		/// valid messages, some other `Error` value for invalid
		/// messages. 
		Error isValid() const
			{
			if (this->m_hdr != Header::SENSOR)
				return Error::kBadHeader;

			switch (this->m_type)
				{
			case MessageType::DATA_REPORT:
			case MessageType::DATA_REQUEST:
			case MessageType::DATA_REPORT2:
				break;
			default:
				return Error::kBadType;
				}

			// check the checksum
			if (checksum((const std::uint8_t *)this, sizeof(this)) != 0)
				return Error::kBadChecksum;
			
			return Error::kOk;
			}

		/// is the Data1 field valid?
		bool isOzonePpmValid() const
			{
			return this->m_type == MessageType::DATA_REPORT;
			}

		/// return Ozone concentration, or 0 if not valid.
		float getOzonePpm() const
			{
			if (! this->isOzonePpmValid())
				return 0.0f;

			// extract an IEEE float, reject NANs, etc.
			union 	{
				std::uint32_t vUint32;
				float vFloat;
			 	} v;
				 
			v.vUint32 = get32le(this->m_data1);

			if ((v.vUint32 & 0x7F800000) == 0x7F800000)
				return 0.0f;

			return v.vFloat;
			}
		
		/// get sensor status
		SensorStatus getSensorStatus() const
			{
			auto const v = this->m_status1 & 0x3;
			switch (v)
				{
			case 0b00: return SensorStatus::kOk;
			case 0b01: return SensorStatus::kFailure;
			case 0b11: return SensorStatus::kAging;
			default:  return SensorStatus::kInvalid;
				}
			}

		/// return pointer to the message body
		std::uint8_t getPointer()
			{
			return (std::uint8_t) this;
			}

		/// get size of message body
		std::uint8_t getSize() const
			{
			static_assert(sizeof(*this) == 15, "message size should be 15 bytes");
			return sizeof(*this);			
			}

	private:
		Header		m_hdr;
		MessageType	m_type;
		std::uint8_t	m_data1[4];
		std::uint8_t	m_data2[4];
		std::uint8_t	m_reserved[2];
		std::uint8_t	m_status1;
		std::uint8_t	m_status2;
		std::uint8_t	m_cs;
		};

	/// the sensor request message
	class SensorInfoRequest
		{
		/// constructor
		SensorInfoRequest()
			: m_hdr(Header::BASE)
			, m_type(MessageType::SENSOR_INFO)
			, m_reserved(0)
			, m_cs(0xB0)
			{
			}

		/// return pointer to the message body
		std::uint8_t *getPointer()
			{
			return (std::uint8_t *) this;
			}

		/// get size of message body
		size_t getSize() const
			{
			static_assert(sizeof(*this) == 4, "message size should be 4 bytes");
			return sizeof(*this);			
			}

	private:
		Header		m_hdr;
		MessageType	m_type;
		std::uint8_t	m_reserved;
		std::uint8_t	m_cs;
		};

	/// the sensor info response
	class SensorInfoReport
		{
	public:
		SensorInfoReport() {}

		std::uint8_t *getPointer()
			{
			return (std::uint8_t *) this;
			}
		const std::uint8_t *getPointer() const
			{
			return (const std::uint8_t *) this;
			}
		size_t getSize() const
			{
			static_assert(sizeof(*this) == 14, "message size should be 14 bytes");
			return sizeof(*this);
			}

		Error isValid() const
			{
			if (this->m_hdr != Header::SENSOR)
				return Error::kBadHeader;

			if (this->m_type != MessageType::SENSOR_INFO)
				return Error::kBadType;

			if (this->m_nameLength > sizeof(this->m_sensorName))
				return Error::kBadNameLength;

			if (checksum(this->getPointer(), this->getSize()) != 0)
				return Error::kBadChecksum;

			return Error::kOk;
			}

		constexpr size_t getNameBufSize() const
			{
			return sizeof(this->m_sensorName) + 1;
			}

		size_t getName(char *buf, size_t nbuf) const
			{
			if (buf == nullptr || nbuf == 0)
				return 0;
			if (nbuf < this->getNameBufSize())
				{
				buf[0] = '\0';
				return 0;
				}
			if (this->m_nameLength > sizeof(this->m_sensorName))
				{
				buf[0] = '\0';
				return 0;
				}

			for (unsigned i = 0; i < this->m_nameLength; ++i)
				buf[i] = this->m_sensorName[i];

			buf[this->m_nameLength] = '\0';
			return this->m_nameLength;
			}

		std::uint8_t getVersion() const
			{
			return this->m_version;
			}

		DisplayFormat getDisplayFormat() const
			{
			switch (this->m_display)
				{
			case DisplayFormat::k1_3:
			case DisplayFormat::k2_2:
			case DisplayFormat::k3_1:
			case DisplayFormat::k4_0:
				return this->m_display;
			default:
				return DisplayFormat::kInvalid;
				}
			}

	private:
		Header		m_hdr;
		MessageType	m_type;
		std::uint8_t	m_version;
		DisplayFormat	m_display;
		std::uint8_t	m_nameLength;
		char		m_sensorName[7];
		std::uint8_t	m_reserved;
		std::uint8_t	m_cs;
		};

//
// at last, we get to the top-level object.
//
private:
	struct Request;
	/// the default baudrate of the SM70
	static constexpr std::uint32_t kBaud = 4800;

public:
	cSM70(cSerialAbstract *pSerialAbstract)
		: m_pSerial(m_pSerial) 
		{}

        // neither copyable nor movable.
        cSM70(const cSM70&) = delete;
        cSM70& operator=(const cSM70&) = delete;
        cSM70(const cSM70&&) = delete;
        cSM70& operator=(const cSM70&&) = delete;

	/// the handle for request blobs
	typedef struct Request *HRequest_t;
	/// type for declaring completion functions
	typedef void CompletionFn(HRequest_t hRequest, void *pUserData, Error errcode);

	/// start operation.
	bool begin();
	/// stop operation (e.g., before suspend)
	void end();
	/// drive the FSMs forward
	void poll();
	/// cancel a pending request
	void cancel(HRequest_t hRequest);

	/// start a read operation, and return a non-null handle
	HRequest_t startReadData(CompletionFn *pDoneFn, void *pUserData);
	/// start a getInfo operation and return a non-null handle.
	HRequest_t startReadInfo(CompletionFn *pDoneFn, void *pUserData);

	/// simple wrapper for synchronous data fetches
	Error readData();

	/// simple wrapper for synchronus info fetches
	Error readInfo();

	/// load a pointer to the data blob.
	DataReport *getData()
		{
		return &this->m_DataReport;
		}
	/// load a pointer to the info blob.
	SensorInfoReport *getSensorInfo()
		{
		return &this->m_SensorInfo;
		}

private:
	cSerialAbstract			*m_pSerial;
	static const DataRequest	m_DataRequest;
	static const SensorInfoRequest	m_SensorInfoRequest;
	DataReport			m_DataReport;
	SensorInfoReport		m_SensorInfo;
	};

} // namespace McciCatenaSm70

#endif /* _MCCI_Catena_SM70_h_ */

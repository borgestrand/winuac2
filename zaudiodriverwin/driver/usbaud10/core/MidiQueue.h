/*
   This file is part of the EMU CA0189 USB Audio Driver.

   Copyright (C) 2008 EMU Systems/Creative Technology Ltd. 

   This driver is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This driver is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library.   If not, a copy of the GNU Lesser General Public 
   License can be found at <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
*
*     Copyright (c) E-mu Systems, Inc. 1994. All rights Reserved.
*                             
*******************************************************************************
*/

/*****************************************************************************
*
* Filename: MidiQueue.h
*
* Description: Circular Queue
*
* Revision History:
*
* Version    Person        Date         Reason
* -------    ---------   -----------  ---------------------------------------
*
*******************************************************************************
*/
#ifndef __MIDI_QUEUE_H
#define __MIDI_QUEUE_H

/*****************************************************************************
* Defines
*/
#define DEFAULT_QUEUE_SIZE 256
#define QUEUE_OVERFLOW 1
#define QUEUE_SUCCESS 0
#define QUEUE_NODATA  2

typedef LONG QSTATUS;

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CMidiQueue
 *****************************************************************************
 * @ingroup MIDIUART_GROUP
 * @brief
 * Queue object.
 */
class CMidiQueue
{
private:
    PUCHAR		m_byQ;             // The MIDI buffer for pulling data
    USHORT		m_Mask;
    USHORT		m_uiIn, m_uiOut;   // Where we are in the circular buffer
    USHORT		m_uiBytes;         // How many unread bytes are left in the queue

    USHORT		m_uiInSave, m_uiOutSave, m_uiBytesSave;

	void EstablishQueue(USHORT QueueSize)
	{
		if (m_byQ != NULL)
		{
			ExFreePool(m_byQ);
		}

		ULONG Detect = (ULONG)QueueSize;
		USHORT Count = 0;

		while ((Detect & 0x10000) == 0)
		{
			Detect <<= 1;
			Count++;
		}

		QueueSize = (Detect & 0x8000) ? 1 << (17-Count) : 1 << (16-Count);
		m_Mask = QueueSize-1;

		m_byQ = PUCHAR(ExAllocatePoolWithTag(NonPagedPool, QueueSize, 'mdW'));

		if (m_byQ)
		{
			RtlZeroMemory(m_byQ, QueueSize);
			Reset();
			SaveQueuePointers();
		}
	}

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    CMidiQueue(void)
	{ 
		m_byQ = NULL;
		EstablishQueue(DEFAULT_QUEUE_SIZE); 
	}

    CMidiQueue(USHORT QueueSize)
	{
		m_byQ = NULL;
		EstablishQueue(QueueSize);
	}

    ~CMidiQueue(void)
	{
		Reset();

		if (m_byQ != NULL)
		{
			ExFreePool(m_byQ);
		}
	}

	void Reset(void)
	{
		m_uiIn    = 0;
		m_uiOut   = 0;
		m_uiBytes = 0;
	}

	QSTATUS PutQueue(UCHAR byMIDI)
	{
		m_byQ[++m_uiIn &= m_Mask] = byMIDI;
		m_uiBytes++;
		
		QSTATUS qStatus = (m_uiIn == m_uiOut) ? QUEUE_OVERFLOW : QUEUE_SUCCESS;

		return qStatus;
	}

	QSTATUS GetQueue(UCHAR * byMIDI)
	{
		QSTATUS qStatus;

		if (m_uiBytes == 0)
		{
			qStatus = QUEUE_NODATA;
		}
		else
		{
			*byMIDI = m_byQ[++m_uiOut &= m_Mask];
			m_uiBytes--;

			qStatus = QUEUE_SUCCESS;
		}

		return qStatus;
	}

	void SaveQueuePointers(void) 
	{  
		m_uiInSave = m_uiIn; 
		m_uiOutSave = m_uiOut; 
		m_uiBytesSave = m_uiBytes;
	}

	void RestoreQueuePointers(void) 
	{
		m_uiIn = m_uiInSave; 
		m_uiOut = m_uiOutSave; 
		m_uiBytes = m_uiBytesSave;
	}

    USHORT CanGetQueue(void) 
	{
		return m_uiBytes;
	}
};

#endif // __MIDI_QUEUE_H

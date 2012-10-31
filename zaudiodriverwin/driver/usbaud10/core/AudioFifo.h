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
/*
 *****************************************************************************
 *//*!
 * @file	   AudioFifo.h
 * @brief	   Audio FIFO work items definition.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _AUDIO_FIFO_H_
#define _AUDIO_FIFO_H_

/*****************************************************************************
 * Defines
 */
/*! @brief Size of the hardware FIFO in bytes. */
#define INTERRUPT_FIFO_BUFFER_SIZE		128

/*****************************************************************************
 *//*! @class INTERRUPT_FIFO_WORK_ITEM
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * FIFO work item.
 */
class INTERRUPT_FIFO_WORK_ITEM
{
private:
	INTERRUPT_FIFO_WORK_ITEM *	m_Next;		/*!< @brief The next FIFO in the linked list. */
	INTERRUPT_FIFO_WORK_ITEM *	m_Prev;		/*!< @brief The previous FIFO in the linked list. */
    PVOID						m_Owner;	/*!< @brief The link list owner. */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    INTERRUPT_FIFO_WORK_ITEM()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~INTERRUPT_FIFO_WORK_ITEM() {}
    /*! @brief Self-destructor. */
	void Destruct() { /* Items are allocated & free elsewhere. */ }

    PVOID       Context;
    PIRP        Irp;
    URB         Urb;
    UCHAR       FifoBuffer[INTERRUPT_FIFO_BUFFER_SIZE];
	ULONG		BytesInFifoBuffer;

    /*************************************************************************
     * Friends
     */
	friend class CList<INTERRUPT_FIFO_WORK_ITEM>;
};

typedef INTERRUPT_FIFO_WORK_ITEM * PINTERRUPT_FIFO_WORK_ITEM;

/*****************************************************************************
 *//*! @class SYNCH_FIFO_WORK_ITEM
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * FIFO work item.
 */
class SYNCH_FIFO_WORK_ITEM
{
private:
	SYNCH_FIFO_WORK_ITEM *	m_Next;		/*!< @brief The next FIFO in the linked list. */
	SYNCH_FIFO_WORK_ITEM *	m_Prev;		/*!< @brief The previous FIFO in the linked list. */
    PVOID					m_Owner;	/*!< @brief The link list owner. */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    SYNCH_FIFO_WORK_ITEM()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~SYNCH_FIFO_WORK_ITEM() { if (FifoBuffer) ExFreePool(FifoBuffer); if (Irp) IoFreeIrp(Irp); if (Urb) ExFreePool(Urb); }
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    PVOID       Context;
    PIRP        Irp;
    PURB        Urb;
    PUCHAR      FifoBuffer;
	ULONG		FifoBufferSize;
	ULONG		BytesInFifoBuffer;
	ULONG		TransferSize;
	BOOL		Read;
	BOOL		TransferAsap;
	ULONG		NumberOfPackets;
	ULONG		PacketSize;
	ULONG		Flags;

	ULONG		SkipPackets;

	/*************************************************************************
     * Friends
     */
	friend class CList<SYNCH_FIFO_WORK_ITEM>;
};

typedef SYNCH_FIFO_WORK_ITEM * PSYNCH_FIFO_WORK_ITEM;

/*****************************************************************************
 *//*! @class AUDIO_FIFO_WORK_ITEM
 *****************************************************************************
 * @ingroup MIDIUART_GROUP
 * @brief
 * FIFO work item.
 */
class AUDIO_FIFO_WORK_ITEM
{
private:
	AUDIO_FIFO_WORK_ITEM *	m_Next;		/*!< @brief The next FIFO in the linked list. */
	AUDIO_FIFO_WORK_ITEM *	m_Prev;		/*!< @brief The previous FIFO in the linked list. */
    PVOID					m_Owner;	/*!< @brief The link list owner. */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    AUDIO_FIFO_WORK_ITEM()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~AUDIO_FIFO_WORK_ITEM() { if (FifoBuffer) ExFreePool(FifoBuffer); if (Irp) IoFreeIrp(Irp); if (Urb) ExFreePool(Urb); }
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    PVOID       Context;
    PIRP        Irp;
    PURB        Urb;
    PUCHAR      FifoBuffer;
	ULONG		FifoBufferSize;
	ULONG		BytesInFifoBuffer;
	ULONG		TransferSize;
	BOOL		Read;
	BOOL		TransferAsap;
	ULONG		NumberOfPackets;
	ULONG		PacketSize;
	ULONG		Flags;
	PVOID		Tag;
	ULONG		SkipPackets;

    // statistics.
    ULONG		TimesRecycled;
    ULONG		TotalPacketsProcessed;
    ULONG		TotalBytesProcessed;
    ULONG		ErrorPacketCount;

    /*************************************************************************
     * Friends
     */
	friend class CList<AUDIO_FIFO_WORK_ITEM>;
};

typedef AUDIO_FIFO_WORK_ITEM * PAUDIO_FIFO_WORK_ITEM;

#endif // _AUDIO_FIFO_H_
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
 * @file	   MidiFifo.h
 * @brief	   MIDI FIFO work item definition.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _MIDI_FIFO_H_
#define _MIDI_FIFO_H_

/*****************************************************************************
 * Defines
 */
/*! @brief Size of the hardware FIFO in bytes. */
#define MIDI_FIFO_BUFFER_SIZE		64

/*****************************************************************************
 *//*! @class MIDI_FIFO_WORK_ITEM
 *****************************************************************************
 * @ingroup MIDIUART_GROUP
 * @brief
 * FIFO work item.
 */
class MIDI_FIFO_WORK_ITEM
{
private:
	MIDI_FIFO_WORK_ITEM *	m_Next;		/*!< @brief The next FIFO in the linked list. */
	MIDI_FIFO_WORK_ITEM *	m_Prev;		/*!< @brief The previous FIFO in the linked list. */
    PVOID					m_Owner;	/*!< @brief The link list owner. */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    MIDI_FIFO_WORK_ITEM()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~MIDI_FIFO_WORK_ITEM() {}
    /*! @brief Self-destructor. */
	void Destruct() { /* Items are allocated & free elsewhere. */ }

    PVOID       Context;
    PIRP        Irp;
    URB         Urb;
	ULONG		BytesInFifoBuffer;
    UCHAR       FifoBuffer[MIDI_FIFO_BUFFER_SIZE];
	PVOID		Tags[MIDI_FIFO_BUFFER_SIZE/sizeof(USB_MIDI_EVENT_PACKET)];

    /*************************************************************************
     * Friends
     */
	friend class CList<MIDI_FIFO_WORK_ITEM>;
};

typedef MIDI_FIFO_WORK_ITEM * PMIDI_FIFO_WORK_ITEM;

#endif // _MIDI_FIFO_H_
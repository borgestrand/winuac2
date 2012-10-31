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
*//*
 *****************************************************************************
 *//*!
 * @file       Event.h
 * @brief      Audio event private definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _AUDIO_EVENT_PRIVATE_H_
#define _AUDIO_EVENT_PRIVATE_H_

#include "IKsAdapter.h"

/*!
 * @brief
 * AUDIO_EVENT_HANDLER_ROUTINE specifies the form of a callback function
 * which gets invoked when an audio control event/change occurs.
 */
typedef VOID (*AUDIO_EVENT_HANDLER_ROUTINE)(PVOID Context, GUID * Set, ULONG EventId, BOOL PinEvent, ULONG PinId, BOOL NodeEvent, ULONG NodeId);

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CAudioEventHandler
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Topology event handler object.
 */
class CAudioEventHandler
{
private:
    CAudioEventHandler *		m_Next;				/*!< @brief The next callback in the linked list. */
    CAudioEventHandler *		m_Prev;				/*!< @brief The previous callback in the linked list. */
    PVOID						m_Owner;			/*!< @brief The link list owner. */

    AUDIO_EVENT_HANDLER_ROUTINE	m_EventHandlerRoutine;	/*!< @brief Client's event handler routine. */
	PVOID						m_EventHandlerContext;	/*!< @brief Client's event handler user data, if any */

	/*************************************************************************
     * CAudioEventHandler private methods
     *
     * These are private member functions used internally by the object.  See
     * EVENT.CPP for specific descriptions.
     */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    CAudioEventHandler()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CAudioEventHandler() {}
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioEventHandler public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		AUDIO_EVENT_HANDLER_ROUTINE	EventHandlerRoutine,
		IN		PVOID						EventHandlerContext
	)
	{
		m_EventHandlerContext = EventHandlerContext;
		m_EventHandlerRoutine = EventHandlerRoutine;

		return STATUS_SUCCESS;
	}

	VOID Service
	(
		IN		PVOID	InterruptFilter, 
		IN		GUID *	Set,
		IN		ULONG	EventId,
		IN		BOOL	PinEvent,
		IN		ULONG	PinId,
		IN		BOOL	NodeEvent,
		IN		ULONG	NodeId
	)
	{
		if (m_EventHandlerRoutine)
		{
			if ((!InterruptFilter) || (InterruptFilter != m_EventHandlerContext))
			{
				m_EventHandlerRoutine(m_EventHandlerContext, Set, EventId, PinEvent, PinId, NodeEvent, NodeId);
			}
		}
	}

    /*************************************************************************
     * Friends
     */
	friend class CList<CAudioEventHandler>;
};

typedef CAudioEventHandler * PAUDIO_EVENT_HANDLER;

#endif // _AUDIO_EVENT_PRIVATE_H_

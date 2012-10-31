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
 * @file	   endpoint.h
 * @brief	   USB endpoint definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __USB_ENDPOINT_H__
#define __USB_ENDPOINT_H__

#include "common.h"
#include "utils.h"

/*****************************************************************************
 * Classes
 */
class CUsbDevice;

/*****************************************************************************
 *//*! @class CUsbEndpoint
 *****************************************************************************
 */
class CUsbEndpoint
{
private:
    CUsbEndpoint *				m_Next;			/*!< @brief The next enity in the linked list. */
    CUsbEndpoint *				m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

protected:
	CUsbDevice *						m_UsbDevice;
	PUSB_INTERFACE_DESCRIPTOR			m_InterfaceDescriptor;
	PUSB_AUDIO_10_ENDPOINT_DESCRIPTOR	m_EndpointDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUsbEndpoint() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
	~CUsbEndpoint() {}
    /*! @brief Self-destructor. */
	virtual void Destruct() = 0;

    /*************************************************************************
     * CUsbEndpoint public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */

    /*************************************************************************
	 * The other USB-Audio specification descriptions.
     */
	virtual ULONG GetOtherUsbAudioDescriptorSize
	(	void
	) = 0;

	virtual ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	) = 0;

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CUsbEndpoint>;
};

typedef CUsbEndpoint * PUSB_ENDPOINT;

#endif // __USB_ENDPOINT_H__

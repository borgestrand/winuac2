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
 * @file	   alt.h
 * @brief	   USB alternate setting definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __USB_ALT_SETTING_H__
#define __USB_ALT_SETTING_H__

#include "common.h"
#include "utils.h"

#include "endpoint.h"
#include "entity.h"

/*****************************************************************************
 * Classes
 */
class CUsbDevice;

/*****************************************************************************
 *//*! @class CUsbAlternateSetting
 *****************************************************************************
 */
class CUsbAlternateSetting
{
private:
    CUsbAlternateSetting *		m_Next;			/*!< @brief The next enity in the linked list. */
    CUsbAlternateSetting *		m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

protected:
	CUsbDevice *				m_UsbDevice;
	PUSB_INTERFACE_DESCRIPTOR	m_InterfaceDescriptor;

	CList<CUsbEndpoint>			m_EndpointList;
	CList<CEntity>				m_EntityList;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUsbAlternateSetting() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
	~CUsbAlternateSetting() {}
    /*! @brief Self-destructor. */
	virtual void Destruct() = 0;

    /*************************************************************************
     * CUsbAlternateSetting public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */
	virtual BOOL FindEntity
	(
		IN		UCHAR		EntityID,
		OUT		PENTITY *	OutEntity
	) = 0;

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
	friend class CList<CUsbAlternateSetting>;
};

typedef CUsbAlternateSetting * PUSB_ALTERNATE_SETTING;

#endif // __USB_ALT_SETTING_H__

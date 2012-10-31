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
 * @file	   interface.h
 * @brief	   USB interface definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __USB_INTERFACE_H__
#define __USB_INTERFACE_H__

#include "common.h"
#include "utils.h"

#include "alt.h"
#include "aci.h"
#include "asi.h"
#include "msi.h"

/*****************************************************************************
 * Classes
 */
class CUsbDevice;
class CUsbConfiguration;

/*****************************************************************************
 *//*! @class CUsbInterface
 *****************************************************************************
 */
class CUsbInterface
{
private:
    CUsbInterface *				m_Next;			/*!< @brief The next enity in the linked list. */
    CUsbInterface *				m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	CUsbDevice *				m_UsbDevice;
	CUsbConfiguration *			m_UsbConfiguration;

	UCHAR						m_InterfaceNumber;
	UCHAR						m_InterfaceSubClass;
	UCHAR						m_CurrentAlternateSetting;

	CList<CUsbAlternateSetting>	m_AlternateSettingList;

	NTSTATUS _EnumerateAlternateSettings
	(
		IN		UCHAR	InterfaceNumber
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUsbInterface() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
	~CUsbInterface();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CUsbInterface public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbConfiguration *	UsbConfiguration,
		IN		CUsbDevice *		UsbDevice,
		IN		UCHAR				InterfaceNumber
	);

	UCHAR InterfaceNumber
	(	void
	);

	UCHAR InterfaceSubClass
	(	void
	);

	NTSTATUS SelectAlternateSetting
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutActualIndex
	);

	CUsbAlternateSetting * GetAlternateSetting
	(
		IN		UCHAR	Index
	);

	NTSTATUS GetEntity
	(
		IN		UCHAR		EntityID,
		OUT		PENTITY *	OutEntity
	);

	/*************************************************************************
	 * The other USB-Audio specification descriptions.
     */
	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CUsbInterface>;
};

typedef CUsbInterface * PUSB_INTERFACE;

#endif // __USB_INTERFACE_H__

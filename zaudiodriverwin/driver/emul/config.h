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
 * @file	   config.h
 * @brief	   USB configuration definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __USB_CONFIGURATION_H__
#define __USB_CONFIGURATION_H__

#include "common.h"
#include "utils.h"

#include "interface.h"

/*****************************************************************************
 * Classes
 */
class CUsbDevice;
class CAudioControlInterface;

/*****************************************************************************
 *//*! @class CUsbConfiguration
 *****************************************************************************
 */
class CUsbConfiguration
{
private:
    CUsbConfiguration *				m_Next;			/*!< @brief The next enity in the linked list. */
    CUsbConfiguration *				m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID							m_Owner;		/*!< @brief The link list owner. */

	CUsbDevice *					m_UsbDevice;

	UCHAR							m_ConfigurationIndex;

	PUSB_CONFIGURATION_DESCRIPTOR	m_ConfigurationDescriptor;

	CList<CUsbInterface>			m_InterfaceList;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUsbConfiguration() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
	~CUsbConfiguration();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CUsbConfiguration public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *	UsbDevice,
		IN		UCHAR			ConfigurationNumber
	);

	NTSTATUS GetAudioControlInterface
	(
		OUT		CAudioControlInterface **	OutAudioControlInterface
	);

	NTSTATUS GetInterface
	(
		IN		UCHAR				InterfaceNumber,
		OUT		CUsbInterface **	OutUsbInterface
	);

	NTSTATUS GetConfigurationDescriptor
	(	
		OUT		PUSB_CONFIGURATION_DESCRIPTOR *		OutConfigurationDescriptor
	);

	NTSTATUS GetInterfaceDescriptor
	(
		IN		LONG							InterfaceNumber,
		IN		LONG							AlternateSetting,
		IN		LONG							InterfaceClass,
		IN		LONG							InterfaceSubClass,
		IN		LONG							InterfaceProtocol,
		OUT		PUSB_INTERFACE_DESCRIPTOR *		OutInterfaceDescriptor
	);

	NTSTATUS GetEndpointDescriptor
	(
		IN		UCHAR							InterfaceNumber,
		IN		UCHAR							AlternateSetting,
		IN		UCHAR							EndpointAddress,
		OUT		PUSB_ENDPOINT_DESCRIPTOR *		OutEndpointDescriptor
	);

	NTSTATUS GetEndpointDescriptorByIndex
	(
		IN		UCHAR							InterfaceNumber,
		IN		UCHAR							AlternateSetting,
		IN		UCHAR							PipeIndex,
		OUT		PUSB_ENDPOINT_DESCRIPTOR *		OutEndpointDescriptor
	);

	NTSTATUS GetClassInterfaceDescriptor
	(
		IN		UCHAR							InterfaceNumber,
		IN		UCHAR							AlternateSetting,
		IN		UCHAR							ClassSpecificDescriptorType,
		OUT		PUSB_INTERFACE_DESCRIPTOR *		OutInterfaceDescriptor
	);

	NTSTATUS GetClassEndpointDescriptor
	(
		IN		UCHAR							InterfaceNumber,
		IN		UCHAR							AlternateSetting,
		IN		UCHAR							EndpointAddress,
		IN		UCHAR							ClassSpecificDescriptorType,
		OUT		PUSB_ENDPOINT_DESCRIPTOR *		OutEndpointDescriptor
	);

    /*************************************************************************
	 * The other USB-Audio specification descriptions.
     */
	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer,
		IN		ULONG	BufferSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CUsbConfiguration>;
};

typedef CUsbConfiguration * PUSB_CONFIGURATION;

#endif // __USB_CONFIGURATION_H__

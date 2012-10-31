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
 * @file	   asdatep.cpp
 * @brief	   AS isochronous data endpoint implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "asdatep.h"

#define STR_MODULENAME "asdatep: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CAsIsochronousDataEndpoint::~CAsIsochronousDataEndpoint()
 *****************************************************************************
 */
CAsIsochronousDataEndpoint::
~CAsIsochronousDataEndpoint
(	void
)
{
	PAGED_CODE();
}

/*****************************************************************************
 * CAsIsochronousDataEndpoint::Init()
 *****************************************************************************
 */
NTSTATUS
CAsIsochronousDataEndpoint::
Init
(
	IN		CUsbDevice *				UsbDevice,
	IN		CUsbConfiguration *			UsbConfiguration,
	IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor,
	IN		PUSB_ENDPOINT_DESCRIPTOR	EndpointDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_UsbConfiguration = UsbConfiguration;

	m_InterfaceDescriptor = InterfaceDescriptor;

	m_EndpointDescriptor = PUSB_AUDIO_10_ENDPOINT_DESCRIPTOR(EndpointDescriptor);

	m_UsbConfiguration->GetClassEndpointDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, EndpointDescriptor->bEndpointAddress, USB_AUDIO_10_CS_ENDPOINT, (PUSB_ENDPOINT_DESCRIPTOR *)&m_CsAsEndpointDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CAsIsochronousDataEndpoint::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CAsIsochronousDataEndpoint::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_ENDPOINT_DESCRIPTOR);

	TotalLength += sizeof(USB_AUDIO_20_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * CAsIsochronousDataEndpoint::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CAsIsochronousDataEndpoint::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// standard endpoint descriptor.
	PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = PUSB_ENDPOINT_DESCRIPTOR(Buffer);

	EndpointDescriptor->bLength = sizeof(USB_ENDPOINT_DESCRIPTOR);
	EndpointDescriptor->bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE;
	EndpointDescriptor->bEndpointAddress = m_EndpointDescriptor->bEndpointAddress;
	EndpointDescriptor->bmAttributes = m_EndpointDescriptor->bmAttributes;
	EndpointDescriptor->bmAttributes |= m_EndpointDescriptor->bSynchAddress ? 0 : 0x20; // FIXME: Is this correct regarding implict FEEDBACK ?
	EndpointDescriptor->wMaxPacketSize = m_EndpointDescriptor->wMaxPacketSize;
	EndpointDescriptor->bInterval = m_EndpointDescriptor->bInterval;

	Buffer += sizeof(USB_ENDPOINT_DESCRIPTOR);
	TotalLength += sizeof(USB_ENDPOINT_DESCRIPTOR);

	// class-specific endpoint descriptor.
	PUSB_AUDIO_20_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR CsAsEndpointDescriptor = PUSB_AUDIO_20_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR(Buffer);

	CsAsEndpointDescriptor->bLength = sizeof(USB_AUDIO_20_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR);
	CsAsEndpointDescriptor->bDescriptorType = USB_AUDIO_20_CS_ENDPOINT;
	CsAsEndpointDescriptor->bDescriptorSubtype = USB_AUDIO_20_EP_DESCRIPTOR_GENERAL;
	CsAsEndpointDescriptor->bmAttributes = m_CsAsEndpointDescriptor->bmAttributes & USB_AUDIO_10_DATA_EP_ATTR_MAX_PACKETS_ONLY;
	CsAsEndpointDescriptor->bmControls = (m_CsAsEndpointDescriptor->bmAttributes & USB_AUDIO_10_DATA_EP_ATTR_PITCH) ? 0x3 : 0;
	CsAsEndpointDescriptor->bLockDelayUnits = m_CsAsEndpointDescriptor->bLockDelayUnits;
	CsAsEndpointDescriptor->wLockDelay = m_CsAsEndpointDescriptor->wLockDelay;

	Buffer += sizeof(USB_AUDIO_20_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR);
	TotalLength += sizeof(USB_AUDIO_20_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR);

	return TotalLength;
}

#pragma code_seg()

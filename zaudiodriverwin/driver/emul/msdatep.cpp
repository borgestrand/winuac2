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
 * @file	   msdatep.cpp
 * @brief	   MS [xfer] bulk data endpoint implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "msdatep.h"

#define STR_MODULENAME "msdatep: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CMsBulkDataEndpoint::~CMsBulkDataEndpoint()
 *****************************************************************************
 */
CMsBulkDataEndpoint::
~CMsBulkDataEndpoint
(	void
)
{
	PAGED_CODE();
}

/*****************************************************************************
 * CMsBulkDataEndpoint::Init()
 *****************************************************************************
 */
NTSTATUS
CMsBulkDataEndpoint::
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

	m_UsbConfiguration->GetClassEndpointDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, EndpointDescriptor->bEndpointAddress, USB_AUDIO_10_CS_ENDPOINT, (PUSB_ENDPOINT_DESCRIPTOR *)&m_CsMsEndpointDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CMsBulkDataEndpoint::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CMsBulkDataEndpoint::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = m_EndpointDescriptor->bLength;

	if (m_CsMsEndpointDescriptor)
	{
		TotalLength += m_CsMsEndpointDescriptor->bLength;
	}

	return TotalLength;
}

/*****************************************************************************
 * CMsBulkDataEndpoint::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CMsBulkDataEndpoint::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// standard audio endpoint descriptor.
	RtlCopyMemory(Buffer, m_EndpointDescriptor, m_EndpointDescriptor->bLength);

	Buffer += m_EndpointDescriptor->bLength;
	TotalLength =+ m_EndpointDescriptor->bLength;

	if (m_CsMsEndpointDescriptor)
	{
		RtlCopyMemory(Buffer, m_CsMsEndpointDescriptor, m_CsMsEndpointDescriptor->bLength);

		Buffer += m_CsMsEndpointDescriptor->bLength;
		TotalLength += m_CsMsEndpointDescriptor->bLength;
	}

	return TotalLength;
}

#pragma code_seg()

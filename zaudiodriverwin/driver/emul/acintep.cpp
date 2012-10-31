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
 * @file	   acintep.cpp
 * @brief	   AC interrupt endpoint implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "acintep.h"

#define STR_MODULENAME "acintep: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CAcInterruptEndpoint::~CAcInterruptEndpoint()
 *****************************************************************************
 */
CAcInterruptEndpoint::
~CAcInterruptEndpoint
(	void
)
{
	PAGED_CODE();
}

/*****************************************************************************
 * CAcInterruptEndpoint::Init()
 *****************************************************************************
 */
NTSTATUS
CAcInterruptEndpoint::
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

	return ntStatus;
}

/*****************************************************************************
 * CAcInterruptEndpoint::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CAcInterruptEndpoint::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_ENDPOINT_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * CAcInterruptEndpoint::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CAcInterruptEndpoint::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	RtlCopyMemory(Buffer, m_EndpointDescriptor, sizeof(USB_ENDPOINT_DESCRIPTOR));

	return sizeof(USB_ENDPOINT_DESCRIPTOR);
}

#pragma code_seg()

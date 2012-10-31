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
 * @file	   terminal.cpp
 * @brief	   Terminal implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "terminal.h"
#include "clock.h"

#define STR_MODULENAME "terminal: "

#pragma code_seg()

/*****************************************************************************
 * CTerminal::TerminalID()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CTerminal::
TerminalID
(	void
)
{
	return m_TerminalDescriptor->bTerminalID;
}

/*****************************************************************************
 * CTerminal::TerminalType()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
USHORT
CTerminal::
TerminalType
(	void
)
{
	return m_TerminalDescriptor->wTerminalType;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CTerminal::SetClockEntity()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
VOID
CTerminal::
SetClockEntity
(
	IN		CClockEntity *	ClockEntity
)
{
	PAGED_CODE();

	m_ClockEntity = ClockEntity;
}

/*****************************************************************************
 * CTerminal::GetClockEntity()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
CClockEntity *
CTerminal::
GetClockEntity
(	void
)
{
	PAGED_CODE();

	return m_ClockEntity;
}

/*****************************************************************************
 * CTerminal::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_TE_CONTROL_COPY_PROTECT:
		case USB_AUDIO_20_TE_CONTROL_CONNECTOR:
		case USB_AUDIO_20_TE_CONTROL_OVERLOAD:
		case USB_AUDIO_20_TE_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_TE_CONTROL_OVERFLOW:
		case USB_AUDIO_20_TE_CONTROL_CLUSTER:
		case USB_AUDIO_20_TE_CONTROL_LATENCY:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		default:
		{
			ASSERT(0);
			ntStatus = STATUS_INVALID_PARAMETER;
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CTerminal::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
ReadParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_TE_CONTROL_COPY_PROTECT:
		case USB_AUDIO_20_TE_CONTROL_CONNECTOR:
		case USB_AUDIO_20_TE_CONTROL_OVERLOAD:
		case USB_AUDIO_20_TE_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_TE_CONTROL_OVERFLOW:
		case USB_AUDIO_20_TE_CONTROL_CLUSTER:
		case USB_AUDIO_20_TE_CONTROL_LATENCY:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		default:
		{
			ASSERT(0);
			ntStatus = STATUS_INVALID_PARAMETER;
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CInputTerminal::~CInputTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CInputTerminal::
~CInputTerminal
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CInputTerminal::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the terminal.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Terminal's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CInputTerminal::
Init
(
	IN		CUsbDevice *								UsbDevice,
	IN		UCHAR										InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_TERMINAL_DESCRIPTOR	TerminalDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = TerminalDescriptor->bDescriptorSubtype;

	m_EntityID = TerminalDescriptor->bTerminalID;

	m_InputTerminalDescriptor = PUSB_AUDIO_10_INPUT_TERMINAL_DESCRIPTOR(TerminalDescriptor);

	m_TerminalDescriptor = TerminalDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CInputTerminal::iTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CInputTerminal::
iTerminal
(	void
)
{
	return m_InputTerminalDescriptor->iTerminal;
}

/*****************************************************************************
 * CInputTerminal::ParseSources()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Parses the sources connected to the terminal.
 * @param
 * Index Enumeration index.
 * @param
 * OutSourceID Pointer to the memory location to store the returned source ID.
 * @return
 * Returns TRUE if there is a source ID at the specified enumeration index,
 * otherwise FALSE.
 */
BOOL 
CInputTerminal::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	// No sources connected to the input terminal.
	return FALSE;
}

/*****************************************************************************
 * CInputTerminal::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CInputTerminal::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_INPUT_TERMINAL_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * CInputTerminal::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CInputTerminal::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_INPUT_TERMINAL_DESCRIPTOR TerminalDescriptor = PUSB_AUDIO_20_INPUT_TERMINAL_DESCRIPTOR(Buffer);

	TerminalDescriptor->bLength = sizeof(USB_AUDIO_20_INPUT_TERMINAL_DESCRIPTOR);
	TerminalDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	TerminalDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_INPUT_TERMINAL;
	TerminalDescriptor->bTerminalID = m_InputTerminalDescriptor->bTerminalID;
	TerminalDescriptor->wTerminalType = m_InputTerminalDescriptor->wTerminalType;
	TerminalDescriptor->bAssocTerminal = m_InputTerminalDescriptor->bAssocTerminal;
	TerminalDescriptor->bCSourceID = m_ClockEntity->ClockID();
	TerminalDescriptor->bNrChannels = m_InputTerminalDescriptor->bNrChannels;
	TerminalDescriptor->bmChannelConfig = m_InputTerminalDescriptor->wChannelConfig;
	TerminalDescriptor->iChannelNames = m_InputTerminalDescriptor->iChannelNames;
	TerminalDescriptor->bmControls = 0; // no controls.
	TerminalDescriptor->iTerminal = m_InputTerminalDescriptor->iTerminal;

	return TerminalDescriptor->bLength;
}

/*****************************************************************************
 * COutputTerminal::~COutputTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
COutputTerminal::
~COutputTerminal
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * COutputTerminal::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the terminal.
 * @param
 * Device Pointer to the topology device object.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Terminal's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
COutputTerminal::
Init
(
	IN		CUsbDevice *								UsbDevice,
	IN		UCHAR										InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_TERMINAL_DESCRIPTOR	TerminalDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = TerminalDescriptor->bDescriptorSubtype;

	m_EntityID = TerminalDescriptor->bTerminalID;

	m_OutputTerminalDescriptor = PUSB_AUDIO_10_OUTPUT_TERMINAL_DESCRIPTOR(TerminalDescriptor);

	m_TerminalDescriptor = TerminalDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * COutputTerminal::iTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
COutputTerminal::
iTerminal
(	void
)
{
	return m_OutputTerminalDescriptor->iTerminal;
}

/*****************************************************************************
 * COutputTerminal::ParseSources()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Parses the sources connected to the terminal.
 * @param
 * Index Enumeration index.
 * @param
 * OutSourceID Pointer to the memory location to store the returned source ID.
 * @return
 * Returns TRUE if there is a source ID at the specified enumeration index,
 * otherwise FALSE.
 */
BOOL 
COutputTerminal::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	if (Index == 0)
	{
		*OutSourceID = m_OutputTerminalDescriptor->bSourceID;

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * COutputTerminal::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
COutputTerminal::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_OUTPUT_TERMINAL_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * COutputTerminal::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
COutputTerminal::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_OUTPUT_TERMINAL_DESCRIPTOR TerminalDescriptor = PUSB_AUDIO_20_OUTPUT_TERMINAL_DESCRIPTOR(Buffer);

	TerminalDescriptor->bLength = sizeof(USB_AUDIO_20_OUTPUT_TERMINAL_DESCRIPTOR);
	TerminalDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	TerminalDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_OUTPUT_TERMINAL;
	TerminalDescriptor->bTerminalID = m_OutputTerminalDescriptor->bTerminalID;
	TerminalDescriptor->wTerminalType = m_OutputTerminalDescriptor->wTerminalType;
	TerminalDescriptor->bAssocTerminal = m_OutputTerminalDescriptor->bAssocTerminal;
	TerminalDescriptor->bSourceID = m_OutputTerminalDescriptor->bSourceID;
	TerminalDescriptor->bCSourceID = m_ClockEntity->ClockID();
	TerminalDescriptor->bmControls = 0; // no controls.
	TerminalDescriptor->iTerminal = m_OutputTerminalDescriptor->iTerminal;

	return TerminalDescriptor->bLength;
}

#pragma code_seg()

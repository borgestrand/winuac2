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
 * @file	   Element.cpp
 * @brief	   MIDI element implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "element.h"

#define STR_MODULENAME "element: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiElement::~CMidiElement()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CMidiElement::
~CMidiElement
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CMidiElement::ElementID()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CMidiElement::
ElementID
(	void
)
{
    PAGED_CODE();

	return m_MidiElementDescriptor->bElementID;
}

/*****************************************************************************
 * CMidiElement::iElement()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CMidiElement::
iElement
(	void
)
{
    PAGED_CODE();

	PUSB_AUDIO_10_MIDI_ELEMENT_INFORMATION Information = PUSB_AUDIO_10_MIDI_ELEMENT_INFORMATION(PUCHAR(m_MidiElementDescriptor)+USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_INFORMATION_OFFSET(m_MidiElementDescriptor->bNrInputPins));

	UCHAR iElement = *(PUCHAR(m_MidiElementDescriptor) + USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_IELEMENT_OFFSET(m_MidiElementDescriptor->bNrInputPins, Information->bElCapsSize));

	return iElement;
}

/*****************************************************************************
 * CMidiElement::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the MIDI element.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiElement::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR	MidiElementDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = MidiElementDescriptor->bDescriptorSubtype;

	m_EntityID = MidiElementDescriptor->bElementID;

	m_MidiElementDescriptor = MidiElementDescriptor;

	return ntStatus;
}

/*****************************************************************************
 * CMidiElement::ParseSources()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Parses the sources connected to the terminal.
 * @param
 * Index Enumeration index.
 * @param
 * OutSourceID Pointer to the memory location to store the returned source ID.
 * @param
 * OutSourcePin Pointer to the memory location to store the returned source pin.
 * @return
 * Returns TRUE if there is a source ID at the specified enumeration index,
 * otherwise FALSE.
 */
BOOL 
CMidiElement::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID,
	OUT		UCHAR * OutSourcePin
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR Source = PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR(PUCHAR(m_MidiElementDescriptor)+USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_SOURCE_OFFSET);

	for (UCHAR i=0; i<m_MidiElementDescriptor->bNrInputPins; i++)
	{
		if (Index == i)
		{
			*OutSourceID  = Source[i].bSourceID;
			*OutSourcePin = Source[i].bSourcePin;

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CMidiElement::ParseCapabilities()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL
CMidiElement::
ParseCapabilities
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutCapability
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUSB_AUDIO_10_MIDI_ELEMENT_INFORMATION Information = PUSB_AUDIO_10_MIDI_ELEMENT_INFORMATION(PUCHAR(m_MidiElementDescriptor)+USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_INFORMATION_OFFSET(m_MidiElementDescriptor->bNrInputPins));

	PUCHAR bmElementCaps = Information->bmElementCaps;

	UCHAR ByteOffset = Index / 8;
	UCHAR BitMask = 0x01 << (Index % 8);

	if (ByteOffset < Information->bElCapsSize)
	{
		*OutCapability = 0;

		if (bmElementCaps[ByteOffset] & BitMask)
		{
			*OutCapability = Index+1;
		}

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * CMidiElement::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CMidiElement::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = m_MidiElementDescriptor->bLength;

	return TotalLength;
}

/*****************************************************************************
 * CMidiElement::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CMidiElement::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	RtlCopyMemory(Buffer, m_MidiElementDescriptor, m_MidiElementDescriptor->bLength);

	return m_MidiElementDescriptor->bLength;
}

/*****************************************************************************
 * CMidiElement::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMidiElement::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	RequestValueH,
	IN		UCHAR	RequestValueL,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	USHORT RequestValue = USHORT(RequestValueH)<<8 | RequestValueL;

	if ((RequestCode == USB_AUDIO_10_REQUEST_CUR) || (RequestCode == USB_AUDIO_10_REQUEST_MIN) ||
		(RequestCode == USB_AUDIO_10_REQUEST_MAX) || (RequestCode == USB_AUDIO_10_REQUEST_RES) ||
		(RequestCode == USB_AUDIO_10_REQUEST_MEM))
	{
		ntStatus = SetRequest(RequestCode, RequestValue, ParameterBlock, ParameterBlockSize);
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiElement::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMidiElement::
ReadParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	RequestValueH,
	IN		UCHAR	RequestValueL,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	USHORT RequestValue = USHORT(RequestValueH)<<8 | RequestValueL;

	if ((RequestCode == USB_AUDIO_10_REQUEST_CUR) || (RequestCode == USB_AUDIO_10_REQUEST_MIN) ||
		(RequestCode == USB_AUDIO_10_REQUEST_MAX) || (RequestCode == USB_AUDIO_10_REQUEST_RES) ||
		(RequestCode == USB_AUDIO_10_REQUEST_MEM))
	{
		ntStatus = GetRequest(RequestCode, RequestValue, ParameterBlock, ParameterBlockSize, OutParameterBlockSize);
	}

	return ntStatus;
}

#pragma code_seg()

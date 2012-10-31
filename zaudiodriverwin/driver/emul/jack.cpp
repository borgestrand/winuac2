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
 * @file	   Jack.cpp
 * @brief	   MIDI jack implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "jack.h"

#define STR_MODULENAME "jack: "

#pragma code_seg()

/*****************************************************************************
 * CMidiJack::JackID()
 *****************************************************************************
 */
UCHAR
CMidiJack::
JackID
(	void
)
{
	return m_MidiJackDescriptor->bJackID;
}

/*****************************************************************************
 * CMidiJack::JackType()
 *****************************************************************************
 */
UCHAR
CMidiJack::
JackType
(	void
)
{
	return m_MidiJackDescriptor->bJackType;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiJack::WriteParameterBlock()
 *****************************************************************************
 */
NTSTATUS 
CMidiJack::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	RequestValueH,
	IN		UCHAR	RequestValueL,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

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
 * CMidiJack::ReadParameterBlock()
 *****************************************************************************
 */
NTSTATUS 
CMidiJack::
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
	PAGED_CODE();

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

/*****************************************************************************
 * CMidiInJack::~CMidiInJack()
 *****************************************************************************
 * @brief
 * Destructor.
 */
CMidiInJack::
~CMidiInJack
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CMidiInJack::Init()
 *****************************************************************************
 * @brief
 * Initialize the jack.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Jack's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiInJack::
Init
(
	IN		CUsbDevice *								UsbDevice,
	IN		UCHAR										InterfaceNumber,
	IN		PUSB_AUDIO_10_MIDI_COMMON_JACK_DESCRIPTOR	MidiJackDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = MidiJackDescriptor->bDescriptorSubtype;

	m_EntityID = MidiJackDescriptor->bJackID;

	m_MidiInJackDescriptor = PUSB_AUDIO_10_MIDI_IN_JACK_DESCRIPTOR(MidiJackDescriptor);

	m_MidiJackDescriptor = MidiJackDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CMidiInJack::ParseSources()
 *****************************************************************************
 * @brief
 * Parses the sources connected to the jack.
 * @param
 * Index Enumeration index.
 * @param
 * OutSourceID Pointer to the memory location to store the returned source ID.
 * @return
 * Returns TRUE if there is a source ID at the specified enumeration index,
 * otherwise FALSE.
 */
BOOL 
CMidiInJack::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID,
	OUT		UCHAR * OutSourcePin
)
{
	PAGED_CODE();

	// No sources connected to the input jack.
	return FALSE;
}

/*****************************************************************************
 * CMidiInJack::iJack()
 *****************************************************************************
 * @brief
 */
UCHAR
CMidiInJack::
iJack
(	void
)
{
	PAGED_CODE();

	return m_MidiInJackDescriptor->iJack;
}

/*****************************************************************************
 * CMidiInJack::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CMidiInJack::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = m_MidiInJackDescriptor->bLength;

	return TotalLength;
}

/*****************************************************************************
 * CMidiInJack::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CMidiInJack::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	RtlCopyMemory(Buffer, m_MidiInJackDescriptor, m_MidiInJackDescriptor->bLength);

	return m_MidiInJackDescriptor->bLength;
}

/*****************************************************************************
 * CMidiOutJack::~CMidiOutJack()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CMidiOutJack::
~CMidiOutJack
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CMidiOutJack::Init()
 *****************************************************************************
 * @brief
 * Initialize the jack.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Jack's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiOutJack::
Init
(
	IN		CUsbDevice *								UsbDevice,
	IN		UCHAR										InterfaceNumber,
	IN		PUSB_AUDIO_10_MIDI_COMMON_JACK_DESCRIPTOR	MidiJackDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = MidiJackDescriptor->bDescriptorSubtype;

	m_EntityID = MidiJackDescriptor->bJackID;

	m_MidiOutJackDescriptor = PUSB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR(MidiJackDescriptor);

	m_MidiJackDescriptor = MidiJackDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CMidiOutJack::ParseSources()
 *****************************************************************************
 * @brief
 * Parses the sources connected to the jack.
 * @param
 * Index Enumeration index.
 * @param
 * OutSourceID Pointer to the memory location to store the returned source ID.
 * @return
 * Returns TRUE if there is a source ID at the specified enumeration index,
 * otherwise FALSE.
 */
BOOL 
CMidiOutJack::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID,
	OUT		UCHAR * OutSourcePin
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR Source = PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR(PUCHAR(m_MidiOutJackDescriptor)+USB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR_SOURCE_OFFSET);

	for (UCHAR i=0; i<m_MidiOutJackDescriptor->bNrInputPins; i++)
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
 * CMidiOutJack::iJack()
 *****************************************************************************
 */
UCHAR
CMidiOutJack::
iJack
(	void
)
{
	PAGED_CODE();

	UCHAR iJack = *(PUCHAR(m_MidiOutJackDescriptor) + USB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR_IJACK_OFFSET(m_MidiOutJackDescriptor->bNrInputPins));

	return iJack;
}

/*****************************************************************************
 * CMidiOutJack::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CMidiOutJack::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = m_MidiOutJackDescriptor->bLength;

	return TotalLength;
}

/*****************************************************************************
 * CMidiOutJack::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CMidiOutJack::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	RtlCopyMemory(Buffer, m_MidiOutJackDescriptor, m_MidiOutJackDescriptor->bLength);

	return m_MidiOutJackDescriptor->bLength;
}

#pragma code_seg()

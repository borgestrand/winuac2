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
 * @file	   asi.cpp
 * @brief	   AudioStreaming interface implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "asi.h"
#include "aci.h"

#define STR_MODULENAME "asi: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioStreamingInterface::~CAudioStreamingInterface()
 *****************************************************************************
 */
CAudioStreamingInterface::
~CAudioStreamingInterface
(	void
)
{
	PAGED_CODE();

	m_EntityList.DeleteAllItems();

	m_EndpointList.DeleteAllItems();
}

/*****************************************************************************
 * CAudioStreamingInterface::Init()
 *****************************************************************************
 */
NTSTATUS
CAudioStreamingInterface::
Init
(
	IN		CUsbConfiguration *			UsbConfiguration,
	IN		CUsbDevice *				UsbDevice,
	IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbConfiguration = UsbConfiguration;

	m_UsbDevice = UsbDevice;

	m_UsbConfiguration->GetAudioControlInterface(&m_AudioControlInterface);

	m_InterfaceDescriptor = InterfaceDescriptor;

	m_UsbConfiguration->GetClassInterfaceDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, USB_AUDIO_10_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&m_CsAsInterfaceDescriptor);

	if (m_CsAsInterfaceDescriptor)
	{
		// Class-specific AS format type descriptor.
		m_CsAsFormatTypeDescriptor = PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR(PUCHAR(m_CsAsInterfaceDescriptor)+m_CsAsInterfaceDescriptor->bLength);

		_ConfigureClocks();
	}

	_EnumerateEndpoints(InterfaceDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CAudioStreamingInterface::FindEntity()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Find the entity with the specified ID.
 * @param
 * EntityID Entity identifier.
 * @param
 * OutEntity A pointer to the PENTITY which will receive the entity.
 * @return
 * TRUE if the specified ID matches one of the entity, otherwise FALSE.
 */
BOOL
CAudioStreamingInterface::
FindEntity
(
	IN		UCHAR		EntityID,
	OUT		PENTITY *	OutEntity
)
{
	BOOL Found = FALSE;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		if (Entity->EntityID() == EntityID)
		{
			*OutEntity = Entity;

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CAudioStreamingInterface::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CAudioStreamingInterface::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// Standard AS interface descriptor.
	TotalLength += sizeof(USB_INTERFACE_DESCRIPTOR);

	// Class-specific AS interface descriptor.
	if (m_CsAsInterfaceDescriptor)
	{
		TotalLength += sizeof(USB_AUDIO_20_CS_AS_INTERFACE_DESCRIPTOR);

		// Class-specific AS format type descriptor.
		switch (m_CsAsFormatTypeDescriptor->bFormatType)
		{
			case USB_AUDIO_10_FORMAT_TYPE_I:
			{
				TotalLength += sizeof(USB_AUDIO_20_TYPE_I_FORMAT_DESCRIPTOR);
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_II:
			{
				TotalLength += sizeof(USB_AUDIO_20_TYPE_II_FORMAT_DESCRIPTOR);
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III:
			{
				TotalLength += sizeof(USB_AUDIO_20_TYPE_III_FORMAT_DESCRIPTOR);
			}
			break;

			default:
			{
				TotalLength += sizeof(USB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR);
			}
			break;
		}
	}

	// All the entities...
	for (CEntity * Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		TotalLength += Entity->GetOtherUsbAudioDescriptorSize();
	}

	// All the endpoints...
	for (CUsbEndpoint * Endpoint = m_EndpointList.First(); Endpoint; Endpoint = m_EndpointList.Next(Endpoint))
	{
		TotalLength += Endpoint->GetOtherUsbAudioDescriptorSize();
	}

	return TotalLength;
}

/*****************************************************************************
 * CAudioStreamingInterface::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CAudioStreamingInterface::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// Standard AS interface descriptor.
	PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = PUSB_INTERFACE_DESCRIPTOR(Buffer);

	InterfaceDescriptor->bLength = sizeof(USB_INTERFACE_DESCRIPTOR);
	InterfaceDescriptor->bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE;
	InterfaceDescriptor->bInterfaceNumber = m_InterfaceDescriptor->bInterfaceNumber;
	InterfaceDescriptor->bAlternateSetting = m_InterfaceDescriptor->bAlternateSetting;
	InterfaceDescriptor->bNumEndpoints = (UCHAR)m_EndpointList.Count();
	InterfaceDescriptor->bInterfaceClass = USB_CLASS_CODE_AUDIO;
	InterfaceDescriptor->bInterfaceSubClass = USB_AUDIO_20_SUBCLASS_AUDIOSTREAMING;
	InterfaceDescriptor->bInterfaceProtocol = USB_AUDIO_20_PROTOCOL_VERSION_02_00;
	InterfaceDescriptor->iInterface = 0; // no name

	Buffer += sizeof(USB_INTERFACE_DESCRIPTOR);
	TotalLength += sizeof(USB_INTERFACE_DESCRIPTOR);

	// Class-specific AS interface descriptor.
	PUSB_AUDIO_20_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = NULL;

	if (m_CsAsInterfaceDescriptor)
	{
		CsAsInterfaceDescriptor = PUSB_AUDIO_20_CS_AS_INTERFACE_DESCRIPTOR(Buffer);

		CsAsInterfaceDescriptor->bLength = sizeof(USB_AUDIO_20_CS_AS_INTERFACE_DESCRIPTOR);
		CsAsInterfaceDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
		CsAsInterfaceDescriptor->bDescriptorSubtype = USB_AUDIO_20_AS_DESCRIPTOR_GENERAL;
		CsAsInterfaceDescriptor->bTerminalLink = m_CsAsInterfaceDescriptor->bTerminalLink;
		CsAsInterfaceDescriptor->bmControls = 0x0F; // active & valid alternate settings control
		
		switch (m_CsAsInterfaceDescriptor->wFormatTag)
		{
			case USB_AUDIO_10_FORMAT_TYPE_I_PCM:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_I;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_I_PCM;

				PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType1Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_I_PCM8:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_I;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_I_PCM8;

				PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType1Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_I_IEEE_FLOAT:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_I;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_I_IEEE_FLOAT;

				PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType1Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_I_ALAW:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_I;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_I_ALAW;

				PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType1Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_I_MULAW:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_I;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_I_MULAW;

				PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType1Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_II_MPEG:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_II;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_II_MPEG;
				CsAsInterfaceDescriptor->bNrChannels = 6; // FIXME: Is this correct ?
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_II_AC3:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_II;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_II_AC3;
				CsAsInterfaceDescriptor->bNrChannels = 6; // FIXME: Is this correct ?
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III_IEC1937_AC3:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_III_IEC61937_AC3;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType3Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER_1:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_III_IEC61937_MPEG1_LAYER_1;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType3Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER_23/*USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_NOEXT*/:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_III_IEC61937_MPEG1_LAYER_23;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType3Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III_IEC1937_MPEG2_EXT:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_III_IEC61937_MPEG2_EXT;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType3Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER_1_LS:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_III_IEC61937_MPEG2_LAYER_1_LS;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType3Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER_23_LS:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;
				CsAsInterfaceDescriptor->bmFormats = USB_AUDIO_20_FORMAT_TYPE_III_IEC61937_MPEG2_LAYER_23_LS;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsInterfaceDescriptor->bNrChannels = FormatType3Descriptor->bNrChannels; 
				CsAsInterfaceDescriptor->bmChannelConfig = (1<<CsAsInterfaceDescriptor->bNrChannels)-1; // TODO: FIXME: Proper channels.
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;

			default:
			{
				CsAsInterfaceDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_UNDEFINED;
				CsAsInterfaceDescriptor->bmFormats = 0;
				CsAsInterfaceDescriptor->bNrChannels = 0;
				CsAsInterfaceDescriptor->bmChannelConfig = 0; 
				CsAsInterfaceDescriptor->iChannelNames = 0; // no names.
			}
			break;
		}

		Buffer += sizeof(USB_AUDIO_20_CS_AS_INTERFACE_DESCRIPTOR);
		TotalLength += sizeof(USB_AUDIO_20_CS_AS_INTERFACE_DESCRIPTOR);

		// Class-specific AS format type descriptor.
		switch (m_CsAsFormatTypeDescriptor->bFormatType)
		{
			case USB_AUDIO_10_FORMAT_TYPE_I:
			{
				PUSB_AUDIO_20_TYPE_I_FORMAT_DESCRIPTOR CsAsFormatTypeDescriptor = PUSB_AUDIO_20_TYPE_I_FORMAT_DESCRIPTOR(Buffer);

				CsAsFormatTypeDescriptor->bLength = sizeof(USB_AUDIO_20_TYPE_I_FORMAT_DESCRIPTOR);
				CsAsFormatTypeDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
				CsAsFormatTypeDescriptor->bDescriptorSubtype = USB_AUDIO_20_AS_DESCRIPTOR_FORMAT_TYPE;
				CsAsFormatTypeDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_I;

				PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsFormatTypeDescriptor->bSubSlotSize = FormatType1Descriptor->bSubframeSize;
				CsAsFormatTypeDescriptor->bBitResolution = FormatType1Descriptor->bBitResolution;

				Buffer += sizeof(USB_AUDIO_20_TYPE_I_FORMAT_DESCRIPTOR);
				TotalLength += sizeof(USB_AUDIO_20_TYPE_I_FORMAT_DESCRIPTOR);
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_II:
			{
				PUSB_AUDIO_20_TYPE_II_FORMAT_DESCRIPTOR CsAsFormatTypeDescriptor = PUSB_AUDIO_20_TYPE_II_FORMAT_DESCRIPTOR(Buffer);

				CsAsFormatTypeDescriptor->bLength = sizeof(USB_AUDIO_20_TYPE_II_FORMAT_DESCRIPTOR);
				CsAsFormatTypeDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
				CsAsFormatTypeDescriptor->bDescriptorSubtype = USB_AUDIO_20_AS_DESCRIPTOR_FORMAT_TYPE;
				CsAsFormatTypeDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_II;

				PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR FormatType2Descriptor = PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsFormatTypeDescriptor->wMaxBitrate = FormatType2Descriptor->wMaxBitrate;
				CsAsFormatTypeDescriptor->wSlotsPerFrame = FormatType2Descriptor->wSamplesPerFrame;

				Buffer += sizeof(USB_AUDIO_20_TYPE_II_FORMAT_DESCRIPTOR);
				TotalLength += sizeof(USB_AUDIO_20_TYPE_II_FORMAT_DESCRIPTOR);
			}
			break;

			case USB_AUDIO_10_FORMAT_TYPE_III:
			{
				PUSB_AUDIO_20_TYPE_III_FORMAT_DESCRIPTOR CsAsFormatTypeDescriptor = PUSB_AUDIO_20_TYPE_III_FORMAT_DESCRIPTOR(Buffer);

				CsAsFormatTypeDescriptor->bLength = sizeof(USB_AUDIO_20_TYPE_III_FORMAT_DESCRIPTOR);
				CsAsFormatTypeDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
				CsAsFormatTypeDescriptor->bDescriptorSubtype = USB_AUDIO_20_AS_DESCRIPTOR_FORMAT_TYPE;
				CsAsFormatTypeDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_III;

				PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR FormatType3Descriptor = PUSB_AUDIO_10_TYPE_III_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
				CsAsFormatTypeDescriptor->bSubSlotSize = FormatType3Descriptor->bSubframeSize;
				CsAsFormatTypeDescriptor->bBitResolution = FormatType3Descriptor->bBitResolution;

				Buffer += sizeof(USB_AUDIO_20_TYPE_III_FORMAT_DESCRIPTOR);
				TotalLength += sizeof(USB_AUDIO_20_TYPE_III_FORMAT_DESCRIPTOR);
			}
			break;

			default:
			{
				PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR CsAsFormatTypeDescriptor = PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR(Buffer);

				CsAsFormatTypeDescriptor->bLength = sizeof(USB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR);
				CsAsFormatTypeDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
				CsAsFormatTypeDescriptor->bDescriptorSubtype = USB_AUDIO_20_AS_DESCRIPTOR_FORMAT_TYPE;
				CsAsFormatTypeDescriptor->bFormatType = USB_AUDIO_20_FORMAT_TYPE_UNDEFINED;

				Buffer += sizeof(USB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR);
				TotalLength += sizeof(USB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR);
			}
			break;
		}
	}

	// All the entities...
	for (CEntity * Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		ULONG DescriptorSize = Entity->GetOtherUsbAudioDescriptor(Buffer);

		Buffer += DescriptorSize;
		TotalLength += DescriptorSize;
	}

	// All the endpoints...
	for (CUsbEndpoint * Endpoint = m_EndpointList.First(); Endpoint; Endpoint = m_EndpointList.Next(Endpoint))
	{
		ULONG DescriptorSize = Endpoint->GetOtherUsbAudioDescriptor(Buffer);

		Buffer += DescriptorSize;
		TotalLength += DescriptorSize;
	}

	return TotalLength;
}

#define SAMFREQ(tSamFreq) ((ULONG(tSamFreq[2])<<16) | (ULONG(tSamFreq[1])<<8) | ULONG(tSamFreq[0]))

/*****************************************************************************
 * CAudioStreamingInterface::GetAudioFormatInformation()
 *****************************************************************************
 */
VOID 
CAudioStreamingInterface::
GetAudioFormatInformation
(
	OUT		USHORT *										OutFormatTag,
	OUT		PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR *	OutFormatTypeDescriptor,
	OUT		ULONG *											OutSamplingFrequency
)
{
	if (m_CsAsInterfaceDescriptor)
	{
		*OutFormatTag = m_CsAsInterfaceDescriptor->wFormatTag;

		if (m_CsAsFormatTypeDescriptor)
		{
			*OutFormatTypeDescriptor = m_CsAsFormatTypeDescriptor;
		}

		PTERMINAL Terminal = NULL;

		m_AudioControlInterface->FindEntity(m_CsAsInterfaceDescriptor->bTerminalLink, (PENTITY*)&Terminal);

		if (Terminal)
		{
			CClockSource * ClockSource = (CClockSource*)Terminal->GetClockEntity();

			if (ClockSource)
			{
				ClockSource->ReadParameterBlock(USB_AUDIO_20_REQUEST_CUR, USB_AUDIO_20_CS_CONTROL_FREQUENCY, 0, OutSamplingFrequency, sizeof(ULONG), NULL);
			}
		}
	}
}

/*****************************************************************************
 * CAudioStreamingInterface::IsAudioFormatSupported()
 *****************************************************************************
 */
BOOL 
CAudioStreamingInterface::
IsAudioFormatSupported
(
	IN		USHORT										FormatTag,										
	IN		PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR	FormatTypeDescriptor,
	IN		ULONG										SamplingFrequency
)
{
	BOOL Supported = FALSE;

	if (m_CsAsInterfaceDescriptor)
	{
		if (m_CsAsInterfaceDescriptor->wFormatTag == FormatTag)
		{
			if (m_CsAsFormatTypeDescriptor)
			{
				if ((m_CsAsFormatTypeDescriptor->bLength == FormatTypeDescriptor->bLength) ||
					(m_CsAsFormatTypeDescriptor->bDescriptorType == FormatTypeDescriptor->bDescriptorType) ||
					(m_CsAsFormatTypeDescriptor->bDescriptorSubtype == FormatTypeDescriptor->bDescriptorSubtype) ||
					(m_CsAsFormatTypeDescriptor->bFormatType == FormatTypeDescriptor->bFormatType))
				{
					switch (m_CsAsFormatTypeDescriptor->bFormatType)
					{
						case USB_AUDIO_10_FORMAT_TYPE_I:
						case USB_AUDIO_10_FORMAT_TYPE_III:
						{
							PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
							PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatType1Descriptor_ = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(FormatTypeDescriptor);

							if ((FormatType1Descriptor->bNrChannels == FormatType1Descriptor_->bNrChannels) ||
								(FormatType1Descriptor->bSubframeSize == FormatType1Descriptor_->bSubframeSize) ||
								(FormatType1Descriptor->bBitResolution == FormatType1Descriptor_->bBitResolution))
							{
								if (FormatType1Descriptor->bSamFreqType == 0)
								{
									// Continuous sample rate.
									ULONG MinimumSampleFrequency = SAMFREQ(FormatType1Descriptor->tSamFreq[0]); // lower bound
									ULONG MaximumSampleFrequency = SAMFREQ(FormatType1Descriptor->tSamFreq[1]); // upper bound

									if ((SamplingFrequency >= MinimumSampleFrequency) && (SamplingFrequency <= MaximumSampleFrequency))
									{
										Supported = TRUE;
									}
								}
								else
								{
									// Discrete sampling rates.
									for (ULONG i=0; i<FormatType1Descriptor->bSamFreqType; i++)
									{
										if (SamplingFrequency == SAMFREQ(FormatType1Descriptor->tSamFreq[i]))
										{
											Supported = TRUE;
											break;
										}
									}
								}
							}
						}
						break;

						case USB_AUDIO_10_FORMAT_TYPE_II:
						{
							PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR FormatType2Descriptor = PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);
							PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR FormatType2Descriptor_ = PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR(FormatTypeDescriptor);

							if ((FormatType2Descriptor->wMaxBitrate == FormatType2Descriptor_->wMaxBitrate) ||
								(FormatType2Descriptor->wSamplesPerFrame == FormatType2Descriptor_->wSamplesPerFrame))
							{
								if (FormatType2Descriptor->bSamFreqType == 0)
								{
									// Continuous sample rate.
									ULONG MinimumSampleFrequency = SAMFREQ(FormatType2Descriptor->tSamFreq[0]); // lower bound
									ULONG MaximumSampleFrequency = SAMFREQ(FormatType2Descriptor->tSamFreq[1]); // upper bound

									if ((SamplingFrequency >= MinimumSampleFrequency) && (SamplingFrequency <= MaximumSampleFrequency))
									{
										Supported = TRUE;
									}
								}
								else
								{
									// Discrete sampling rates.
									for (ULONG i=0; i<FormatType2Descriptor->bSamFreqType; i++)
									{
										if (SamplingFrequency == SAMFREQ(FormatType2Descriptor->tSamFreq[i]))
										{
											Supported = TRUE;
											break;
										}
									}
								}
							}
						}
						break;
					}
				}
			}
		}
	}

	return Supported;
}

/*****************************************************************************
 * CAudioStreamingInterface::_ConfigureClocks()
 *****************************************************************************
 */
VOID
CAudioStreamingInterface::
_ConfigureClocks
(	void
)
{
	if (m_CsAsInterfaceDescriptor)
	{
		PTERMINAL Terminal = NULL;

		m_AudioControlInterface->FindEntity(m_CsAsInterfaceDescriptor->bTerminalLink, (PENTITY*)&Terminal);

		if (Terminal)
		{
			CClockSource * ClockSource = (CClockSource*)Terminal->GetClockEntity();

			switch (m_CsAsFormatTypeDescriptor->bFormatType)
			{
				case USB_AUDIO_10_FORMAT_TYPE_I:
				case USB_AUDIO_10_FORMAT_TYPE_III:
				{
					PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR FormatTypeDescriptor = PUSB_AUDIO_10_TYPE_I_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);

					if (FormatTypeDescriptor->bSamFreqType == 0)
					{
						// Continuous sample rate.
						ULONG MinimumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[0]); // lower bound
						ULONG MaximumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[1]); // upper bound

						ClockSource->AddClockFrequency(MinimumSampleFrequency, MaximumSampleFrequency, 1);
					}
					else
					{
						// Discrete sampling rates.
						for (ULONG i=0; i<FormatTypeDescriptor->bSamFreqType; i++)
						{
							ULONG SamplingFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[i]);

							ClockSource->AddClockFrequency(SamplingFrequency, SamplingFrequency, 0);
						}
					}
				}
				break;

				case USB_AUDIO_10_FORMAT_TYPE_II:
				{
					PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR FormatTypeDescriptor = PUSB_AUDIO_10_TYPE_II_FORMAT_DESCRIPTOR(m_CsAsFormatTypeDescriptor);

					if (FormatTypeDescriptor->bSamFreqType == 0)
					{
						// Continuous sample rate.
						ULONG MinimumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[0]); // lower bound
						ULONG MaximumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[1]); // upper bound

						ClockSource->AddClockFrequency(MinimumSampleFrequency, MaximumSampleFrequency, 1);
					}
					else
					{
						// Discrete sampling rates.
						for (ULONG i=0; i<FormatTypeDescriptor->bSamFreqType; i++)
						{
							ULONG SamplingFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[i]);

							ClockSource->AddClockFrequency(SamplingFrequency, SamplingFrequency, 0);
						}
					}
				}
				break;
			}
		}
	}
}

/*****************************************************************************
 * CAudioStreamingInterface::_EnumerateEndpoints()
 *****************************************************************************
 */
NTSTATUS
CAudioStreamingInterface::
_EnumerateEndpoints
(
	IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
)
{
    PAGED_CODE();

	for (UCHAR i=0; i<InterfaceDescriptor->bNumEndpoints; i++)
	{
		PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

		if (NT_SUCCESS(m_UsbConfiguration->GetEndpointDescriptorByIndex(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, i, &EndpointDescriptor)))
		{
			if (i == 0)
			{
				// data endpoint.
				CAsIsochronousDataEndpoint * AsIsochronousDataEndpoint = new(NonPagedPool) CAsIsochronousDataEndpoint();

				if (AsIsochronousDataEndpoint)
				{
					if (NT_SUCCESS(AsIsochronousDataEndpoint->Init(m_UsbDevice, m_UsbConfiguration, InterfaceDescriptor, EndpointDescriptor)))
					{
						m_EndpointList.Put(AsIsochronousDataEndpoint);
					}
					else
					{
						delete AsIsochronousDataEndpoint;
					}
				}
			}
			else
			{
				// feedback endpoint.
				CAsIsochronousFeedbackEndpoint * AsIsochronousFeedbackEndpoint = new(NonPagedPool) CAsIsochronousFeedbackEndpoint();

				if (AsIsochronousFeedbackEndpoint)
				{
					if (NT_SUCCESS(AsIsochronousFeedbackEndpoint->Init(m_UsbDevice, m_UsbConfiguration, InterfaceDescriptor, EndpointDescriptor)))
					{
						m_EndpointList.Put(AsIsochronousFeedbackEndpoint);
					}
					else
					{
						delete AsIsochronousFeedbackEndpoint;
					}
				}
			}
		}
	}

	return STATUS_SUCCESS;
}

#pragma code_seg()

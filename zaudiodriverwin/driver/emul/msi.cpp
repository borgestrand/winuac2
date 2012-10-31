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
 * @file	   msi.cpp
 * @brief	   MidiStreaming interface implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "msi.h"

#define STR_MODULENAME "msi: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiStreamingInterface::~CMidiStreamingInterface()
 *****************************************************************************
 */
CMidiStreamingInterface::
~CMidiStreamingInterface
(	void
)
{
	PAGED_CODE();

	m_EntityList.DeleteAllItems();

	m_EndpointList.DeleteAllItems();
}

/*****************************************************************************
 * CMidiStreamingInterface::Init()
 *****************************************************************************
 */
NTSTATUS
CMidiStreamingInterface::
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

	m_InterfaceDescriptor = InterfaceDescriptor;

	m_UsbConfiguration->GetClassInterfaceDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, USB_AUDIO_10_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&m_CsMsInterfaceDescriptor);

	_ParseCsMsInterfaceDescriptor(InterfaceDescriptor);

	_EnumerateEndpoints(InterfaceDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CMidiStreamingInterface::FindEntity()
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
CMidiStreamingInterface::
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
 * CMidiStreamingInterface::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CMidiStreamingInterface::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// Standard MS interface descriptor.
	TotalLength += sizeof(USB_INTERFACE_DESCRIPTOR);

	// Class-specific MS interface descriptor.
	if (m_CsMsInterfaceDescriptor)
	{
		TotalLength += sizeof(USB_AUDIO_20_CS_MS_INTERFACE_DESCRIPTOR);
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
 * CMidiStreamingInterface::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CMidiStreamingInterface::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// Standard MS interface descriptor.
	PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = PUSB_INTERFACE_DESCRIPTOR(Buffer);

	InterfaceDescriptor->bLength = sizeof(USB_INTERFACE_DESCRIPTOR);
	InterfaceDescriptor->bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE;
	InterfaceDescriptor->bInterfaceNumber = m_InterfaceDescriptor->bInterfaceNumber;
	InterfaceDescriptor->bAlternateSetting = m_InterfaceDescriptor->bAlternateSetting;
	InterfaceDescriptor->bNumEndpoints = (UCHAR)m_EndpointList.Count();
	InterfaceDescriptor->bInterfaceClass = USB_CLASS_CODE_AUDIO;
	InterfaceDescriptor->bInterfaceSubClass = USB_AUDIO_20_SUBCLASS_MIDISTREAMING;
	InterfaceDescriptor->bInterfaceProtocol = 0;
	InterfaceDescriptor->iInterface = 0; // no name

	Buffer += sizeof(USB_INTERFACE_DESCRIPTOR);
	TotalLength += sizeof(USB_INTERFACE_DESCRIPTOR);

	// Class-specific MS interface descriptor.
	PUSB_AUDIO_20_CS_MS_INTERFACE_DESCRIPTOR CsMsInterfaceDescriptor = NULL;

	if (m_CsMsInterfaceDescriptor)
	{
		CsMsInterfaceDescriptor = PUSB_AUDIO_20_CS_MS_INTERFACE_DESCRIPTOR(Buffer);

		CsMsInterfaceDescriptor->bLength = sizeof(USB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR);
		CsMsInterfaceDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
		CsMsInterfaceDescriptor->bDescriptorSubtype = USB_AUDIO_20_MS_DESCRIPTOR_HEADER;
		CsMsInterfaceDescriptor->bcdMSC = 0x0100; // 1.00
		//CsMsInterfaceDescriptor->wTotalLength = 0;

		Buffer += sizeof(USB_AUDIO_20_CS_MS_INTERFACE_DESCRIPTOR);
		TotalLength += sizeof(USB_AUDIO_20_CS_MS_INTERFACE_DESCRIPTOR);
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

	// Update the total length for the CS MS interface descriptor.
	if (CsMsInterfaceDescriptor)
	{
		CsMsInterfaceDescriptor->wTotalLength = (USHORT)TotalLength;
	}

	return TotalLength;
}

/*****************************************************************************
 * CMidiStreamingInterface::_ParseCsMsInterfaceDescriptor()
 *****************************************************************************
 */
NTSTATUS
CMidiStreamingInterface::
_ParseCsMsInterfaceDescriptor
(
	IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
)
{
    PAGED_CODE();

	PUSB_AUDIO_10_CS_MS_INTERFACE_DESCRIPTOR CsMsInterfaceDescriptor = NULL;

	NTSTATUS ntStatus = m_UsbConfiguration->GetClassInterfaceDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, USB_AUDIO_10_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsMsInterfaceDescriptor);

	if (NT_SUCCESS(ntStatus))
	{
		// Parse the input/output terminals and units...
		PUCHAR DescriptorEnd = PUCHAR(CsMsInterfaceDescriptor) + CsMsInterfaceDescriptor->wTotalLength;

		PUSB_AUDIO_10_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_AUDIO_10_COMMON_DESCRIPTOR)(PUCHAR(CsMsInterfaceDescriptor)+CsMsInterfaceDescriptor->bLength);

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_AUDIO_10_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			// ESI-ROMIO firmware has a bug in its CS MS interface descriptor. The total length it specify 
			// for the CS MS interface is 0x4D00, but its configuration length is only 0x71. Go figure!!
			// This will break the infinite loop, but driver verifier will catch the access beyond allocated
			// memory, so don't run this routine if you have the verifier on.
			if (CommonDescriptor->bLength == 0) break;

			if (CommonDescriptor->bDescriptorType == USB_AUDIO_10_CS_INTERFACE)
			{
				switch (CommonDescriptor->bDescriptorSubtype)
				{
					case USB_AUDIO_10_MS_DESCRIPTOR_MIDI_IN_JACK:
					{
						CMidiInJack * Jack = new(NonPagedPool) CMidiInJack();

						if (Jack)
						{
							if (NT_SUCCESS(Jack->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_MIDI_COMMON_JACK_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Jack);
							}
							else
							{
								delete Jack;
							}
						}
					}
					break;

					case USB_AUDIO_10_MS_DESCRIPTOR_MIDI_OUT_JACK:
					{
						CMidiOutJack * Jack = new(NonPagedPool) CMidiOutJack();

						if (Jack)
						{
							if (NT_SUCCESS(Jack->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_MIDI_COMMON_JACK_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Jack);
							}
							else
							{
								delete Jack;
							}
						}
					}
					break;

					case USB_AUDIO_10_MS_DESCRIPTOR_ELEMENT:
					{
						CMidiElement * Element = new(NonPagedPool) CMidiElement();

						if (Element)
						{
							if (NT_SUCCESS(Element->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Element);
							}
							else
							{
								delete Element;
							}
						}
					}
					break;

					default:
						_DbgPrintF(DEBUGLVL_VERBOSE,("Unknown/unsupport descriptor subtype: 0x%x", CommonDescriptor->bDescriptorSubtype));
						break;
				}
			}

			CommonDescriptor = (PUSB_AUDIO_10_COMMON_DESCRIPTOR)(PUCHAR(CommonDescriptor) + CommonDescriptor->bLength);
		} // while
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiStreamingInterface::_EnumerateEndpoints()
 *****************************************************************************
 */
NTSTATUS
CMidiStreamingInterface::
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
			CMsBulkDataEndpoint * MsBulkDataEndpoint = new(NonPagedPool) CMsBulkDataEndpoint();

			if (MsBulkDataEndpoint)
			{
				if (NT_SUCCESS(MsBulkDataEndpoint->Init(m_UsbDevice, m_UsbConfiguration, InterfaceDescriptor, EndpointDescriptor)))
				{
					m_EndpointList.Put(MsBulkDataEndpoint);
				}
				else
				{
					delete MsBulkDataEndpoint;
				}
			}
		}
	}

	return STATUS_SUCCESS;
}

#pragma code_seg()

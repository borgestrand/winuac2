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
#include "utils.h"

#define STR_MODULENAME "utils: "

/*****************************************************************************
 * PrintUsb10Descriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
PrintUsb10Descriptor
(
	IN		PVOID	Descriptor
)
{
	PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)Descriptor;

	switch (CommonDescriptor->bDescriptorType)
	{
		case USB_CONFIGURATION_DESCRIPTOR_TYPE:
		{
			PUSB_CONFIGURATION_DESCRIPTOR Descriptor = PUSB_CONFIGURATION_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE,("Configuration Descriptor: "));
			_DbgPrintF(DEBUGLVL_TERSE, ("------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE,("wTotalLength %d", Descriptor->wTotalLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bNumInterfaces 0x%x", Descriptor->bNumInterfaces));
			_DbgPrintF(DEBUGLVL_TERSE,("bConfigurationValue 0x%x", Descriptor->bConfigurationValue));
			_DbgPrintF(DEBUGLVL_TERSE,("iConfiguration 0x%x", Descriptor->iConfiguration));
			_DbgPrintF(DEBUGLVL_TERSE,("bmAttributes 0x%x", Descriptor->bmAttributes));
			_DbgPrintF(DEBUGLVL_TERSE,("MaxPower 0x%x", Descriptor->MaxPower));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		case USB_INTERFACE_DESCRIPTOR_TYPE:
		{
			PUSB_INTERFACE_DESCRIPTOR Descriptor = PUSB_INTERFACE_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Interface Descriptor: "));
			_DbgPrintF(DEBUGLVL_TERSE, ("---------------------"));
			_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceNumber 0x%x", Descriptor->bInterfaceNumber));
			_DbgPrintF(DEBUGLVL_TERSE,("bAlternateSetting 0x%x", Descriptor->bAlternateSetting));
			_DbgPrintF(DEBUGLVL_TERSE,("bNumEndPoints 0x%x", Descriptor->bNumEndpoints));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceClass 0x%x", Descriptor->bInterfaceClass));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceSubClass 0x%x", Descriptor->bInterfaceSubClass));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceProtocol 0x%x", Descriptor->bInterfaceProtocol));
			_DbgPrintF(DEBUGLVL_TERSE,("iInterface 0x%x", Descriptor->iInterface));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		case USB_ENDPOINT_DESCRIPTOR_TYPE:
		{
			PUSB_ENDPOINT_DESCRIPTOR Descriptor = PUSB_ENDPOINT_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Endpoint Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("--------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, ("bEndpointAddress 0x%x", Descriptor->bEndpointAddress));
			_DbgPrintF(DEBUGLVL_TERSE, ("bmAttributes 0x%x", Descriptor->bmAttributes));
			_DbgPrintF(DEBUGLVL_TERSE, ("wMaxPacketSize 0x%x", Descriptor->wMaxPacketSize));
			_DbgPrintF(DEBUGLVL_TERSE, ("bInterval 0x%x", Descriptor->bInterval));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		case USB_STRING_DESCRIPTOR_TYPE:
		{
			PUSB_STRING_DESCRIPTOR Descriptor = PUSB_STRING_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE,("String Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE,("-------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE,("bString %ws", Descriptor->bString));
		}
		break;

		case USB_AUDIO_10_CS_INTERFACE:
		{
			// FIXME:
			PUSB_AUDIO_10_CS_MS_INTERFACE_DESCRIPTOR Descriptor = PUSB_AUDIO_10_CS_MS_INTERFACE_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Class-specific MS Interface Header Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
			_DbgPrintF(DEBUGLVL_TERSE, ("bcdMSC 0x%x", Descriptor->bcdMSC));
			_DbgPrintF(DEBUGLVL_TERSE, ("wTotalLength 0x%x", Descriptor->wTotalLength));
			_DbgPrintF(DEBUGLVL_TERSE, (""));

			if (Descriptor->wTotalLength > Descriptor->bLength)
			{
				PUCHAR DescriptorEnd = PUCHAR(Descriptor) + Descriptor->wTotalLength;

				PUSB_AUDIO_10_COMMON_DESCRIPTOR CommonDescriptor = PUSB_AUDIO_10_COMMON_DESCRIPTOR(PUCHAR(Descriptor) + Descriptor->bLength);

				while (((PUCHAR(CommonDescriptor) + sizeof(USB_AUDIO_10_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				  	   ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
				{
					// ESI-ROMIO firmware has a bug in its CS MS interface descriptor. The total length it specify
					// for the CS MS interface is 0x4D00, but its configuration length is only 0x71. Go figure!!
					// This will break the infinite loop, but driver verifier will catch the access beyond allocated
					// memory, so don't run this routine if you have the verifier on.
					if (CommonDescriptor->bLength == 0) break;

					switch (CommonDescriptor->bDescriptorSubtype)
					{
						case USB_AUDIO_10_MS_DESCRIPTOR_MIDI_IN_JACK:
						{
							PUSB_AUDIO_10_MIDI_IN_JACK_DESCRIPTOR Descriptor = PUSB_AUDIO_10_MIDI_IN_JACK_DESCRIPTOR(CommonDescriptor);

							_DbgPrintF(DEBUGLVL_TERSE, ("MIDI IN Jack Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackType 0x%x", Descriptor->bJackType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackID 0x%x", Descriptor->bJackID));
							_DbgPrintF(DEBUGLVL_TERSE, ("iJack 0x%x", Descriptor->iJack));
							_DbgPrintF(DEBUGLVL_TERSE, (""));
						}
						break;

						case USB_AUDIO_10_MS_DESCRIPTOR_MIDI_OUT_JACK:
						{
							PUSB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR Descriptor = PUSB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR(CommonDescriptor);

							_DbgPrintF(DEBUGLVL_TERSE, ("MIDI OUT Jack Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackType 0x%x", Descriptor->bJackType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackID 0x%x", Descriptor->bJackID));
							_DbgPrintF(DEBUGLVL_TERSE, ("bNrInputPins 0x%x", Descriptor->bNrInputPins));

							PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR Pair = PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR(PUCHAR(Descriptor) + USB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR_SOURCE_OFFSET);

							for (UCHAR i=0; i<Descriptor->bNrInputPins; i++)
							{
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourceID[%d] 0x%x", i, Pair[i].bSourceID));
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourcePin[%d] 0x%x", i, Pair[i].bSourcePin));
							}

							UCHAR iJack = *(PUCHAR(Descriptor) + USB_AUDIO_10_MIDI_OUT_JACK_DESCRIPTOR_IJACK_OFFSET(Descriptor->bNrInputPins));
							_DbgPrintF(DEBUGLVL_TERSE, ("iJack 0x%x", iJack));

							_DbgPrintF(DEBUGLVL_TERSE, (""));
						}
						break;

						case USB_AUDIO_10_MS_DESCRIPTOR_ELEMENT:
						{
							PUSB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR Descriptor = PUSB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR(CommonDescriptor);

							_DbgPrintF(DEBUGLVL_TERSE, ("MIDI Element Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
							_DbgPrintF(DEBUGLVL_TERSE, ("bElementID 0x%x", Descriptor->bElementID));
							_DbgPrintF(DEBUGLVL_TERSE, ("bNrInputPins 0x%x", Descriptor->bNrInputPins));

							PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR Pair = PUSB_AUDIO_10_MIDI_SOURCE_ID_PIN_PAIR(PUCHAR(Descriptor) + USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_SOURCE_OFFSET);

							for (UCHAR i=0; i<Descriptor->bNrInputPins; i++)
							{
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourceID[%d] 0x%x", i, Pair[i].bSourceID));
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourcePin[%d] 0x%x", i, Pair[i].bSourcePin));
							}

							PUSB_AUDIO_10_MIDI_ELEMENT_INFORMATION Information = PUSB_AUDIO_10_MIDI_ELEMENT_INFORMATION(PUCHAR(Descriptor) + USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_INFORMATION_OFFSET(Descriptor->bNrInputPins));

							_DbgPrintF(DEBUGLVL_TERSE, ("bNrOutputPins 0x%x", Information->bNrOutputPins));
							_DbgPrintF(DEBUGLVL_TERSE, ("bInTerminalLink 0x%x", Information->bInTerminalLink));
							_DbgPrintF(DEBUGLVL_TERSE, ("bOutTerminalLink 0x%x", Information->bOutTerminalLink));
							_DbgPrintF(DEBUGLVL_TERSE, ("bElCapsSize 0x%x", Information->bElCapsSize));

							for (i=0; i<Information->bElCapsSize; i++)
							{
								_DbgPrintF(DEBUGLVL_TERSE, ("bmElementCaps[%d] 0x%x", i, Information->bmElementCaps[i]));
							}

							UCHAR iElement = *(PUCHAR(Descriptor) + USB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR_IELEMENT_OFFSET(Descriptor->bNrInputPins, Information->bElCapsSize));
							_DbgPrintF(DEBUGLVL_TERSE, ("iElement 0x%x", iElement));

							_DbgPrintF(DEBUGLVL_TERSE, (""));
						}
						break;

						default:
							_DbgPrintF(DEBUGLVL_TERSE, ("Unknown MIDI Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", CommonDescriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", CommonDescriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", CommonDescriptor->bDescriptorSubtype));
							break;
					}

					CommonDescriptor = (PUSB_AUDIO_10_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
				}
			}
		}
		break;

		case USB_AUDIO_10_CS_ENDPOINT:
		{
			// FIXME:
			PUSB_AUDIO_10_CS_MS_DATA_ENDPOINT_DESCRIPTOR Descriptor = PUSB_AUDIO_10_CS_MS_DATA_ENDPOINT_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Class-specific MS Bulk Data Endpoint Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("-----------------------------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
			_DbgPrintF(DEBUGLVL_TERSE, ("bNumEmbMIDIJacks 0x%x", Descriptor->bNumEmbMIDIJacks));
			for (ULONG i=0; i<Descriptor->bNumEmbMIDIJacks;i++)
			{
				_DbgPrintF(DEBUGLVL_TERSE, ("baAssocJackID[%d] 0x%x", i, Descriptor->baAssocJackID[i]));
			}
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		default:
			_DbgPrintF(DEBUGLVL_TERSE, ("Unknown Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("-----------------------------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d\n", CommonDescriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x\n", CommonDescriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
			break;
	}
}

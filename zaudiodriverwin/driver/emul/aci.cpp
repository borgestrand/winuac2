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
 * @file	   aci.cpp
 * @brief	   AudioControl interface implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "aci.h"

#define STR_MODULENAME "aci: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioControlInterface::~CAudioControlInterface()
 *****************************************************************************
 */
CAudioControlInterface::
~CAudioControlInterface
(	void
)
{
	PAGED_CODE();

	m_EntityList.DeleteAllItems();

	m_EndpointList.DeleteAllItems();
}

/*****************************************************************************
 * CAudioControlInterface::Init()
 *****************************************************************************
 */
NTSTATUS
CAudioControlInterface::
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

	m_UsbConfiguration->GetClassInterfaceDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, USB_AUDIO_10_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&m_CsAcInterfaceDescriptor);

	_ParseCsAcInterfaceDescriptor(InterfaceDescriptor);	

	_EnumerateEndpoints(InterfaceDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CAudioControlInterface::FindEntity()
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
CAudioControlInterface::
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
 * CAudioControlInterface::ParseTerminals()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the terminals that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutTerminal A pointer to the PTERMINAL which will receive the terminal.
 * @return
 * TRUE if the specified index matches one of the terminals, otherwise FALSE.
 */
BOOL
CAudioControlInterface::
ParseTerminals
(
	IN		ULONG		Index,
	OUT		PTERMINAL *	OutTerminal
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if ((DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_INPUT_TERMINAL) ||
			(DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_OUTPUT_TERMINAL))
		{
			if (idx == Index)
			{
				*OutTerminal = PTERMINAL(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * CAudioControlInterface::ParseUnits()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the units that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutTerminal A pointer to the PUNIT which will receive the unit.
 * @return
 * TRUE if the specified index matches one of the units, otherwise FALSE.
 */
BOOL
CAudioControlInterface::
ParseUnits
(
	IN		ULONG	Index,
	OUT		PUNIT *	OutUnit
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if ((DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_MIXER_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_SELECTOR_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_FEATURE_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_PROCESSING_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_10_AC_DESCRIPTOR_EXTENSION_UNIT))
		{
			if (idx == Index)
			{
				*OutUnit = PUNIT(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * CAudioControlInterface::ParseClocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the units that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutClockEntity A pointer to the PCLOCK_ENTITY which will receive the clock.
 * @return
 * TRUE if the specified index matches one of the units, otherwise FALSE.
 */
BOOL
CAudioControlInterface::
ParseClocks
(
	IN		ULONG			Index,
	OUT		PCLOCK_ENTITY *	OutClockEntity
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if ((DescriptorSubtype == USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_SOURCE) ||
			(DescriptorSubtype == USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_SELECTOR) ||
			(DescriptorSubtype == USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_MULTIPLIER))
		{
			if (idx == Index)
			{
				*OutClockEntity = PCLOCK_ENTITY(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * CAudioControlInterface::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CAudioControlInterface::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// Standard AC interface descriptor.
	TotalLength += sizeof(USB_INTERFACE_DESCRIPTOR);

	// Class-specific AC interface descriptor.
	TotalLength += sizeof(USB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR);

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
 * CAudioControlInterface::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CAudioControlInterface::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// Standard AC interface descriptor.
	PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = PUSB_INTERFACE_DESCRIPTOR(Buffer);

	InterfaceDescriptor->bLength = sizeof(USB_INTERFACE_DESCRIPTOR);
	InterfaceDescriptor->bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE;
	InterfaceDescriptor->bInterfaceNumber = m_InterfaceDescriptor->bInterfaceNumber;
	InterfaceDescriptor->bAlternateSetting = m_InterfaceDescriptor->bAlternateSetting;
	InterfaceDescriptor->bNumEndpoints = (UCHAR)m_EndpointList.Count();
	InterfaceDescriptor->bInterfaceClass = USB_CLASS_CODE_AUDIO;
	InterfaceDescriptor->bInterfaceSubClass = USB_AUDIO_20_SUBCLASS_AUDIOCONTROL;
	InterfaceDescriptor->bInterfaceProtocol = USB_AUDIO_20_PROTOCOL_VERSION_02_00;
	InterfaceDescriptor->iInterface = 0; // no name

	Buffer += sizeof(USB_INTERFACE_DESCRIPTOR);
	TotalLength += sizeof(USB_INTERFACE_DESCRIPTOR);

	// Class-specific AC interface descriptor.
	PUSB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = PUSB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR(Buffer);

	CsAcInterfaceDescriptor->bLength = sizeof(USB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR);
	CsAcInterfaceDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	CsAcInterfaceDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_HEADER;
	CsAcInterfaceDescriptor->bcdADC = 0x0200; // USB-Audio 2.00
	CsAcInterfaceDescriptor->bCategory = USB_AUDIO_20_CATEGORY_PRO_AUDIO; // whatever...
	//CsAcInterfaceDescriptor->wTotalLength = 0;
	CsAcInterfaceDescriptor->bmControls = 0; // no latency controls.

	Buffer += sizeof(USB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR);
	TotalLength += sizeof(USB_AUDIO_20_CS_AC_INTERFACE_DESCRIPTOR);

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

	// Update the total length for the CS AC interface descriptor.
	CsAcInterfaceDescriptor->wTotalLength = (USHORT)TotalLength;

	return TotalLength;
}

/*****************************************************************************
 * CAudioControlInterface::_ParseCsAcInterfaceDescriptor()
 *****************************************************************************
 */
NTSTATUS
CAudioControlInterface::
_ParseCsAcInterfaceDescriptor
(
	IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
)
{
    PAGED_CODE();

	PUSB_AUDIO_10_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = NULL;

	NTSTATUS ntStatus = m_UsbConfiguration->GetClassInterfaceDescriptor(InterfaceDescriptor->bInterfaceNumber, InterfaceDescriptor->bAlternateSetting, USB_AUDIO_10_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAcInterfaceDescriptor);

	if (NT_SUCCESS(ntStatus))
	{
		// Parse the input/output terminals and units...
		PUCHAR DescriptorEnd = PUCHAR(CsAcInterfaceDescriptor) + CsAcInterfaceDescriptor->wTotalLength;

		PUSB_AUDIO_10_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_AUDIO_10_COMMON_DESCRIPTOR)(PUCHAR(CsAcInterfaceDescriptor)+CsAcInterfaceDescriptor->bLength);

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_AUDIO_10_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			if (CommonDescriptor->bDescriptorType == USB_AUDIO_10_CS_INTERFACE)
			{
				switch (CommonDescriptor->bDescriptorSubtype)
				{
					case USB_AUDIO_10_AC_DESCRIPTOR_INPUT_TERMINAL:
					{
						CInputTerminal * Terminal = new(NonPagedPool) CInputTerminal();

						if (Terminal)
						{
							if (NT_SUCCESS(Terminal->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_TERMINAL_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Terminal);
							}
							else
							{
								delete Terminal;
							}
						}
					}
					break;

					case USB_AUDIO_10_AC_DESCRIPTOR_OUTPUT_TERMINAL:
					{
						COutputTerminal * Terminal = new(NonPagedPool) COutputTerminal();

						if (Terminal)
						{
							if (NT_SUCCESS(Terminal->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_TERMINAL_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Terminal);
							}
							else
							{
								delete Terminal;
							}
						}
					}
					break;

					case USB_AUDIO_10_AC_DESCRIPTOR_MIXER_UNIT:
					{
						CMixerUnit * Unit = new(NonPagedPool) CMixerUnit();

						if (Unit)
						{
							if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
							}
						}
					}
					break;

					case USB_AUDIO_10_AC_DESCRIPTOR_SELECTOR_UNIT:
					{
						CSelectorUnit * Unit = new(NonPagedPool) CSelectorUnit();

						if (Unit)
						{
							if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
							}
						}
					}
					break;

					case USB_AUDIO_10_AC_DESCRIPTOR_FEATURE_UNIT:
					{
						CFeatureUnit * Unit = new(NonPagedPool) CFeatureUnit();

						if (Unit)
						{
							if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
							}
						}
					}
					break;

					case USB_AUDIO_10_AC_DESCRIPTOR_PROCESSING_UNIT:
					{
						PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR ProcessingUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(CommonDescriptor);

						switch (ProcessingUnitDescriptor->wProcessType)
						{
							case USB_AUDIO_10_PROCESS_UPMIX_DOWNMIX:
							{
								CUpDownMixUnit * Unit = new(NonPagedPool) CUpDownMixUnit();

								if (Unit)
								{
									if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_10_PROCESS_DOLBY_PROLOGIC:
							{
								CDolbyPrologicUnit * Unit = new(NonPagedPool) CDolbyPrologicUnit();

								if (Unit)
								{
									if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_10_PROCESS_3D_STEREO_EXTENDER:
							{
								C3dStereoExtenderUnit * Unit = new(NonPagedPool) C3dStereoExtenderUnit();

								if (Unit)
								{
									if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_10_PROCESS_REVERBERATION:
							{
								CReverberationUnit * Unit = new(NonPagedPool) CReverberationUnit();

								if (Unit)
								{
									if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_10_PROCESS_CHORUS:
							{
								CChorusUnit * Unit = new(NonPagedPool) CChorusUnit();

								if (Unit)
								{
									if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_10_PROCESS_DYNAMIC_RANGE_COMPRESSION:
							{
								CDynamicRangeCompressionUnit * Unit = new(NonPagedPool) CDynamicRangeCompressionUnit();

								if (Unit)
								{
									if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							default:
								break;
						}
					}
					break;

					case USB_AUDIO_10_AC_DESCRIPTOR_EXTENSION_UNIT:
					{
						CExtensionUnit * Unit = new(NonPagedPool) CExtensionUnit();

						if (Unit)
						{
							if (NT_SUCCESS(Unit->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
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

	if (NT_SUCCESS(ntStatus))
	{
		// Find all terminals and attach a clock source to them.
		for (CEntity * Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
		{
			if ((Entity->DescriptorSubtype() == USB_AUDIO_10_AC_DESCRIPTOR_INPUT_TERMINAL) ||
				(Entity->DescriptorSubtype() == USB_AUDIO_10_AC_DESCRIPTOR_OUTPUT_TERMINAL))
			{
				CClockSource * ClockSource = new(NonPagedPool) CClockSource();

				if (ClockSource)
				{
					if (NT_SUCCESS(ClockSource->Init(m_UsbDevice, InterfaceDescriptor->bInterfaceNumber, UCHAR(m_EntityList.Count() + 1))))
					{
						m_EntityList.Put(ClockSource);

						// Set the clock source on the terminal.
						CTerminal * Terminal = (CTerminal*)Entity;

						Terminal->SetClockEntity(ClockSource);
					}
					else
					{
						delete ClockSource;
					}
				}
			}
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioControlInterface::_EnumerateEndpoints()
 *****************************************************************************
 */
NTSTATUS
CAudioControlInterface::
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
			CAcInterruptEndpoint * AcInterruptEndpoint = new(NonPagedPool) CAcInterruptEndpoint();

			if (AcInterruptEndpoint)
			{
				if (NT_SUCCESS(AcInterruptEndpoint->Init(m_UsbDevice, m_UsbConfiguration, InterfaceDescriptor, EndpointDescriptor)))
				{
					m_EndpointList.Put(AcInterruptEndpoint);
				}
				else
				{
					delete AcInterruptEndpoint;
				}
			}
		}
	}

	return STATUS_SUCCESS;
}

#pragma code_seg()

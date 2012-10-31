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
 * @file	   interface.cpp
 * @brief	   USB interface implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "interface.h"

#define STR_MODULENAME "interface: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbInterface::~CUsbInterface()
 *****************************************************************************
 */
CUsbInterface::
~CUsbInterface
(	void
)
{
	PAGED_CODE();
}

/*****************************************************************************
 * CUsbInterface::Init()
 *****************************************************************************
 */
NTSTATUS
CUsbInterface::
Init
(
	IN		CUsbConfiguration *	UsbConfiguration,
	IN		CUsbDevice *		UsbDevice,
	IN		UCHAR				InterfaceNumber
)
{
	PAGED_CODE();

	m_UsbConfiguration = UsbConfiguration;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

	m_UsbConfiguration->GetInterfaceDescriptor(InterfaceNumber, 0, USB_CLASS_CODE_AUDIO, -1, -1, &InterfaceDescriptor);

	m_InterfaceSubClass = InterfaceDescriptor->bInterfaceSubClass;

	NTSTATUS ntStatus = _EnumerateAlternateSettings(InterfaceNumber);

	return ntStatus;
}

/*****************************************************************************
 * CUsbInterface::InterfaceNumber()
 *****************************************************************************
 */
UCHAR 
CUsbInterface::
InterfaceNumber
(	void
)
{
	PAGED_CODE();

	return m_InterfaceNumber;
}

/*****************************************************************************
 * CUsbInterface::InterfaceSubClass()
 *****************************************************************************
 */
UCHAR 
CUsbInterface::
InterfaceSubClass
(	void
)
{
	PAGED_CODE();

	return m_InterfaceSubClass;
}

/*****************************************************************************
 * CUsbInterface::SelectAlternateSetting()
 *****************************************************************************
 */
NTSTATUS 
CUsbInterface::
SelectAlternateSetting
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutActualIndex
)
{
	PAGED_CODE();

	*OutActualIndex = Index;

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (Index < m_AlternateSettingList.Count())
	{
		if (m_InterfaceSubClass == USB_AUDIO_10_SUBCLASS_AUDIOSTREAMING)
		{
			CAudioStreamingInterface * ASI = (CAudioStreamingInterface*)GetAlternateSetting(Index);

			USHORT FormatTag = 0;

			PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor = NULL;

			ULONG SamplingFrequency = 0;

			ASI->GetAudioFormatInformation(&FormatTag, &FormatTypeDescriptor, &SamplingFrequency);

			if (FormatTypeDescriptor)
			{		
				UCHAR ActualIndex = 0;

				for (CUsbAlternateSetting * alt = m_AlternateSettingList.First(); alt; alt = m_AlternateSettingList.Next(alt))
				{
					CAudioStreamingInterface * asi = (CAudioStreamingInterface*)alt;

					if (asi->IsAudioFormatSupported(FormatTag, FormatTypeDescriptor, SamplingFrequency))
					{
						*OutActualIndex = ActualIndex;

						break;
					}

					ActualIndex++;
				}
			}
		}

		m_CurrentAlternateSetting = *OutActualIndex;

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbInterface::AlternateSeting()
 *****************************************************************************
 */
CUsbAlternateSetting * 
CUsbInterface::
GetAlternateSetting
(
	IN		UCHAR	Index_
)
{
	PAGED_CODE();

	ULONG Index = 0;

	for (CUsbAlternateSetting * AlternateSetting = m_AlternateSettingList.First(); AlternateSetting; AlternateSetting = m_AlternateSettingList.Next(AlternateSetting))
	{
		if (Index == Index_)
		{
			return AlternateSetting;
		}

		Index++;
	}

	return NULL;
}

#pragma code_seg()

/*****************************************************************************
 * CUsbInterface::GetEntity()
 *****************************************************************************
 */
NTSTATUS 
CUsbInterface::
GetEntity
(
	IN		UCHAR		EntityID,
	OUT		PENTITY *	OutEntity
)
{
	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	CUsbAlternateSetting * AlternateSetting = GetAlternateSetting(m_CurrentAlternateSetting);

	if (AlternateSetting)
	{
		if (AlternateSetting->FindEntity(EntityID, OutEntity))
		{
			ntStatus = STATUS_SUCCESS;
		}
	}

	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbInterface::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CUsbInterface::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// All the alternate settings.
	for (CUsbAlternateSetting * AlternateSetting = m_AlternateSettingList.First(); AlternateSetting; AlternateSetting = m_AlternateSettingList.Next(AlternateSetting))
	{
		TotalLength += AlternateSetting->GetOtherUsbAudioDescriptorSize();
	}

	return TotalLength;
}

/*****************************************************************************
 * CUsbInterface::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CUsbInterface::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// All the alternate settings.
	for (CUsbAlternateSetting * AlternateSetting = m_AlternateSettingList.First(); AlternateSetting; AlternateSetting = m_AlternateSettingList.Next(AlternateSetting))
	{
		ULONG DescriptorSize = AlternateSetting->GetOtherUsbAudioDescriptor(Buffer);

		Buffer += DescriptorSize;
		TotalLength += DescriptorSize;
	}

	return TotalLength;
}

/*****************************************************************************
 * CUsbInterface::_EnumerateAlternateSettings()
 *****************************************************************************
 */
NTSTATUS
CUsbInterface::
_EnumerateAlternateSettings
(
	IN		UCHAR	InterfaceNumber
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	for (ULONG AlternateSetting=0; ; AlternateSetting++)
	{
		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		if (NT_SUCCESS(m_UsbConfiguration->GetInterfaceDescriptor(InterfaceNumber, AlternateSetting, USB_CLASS_CODE_AUDIO, -1, -1, &InterfaceDescriptor)))
		{
			if (InterfaceDescriptor->bInterfaceSubClass == USB_AUDIO_10_SUBCLASS_AUDIOCONTROL)
			{
				CAudioControlInterface * AcInterface = new(NonPagedPool) CAudioControlInterface();

				if (AcInterface)
				{
					ntStatus = AcInterface->Init(m_UsbConfiguration, m_UsbDevice, InterfaceDescriptor);

					if (NT_SUCCESS(ntStatus))
					{
						m_AlternateSettingList.Put(AcInterface);
					}
					else
					{
						delete AcInterface;
						break;
					}
				}
				else
				{
					ntStatus = STATUS_NO_MEMORY;
					break;
				}
			}
			else if (InterfaceDescriptor->bInterfaceSubClass == USB_AUDIO_10_SUBCLASS_AUDIOSTREAMING)
			{
				CAudioStreamingInterface * AsInterface = new(NonPagedPool) CAudioStreamingInterface();

				if (AsInterface)
				{
					ntStatus = AsInterface->Init(m_UsbConfiguration, m_UsbDevice, InterfaceDescriptor);

					if (NT_SUCCESS(ntStatus))
					{
						m_AlternateSettingList.Put(AsInterface);
					}
					else
					{
						delete AsInterface;
						break;
					}
				}
				else
				{
					ntStatus = STATUS_NO_MEMORY;
					break;
				}
			}
			else if (InterfaceDescriptor->bInterfaceSubClass == USB_AUDIO_10_SUBCLASS_MIDISTREAMING)
			{
				CMidiStreamingInterface * MsInterface = new(NonPagedPool) CMidiStreamingInterface();

				if (MsInterface)
				{
					ntStatus = MsInterface->Init(m_UsbConfiguration, m_UsbDevice, InterfaceDescriptor);

					if (NT_SUCCESS(ntStatus))
					{
						m_AlternateSettingList.Put(MsInterface);
					}
					else
					{
						delete MsInterface;
						break;
					}
				}
				else
				{
					ntStatus = STATUS_NO_MEMORY;
					break;
				}
			}
		}
		else
		{
			break;
		}
	}

	return ntStatus;
}

#pragma code_seg()

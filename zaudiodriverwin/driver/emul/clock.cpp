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
 * @file	   Clock.cpp
 * @brief	   Clock entities implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "clock.h"

#define STR_MODULENAME "clock: "



#pragma code_seg()

/*****************************************************************************
 * CClockEntity::ClockID()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CClockEntity::
ClockID
(	void
)
{
	return m_EntityID;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CClockSource::~CClockSource()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CClockSource::
~CClockSource
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CClockSource::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Clock entity descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CClockSource::
Init
(
	IN		CUsbDevice *	UsbDevice,
	IN		UCHAR			InterfaceNumber,
	IN		UCHAR			ClockID
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_SOURCE;

	m_EntityID = ClockID;

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CClockSource::iClock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CClockSource::
iClock
(	void
)
{
	return 0;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CClockSource::SupportClockFrequency()
 *****************************************************************************
 */
VOID 
CClockSource::
AddClockFrequency
(
	IN		ULONG	MinFrequency,
	IN		ULONG	MaxFrequency,
	IN		ULONG	Resolution
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	for (ULONG i=0; i<m_NumberOfFrequencyRanges; i++)
	{
		if ((m_ClockFrequencyRanges[i].Unsigned.dMIN == MinFrequency) &&
			(m_ClockFrequencyRanges[i].Unsigned.dMAX == MaxFrequency) &&
			(m_ClockFrequencyRanges[i].Unsigned.dRES == Resolution))
		{
			Found = TRUE;
			break;
		}
	}

	if (!Found)
	{
		m_ClockFrequencyRanges[m_NumberOfFrequencyRanges].Unsigned.dMIN = MinFrequency;
		m_ClockFrequencyRanges[m_NumberOfFrequencyRanges].Unsigned.dMAX = MaxFrequency;
		m_ClockFrequencyRanges[m_NumberOfFrequencyRanges].Unsigned.dRES = Resolution;
		m_NumberOfFrequencyRanges++;
	}

	m_CurrentClockFrequency = m_ClockFrequencyRanges[0].Unsigned.dMIN;
}

/*****************************************************************************
 * CClockSource::IsClockFrequencySupported()
 *****************************************************************************
 */
BOOL 
CClockSource::
IsClockFrequencySupported
(
	IN		ULONG	ClockFrequency
)
{
	PAGED_CODE();

	BOOL Supported = FALSE;

	for (ULONG i=0; i<m_NumberOfFrequencyRanges; i++)
	{
		if ((m_ClockFrequencyRanges[i].Unsigned.dMIN <= ClockFrequency) &&
			(m_ClockFrequencyRanges[i].Unsigned.dMAX >= ClockFrequency))
		{
			Supported = TRUE;
			break;
		}
	}

	return Supported;
}

/*****************************************************************************
 * CClockSource::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CClockSource::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_CLOCK_SOURCE_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * CClockSource::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CClockSource::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_CLOCK_SOURCE_DESCRIPTOR ClockSourceDescriptor = PUSB_AUDIO_20_CLOCK_SOURCE_DESCRIPTOR(Buffer);

	ClockSourceDescriptor->bLength = sizeof(USB_AUDIO_20_CLOCK_SOURCE_DESCRIPTOR);
	ClockSourceDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	ClockSourceDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_SOURCE;
	ClockSourceDescriptor->bClockID = m_EntityID;
	ClockSourceDescriptor->bmAttributes = 0x1; // internal fixed clock.
	ClockSourceDescriptor->bmControls = 0x7; // clock frequency & validity control.
	ClockSourceDescriptor->bAssocTerminal = 0; // no associated terminal
	ClockSourceDescriptor->iClockSource = 0; // no name.
	
	return ClockSourceDescriptor->bLength;
}

/*****************************************************************************
 * CClockSource::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CClockSource::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR	ChannelNumber,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_CS_CONTROL_FREQUENCY:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(ULONG))
				{
					ULONG Frequency = *(PULONG(ParameterBlock));

					if (IsClockFrequencySupported(Frequency))
					{
						m_CurrentClockFrequency = Frequency;

						ntStatus = STATUS_SUCCESS;
					}
					else
					{
						ntStatus = STATUS_INVALID_DEVICE_REQUEST;
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_CS_CONTROL_VALIDITY:
		{
			// Read only.
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
 * CClockSource::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CClockSource::
ReadParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR	ChannelNumber,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_CS_CONTROL_FREQUENCY:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(ULONG))
				{
					PULONG Frequency = PULONG(ParameterBlock);
					
					*Frequency = m_CurrentClockFrequency;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(ULONG);
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{			
				if (ParameterBlockSize >= (sizeof(USHORT) + m_NumberOfFrequencyRanges * sizeof(RANGE4)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = m_NumberOfFrequencyRanges;

					PRANGE4 Range = PRANGE4(wNumSubRanges+1);

					for (ULONG i=0; i<m_NumberOfFrequencyRanges; i++)
					{
						Range[i].Unsigned.dMIN = m_ClockFrequencyRanges[i].Unsigned.dMIN;
						Range[i].Unsigned.dMAX = m_ClockFrequencyRanges[i].Unsigned.dMAX;
						Range[i].Unsigned.dRES = m_ClockFrequencyRanges[i].Unsigned.dRES;
					}

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + m_NumberOfFrequencyRanges * sizeof(RANGE4);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_CS_CONTROL_VALIDITY:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					PUCHAR Validity = PUCHAR(ParameterBlock);
										
					*Validity = TRUE;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				
				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(UCHAR);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
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
 * CClockSelector::~CClockSelector()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CClockSelector::
~CClockSelector
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CClockSelector::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Clock entity descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CClockSelector::
Init
(
	IN		CUsbDevice *	UsbDevice,
	IN		UCHAR			InterfaceNumber,
	IN		UCHAR			ClockID
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_SELECTOR;

	m_EntityID = ClockID;

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CClockSelector::iClock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CClockSelector::
iClock
(	void
)
{
	UCHAR iClockSelector = 0;

	return iClockSelector;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CClockSelector::AddClockSource()
 *****************************************************************************
 */
VOID 
CClockSelector::
AddClockSource
(
	UCHAR	ClockSourceID
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	for (ULONG i=0; i<m_NrInPins; i++)
	{
		if (m_ClockSourceIDs[i] == ClockSourceID)
		{
			Found = TRUE;
			break;
		}
	}

	if (!Found)
	{
		m_ClockSourceIDs[m_NrInPins] = ClockSourceID;
		m_NrInPins++;
	}

	m_PinId = 1;
}

/*****************************************************************************
 * CClockSelector::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CClockSelector::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_CLOCK_SELECTOR_DESCRIPTOR);

	TotalLength += m_NrInPins; // baCSourceIDs[]

	TotalLength += 1; // bmControls

	TotalLength += 1; // iClockSelector

	return TotalLength;
}

/*****************************************************************************
 * CClockSelector::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CClockSelector::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_CLOCK_SELECTOR_DESCRIPTOR ClockSelectorDescriptor = PUSB_AUDIO_20_CLOCK_SELECTOR_DESCRIPTOR(Buffer);

	ClockSelectorDescriptor->bLength = 0;
	ClockSelectorDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	ClockSelectorDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_SELECTOR;
	ClockSelectorDescriptor->bClockID = m_EntityID;
	ClockSelectorDescriptor->bNrInPins = m_NrInPins;

	Buffer += sizeof(USB_AUDIO_20_CLOCK_SELECTOR_DESCRIPTOR);
	ClockSelectorDescriptor->bLength += sizeof(USB_AUDIO_20_CLOCK_SELECTOR_DESCRIPTOR);

	// baCSourceID[]
	PUCHAR baCSourceID = PUCHAR(Buffer);

	for (ULONG i=0; i<m_NrInPins; i++)
	{
		baCSourceID[i] = m_ClockSourceIDs[i];
	}

	Buffer += m_NrInPins;
	ClockSelectorDescriptor->bLength += m_NrInPins;

	// bmControls
	PUCHAR bmControls = PUCHAR(Buffer);
	
	*bmControls = 0x3; // selector control.

	Buffer += 1;
	ClockSelectorDescriptor->bLength += 1;

	// iClockSelector
	PUCHAR iClockSelector = PUCHAR(Buffer);

	*iClockSelector = 0; // no name.

	Buffer += 1;
	ClockSelectorDescriptor->bLength += 1;

	return ClockSelectorDescriptor->bLength;
}

/*****************************************************************************
 * CClockSelector::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CClockSelector::
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
		case USB_AUDIO_20_CX_CONTROL_SELECTOR:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					m_PinId = *(PUCHAR(ParameterBlock));

					ntStatus = STATUS_SUCCESS;
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
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
 * CClockSelector::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CClockSelector::
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
		case USB_AUDIO_20_CX_CONTROL_SELECTOR:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					PUCHAR PinId = PUCHAR(ParameterBlock);
					
					*PinId = m_PinId;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				
				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(UCHAR);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
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
 * CClockMultiplier::~CClockMultiplier()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CClockMultiplier::
~CClockMultiplier
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CClockMultiplier::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Clock entity descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CClockMultiplier::
Init
(
	IN		CUsbDevice *	UsbDevice,
	IN		UCHAR			InterfaceNumber,
	IN		UCHAR			ClockID,
	IN		UCHAR			ClockSourceID,
	IN		USHORT			Numerator,
	IN		USHORT			Denominator
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_MULTIPLIER;

	m_EntityID = ClockID;

	m_ClockSourceID = ClockSourceID;

	m_Numerator = Numerator;

	m_Denominator = Denominator;

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CClockMultiplier::iClock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CClockMultiplier::
iClock
(	void
)
{
	return 0;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CClockMultiplier::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CClockMultiplier::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_CLOCK_MULTIPLIER_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * CClockMultiplier::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CClockMultiplier::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_CLOCK_MULTIPLIER_DESCRIPTOR ClockSourceDescriptor = PUSB_AUDIO_20_CLOCK_MULTIPLIER_DESCRIPTOR(Buffer);

	ClockSourceDescriptor->bLength = sizeof(USB_AUDIO_20_CLOCK_MULTIPLIER_DESCRIPTOR);
	ClockSourceDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	ClockSourceDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_CLOCK_MULTIPLIER;
	ClockSourceDescriptor->bClockID = m_EntityID;
	ClockSourceDescriptor->bCSourceID = m_ClockSourceID;
	ClockSourceDescriptor->bmControls = 0xF; // clock numerator & denominator controls.
	ClockSourceDescriptor->iClockMultiplier = 0; // no name.
	
	return ClockSourceDescriptor->bLength;
}

/*****************************************************************************
 * CClockMultiplier::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CClockMultiplier::
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
		case USB_AUDIO_20_CM_CONTROL_NUMERATOR:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		case USB_AUDIO_20_CM_CONTROL_DENOMINATOR:
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
 * CClockMultiplier::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CClockMultiplier::
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
		case USB_AUDIO_20_CM_CONTROL_NUMERATOR:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					PUSHORT Numerator = PUSHORT(ParameterBlock);
					
					*Numerator = m_Numerator;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				
				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_CM_CONTROL_DENOMINATOR:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					PUSHORT Denominator = PUSHORT(ParameterBlock);
					
					*Denominator = m_Denominator;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				
				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
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

#pragma code_seg()

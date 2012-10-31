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
 * @file	   Unit.cpp
 * @brief	   Unit implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "unit.h"

#define STR_MODULENAME "unit: "



#pragma code_seg()

/*****************************************************************************
 * CUnit::UnitID()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CUnit::
UnitID
(	void
)
{
	return m_UnitDescriptor->bUnitID;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMixerUnit::~CMixerUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CMixerUnit::
~CMixerUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CMixerUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMixerUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_MixerUnitDescriptor = PUSB_AUDIO_10_MIXER_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMixerUnit::iUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CMixerUnit::
iUnit
(	void
)
{
	ULONG ControlSize = m_MixerUnitDescriptor->bLength - 5 - m_MixerUnitDescriptor->bNrInPins - sizeof(USB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR) - 1;

	UCHAR iMixer = *(PUCHAR(m_MixerUnitDescriptor) + USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_IMIXER_OFFSET(m_MixerUnitDescriptor->bNrInPins, ControlSize));

	return iMixer;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMixerUnit::ParseSources()
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
CMixerUnit::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUCHAR baSourceID = PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

	for (UCHAR i=0; i<m_MixerUnitDescriptor->bNrInPins; i++)
	{
		if (Index == i)
		{
			*OutSourceID = baSourceID[i];

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CMixerUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CMixerUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 5; // First 5 bytes
	
	TotalLength += m_MixerUnitDescriptor->bNrInPins;
	TotalLength += sizeof(USB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR);

	ULONG ControlSize = m_MixerUnitDescriptor->bLength - 5 - m_MixerUnitDescriptor->bNrInPins - sizeof(USB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR) - 1;

	TotalLength += ControlSize;

	TotalLength += 1; // bmControls
	TotalLength += 1; // iMixer

	return TotalLength;
}

/*****************************************************************************
 * CMixerUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CMixerUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_MIXER_UNIT_DESCRIPTOR MixerUnitDescriptor = PUSB_AUDIO_20_MIXER_UNIT_DESCRIPTOR(Buffer);

	MixerUnitDescriptor->bLength = 0;
	MixerUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	MixerUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_MIXER_UNIT;
	MixerUnitDescriptor->bUnitID = m_MixerUnitDescriptor->bUnitID;
	MixerUnitDescriptor->bNrInPins = m_MixerUnitDescriptor->bNrInPins;

	Buffer += 5;
	MixerUnitDescriptor->bLength += 5;

	// baSourceID[]
	RtlCopyMemory(Buffer, PUCHAR(m_MixerUnitDescriptor) + USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET, m_MixerUnitDescriptor->bNrInPins);

	Buffer += m_MixerUnitDescriptor->bNrInPins;
	MixerUnitDescriptor->bLength += m_MixerUnitDescriptor->bNrInPins;

	// Cluster descriptor
	PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor10 = PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR(PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_CLUSTER_OFFSET(m_MixerUnitDescriptor->bNrInPins));
	PUSB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor20 = PUSB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR(Buffer);

	ClusterDescriptor20->bNrChannels = ClusterDescriptor10->bNrChannels;
	ClusterDescriptor20->bmChannelConfig = ClusterDescriptor10->wChannelConfig;
	ClusterDescriptor20->iChannelNames = ClusterDescriptor10->iChannelNames;

	Buffer += sizeof(USB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR);
	MixerUnitDescriptor->bLength += sizeof(USB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR);

	// bmMixerControls
	UCHAR ControlSize = m_MixerUnitDescriptor->bLength - 5 - m_MixerUnitDescriptor->bNrInPins - sizeof(USB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR) - 1;

	RtlCopyMemory(Buffer, PUCHAR(m_MixerUnitDescriptor) + USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(m_MixerUnitDescriptor->bNrInPins), ControlSize);

	Buffer += ControlSize;
	MixerUnitDescriptor->bLength += ControlSize;

	// bmControls
	PUCHAR bmControls = PUCHAR(Buffer);

	*bmControls = 0; // no additional controls

	Buffer += 1;
	MixerUnitDescriptor->bLength += 1;
	
	// iMixer
	PUCHAR iMixer = PUCHAR(Buffer);

	*iMixer = iUnit();

	Buffer += 1;
	MixerUnitDescriptor->bLength += 1;

	return MixerUnitDescriptor->bLength;
}

/*****************************************************************************
 * CMixerUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMixerUnit::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR	MixerControlNumber,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_MU_CONTROL_MIXER:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor = PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR(PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_CLUSTER_OFFSET(m_MixerUnitDescriptor->bNrInPins));

				ULONG InputChannelNumber = (MixerControlNumber / ClusterDescriptor->bNrChannels) + 1;
				ULONG OutputChannelNumber = MixerControlNumber - ((InputChannelNumber - 1) * ClusterDescriptor->bNrChannels) + 1;

				USHORT Control = USHORT((USHORT(InputChannelNumber)<<8) | (OutputChannelNumber));

				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MU_CONTROL_CLUSTER:
		case USB_AUDIO_20_MU_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_MU_CONTROL_OVERFLOW:
		case USB_AUDIO_20_MU_CONTROL_LATENCY:
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
 * CMixerUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMixerUnit::
ReadParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR	MixerControlNumber,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_MU_CONTROL_MIXER:
		{
			PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor = PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR(PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_10_MIXER_UNIT_DESCRIPTOR_CLUSTER_OFFSET(m_MixerUnitDescriptor->bNrInPins));

			ULONG InputChannelNumber = (MixerControlNumber / ClusterDescriptor->bNrChannels) + 1;
			ULONG OutputChannelNumber = MixerControlNumber - ((InputChannelNumber - 1) * ClusterDescriptor->bNrChannels) + 1;

			USHORT Control = USHORT((USHORT(InputChannelNumber)<<8) | (OutputChannelNumber));

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(SHORT);
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.wMIN, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.wMAX, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.wRES, sizeof(SHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MU_CONTROL_CLUSTER:
		case USB_AUDIO_20_MU_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_MU_CONTROL_OVERFLOW:
		case USB_AUDIO_20_MU_CONTROL_LATENCY:
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
 * CSelectorUnit::~CSelectorUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CSelectorUnit::
~CSelectorUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CSelectorUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CSelectorUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_SelectorUnitDescriptor = PUSB_AUDIO_10_SELECTOR_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CSelectorUnit::iUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CSelectorUnit::
iUnit
(	void
)
{
	UCHAR iSelector = *(PUCHAR(m_SelectorUnitDescriptor) + USB_AUDIO_10_SELECTOR_UNIT_DESCRIPTOR_ISELECTOR_OFFSET(m_SelectorUnitDescriptor->bNrInPins));

	return iSelector;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CSelectorUnit::ParseSources()
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
CSelectorUnit::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	for (UCHAR i=0; i<m_SelectorUnitDescriptor->bNrInPins; i++)
	{
		if (Index == i)
		{
			PUCHAR baSourceID = PUCHAR(m_SelectorUnitDescriptor)+USB_AUDIO_10_SELECTOR_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

			*OutSourceID = baSourceID[i];

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CSelectorUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CSelectorUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 5; // First 5 bytes
	
	TotalLength += m_SelectorUnitDescriptor->bNrInPins;

	TotalLength += 1; // bmControls
	TotalLength += 1; // iSelector

	return TotalLength;
}

/*****************************************************************************
 * CSelectorUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CSelectorUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_SELECTOR_UNIT_DESCRIPTOR SelectorUnitDescriptor = PUSB_AUDIO_20_SELECTOR_UNIT_DESCRIPTOR(Buffer);

	SelectorUnitDescriptor->bLength = 0;
	SelectorUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	SelectorUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_SELECTOR_UNIT;
	SelectorUnitDescriptor->bUnitID = m_SelectorUnitDescriptor->bUnitID;
	SelectorUnitDescriptor->bNrInPins = m_SelectorUnitDescriptor->bNrInPins;

	Buffer += 5;
	SelectorUnitDescriptor->bLength += 5;

	// baSourceID[]
	RtlCopyMemory(Buffer, PUCHAR(m_SelectorUnitDescriptor) + USB_AUDIO_10_SELECTOR_UNIT_DESCRIPTOR_BASOURCEID_OFFSET, m_SelectorUnitDescriptor->bNrInPins);

	Buffer += m_SelectorUnitDescriptor->bNrInPins;
	SelectorUnitDescriptor->bLength += m_SelectorUnitDescriptor->bNrInPins;

	// bmControls
	PUCHAR bmControls = PUCHAR(Buffer);

	*bmControls = 0x3; // selector control

	Buffer += 1;
	SelectorUnitDescriptor->bLength += 1;
	
	// iSelector
	PUCHAR iSelector = PUCHAR(Buffer);

	*iSelector = iUnit();

	Buffer += 1;
	SelectorUnitDescriptor->bLength += 1;

	return SelectorUnitDescriptor->bLength;
}

/*****************************************************************************
 * CSelectorUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CSelectorUnit::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR	ChannelNumber,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_SU_CONTROL_SELECTOR:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, 0, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_SU_CONTROL_LATENCY:
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
 * CSelectorUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CSelectorUnit::
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
		case USB_AUDIO_20_SU_CONTROL_SELECTOR:
		{
			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, 0, ParameterBlock, sizeof(UCHAR), NULL);
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
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_20_SU_CONTROL_LATENCY:
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
 * CFeatureUnit::~CFeatureUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CFeatureUnit::
~CFeatureUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CFeatureUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CFeatureUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_FeatureUnitDescriptor = PUSB_AUDIO_10_FEATURE_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CFeatureUnit::iUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CFeatureUnit::
iUnit
(	void
)
{
	UCHAR NumberOfChannels = (m_FeatureUnitDescriptor->bLength - 6 - 1) / m_FeatureUnitDescriptor->bControlSize;

	UCHAR iFeature = *(PUCHAR(m_FeatureUnitDescriptor) + USB_AUDIO_10_FEATURE_UNIT_DESCRIPTOR_IFEATURE_OFFSET(m_FeatureUnitDescriptor->bControlSize, NumberOfChannels));

	return iFeature;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CFeatureUnit::ParseSources()
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
CFeatureUnit::
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
		*OutSourceID = m_FeatureUnitDescriptor->bSourceID;

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * CFeatureUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CFeatureUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = 5; // First 5 bytes
	
	UCHAR NumberOfChannels = (m_FeatureUnitDescriptor->bLength - 6 - 1) / m_FeatureUnitDescriptor->bControlSize;

	TotalLength += NumberOfChannels * 4; // bmaControls

	TotalLength += 1; // iFeature

	return TotalLength;
}

/*****************************************************************************
 * CFeatureUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CFeatureUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_FEATURE_UNIT_DESCRIPTOR FeatureUnitDescriptor = PUSB_AUDIO_20_FEATURE_UNIT_DESCRIPTOR(Buffer);

	FeatureUnitDescriptor->bLength = 0;
	FeatureUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	FeatureUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_FEATURE_UNIT;
	FeatureUnitDescriptor->bUnitID = m_FeatureUnitDescriptor->bUnitID;
	FeatureUnitDescriptor->bSourceID = m_FeatureUnitDescriptor->bSourceID;

	Buffer += 5;
	FeatureUnitDescriptor->bLength += 5;

	// bmaControls[]
	PULONG bmaControls = PULONG(Buffer);

	UCHAR NumberOfChannels = (m_FeatureUnitDescriptor->bLength - 6 - 1) / m_FeatureUnitDescriptor->bControlSize;

	for (UCHAR ch=0; ch<NumberOfChannels; ch++)
	{
		bmaControls[ch] = 0;

		for (UCHAR i=0; (i<=9)&&(i<(m_FeatureUnitDescriptor->bControlSize*8)); i++)
		{
			if (_FindControl(ch, i, NULL))
			{
				bmaControls[ch]	|= (0x00000003<<(i*2));
			}
		}
	}
	
	Buffer += NumberOfChannels * 4;
	FeatureUnitDescriptor->bLength += NumberOfChannels * 4;

	// iFeature
	PUCHAR iFeature = PUCHAR(Buffer);

	*iFeature = iUnit();

	Buffer += 1;
	FeatureUnitDescriptor->bLength += 1;

	return FeatureUnitDescriptor->bLength;
}

/*****************************************************************************
 * CFeatureUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CFeatureUnit::
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
		case USB_AUDIO_20_FU_CONTROL_MUTE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_MUTE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_VOLUME:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_VOLUME)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_BASS:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_BASS)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(CHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(CHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_MID:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_MID)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(CHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(CHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_TREBLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_TREBLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(CHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(CHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_GRAPHIC_EQ:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_GRAPHIC_EQ)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, ParameterBlockSize);
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_AUTOMATIC_GAIN:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_AUTOMATIC_GAIN)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_DELAY:
		{					
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_DELAY)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(ULONG))
				{				
					ULONG Delay = *(PULONG(ParameterBlock));

					USHORT Current = USHORT((ULONGLONG(Delay) * 64 * 1000) / 4194304);

					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_BASS_BOOST:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_BASS_BOOST)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_LOUDNESS:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_LOUDNESS)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_INPUT_GAIN:
		case USB_AUDIO_20_FU_CONTROL_INPUT_GAIN_PAD:
		case USB_AUDIO_20_FU_CONTROL_PHASE_INVERTER:
		case USB_AUDIO_20_FU_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_FU_CONTROL_OVERFLOW:
		case USB_AUDIO_20_FU_CONTROL_LATENCY:
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
 * CFeatureUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CFeatureUnit::
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
		case USB_AUDIO_20_FU_CONTROL_MUTE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_MUTE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_FU_CONTROL_VOLUME:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_VOLUME)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(SHORT);
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.wMIN, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.wMAX, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.wRES, sizeof(SHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_BASS:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_BASS)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(CHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(CHAR), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(CHAR);
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE1)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE1 Range = PRANGE1(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.bMIN, sizeof(CHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.bMAX, sizeof(CHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.bRES, sizeof(CHAR), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE1);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_MID:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_MID)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(CHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(CHAR), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(CHAR);
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE1)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE1 Range = PRANGE1(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.bMIN, sizeof(CHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.bMAX, sizeof(CHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.bRES, sizeof(CHAR), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE1);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_TREBLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_TREBLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(CHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(CHAR), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(CHAR);
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE1)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE1 Range = PRANGE1(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.bMIN, sizeof(CHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.bMAX, sizeof(CHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.bRES, sizeof(CHAR), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE1);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_GRAPHIC_EQ:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_GRAPHIC_EQ)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				struct
				{
					ULONG	bmBandsPresent;
					CHAR	bBand[32];
				} GraphicEQ;

				ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);

				if (NT_SUCCESS(ntStatus))
				{
					USHORT NumberOfBands = 0;

					for (ULONG i=0; i<32; i++)
					{
						if (GraphicEQ.bmBandsPresent & (1<<i))
						{
							NumberOfBands++;
						}
					}

					if (ParameterBlockSize >= (sizeof(ULONG) + NumberOfBands * sizeof(CHAR)))
					{
						RtlCopyMemory(ParameterBlock, &GraphicEQ, sizeof(ULONG) + NumberOfBands * sizeof(CHAR));
					}
					else
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}

					if (OutParameterBlockSize)
					{
						*OutParameterBlockSize = sizeof(ULONG) + NumberOfBands * sizeof(CHAR);
					}
				}
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				struct
				{
					ULONG	bmBandsPresent;
					CHAR	bBand[32];
				} GraphicEQ;

				ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);

				if (NT_SUCCESS(ntStatus))
				{
					USHORT NumberOfBands = 0;

					for (ULONG i=0; i<32; i++)
					{
						if (GraphicEQ.bmBandsPresent & (1<<i))
						{
							NumberOfBands++;
						}
					}

					if (ParameterBlockSize >= (sizeof(USHORT) + NumberOfBands * sizeof(RANGE1)))
					{
						PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
						*wNumSubRanges = NumberOfBands;

						PRANGE1 Range = PRANGE1(wNumSubRanges+1);

						GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);

						for (ULONG i=0; i<NumberOfBands; i++)
						{
							Range[i].Signed.bMIN = GraphicEQ.bBand[i];
						}

						GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);

						for (ULONG i=0; i<NumberOfBands; i++)
						{
							Range[i].Signed.bMAX = GraphicEQ.bBand[i];
						}

						GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);

						for (ULONG i=0; i<NumberOfBands; i++)
						{
							Range[i].Signed.bRES = GraphicEQ.bBand[i];
						}

						ntStatus = STATUS_SUCCESS;
					}
					else
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}

					if (OutParameterBlockSize)
					{
						*OutParameterBlockSize = sizeof(USHORT) + NumberOfBands * sizeof(RANGE1);
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_AUTOMATIC_GAIN:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_AUTOMATIC_GAIN)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_FU_CONTROL_DELAY:
		{					
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_DELAY)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				USHORT Current = 0;					
				
				ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);

				if (NT_SUCCESS(ntStatus))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Delay = PULONG(ParameterBlock);

						*Delay = ULONG((ULONGLONG(Current) * 4194304) / (64 * 1000));

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
			}
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE4)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE4 Range = PRANGE4(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.dMIN, sizeof(ULONG), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.dMAX, sizeof(ULONG), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.dRES, sizeof(ULONG), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE4);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_FU_CONTROL_BASS_BOOST:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_BASS_BOOST)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_FU_CONTROL_LOUDNESS:
		{
			USHORT Control = USHORT(USB_AUDIO_10_FU_CONTROL_LOUDNESS)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_FU_CONTROL_INPUT_GAIN:
		case USB_AUDIO_20_FU_CONTROL_INPUT_GAIN_PAD:
		case USB_AUDIO_20_FU_CONTROL_PHASE_INVERTER:
		case USB_AUDIO_20_FU_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_FU_CONTROL_OVERFLOW:
		case USB_AUDIO_20_FU_CONTROL_LATENCY:
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
 * CFeatureUnit::_FindControl()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL
CFeatureUnit::
_FindControl
(
	IN		UCHAR	Channel,
	IN		UCHAR	Index,
	OUT		UCHAR *	OutControlSelector
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUCHAR bmControls = PUCHAR(m_FeatureUnitDescriptor) + USB_AUDIO_10_FEATURE_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET + (Channel * m_FeatureUnitDescriptor->bControlSize);

	UCHAR ByteOffset = Index / 8;

	UCHAR BitMask = 0x01 << (Index % 8);

	if (bmControls[ByteOffset] & BitMask)
	{
		Found = TRUE;
	}

	if (OutControlSelector)
	{
		*OutControlSelector = Index+1;
	}

	return Found;
}

#pragma code_seg()

/*****************************************************************************
 * CProcessorUnit::ProcessType()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
USHORT
CProcessorUnit::
ProcessType
(	void
)
{
	PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR ProcessDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(m_UnitDescriptor);

	return ProcessDescriptor->wProcessType;
}

/*****************************************************************************
 * CProcessorUnit::iUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CProcessorUnit::
iUnit
(	void
)
{
	UCHAR iProcessor = *(PUCHAR(m_ProcessorUnitDescriptor) + USB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(m_ProcessorUnitDescriptor->bControlSize));

	return iProcessor;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CProcessorUnit::ParseSources()
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
CProcessorUnit::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	if (Index < m_ProcessorUnitDescriptor->bNrInPins)
	{
		*OutSourceID = m_ProcessorUnitDescriptor->bSourceID;

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * CProcessorUnit::_FindControl()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL
CProcessorUnit::
_FindControl
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutControlSelector
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUCHAR bmControls = PUCHAR(m_ProcessorUnitDescriptor) + USB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET;

	UCHAR ByteOffset = Index / 8;

	UCHAR BitMask = 0x01 << (Index % 8);

	if (bmControls[ByteOffset] & BitMask)
	{
		Found = TRUE;
	}

	if (OutControlSelector)
	{
		*OutControlSelector = Index+1;
	}

	return Found;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CUpDownMixUnit::~CUpDownMixUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CUpDownMixUnit::
~CUpDownMixUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CUpDownMixUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CUpDownMixUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_UpDownMixUnitDescriptor = PUSB_AUDIO_10_UP_DOWNMIX_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUpDownMixUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CUpDownMixUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_UP_DOWNMIX_UNIT_DESCRIPTOR);

	UCHAR NumberOfModes = *(PUCHAR(m_UpDownMixUnitDescriptor) + USB_AUDIO_10_UP_DOWNMIX_UNIT_DESCRIPTOR_BNRMODES_OFFSET(m_UpDownMixUnitDescriptor->bControlSize));

	TotalLength +=  NumberOfModes * 4; // daModes[]

	return TotalLength;
}

/*****************************************************************************
 * CUpDownMixUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CUpDownMixUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_UP_DOWNMIX_UNIT_DESCRIPTOR UpDownMixUnitDescriptor = PUSB_AUDIO_20_UP_DOWNMIX_UNIT_DESCRIPTOR(Buffer);

	UpDownMixUnitDescriptor->bLength = 0;
	UpDownMixUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	UpDownMixUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_PROCESSING_UNIT;
	UpDownMixUnitDescriptor->bUnitID = m_UpDownMixUnitDescriptor->bUnitID;
	UpDownMixUnitDescriptor->wProcessType = USB_AUDIO_20_PROCESS_UPMIX_DOWNMIX;
	UpDownMixUnitDescriptor->bNrInPins = m_UpDownMixUnitDescriptor->bNrInPins;
	UpDownMixUnitDescriptor->bSourceID = m_UpDownMixUnitDescriptor->bSourceID;
	UpDownMixUnitDescriptor->bNrChannels = m_UpDownMixUnitDescriptor->bNrChannels;
	UpDownMixUnitDescriptor->bmChannelConfig = m_UpDownMixUnitDescriptor->wChannelConfig;
	UpDownMixUnitDescriptor->iChannelNames = m_UpDownMixUnitDescriptor->iChannelNames;
	UpDownMixUnitDescriptor->bmControls = 0;
	UpDownMixUnitDescriptor->bmControls |= _FindControl(0, NULL) ? 0x3 : 0;
	UpDownMixUnitDescriptor->bmControls |= _FindControl(1, NULL) ? 0xC : 0;
	UpDownMixUnitDescriptor->iProcessing = iUnit();
	UpDownMixUnitDescriptor->bNrModes = *(PUCHAR(m_UpDownMixUnitDescriptor) + USB_AUDIO_10_UP_DOWNMIX_UNIT_DESCRIPTOR_BNRMODES_OFFSET(m_UpDownMixUnitDescriptor->bControlSize));

	Buffer += sizeof(USB_AUDIO_20_UP_DOWNMIX_UNIT_DESCRIPTOR);
	UpDownMixUnitDescriptor->bLength += sizeof(USB_AUDIO_20_UP_DOWNMIX_UNIT_DESCRIPTOR);

	// daModes[]
	PULONG daModes = PULONG(Buffer);

	PUSHORT waModes = PUSHORT(PUCHAR(m_UpDownMixUnitDescriptor) + USB_AUDIO_10_UP_DOWNMIX_UNIT_DESCRIPTOR_WAMODES_OFFSET(m_UpDownMixUnitDescriptor->bControlSize));

	for (ULONG i=0; i<UpDownMixUnitDescriptor->bNrModes; i++)
	{
		daModes[i] = waModes[i];
	}

	UpDownMixUnitDescriptor->bLength += UpDownMixUnitDescriptor->bNrModes * 4;

	return UpDownMixUnitDescriptor->bLength;
}

/*****************************************************************************
 * CUpDownMixUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUpDownMixUnit::
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
		case USB_AUDIO_20_UD_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_UD_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_UD_CONTROL_MODE_SELECT:
		{
			USHORT Control = USHORT(USB_AUDIO_10_UD_CONTROL_MODE_SELECT)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_UD_CONTROL_CLUSTER:
		case USB_AUDIO_20_UD_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_UD_CONTROL_OVERFLOW:
		case USB_AUDIO_20_UD_CONTROL_LATENCY:
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
 * CUpDownMixUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUpDownMixUnit::
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
		case USB_AUDIO_20_UD_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_UD_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);

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

		case USB_AUDIO_20_UD_CONTROL_MODE_SELECT:
		{
			USHORT Control = USHORT(USB_AUDIO_10_UD_CONTROL_MODE_SELECT)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);

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

		case USB_AUDIO_20_UD_CONTROL_CLUSTER:
		case USB_AUDIO_20_UD_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_UD_CONTROL_OVERFLOW:
		case USB_AUDIO_20_UD_CONTROL_LATENCY:
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
 * CDolbyPrologicUnit::~CDolbyPrologicUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CDolbyPrologicUnit::
~CDolbyPrologicUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CDolbyPrologicUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CDolbyPrologicUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_PrologicUnitDescriptor = PUSB_AUDIO_10_PROLOGIC_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CDolbyPrologicUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CDolbyPrologicUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_PROLOGIC_UNIT_DESCRIPTOR);

	UCHAR NumberOfModes = *(PUCHAR(m_PrologicUnitDescriptor) + USB_AUDIO_10_PROLOGIC_UNIT_DESCRIPTOR_BNRMODES_OFFSET(m_PrologicUnitDescriptor->bControlSize));

	TotalLength +=  NumberOfModes * 4; // daModes[]

	return TotalLength;
}

/*****************************************************************************
 * CDolbyPrologicUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CDolbyPrologicUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_PROLOGIC_UNIT_DESCRIPTOR PrologicUnitDescriptor = PUSB_AUDIO_20_PROLOGIC_UNIT_DESCRIPTOR(Buffer);

	PrologicUnitDescriptor->bLength = 0;
	PrologicUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	PrologicUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_PROCESSING_UNIT;
	PrologicUnitDescriptor->bUnitID = m_PrologicUnitDescriptor->bUnitID;
	PrologicUnitDescriptor->wProcessType = USB_AUDIO_20_PROCESS_DOLBY_PROLOGIC;
	PrologicUnitDescriptor->bNrInPins = m_PrologicUnitDescriptor->bNrInPins;
	PrologicUnitDescriptor->bSourceID = m_PrologicUnitDescriptor->bSourceID;
	PrologicUnitDescriptor->bNrChannels = m_PrologicUnitDescriptor->bNrChannels;
	PrologicUnitDescriptor->bmChannelConfig = m_PrologicUnitDescriptor->wChannelConfig;
	PrologicUnitDescriptor->iChannelNames = m_PrologicUnitDescriptor->iChannelNames;
	PrologicUnitDescriptor->bmControls = 0;
	PrologicUnitDescriptor->bmControls |= _FindControl(0, NULL) ? 0x3 : 0;
	PrologicUnitDescriptor->bmControls |= _FindControl(1, NULL) ? 0xC : 0;
	PrologicUnitDescriptor->iProcessing = iUnit();
	PrologicUnitDescriptor->bNrModes = *(PUCHAR(m_PrologicUnitDescriptor) + USB_AUDIO_10_PROLOGIC_UNIT_DESCRIPTOR_BNRMODES_OFFSET(m_PrologicUnitDescriptor->bControlSize));

	Buffer += sizeof(USB_AUDIO_20_PROLOGIC_UNIT_DESCRIPTOR);
	PrologicUnitDescriptor->bLength += sizeof(USB_AUDIO_20_PROLOGIC_UNIT_DESCRIPTOR);

	// daModes[]
	PULONG daModes = PULONG(Buffer);

	PUSHORT waModes = PUSHORT(PUCHAR(m_PrologicUnitDescriptor) + USB_AUDIO_10_PROLOGIC_UNIT_DESCRIPTOR_WAMODES_OFFSET(m_PrologicUnitDescriptor->bControlSize));

	for (ULONG i=0; i<PrologicUnitDescriptor->bNrModes; i++)
	{
		daModes[i] = waModes[i];
	}

	PrologicUnitDescriptor->bLength += PrologicUnitDescriptor->bNrModes * 4;

	return PrologicUnitDescriptor->bLength;
}

/*****************************************************************************
 * CDolbyPrologicUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDolbyPrologicUnit::
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
		case USB_AUDIO_20_DP_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_DP_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DP_CONTROL_MODE_SELECT:
		{
			USHORT Control = USHORT(USB_AUDIO_10_DP_CONTROL_MODE_SELECT)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DP_CONTROL_CLUSTER:
		case USB_AUDIO_20_DP_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_DP_CONTROL_OVERFLOW:
		case USB_AUDIO_20_DP_CONTROL_LATENCY:
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
 * CDolbyPrologicUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDolbyPrologicUnit::
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
		case USB_AUDIO_20_DP_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_DP_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);

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

		case USB_AUDIO_20_DP_CONTROL_MODE_SELECT:
		{
			USHORT Control = USHORT(USB_AUDIO_10_DP_CONTROL_MODE_SELECT)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);

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

		case USB_AUDIO_20_DP_CONTROL_CLUSTER:
		case USB_AUDIO_20_DP_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_DP_CONTROL_OVERFLOW:
		case USB_AUDIO_20_DP_CONTROL_LATENCY:
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
 * C3dStereoExtenderUnit::~C3dStereoExtenderUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
C3dStereoExtenderUnit::
~C3dStereoExtenderUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * C3dStereoExtenderUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
C3dStereoExtenderUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_3dExtenderUnitDescriptor = PUSB_AUDIO_10_3D_EXTENDER_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
C3dStereoExtenderUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_STEREO_EXTENDER_UNIT_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
C3dStereoExtenderUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_STEREO_EXTENDER_UNIT_DESCRIPTOR StereoExtenderUnitDescriptor = PUSB_AUDIO_20_STEREO_EXTENDER_UNIT_DESCRIPTOR(Buffer);

	StereoExtenderUnitDescriptor->bLength = sizeof(USB_AUDIO_20_STEREO_EXTENDER_UNIT_DESCRIPTOR);
	StereoExtenderUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	StereoExtenderUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_PROCESSING_UNIT;
	StereoExtenderUnitDescriptor->bUnitID = m_3dExtenderUnitDescriptor->bUnitID;
	StereoExtenderUnitDescriptor->wProcessType = USB_AUDIO_20_PROCESS_STEREO_EXTENDER;
	StereoExtenderUnitDescriptor->bNrInPins = m_3dExtenderUnitDescriptor->bNrInPins;
	StereoExtenderUnitDescriptor->bSourceID = m_3dExtenderUnitDescriptor->bSourceID;
	StereoExtenderUnitDescriptor->bNrChannels = m_3dExtenderUnitDescriptor->bNrChannels;
	StereoExtenderUnitDescriptor->bmChannelConfig = m_3dExtenderUnitDescriptor->wChannelConfig;
	StereoExtenderUnitDescriptor->iChannelNames = m_3dExtenderUnitDescriptor->iChannelNames;
	StereoExtenderUnitDescriptor->bmControls = 0;
	StereoExtenderUnitDescriptor->bmControls |= _FindControl(0, NULL) ? 0x3 : 0;
	StereoExtenderUnitDescriptor->bmControls |= _FindControl(1, NULL) ? 0xC : 0;
	StereoExtenderUnitDescriptor->iProcessing = iUnit(); 

	return StereoExtenderUnitDescriptor->bLength;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
C3dStereoExtenderUnit::
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
		case USB_AUDIO_20_ST_EXT_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_3D_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_ST_EXT_CONTROL_WIDTH:
		{
			USHORT Control = USHORT(USB_AUDIO_10_3D_CONTROL_SPACIOUSNESS)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_ST_EXT_CONTROL_CLUSTER:
		case USB_AUDIO_20_ST_EXT_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_ST_EXT_CONTROL_OVERFLOW:
		case USB_AUDIO_20_ST_EXT_CONTROL_LATENCY:
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
 * C3dStereoExtenderUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
C3dStereoExtenderUnit::
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

	USHORT Control = (USHORT(ControlSelector)<<8) | ChannelNumber;

	switch (ControlSelector)
	{
		case USB_AUDIO_20_ST_EXT_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_3D_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);

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

		case USB_AUDIO_20_ST_EXT_CONTROL_WIDTH:
		{
			USHORT Control = USHORT(USB_AUDIO_10_3D_CONTROL_SPACIOUSNESS)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);

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

		case USB_AUDIO_20_ST_EXT_CONTROL_CLUSTER:
		case USB_AUDIO_20_ST_EXT_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_ST_EXT_CONTROL_OVERFLOW:
		case USB_AUDIO_20_ST_EXT_CONTROL_LATENCY:
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
 * CReverberationUnit::~CReverberationUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CReverberationUnit::
~CReverberationUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CReverberationUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CReverberationUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_ReverberationUnitDescriptor = PUSB_AUDIO_10_REVERBERATION_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CReverberationUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CReverberationUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_REVERBERATION_UNIT_DESCRIPTOR);

	TotalLength += m_ReverberationUnitDescriptor->bNrChannels * 4; // bmaControls[]

	TotalLength += 1; // iEffects

	return TotalLength;
}

/*****************************************************************************
 * CReverberationUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CReverberationUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_REVERBERATION_UNIT_DESCRIPTOR ReverberationUnitDescriptor = PUSB_AUDIO_20_REVERBERATION_UNIT_DESCRIPTOR(Buffer);

	ReverberationUnitDescriptor->bLength = 0;
	ReverberationUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	ReverberationUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_EFFECT_UNIT;
	ReverberationUnitDescriptor->bUnitID = m_ReverberationUnitDescriptor->bUnitID;
	ReverberationUnitDescriptor->wEffectType = USB_AUDIO_20_EFFECT_REVERBERATION;
	ReverberationUnitDescriptor->bSourceID = m_ReverberationUnitDescriptor->bSourceID;

	Buffer += sizeof(USB_AUDIO_20_REVERBERATION_UNIT_DESCRIPTOR);
	ReverberationUnitDescriptor->bLength += sizeof(USB_AUDIO_20_REVERBERATION_UNIT_DESCRIPTOR);

	// bmaControls[]
	PULONG bmaControls = PULONG(Buffer);

	for (ULONG ch=0; ch<m_ReverberationUnitDescriptor->bNrChannels; ch++)
	{
		bmaControls[ch] = 0;

		for (UCHAR i=0; i<=4; i++)
		{
			if (_FindControl(i, NULL))
			{
				bmaControls[ch]	|= (0x00000003<<(i*2));
			}
		}
	}

	Buffer += m_ReverberationUnitDescriptor->bNrChannels * 4;
	ReverberationUnitDescriptor->bLength += m_ReverberationUnitDescriptor->bNrChannels * 4; 
	
	// iEffects
	PUCHAR iEffects = PUCHAR(Buffer);

	* iEffects = iUnit();

	Buffer += 1;
	ReverberationUnitDescriptor->bLength += 1; 

	return ReverberationUnitDescriptor->bLength;
}

/*****************************************************************************
 * CReverberationUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CReverberationUnit::
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
		case USB_AUDIO_20_RV_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_RV_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_TYPE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_RV_CONTROL_TYPE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_LEVEL:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_RV_CONTROL_LEVEL)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_TIME:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_RV_CONTROL_TIME)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_FEEDBACK:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_RV_CONTROL_FEEDBACK)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_PREDELAY:
		case USB_AUDIO_20_RV_CONTROL_DENSITY:
		case USB_AUDIO_20_RV_CONTROL_HIFREQ_ROLLOFF:
		case USB_AUDIO_20_RV_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_RV_CONTROL_OVERFLOW:
		case USB_AUDIO_20_RV_CONTROL_LATENCY:
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
 * CReverberationUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CReverberationUnit::
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
		case USB_AUDIO_20_RV_CONTROL_ENABLE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_RV_CONTROL_ENABLE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_RV_CONTROL_TYPE:
		{
			USHORT Control = USHORT(USB_AUDIO_10_RV_CONTROL_TYPE)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE1)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE1 Range = PRANGE1(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.bMIN, sizeof(UCHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.bMAX, sizeof(UCHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.bRES, sizeof(UCHAR), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE1);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_LEVEL:
		{
			USHORT Control = USHORT(USB_AUDIO_10_RV_CONTROL_LEVEL)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE1)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE1 Range = PRANGE1(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.bMIN, sizeof(UCHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.bMAX, sizeof(UCHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.bRES, sizeof(UCHAR), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE1);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_TIME:
		{
			USHORT Control = USHORT(USB_AUDIO_10_RV_CONTROL_TIME)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.wMIN, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.wMAX, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.wRES, sizeof(USHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_FEEDBACK:
		{
			USHORT Control = USHORT(USB_AUDIO_10_RV_CONTROL_FEEDBACK)<<8 | USHORT(ChannelNumber);

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE1)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE1 Range = PRANGE1(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.bMIN, sizeof(UCHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.bMAX, sizeof(UCHAR), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.bRES, sizeof(UCHAR), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE1);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_PREDELAY:
		case USB_AUDIO_20_RV_CONTROL_DENSITY:
		case USB_AUDIO_20_RV_CONTROL_HIFREQ_ROLLOFF:
		case USB_AUDIO_20_RV_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_RV_CONTROL_OVERFLOW:
		case USB_AUDIO_20_RV_CONTROL_LATENCY:
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
 * CChorusUnit::~CChorusUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CChorusUnit::
~CChorusUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CChorusUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CChorusUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_ChorusUnitDescriptor = PUSB_AUDIO_10_CHORUS_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CChorusUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CChorusUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_MODULATION_DELAY_UNIT_DESCRIPTOR);

	TotalLength += m_ChorusUnitDescriptor->bNrChannels * 4; // bmaControls[]

	TotalLength += 1; // iEffects

	return TotalLength;
}

/*****************************************************************************
 * CChorusUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CChorusUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_MODULATION_DELAY_UNIT_DESCRIPTOR ModulationDelayUnitDescriptor = PUSB_AUDIO_20_MODULATION_DELAY_UNIT_DESCRIPTOR(Buffer);

	ModulationDelayUnitDescriptor->bLength = 0;
	ModulationDelayUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	ModulationDelayUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_EFFECT_UNIT;
	ModulationDelayUnitDescriptor->bUnitID = m_ChorusUnitDescriptor->bUnitID;
	ModulationDelayUnitDescriptor->wEffectType = USB_AUDIO_20_EFFECT_MODULATION_DELAY;
	ModulationDelayUnitDescriptor->bSourceID = m_ChorusUnitDescriptor->bSourceID;

	Buffer += sizeof(USB_AUDIO_20_MODULATION_DELAY_UNIT_DESCRIPTOR);
	ModulationDelayUnitDescriptor->bLength += sizeof(USB_AUDIO_20_MODULATION_DELAY_UNIT_DESCRIPTOR);

	// bmaControls[]
	PULONG bmaControls = PULONG(Buffer);

	for (ULONG ch=0; ch<m_ChorusUnitDescriptor->bNrChannels; ch++)
	{
		bmaControls[ch] = 0;

		for (UCHAR i=0; i<=3; i++)
		{
			if (_FindControl(i, NULL))
			{
				bmaControls[ch]	|= (0x00000003<<(i*2));
			}
		}

		bmaControls[ch] &= ~(0x3<<USB_AUDIO_20_MD_CONTROL_BALANCE); // remove unsupported balance control.
	}

	Buffer += m_ChorusUnitDescriptor->bNrChannels * 4;
	ModulationDelayUnitDescriptor->bLength += m_ChorusUnitDescriptor->bNrChannels * 4; 
	
	// iEffects
	PUCHAR iEffects = PUCHAR(Buffer);

	* iEffects = iUnit();

	Buffer += 1;
	ModulationDelayUnitDescriptor->bLength += 1; 

	return ModulationDelayUnitDescriptor->bLength;
}

/*****************************************************************************
 * CChorusUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CChorusUnit::
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
		case USB_AUDIO_20_MD_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_CH_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_BALANCE:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_RATE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_CH_CONTROL_RATE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_DEPTH:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_CH_CONTROL_DEPTH)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_TIME:
		case USB_AUDIO_20_MD_CONTROL_FEEDBACK:
		case USB_AUDIO_20_MD_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_MD_CONTROL_OVERFLOW:
		case USB_AUDIO_20_MD_CONTROL_LATENCY:
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
 * CChorusUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CChorusUnit::
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
		case USB_AUDIO_20_MD_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_CH_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_MD_CONTROL_BALANCE:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_RATE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_CH_CONTROL_RATE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.wMIN, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.wMAX, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.wRES, sizeof(USHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_DEPTH:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_CH_CONTROL_DEPTH)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.wMIN, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.wMAX, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.wRES, sizeof(USHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_MD_CONTROL_TIME:
		case USB_AUDIO_20_MD_CONTROL_FEEDBACK:
		case USB_AUDIO_20_MD_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_MD_CONTROL_OVERFLOW:
		case USB_AUDIO_20_MD_CONTROL_LATENCY:
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
 * CDynamicRangeCompressionUnit::~CDynamicRangeCompressionUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CDynamicRangeCompressionUnit::
~CDynamicRangeCompressionUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CDynamicRangeCompressionUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_DrcUnitDescriptor = PUSB_AUDIO_10_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CDynamicRangeCompressionUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR);

	TotalLength += m_DrcUnitDescriptor->bNrChannels * 4; // bmaControls[]

	TotalLength += 1; // iEffects

	return TotalLength;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CDynamicRangeCompressionUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_AUDIO_20_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR DrcUnitDescriptor = PUSB_AUDIO_20_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR(Buffer);

	DrcUnitDescriptor->bLength = 0;
	DrcUnitDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	DrcUnitDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_EFFECT_UNIT;
	DrcUnitDescriptor->bUnitID = m_DrcUnitDescriptor->bUnitID;
	DrcUnitDescriptor->wEffectType = USB_AUDIO_20_EFFECT_DYNAMIC_RANGE_COMPRESSION;
	DrcUnitDescriptor->bSourceID = m_DrcUnitDescriptor->bSourceID;

	Buffer += sizeof(USB_AUDIO_20_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR);
	DrcUnitDescriptor->bLength += sizeof(USB_AUDIO_20_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR);

	// bmaControls[]
	PULONG bmaControls = PULONG(Buffer);

	for (ULONG ch=0; ch<m_DrcUnitDescriptor->bNrChannels; ch++)
	{
		bmaControls[ch] = 0;

		for (UCHAR i=0; i<=5; i++)
		{
			if (_FindControl(i, NULL))
			{
				bmaControls[ch]	|= (0x00000003<<(i*2));
			}
		}
	}

	Buffer += m_DrcUnitDescriptor->bNrChannels * 4;
	DrcUnitDescriptor->bLength += m_DrcUnitDescriptor->bNrChannels * 4; 
	
	// iEffects
	PUCHAR iEffects = PUCHAR(Buffer);

	* iEffects = iUnit();

	Buffer += 1;
	DrcUnitDescriptor->bLength += 1; 

	return DrcUnitDescriptor->bLength;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDynamicRangeCompressionUnit::
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
		case USB_AUDIO_20_DR_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_COMPRESSION_RATIO:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_COMPRESSION_RATIO)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_MAX_AMPLITUDE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_MAX_AMPLITUDE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_THRESHOLD:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_THRESHOLD)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_ATTACK_TIME:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_ATTACK_TIME)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_RELEASE_TIME:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_RELEASE_TIME)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_RV_CONTROL_OVERFLOW:
		case USB_AUDIO_20_RV_CONTROL_LATENCY:
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
 * CDynamicRangeCompressionUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDynamicRangeCompressionUnit::
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
		case USB_AUDIO_20_DR_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_DR_CONTROL_COMPRESSION_RATIO:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_COMPRESSION_RATIO)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.wMIN, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.wMAX, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.wRES, sizeof(USHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_MAX_AMPLITUDE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_MAX_AMPLITUDE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.wMIN, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.wMAX, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.wRES, sizeof(SHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_THRESHOLD:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_THRESHOLD)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(SHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(SHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Signed.wMIN, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Signed.wMAX, sizeof(SHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Signed.wRES, sizeof(SHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_ATTACK_TIME:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_ATTACK_TIME)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.wMIN, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.wMAX, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.wRES, sizeof(USHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_DR_CONTROL_RELEASE_TIME:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_DR_CONTROL_RELEASE_TIME)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(USHORT))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(USHORT), NULL);
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
			else if (RequestCode == USB_AUDIO_20_REQUEST_RANGE)
			{
				if (ParameterBlockSize >= (sizeof(USHORT) + sizeof(RANGE2)))
				{
					PUSHORT wNumSubRanges = PUSHORT(ParameterBlock);
					*wNumSubRanges = 1;

					PRANGE2 Range = PRANGE2(wNumSubRanges+1);

					GetRequest(USB_AUDIO_10_REQUEST_MIN, Control, &Range->Unsigned.wMIN, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_MAX, Control, &Range->Unsigned.wMAX, sizeof(USHORT), NULL);
					GetRequest(USB_AUDIO_10_REQUEST_RES, Control, &Range->Unsigned.wRES, sizeof(USHORT), NULL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(USHORT) + sizeof(RANGE2);
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_RV_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_RV_CONTROL_OVERFLOW:
		case USB_AUDIO_20_RV_CONTROL_LATENCY:
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
 * CExtensionUnit::~CExtensionUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CExtensionUnit::
~CExtensionUnit
(	void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CExtensionUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Unit's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CExtensionUnit::
Init
(
	IN		CUsbDevice *							UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_ExtensionUnitDescriptor = PUSB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	return STATUS_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CExtensionUnit::iUnit()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CExtensionUnit::
iUnit
(	void
)
{
	UCHAR ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));

	UCHAR iExtension = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR_IEXTENSION_OFFSET(m_ExtensionUnitDescriptor->bNrInPins, ControlSize));

	return iExtension;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CExtensionUnit::ParseSources()
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
CExtensionUnit::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	for (UCHAR i=0; i<m_ExtensionUnitDescriptor->bNrInPins; i++)
	{
		if (Index == i)
		{
			PUCHAR baSourceID = PUCHAR(m_ExtensionUnitDescriptor)+USB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

			*OutSourceID = baSourceID[i];

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CExtensionUnit::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CExtensionUnit::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	ULONG TotalLength = sizeof(USB_AUDIO_20_EXTENSION_UNIT_DESCRIPTOR);

	TotalLength += m_ExtensionUnitDescriptor->bNrInPins; // baSourceID[]

	TotalLength += sizeof(USB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR); // cluster descriptor

	TotalLength += 1; // bmControls

	TotalLength += 1; // iExtension

	return TotalLength;
}

/*****************************************************************************
 * CExtensionUnit::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CExtensionUnit::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();
	
	PUSB_AUDIO_20_EXTENSION_UNIT_DESCRIPTOR ExtensionDescriptor = PUSB_AUDIO_20_EXTENSION_UNIT_DESCRIPTOR(Buffer);

	ExtensionDescriptor->bLength = 0;
	ExtensionDescriptor->bDescriptorType = USB_AUDIO_20_CS_INTERFACE;
	ExtensionDescriptor->bDescriptorSubtype = USB_AUDIO_20_AC_DESCRIPTOR_EXTENSION_UNIT;
	ExtensionDescriptor->bUnitID = m_ExtensionUnitDescriptor->bUnitID;
	ExtensionDescriptor->wExtensionCode = m_ExtensionUnitDescriptor->wExtensionCode;
	ExtensionDescriptor->bNrInPins = m_ExtensionUnitDescriptor->bNrInPins;

	Buffer += sizeof(USB_AUDIO_20_EXTENSION_UNIT_DESCRIPTOR);
	ExtensionDescriptor->bLength += sizeof(USB_AUDIO_20_EXTENSION_UNIT_DESCRIPTOR);

	// baSourceID[]
	RtlCopyMemory(Buffer, PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR_BASOURCEID_OFFSET, m_ExtensionUnitDescriptor->bNrInPins);

	Buffer += m_ExtensionUnitDescriptor->bNrInPins; 
	ExtensionDescriptor->bLength += m_ExtensionUnitDescriptor->bNrInPins; 

	// cluster descriptor
	PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor10 = PUSB_AUDIO_10_CHANNEL_CLUSTER_DESCRIPTOR(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR_CLUSTER_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));
	PUSB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor20 = PUSB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR(Buffer);

	ClusterDescriptor20->bNrChannels = ClusterDescriptor10->bNrChannels;
	ClusterDescriptor20->bmChannelConfig = ClusterDescriptor10->wChannelConfig;
	ClusterDescriptor20->iChannelNames = ClusterDescriptor10->iChannelNames;

	Buffer += sizeof(USB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR);
	ExtensionDescriptor->bLength += sizeof(USB_AUDIO_20_CHANNEL_CLUSTER_DESCRIPTOR);
	
	// bmControls
	PUCHAR bmControls = PUCHAR(Buffer);

	*bmControls = _FindControl(0, NULL) ? 0x3 : 0;

	Buffer += 1;
	ExtensionDescriptor->bLength += 1;

	// iExtension
	PUCHAR iExtension = PUCHAR(Buffer);
	
	*iExtension = iUnit();

	Buffer += 1;
	ExtensionDescriptor->bLength += 1; 

	return ExtensionDescriptor->bLength;
}

/*****************************************************************************
 * CExtensionUnit::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CExtensionUnit::
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
		case USB_AUDIO_20_XU_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_XU_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR));
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;

		case USB_AUDIO_20_XU_CONTROL_CLUSTER:
		case USB_AUDIO_20_XU_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_XU_CONTROL_OVERFLOW:
		case USB_AUDIO_20_XU_CONTROL_LATENCY:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		default:
		{
			USHORT Control = (USHORT(ControlSelector+2-USB_AUDIO_20_XU_NUM_CONTROL_SELECTORS)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				ntStatus = SetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, ParameterBlockSize);
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CExtensionUnit::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CExtensionUnit::
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
		case USB_AUDIO_20_XU_CONTROL_ENABLE:
		{
			USHORT Control = (USHORT(USB_AUDIO_10_XU_CONTROL_ENABLE)<<8) | ChannelNumber;

			if (RequestCode == USB_AUDIO_20_REQUEST_CUR)
			{
				if (ParameterBlockSize >= sizeof(UCHAR))
				{
					ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, sizeof(UCHAR), NULL);
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

		case USB_AUDIO_20_XU_CONTROL_CLUSTER:
		case USB_AUDIO_20_XU_CONTROL_UNDERFLOW:
		case USB_AUDIO_20_XU_CONTROL_OVERFLOW:
		case USB_AUDIO_20_XU_CONTROL_LATENCY:
		{
			ntStatus = STATUS_NOT_SUPPORTED;
		}
		break;

		default:
		{
			USHORT Control = (USHORT(ControlSelector+2-USB_AUDIO_20_XU_NUM_CONTROL_SELECTORS)<<8) | ChannelNumber;

			ntStatus = GetRequest(USB_AUDIO_10_REQUEST_CUR, Control, ParameterBlock, ParameterBlockSize, OutParameterBlockSize);
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CExtensionUnit::_FindControl()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL
CExtensionUnit::
_FindControl
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutControlSelector
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUCHAR bmControls = PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(m_ExtensionUnitDescriptor->bNrInPins);

	UCHAR ByteOffset = Index / 8;

	UCHAR BitMask = 0x01 << (Index % 8);

	if (bmControls[ByteOffset] & BitMask)
	{
		Found = TRUE;
	}

	*OutControlSelector = Index+1;

	return Found;
}

#pragma code_seg()

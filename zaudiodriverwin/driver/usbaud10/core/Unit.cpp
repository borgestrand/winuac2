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
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Unit.h"
#include "Audio.h"

#define STR_MODULENAME "Unit: "



#pragma code_seg()

/*****************************************************************************
 * SGN_16X16()
 *****************************************************************************
 * @brief
 */
inline
LONG SGN_16X16
(
	IN		SHORT	Level16
)
{
	LONG Level32;

	if (Level16 == INFINITY)
	{
		Level32 = (INFINITY * dB);
	}
	else
	{
		Level32 = Level16 * (dB/256);
	}

	return Level32;
}

/*****************************************************************************
 * SGN_16X16()
 *****************************************************************************
 * @brief
 */
inline
LONG SGN_16X16
(
	IN		CHAR	Level8
)
{
	LONG Level32 = Level8 * (dB/4);

	return Level32;
}

/*****************************************************************************
 * SGN_8X8()
 *****************************************************************************
 * @brief
 */
inline
SHORT SGN_8X8
(
	IN		LONG	Level32
)
{
	SHORT Level16;

	if (Level32 == (INFINITY * dB))
	{
		Level16 = INFINITY;
	}
	else
	{
		Level16 = SHORT(Level32 / (dB/256));
	}

	return Level16;
}

/*****************************************************************************
 * SGN_6X2()
 *****************************************************************************
 * @brief
 */
inline
CHAR SGN_6X2
(
	IN		LONG	Level32
)
{
	CHAR Level8 = CHAR(Level32 / (dB/4));

	return Level8;
}

/*****************************************************************************
 * PCT_16X16()
 *****************************************************************************
 * @brief
 */
inline
ULONG PCT_16X16
(
	IN		UCHAR	Percent8
)
{
	ULONG Percent32 = Percent8 * 0x10000 / 100; // 0x10000 is hundred percent

	return Percent32;
}

/*****************************************************************************
 * PCT_8X0()
 *****************************************************************************
 * @brief
 */
inline
UCHAR PCT_8X0
(
	IN		ULONG	Percent32
)
{
	UCHAR Percent8 = UCHAR(Percent32 * 100 / 0x10000);

	return Percent8;
}

/*****************************************************************************
 * SEC_16X16()
 *****************************************************************************
 * @brief
 */
inline
ULONG SEC_16X16
(
	IN		USHORT	Second16
)
{
	ULONG Second32 = Second16 * 0x10000 / 256; // 0x10000 is 1 second

	return Second32;
}

/*****************************************************************************
 * SEC_8X8()
 *****************************************************************************
 * @brief
 */
inline
USHORT SEC_8X8
(
	IN		ULONG	Second32
)
{
	USHORT Second16 = USHORT(Second32 * 256 / 0x10000);

	return Second16;
}

/*****************************************************************************
 * HZ_16X16()
 *****************************************************************************
 * @brief
 */
inline
ULONG HZ_16X16
(
	IN		USHORT	Hz16
)
{
	ULONG Hz32 = Hz16 * 0x10000 / 256; // 0x10000 is 1 Hz

	return Hz32;
}

/*****************************************************************************
 * HZ_8X8()
 *****************************************************************************
 * @brief
 */
inline
USHORT HZ_8X8
(
	IN		ULONG	Hz32
)
{
	USHORT Hz16 = USHORT(Hz32 * 256 / 0x10000);

	return Hz16;
}

/*****************************************************************************
 * MS_16X16()
 *****************************************************************************
 * @brief
 */
inline
ULONG MS_16X16
(
	IN		USHORT	Ms16
)
{
	ULONG Ms32 = Ms16 * 0x10000 / 256; // 0x10000 is 1ms

	return Ms32;
}

/*****************************************************************************
 * MS_8X8()
 *****************************************************************************
 * @brief
 */
inline
USHORT MS_8X8
(
	IN		ULONG	Ms32
)
{
	USHORT Ms16 = USHORT(Ms32 * 256 / 0x10000);

	return Ms16;
}

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
 * CUnit::UpdateParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUnit::
UpdateParameterBlock
(	void
)
{
	PAGED_CODE();

	if (m_ParameterBlockStatusType & USB_AUDIO_STATUS_TYPE_INTERRUPT_PENDING)
	{
		// Service the interrupt.
		if (m_ParameterBlockStatusType & USB_AUDIO_STATUS_TYPE_MEMORY_CHANGED)
		{
			//FIXME: Is this right ???
			ULONG Memory = 0;
	
			GetRequest(REQUEST_MEM, 0, &Memory, sizeof(ULONG), NULL);
		}
		else
		{
			ULONG Status = 0;
			
			GetRequest(REQUEST_STAT, 0, &Status, sizeof(ULONG), NULL);
		}

		RestoreParameterBlock();

		m_ParameterBlockStatusType = 0;
	}

	return STATUS_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CUnit::InvalidateParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUnit::
InvalidateParameterBlock
(
	IN		UCHAR	StatusType
)
{
	m_ParameterBlockStatusType = StatusType;

	return STATUS_SUCCESS;
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

	if (m_ParameterBlock)
	{
		ExFreePool(m_ParameterBlock);
	}

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CMixerUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_MixerUnitDescriptor = PUSB_AUDIO_MIXER_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	// Get these later
	m_NumInputChannels = 0;

	m_NumOutputChannels = 0;

	m_ParameterBlockSize = 0;

	m_ParameterBlock = NULL;

	return ntStatus;
}

/*****************************************************************************
 * CMixerUnit::Configure()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
Configure
( void
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_NumInputChannels = NumberOfChannels(1);

	m_NumOutputChannels = NumberOfChannels(0);

	m_ParameterBlockSize = m_NumInputChannels * m_NumOutputChannels * sizeof(MIXER_UNIT_PARAMETER_BLOCK);

	m_ParameterBlock = (PMIXER_UNIT_PARAMETER_BLOCK)ExAllocatePoolWithTag(NonPagedPool, m_ParameterBlockSize, 'mdW');

	if (m_ParameterBlock)
	{
		RtlZeroMemory(m_ParameterBlock, m_ParameterBlockSize);
	}
	else
	{
		ntStatus = STATUS_NO_MEMORY;
	}

	if (NT_SUCCESS(ntStatus))
	{
		RestoreParameterBlock();
	}

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
	ULONG ControlSize = m_NumInputChannels * m_NumOutputChannels;

	if (ControlSize % 8)
	{
		ControlSize = (ControlSize / 8) + 1;
	}
	else
	{
		ControlSize = ControlSize / 8;
	}

	UCHAR iMixer = *(PUCHAR(m_MixerUnitDescriptor) + USB_AUDIO_MIXER_UNIT_DESCRIPTOR_IMIXER_OFFSET(m_MixerUnitDescriptor->bNrInPins, ControlSize));

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

	PUCHAR baSourceID = PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

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
 * CMixerUnit::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
CMixerUnit::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	PUSB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor = PUSB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR(PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_MIXER_UNIT_DESCRIPTOR_CLUSTER_OFFSET(m_MixerUnitDescriptor->bNrInPins));

	OutClusterDescriptor->bNrChannels = ClusterDescriptor->bNrChannels;
	OutClusterDescriptor->wChannelConfig = ClusterDescriptor->wChannelConfig;
	OutClusterDescriptor->iChannelNames = ClusterDescriptor->iChannelNames;

	return TRUE;
}

/*****************************************************************************
 * CMixerUnit::NumberOfChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CMixerUnit::
NumberOfChannels
(
	IN		BOOL	Direction
)
{
	PAGED_CODE();

	USHORT NumChannels = 0;

	if (Direction == 0) // Output
	{
		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

		if (FindAudioChannelCluster(&ClusterDescriptor))
		{
			NumChannels = ClusterDescriptor.bNrChannels;
		}
	}
	else // Input
	{
		PUCHAR baSourceID = PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

		for (UCHAR i=0; i<m_MixerUnitDescriptor->bNrInPins; i++)
		{
			PENTITY Entity = NULL;

			if (m_AudioTopology->FindEntity(baSourceID[i], &Entity))
			{
				switch (Entity->DescriptorSubtype())
				{
					case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
					case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
					{
						CTerminal * Terminal = (CTerminal*)Entity;

						USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
		
						if (Terminal->FindAudioChannelCluster(&ClusterDescriptor))
						{
							NumChannels += ClusterDescriptor.bNrChannels;
						}				
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
					{
						CUnit * Unit = (CUnit*)Entity;

						USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
		
						if (Unit->FindAudioChannelCluster(&ClusterDescriptor))
						{
							NumChannels += ClusterDescriptor.bNrChannels;
						}				
					}
					break;
				}
			}
		}
	}

	return NumChannels;
}

/*****************************************************************************
 * CMixerUnit::NumberOfInputChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CMixerUnit::
NumberOfInputChannels
(
	IN		UCHAR	Index,
	OUT		ULONG *	OutChannelOffset
)
{
	PAGED_CODE();

	USHORT NumChannels = 0;
	
	ULONG ChannelOffset = 0;

	PUCHAR baSourceID = PUCHAR(m_MixerUnitDescriptor)+USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

	for (UCHAR i=0; i<m_MixerUnitDescriptor->bNrInPins; i++)
	{
		PENTITY Entity = NULL;

		if (m_AudioTopology->FindEntity(baSourceID[i], &Entity))
		{
			switch (Entity->DescriptorSubtype())
			{
				case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
				case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
				{
					CTerminal * Terminal = (CTerminal*)Entity;

					USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
	
					if (Terminal->FindAudioChannelCluster(&ClusterDescriptor))
					{
						if (Index != i)
						{
							ChannelOffset += ClusterDescriptor.bNrChannels;
						}
						NumChannels = ClusterDescriptor.bNrChannels;
					}				
				}
				break;

				case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
				{
					CUnit * Unit = (CUnit*)Entity;

					USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
	
					if (Unit->FindAudioChannelCluster(&ClusterDescriptor))
					{
						if (Index != i)
						{
							ChannelOffset += ClusterDescriptor.bNrChannels;
						}
						NumChannels = ClusterDescriptor.bNrChannels;
					}				
				}
				break;
			}
		}

		if (Index == i)
		{
			break;
		}
	}

	if (OutChannelOffset)
	{
		*OutChannelOffset = ChannelOffset;
	}

	return NumChannels;
}

/*****************************************************************************
 * CMixerUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CMixerUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(m_ParameterBlock, m_ParameterBlockSize);
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR	InputChannelNumber,
	IN		UCHAR	OutputChannelNumber,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = (USHORT(InputChannelNumber)<<8) | (OutputChannelNumber);

	if (Control == 0x0000)
	{
		// Third form of mixer control parameter block.
		ntStatus = STATUS_NOT_SUPPORTED;
	}
	else if (Control == 0xFFFF)
	{
		// Second form of mixer control parameter block.
		if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
			(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
		{
			ULONG NumProgrammableControls = 0;

			for (ULONG n=0, p=0; n<m_NumInputChannels; n++)
			{
				for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
				{
					if (m_ParameterBlock[p].Programmable)
					{
						NumProgrammableControls++;
					}
				}
			}

			if (ParameterBlockSize >= (NumProgrammableControls * sizeof(LONG)))
			{
				PLONG Levels = PLONG(ParameterBlock);

				PSHORT Levels_ = (PSHORT)ExAllocatePoolWithTag(NonPagedPool, NumProgrammableControls * sizeof(SHORT), 'mdW');

				if (Levels_)
				{
					RtlZeroMemory(Levels_, m_NumInputChannels * m_NumOutputChannels * sizeof(SHORT));

					for (ULONG i=0; i<NumProgrammableControls; i++)
					{
						Levels_[i] = SGN_8X8(Levels[i]);
					}

					if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
					{
						ntStatus = SetRequest(RequestCode, Control, &Levels_, NumProgrammableControls*sizeof(SHORT));
					}
					else					
					{
						ntStatus = STATUS_SUCCESS;
					}

					if (NT_SUCCESS(ntStatus))
					{
						if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
						{
							for (ULONG n=0, p=0, q=0; n<m_NumInputChannels; n++)
							{
								for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
								{
									if (m_ParameterBlock[p].Programmable)
									{
										if (RequestCode == REQUEST_CUR)
										{
											m_ParameterBlock[p].Current = Levels[q]; q++;
										}
										else if (RequestCode == REQUEST_MIN)
										{
											m_ParameterBlock[p].Minimum = Levels[q]; q++;
										}
										else if (RequestCode == REQUEST_MAX)
										{
											m_ParameterBlock[p].Maximum = Levels[q]; q++;
										}
										else //if (RequestCode == REQUEST_RES)
										{
											m_ParameterBlock[p].Resolution =Levels[q]; q++;
										}
									}
								}
							}
						}
					}

					ExFreePool(Levels_);
				}
				else
				{
					ntStatus = STATUS_NO_MEMORY;
				}
			}
		}
	}
	else
	{
		// First form of mixer control parameter block.
		if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
			(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
		{
			if (ParameterBlockSize >= sizeof(LONG))
			{
				ULONG i = (InputChannelNumber-1)*m_NumOutputChannels + (OutputChannelNumber-1);

				if (m_ParameterBlock[i].Programmable)
				{
					LONG Level = *(PLONG(ParameterBlock));

					SHORT Level_ = SGN_8X8(Level);

					if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
					{
						ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(SHORT));
					}
					else
					{
						ntStatus = STATUS_SUCCESS;
					}

					if (NT_SUCCESS(ntStatus))
					{
						if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
						{
							if (RequestCode == REQUEST_CUR)
							{
								m_ParameterBlock[i].Current = Level;
							}
							else if (RequestCode == REQUEST_MIN)
							{
								m_ParameterBlock[i].Minimum = Level;
							}
							else if (RequestCode == REQUEST_MAX)
							{
								m_ParameterBlock[i].Maximum = Level;
							}
							else //if (RequestCode == REQUEST_RES)
							{
								m_ParameterBlock[i].Resolution = Level;
							}
						}
					}
				}
				else
				{
					ntStatus = STATUS_SUCCESS;
				}
			}
		}
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
	IN		UCHAR	InputChannelNumber,
	IN		UCHAR	OutputChannelNumber,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = (USHORT(InputChannelNumber)<<8) | (OutputChannelNumber);

	if (Control == 0x0000)
	{
		// Third form of mixer control parameter block.
		if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
			(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
		{
			if (ParameterBlockSize >= (m_NumInputChannels * m_NumOutputChannels * sizeof(LONG)))
			{
				PLONG Levels = PLONG(ParameterBlock);

				for (ULONG n=0, p=0; n<m_NumInputChannels; n++)
				{
					for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
					{
						if (RequestCode == REQUEST_CUR)
						{
							Levels[p] = m_ParameterBlock[p].Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							Levels[p] = m_ParameterBlock[p].Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							Levels[p] = m_ParameterBlock[p].Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							Levels[p] = m_ParameterBlock[p].Resolution;
						}
					}
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = m_NumInputChannels * m_NumOutputChannels * sizeof(LONG);
				}

				ntStatus = STATUS_SUCCESS;
			}
		}
	}
	else if (Control == 0xFFFF)
	{
		// Second form of mixer control parameter block.
		if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
			(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
		{
			ULONG NumProgrammableControls = 0;

			for (ULONG n=0, p=0; n<m_NumInputChannels; n++)
			{
				for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
				{
					if (m_ParameterBlock[p].Programmable)
					{
						NumProgrammableControls++;
					}
				}
			}

			if (ParameterBlockSize >= (NumProgrammableControls * sizeof(LONG)))
			{
				PLONG Levels = PLONG(ParameterBlock);

				for (ULONG n=0, p=0, q=0; n<m_NumInputChannels; n++)
				{
					for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
					{
						if (m_ParameterBlock[p].Programmable)
						{
							if (RequestCode == REQUEST_CUR)
							{
								Levels[q] = m_ParameterBlock[p].Current; q++;
							}
							else if (RequestCode == REQUEST_MIN)
							{
								Levels[q] = m_ParameterBlock[p].Minimum; q++;
							}
							else if (RequestCode == REQUEST_MAX)
							{
								Levels[q] = m_ParameterBlock[p].Maximum; q++;
							}
							else //if (RequestCode == REQUEST_RES)
							{
								Levels[q] = m_ParameterBlock[p].Resolution; q++;
							}
						}
					}
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = NumProgrammableControls * sizeof(LONG);
				}

				ntStatus = STATUS_SUCCESS;
			}
		}
	}
	else
	{
		// First form of mixer control parameter block.
		if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
			(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
		{
			if (ParameterBlockSize >= sizeof(LONG))
			{
				PLONG Level = PLONG(ParameterBlock);

				ULONG i = (InputChannelNumber-1)*m_NumOutputChannels + (OutputChannelNumber-1);

				if (RequestCode == REQUEST_CUR)
				{
					*Level = m_ParameterBlock[i].Current;
				}
				else if (RequestCode == REQUEST_MIN)
				{
					*Level = m_ParameterBlock[i].Minimum;
				}
				else if (RequestCode == REQUEST_MAX)
				{
					*Level = m_ParameterBlock[i].Maximum;
				}
				else //if (RequestCode == REQUEST_RES)
				{
					*Level = m_ParameterBlock[i].Resolution;
				}

				if (OutParameterBlockSize)
				{
					*OutParameterBlockSize = sizeof(LONG);
				}

				ntStatus = STATUS_SUCCESS;
			}
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CMixerUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMixerUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

	if (ParameterBlock && (ParameterBlockSize == m_ParameterBlockSize))
	{
		for (ULONG n=0, p=0; n<m_NumInputChannels; n++)
		{
			for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
			{
				_RestoreParameterBlock(UCHAR(n+1), UCHAR(m+1), &PMIXER_UNIT_PARAMETER_BLOCK(ParameterBlock)[p], FALSE);
			}
		}

		RtlCopyMemory(m_ParameterBlock, ParameterBlock, m_ParameterBlockSize);
	}
	else
	{
		for (ULONG n=0, p=0; n<m_NumInputChannels; n++)
		{
			for (ULONG m=0; m<m_NumOutputChannels; m++, p++)
			{
				_RestoreParameterBlock(UCHAR(n+1), UCHAR(m+1), &m_ParameterBlock[p], TRUE);
			}
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CMixerUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMixerUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= m_ParameterBlockSize))
	{
		RtlCopyMemory(ParameterBlock, m_ParameterBlock, m_ParameterBlockSize);

		*OutParameterBlockSize = m_ParameterBlockSize;

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CMixerUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CMixerUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = m_ParameterBlockSize;

	return ParameterBlockSize;
}

/*****************************************************************************
 * CMixerUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMixerUnit::
_RestoreParameterBlock
(
	IN		UCHAR						InputChannelNumber,
	IN		UCHAR						OutputChannelNumber,
	IN		PMIXER_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL						Read
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	PUCHAR bmControls = PUCHAR(m_MixerUnitDescriptor) + USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(m_MixerUnitDescriptor->bNrInPins);

	ULONG Bits = ((InputChannelNumber-1) * m_NumOutputChannels) + (OutputChannelNumber-1); 
	
	ULONG ByteOffset = Bits / 8; ULONG BitMask = 0x80 >> (Bits % 8);

	ParameterBlock->Programmable = (bmControls[ByteOffset] & BitMask) ? TRUE : FALSE;

	USHORT Control = (USHORT(InputChannelNumber)<<8) | (OutputChannelNumber);

	if (Read)
	{
		SHORT Minimum = 0;				
		GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(SHORT), NULL);
		ParameterBlock->Minimum = SGN_16X16(Minimum);
		
		SHORT Maximum = 0;				
		GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(SHORT), NULL);
		ParameterBlock->Maximum = SGN_16X16(Maximum);

		SHORT Resolution = 1;				
		GetRequest(REQUEST_RES, Control, &Resolution, sizeof(SHORT), NULL);
		ParameterBlock->Resolution = SGN_16X16(Resolution);

		SHORT Current = 0;				
		GetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT), NULL);
		ParameterBlock->Current = SGN_16X16(Current);
	}
	else
	{
		SHORT Current = SGN_8X8(ParameterBlock->Current);				
		SetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT));
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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CSelectorUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_SelectorUnitDescriptor = PUSB_AUDIO_SELECTOR_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

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
	UCHAR iSelector = *(PUCHAR(m_SelectorUnitDescriptor) + USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_ISELECTOR_OFFSET(m_SelectorUnitDescriptor->bNrInPins));

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
			PUCHAR baSourceID = PUCHAR(m_SelectorUnitDescriptor)+USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

			*OutSourceID = baSourceID[i];

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CSelectorUnit::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
CSelectorUnit::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	UCHAR Selector = 1;	GetRequest(REQUEST_CUR, 0, &Selector, sizeof(UCHAR), NULL);

	ASSERT((Selector >= 1)  && (Selector <= m_SelectorUnitDescriptor->bNrInPins));
	
	PUCHAR baSourceID = PUCHAR(m_SelectorUnitDescriptor)+USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

	PENTITY Entity = NULL;

	if (m_AudioTopology->FindEntity(baSourceID[Selector-1], &Entity))
	{
		switch (Entity->DescriptorSubtype())
		{
			case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
			case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
			{
				CTerminal * Terminal = (CTerminal*)Entity;

				Found = Terminal->FindAudioChannelCluster(OutClusterDescriptor);
			}
			break;

			case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
			{
				CUnit * Unit = (CUnit*)Entity;

				Found = Unit->FindAudioChannelCluster(OutClusterDescriptor);
			}
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CSelectorUnit::NumberOfChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CSelectorUnit::
NumberOfChannels
(
	IN		BOOL	Direction
)
{
	PAGED_CODE();

	USHORT NumChannels = 0;

	USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

	if (FindAudioChannelCluster(&ClusterDescriptor))
	{
		NumChannels = ClusterDescriptor.bNrChannels;
	}

	return NumChannels;
}

/*****************************************************************************
 * CSelectorUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CSelectorUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(SELECTOR_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	if (RequestCode == REQUEST_CUR)
	{
		if (ParameterBlockSize >= sizeof(ULONG))
		{
			ULONG PinId = *(PULONG(ParameterBlock));

			UCHAR PinId_ = UCHAR(PinId); // Pin zero is always the output pin.

			if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
			{
				ntStatus = SetRequest(RequestCode, 0, &PinId_, sizeof(UCHAR));
			}
			else
			{
				ntStatus = STATUS_SUCCESS;
			}

			if (NT_SUCCESS(ntStatus))
			{
				if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
				{
					m_ParameterBlock.PinId = PinId;
				}
			}
		}
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
	IN		UCHAR,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	if (RequestCode == REQUEST_CUR)
	{
		if (ParameterBlockSize >= sizeof(ULONG))
		{
			PULONG PinId = PULONG(ParameterBlock);

			*PinId = m_ParameterBlock.PinId;

			if (OutParameterBlockSize)
			{
				*OutParameterBlockSize = sizeof(ULONG);
			}

			ntStatus = STATUS_SUCCESS;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CSelectorUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CSelectorUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

	if (ParameterBlock && (ParameterBlockSize == sizeof(SELECTOR_UNIT_PARAMETER_BLOCK)))
	{
		_RestoreParameterBlock(PSELECTOR_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(SELECTOR_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		_RestoreParameterBlock(&m_ParameterBlock, TRUE);
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CSelectorUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CSelectorUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(SELECTOR_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(SELECTOR_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(SELECTOR_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CSelectorUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CSelectorUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(SELECTOR_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CSelectorUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CSelectorUnit::
_RestoreParameterBlock
(
	IN		PSELECTOR_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL							Read
)
{
	PAGED_CODE();

	if (Read)
	{
		UCHAR PinId = 1;
		GetRequest(REQUEST_CUR, 0, &PinId, sizeof(UCHAR), NULL);
		ParameterBlock->PinId = PinId;
	}
	else
	{
		UCHAR PinId = UCHAR(ParameterBlock->PinId);
		SetRequest(REQUEST_CUR, 0, &PinId, sizeof(UCHAR));
	}

	return STATUS_SUCCESS;
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

	if (m_ParameterBlock)
	{
		ExFreePool(m_ParameterBlock);
	}

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CFeatureUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_FeatureUnitDescriptor = PUSB_AUDIO_FEATURE_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	// Get these later.
	m_ParameterBlockSize = 0;

	m_ParameterBlock = NULL;

	return ntStatus;
}

/*****************************************************************************
 * CFeatureUnit::Configure()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
Configure
( void
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

/*	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_FeatureUnitDescriptor = PUSB_AUDIO_FEATURE_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;*/

	ULONG NumChannels = NumberOfChannels(0);

	m_ParameterBlockSize = (NumChannels + 1) * sizeof(FEATURE_UNIT_PARAMETER_BLOCK);

	m_ParameterBlock = (PFEATURE_UNIT_PARAMETER_BLOCK)ExAllocatePoolWithTag(NonPagedPool, m_ParameterBlockSize, 'mdW');

	if (m_ParameterBlock)
	{
		RtlZeroMemory(m_ParameterBlock, m_ParameterBlockSize);

		RestoreParameterBlock();
	}
	else
	{
		ntStatus = STATUS_NO_MEMORY;
	}

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
	UCHAR iFeature = *(PUCHAR(m_FeatureUnitDescriptor) + USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_IFEATURE_OFFSET(m_FeatureUnitDescriptor->bControlSize, NumberOfChannels(0)));

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
 * CFeatureUnit::ParseControls()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL
CFeatureUnit::
ParseControls
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutControlSelector
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PUCHAR bmControls = PUCHAR(m_FeatureUnitDescriptor) + USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET;

	UCHAR NumChannels = (m_FeatureUnitDescriptor->bLength - 6 - 1) / m_FeatureUnitDescriptor->bControlSize;

	UCHAR ByteOffset = Index / 8;
	UCHAR BitMask = 0x01 << (Index % 8);

	if (ByteOffset < m_FeatureUnitDescriptor->bControlSize)
	{
		if (OutControlSelector)
		{
			*OutControlSelector = 0;
		}

		for (UCHAR i=0; i<NumChannels; i++)
		{
			if (bmControls[ByteOffset] & BitMask)
			{
				if (OutControlSelector)
				{
					*OutControlSelector = Index+1;
				}
				break;
			}

			bmControls += m_FeatureUnitDescriptor->bControlSize;
		}

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * CFeatureUnit::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
CFeatureUnit::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PENTITY Entity = NULL;

	if (m_AudioTopology->FindEntity(m_FeatureUnitDescriptor->bSourceID, &Entity))
	{
		switch (Entity->DescriptorSubtype())
		{
			case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
			case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
			{
				CTerminal * Terminal = (CTerminal*)Entity;

				Found = Terminal->FindAudioChannelCluster(OutClusterDescriptor);
			}
			break;

			case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
			{
				CUnit * Unit = (CUnit*)Entity;

				Found = Unit->FindAudioChannelCluster(OutClusterDescriptor);
			}
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CFeatureUnit::NumberOfChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CFeatureUnit::
NumberOfChannels
(
	IN		BOOL	Direction
)
{
	PAGED_CODE();

	USHORT NumChannels = 0;

	USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

	if (FindAudioChannelCluster(&ClusterDescriptor))
	{
		NumChannels = ClusterDescriptor.bNrChannels;
	}

	return NumChannels;
}

/*****************************************************************************
 * CFeatureUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CFeatureUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(m_ParameterBlock, m_ParameterBlockSize);
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8 | USHORT(ChannelNumber);

	switch (ControlSelector)
	{
		case USB_AUDIO_FU_CONTROL_MUTE:
		{
			_DbgPrintF(DEBUGLVL_BLAB,("Mute Control - UnitID: 0x%x, ChannelNumber: %d", m_EntityID, ChannelNumber));

			if (m_ParameterBlock[ChannelNumber].Mute.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Mute = *(PBOOL(ParameterBlock));

						UCHAR Current = Mute ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for ChannelNumber = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock[ChannelNumber].Mute.Current = Mute;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_VOLUME:
		{
			if (m_ParameterBlock[ChannelNumber].Volume.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(LONG))
					{
						LONG Level = *(PLONG(ParameterBlock));

						SHORT Level_ = SGN_8X8(Level);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(SHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock[ChannelNumber].Volume.Current = Level;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock[ChannelNumber].Volume.Minimum = Level;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock[ChannelNumber].Volume.Maximum = Level;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock[ChannelNumber].Volume.Resolution = Level;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_BASS:
		{
			if (m_ParameterBlock[ChannelNumber].Bass.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(LONG))
					{
						LONG Level = *(PLONG(ParameterBlock));

						CHAR Level_ = SGN_6X2(Level);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(CHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock[ChannelNumber].Bass.Current = Level;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock[ChannelNumber].Bass.Minimum = Level;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock[ChannelNumber].Bass.Maximum = Level;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock[ChannelNumber].Bass.Resolution = Level;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_MID:
		{
			if (m_ParameterBlock[ChannelNumber].Mid.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(LONG))
					{
						LONG Level = *(PLONG(ParameterBlock));

						CHAR Level_ = SGN_6X2(Level);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(CHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock[ChannelNumber].Mid.Current = Level;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock[ChannelNumber].Mid.Minimum = Level;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock[ChannelNumber].Mid.Maximum = Level;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock[ChannelNumber].Mid.Resolution = Level;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_TREBLE:
		{
			if (m_ParameterBlock[ChannelNumber].Treble.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(LONG))
					{
						LONG Level = *(PLONG(ParameterBlock));

						CHAR Level_ = SGN_6X2(Level);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(CHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock[ChannelNumber].Treble.Current = Level;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock[ChannelNumber].Treble.Minimum = Level;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock[ChannelNumber].Treble.Maximum = Level;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock[ChannelNumber].Treble.Resolution = Level;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_GRAPHIC_EQ:
		{
			if (m_ParameterBlock[ChannelNumber].GraphicEQ.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG BandsPresent = *(PULONG(ParameterBlock));

						if (BandsPresent)
						{
							PLONG Levels = PLONG(PUCHAR(ParameterBlock)+sizeof(ULONG));

							struct
							{
								ULONG	bmBandsPresent;
								CHAR	bBand[32];
							} GraphicEQ;

							ULONG N=0;

							for (ULONG i=0; i<32; i++)
							{
								if (BandsPresent & (1<<i))
								{
									GraphicEQ.bBand[N] = SGN_6X2(Levels[N]); N++;
								}
							}
							GraphicEQ.bmBandsPresent = BandsPresent;

							if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
							{
								ntStatus = SetRequest(RequestCode, Control, &GraphicEQ, sizeof(ULONG)+N*sizeof(CHAR));
							}
							else
							{
								ntStatus = STATUS_SUCCESS;
							}

							if (NT_SUCCESS(ntStatus))
							{
								if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
								{
									for (ULONG i=0, N=0; i<32; i++)
									{
										if (BandsPresent & (1<<i))
										{
											//TODO: Add support for Channel = 0xFF
											if (RequestCode == REQUEST_CUR)
											{
												m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Current = Levels[N]; N++;
											}
											else if (RequestCode == REQUEST_MIN)
											{
												m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Minimum = Levels[N]; N++;
											}
											else if (RequestCode == REQUEST_MAX)
											{
												m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Maximum = Levels[N]; N++;
											}
											else //if (RequestCode == REQUEST_RES)
											{
												m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Resolution = Levels[N]; N++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_AUTOMATIC_GAIN:
		{
			if (m_ParameterBlock[ChannelNumber].AGC.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL AGC = *(PBOOL(ParameterBlock));

						UCHAR Current = AGC ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock[ChannelNumber].AGC.Current = AGC;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_DELAY:
		{					
			if (m_ParameterBlock[ChannelNumber].Delay.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(KSTIME))
					{
						KSTIME Delay = *(PKSTIME(ParameterBlock));

						USHORT Delay_ = USHORT((64 * Delay.Time * Delay.Numerator / Delay.Denominator) / GTI_MILLISECONDS(1));

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(REQUEST_CUR, Control, &Delay_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock[ChannelNumber].Delay.Current = Delay;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock[ChannelNumber].Delay.Minimum = Delay;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock[ChannelNumber].Delay.Maximum = Delay;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock[ChannelNumber].Delay.Resolution = Delay;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_BASS_BOOST:
		{
			if (m_ParameterBlock[ChannelNumber].BassBoost.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL BassBoost = *(PBOOL(ParameterBlock));

						UCHAR Current = BassBoost ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock[ChannelNumber].BassBoost.Current = BassBoost;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_LOUDNESS:
		{
			if (m_ParameterBlock[ChannelNumber].Loudness.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Loudness = *(PBOOL(ParameterBlock));

						UCHAR Current = Loudness ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							//TODO: Add support for Channel = 0xFF
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock[ChannelNumber].Loudness.Current = Loudness;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8 | USHORT(ChannelNumber);

	switch (ControlSelector)
	{
		case USB_AUDIO_FU_CONTROL_MUTE:
		{
			if (m_ParameterBlock[ChannelNumber].Mute.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Mute = PBOOL(ParameterBlock);
						
						*Mute = m_ParameterBlock[ChannelNumber].Mute.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_VOLUME:
		{
			if (m_ParameterBlock[ChannelNumber].Volume.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(LONG))
					{
						PLONG Level = PLONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Level = m_ParameterBlock[ChannelNumber].Volume.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Level = m_ParameterBlock[ChannelNumber].Volume.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Level = m_ParameterBlock[ChannelNumber].Volume.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Level = m_ParameterBlock[ChannelNumber].Volume.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(LONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_BASS:
		{
			if (m_ParameterBlock[ChannelNumber].Bass.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(LONG))
					{
						PLONG Level = PLONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Level = m_ParameterBlock[ChannelNumber].Bass.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Level = m_ParameterBlock[ChannelNumber].Bass.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Level = m_ParameterBlock[ChannelNumber].Bass.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Level = m_ParameterBlock[ChannelNumber].Bass.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(LONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_MID:
		{
			if (m_ParameterBlock[ChannelNumber].Mid.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(LONG))
					{
						PLONG Level = PLONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Level = m_ParameterBlock[ChannelNumber].Mid.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Level = m_ParameterBlock[ChannelNumber].Mid.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Level = m_ParameterBlock[ChannelNumber].Mid.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Level = m_ParameterBlock[ChannelNumber].Mid.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(LONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_TREBLE:
		{
			if (m_ParameterBlock[ChannelNumber].Treble.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(LONG))
					{
						PLONG Level = PLONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Level = m_ParameterBlock[ChannelNumber].Treble.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Level = m_ParameterBlock[ChannelNumber].Treble.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Level = m_ParameterBlock[ChannelNumber].Treble.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Level = m_ParameterBlock[ChannelNumber].Treble.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(LONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_GRAPHIC_EQ:
		{
			if (m_ParameterBlock[ChannelNumber].GraphicEQ.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG NumberOfBands = PULONG(ParameterBlock);

						*NumberOfBands = m_ParameterBlock[ChannelNumber].GraphicEQ.NumberOfBands;

						if (ParameterBlockSize >= 2* sizeof(ULONG))
						{
							PULONG BandsPresent = PULONG(NumberOfBands+1);

							*BandsPresent = m_ParameterBlock[ChannelNumber].GraphicEQ.BandsPresent;

							if (ParameterBlockSize >= (2*sizeof(ULONG) + m_ParameterBlock[ChannelNumber].GraphicEQ.NumberOfBands * sizeof(LONG)))
							{
								PLONG Levels = PLONG(BandsPresent+1);

								for (ULONG i=0, N=0; i<32; i++)
								{
									if (m_ParameterBlock[ChannelNumber].GraphicEQ.BandsPresent & (1<<i))
									{
										if (RequestCode == REQUEST_CUR)
										{
											Levels[N] = m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Current; N++;
										}
										else if (RequestCode == REQUEST_MIN)
										{
											Levels[N] = m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Minimum; N++;
										}
										else if (RequestCode == REQUEST_MAX)
										{
											Levels[N] = m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Maximum; N++;
										}
										else //if (RequestCode == REQUEST_RES)
										{
											Levels[N] = m_ParameterBlock[ChannelNumber].GraphicEQ.Levels[i].Resolution; N++;
										}
									}
								}				

								if (OutParameterBlockSize)
								{
									*OutParameterBlockSize = 2 * sizeof(ULONG) + m_ParameterBlock[ChannelNumber].GraphicEQ.NumberOfBands * sizeof(LONG);
								}
							}
							else
							{
								if (OutParameterBlockSize)
								{
									*OutParameterBlockSize = 2 * sizeof(ULONG);
								}
							}
						}
						else
						{
							if (OutParameterBlockSize)
							{
								*OutParameterBlockSize = sizeof(ULONG);
							}
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_AUTOMATIC_GAIN:
		{
			if (m_ParameterBlock[ChannelNumber].AGC.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					//TODO: Add support for ChannelNumber = 0xFF
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL AGC = PBOOL(ParameterBlock);
						
						*AGC = m_ParameterBlock[ChannelNumber].AGC.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_DELAY:
		{					
			if (m_ParameterBlock[ChannelNumber].Delay.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					//TODO: Add support for ChannelNumber = 0xFF
					if (ParameterBlockSize >= sizeof(KSTIME))
					{
						PKSTIME Delay = PKSTIME(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Delay = m_ParameterBlock[ChannelNumber].Delay.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Delay = m_ParameterBlock[ChannelNumber].Delay.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Delay = m_ParameterBlock[ChannelNumber].Delay.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Delay = m_ParameterBlock[ChannelNumber].Delay.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(KSTIME);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_BASS_BOOST:
		{
			if (m_ParameterBlock[ChannelNumber].BassBoost.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL BassBoost = PBOOL(ParameterBlock);
						
						*BassBoost = m_ParameterBlock[ChannelNumber].BassBoost.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_LOUDNESS:
		{
			if (m_ParameterBlock[ChannelNumber].Loudness.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					//TODO: Add support for Channel = 0xFF
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Loudness = PBOOL(ParameterBlock);
						
						*Loudness = m_ParameterBlock[ChannelNumber].Loudness.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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

	PUCHAR bmControls = PUCHAR(m_FeatureUnitDescriptor) + USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET + (Channel * m_FeatureUnitDescriptor->bControlSize);

	UCHAR ByteOffset = Index / 8;

	UCHAR BitMask = 0x01 << (Index % 8);

	if (bmControls[ByteOffset] & BitMask)
	{
		Found = TRUE;
	}

	*OutControlSelector = Index+1;

	return Found;
}

/*****************************************************************************
 * CFeatureUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CFeatureUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	PAGED_CODE();

	if (m_ParameterBlock)
	{
		ULONG NumChannels = NumberOfChannels(0);

		if (ParameterBlock && (ParameterBlockSize == m_ParameterBlockSize))
		{
			for (UCHAR i=0; i<(NumChannels+1); i++)
			{
				// For each channel, initialize all controls on it.
				for (UCHAR j=0; j<(m_FeatureUnitDescriptor->bControlSize*8); j++)
				{
					UCHAR ControlSelector = USB_AUDIO_FU_CONTROL_UNDEFINED;

					BOOL Support = _FindControl(i, j, &ControlSelector);

					_RestoreParameterBlock(ControlSelector, i, Support, &PFEATURE_UNIT_PARAMETER_BLOCK(ParameterBlock)[i], FALSE);
				}
			}

			RtlCopyMemory(m_ParameterBlock, ParameterBlock, m_ParameterBlockSize);
		}
		else
		{
			for (UCHAR i=0; i<(NumChannels+1); i++)
			{
				// For each channel, initialize all controls on it.
				for (UCHAR j=0; j<(m_FeatureUnitDescriptor->bControlSize*8); j++)
				{
					UCHAR ControlSelector = USB_AUDIO_FU_CONTROL_UNDEFINED;

					BOOL Support = _FindControl(i, j, &ControlSelector);

					_RestoreParameterBlock(ControlSelector, i, Support, &m_ParameterBlock[i], TRUE);
				}
			}
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CFeatureUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CFeatureUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= m_ParameterBlockSize))
	{
		RtlCopyMemory(ParameterBlock, m_ParameterBlock, m_ParameterBlockSize);

		*OutParameterBlockSize = m_ParameterBlockSize;

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CFeatureUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CFeatureUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = m_ParameterBlockSize * (NumberOfChannels(0) + 1);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CFeatureUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */

NTSTATUS 
CFeatureUnit::
_RestoreParameterBlock
(
	IN		UCHAR							ControlSelector,
	IN		UCHAR							ChannelNumber,
	IN		BOOL							Support,
	IN		PFEATURE_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL							Read
)
{

	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8 | ChannelNumber;

	switch (ControlSelector)
	{
		case USB_AUDIO_FU_CONTROL_MUTE:
		{
			ParameterBlock->Mute.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Mute.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Mute.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}

				_DbgPrintF(DEBUGLVL_BLAB,("[CFeatureUnit::_RestoreParameterBlock] - ChannelNumber: %d, Mute: 0x%x", ChannelNumber, ParameterBlock->Mute));
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_VOLUME:
		{
			ParameterBlock->Volume.Support = Support;

			if (Support)
			{
				if (Read)
				{
					SHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(SHORT), NULL);
					ParameterBlock->Volume.Minimum = SGN_16X16(Minimum);
					
					SHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(SHORT), NULL);
					ParameterBlock->Volume.Maximum = SGN_16X16(Maximum);

					SHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(SHORT), NULL);
					ParameterBlock->Volume.Resolution = SGN_16X16(Resolution);

					SHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT), NULL);
					ParameterBlock->Volume.Current = SGN_16X16(Current);
				}
				else
				{
					SHORT Current = SGN_8X8(ParameterBlock->Volume.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_BASS:
		{
			ParameterBlock->Bass.Support = Support;

			if (Support)
			{
				if (Read)
				{
					CHAR Minimum = 0;					
					GetRequest(REQUEST_CUR, Control, &Minimum, sizeof(CHAR), NULL);
					ParameterBlock->Bass.Minimum = SGN_16X16(Minimum);

					CHAR Maximum = 0;					
					GetRequest(REQUEST_CUR, Control, &Maximum, sizeof(CHAR), NULL);
					ParameterBlock->Bass.Maximum = SGN_16X16(Maximum);

					CHAR Resolution = 1;					
					GetRequest(REQUEST_CUR, Control, &Resolution, sizeof(CHAR), NULL);
					ParameterBlock->Bass.Resolution = SGN_16X16(Resolution);

					CHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(CHAR), NULL);
					ParameterBlock->Bass.Current = SGN_16X16(Current);
				}
				else
				{
					CHAR Current = SGN_6X2(ParameterBlock->Bass.Current);					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(CHAR));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_MID:
		{
			ParameterBlock->Mid.Support = Support;

			if (Support)
			{
				if (Read)
				{
					CHAR Minimum = 0;						
					GetRequest(REQUEST_CUR, Control, &Minimum, sizeof(CHAR), NULL);
					ParameterBlock->Mid.Minimum = SGN_16X16(Minimum);

					CHAR Maximum = 0;						
					GetRequest(REQUEST_CUR, Control, &Maximum, sizeof(CHAR), NULL);
					ParameterBlock->Mid.Maximum = SGN_16X16(Maximum);

					CHAR Resolution = 1;						
					GetRequest(REQUEST_CUR, Control, &Resolution, sizeof(CHAR), NULL);
					ParameterBlock->Mid.Resolution = SGN_16X16(Resolution);

					CHAR Current = 0;						
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(CHAR), NULL);
					ParameterBlock->Mid.Current = SGN_16X16(Current);
				}
				else
				{
					CHAR Current = SGN_6X2(ParameterBlock->Mid.Current);						
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(CHAR));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_TREBLE:
		{
			ParameterBlock->Treble.Support = Support;

			if (Support)
			{
				if (Read)
				{
					CHAR Minimum = 0;						
					GetRequest(REQUEST_CUR, Control, &Minimum, sizeof(CHAR), NULL);
					ParameterBlock->Treble.Minimum = SGN_16X16(Minimum);

					CHAR Maximum = 0;						
					GetRequest(REQUEST_CUR, Control, &Maximum, sizeof(CHAR), NULL);
					ParameterBlock->Treble.Maximum = SGN_16X16(Maximum);

					CHAR Resolution = 1;						
					GetRequest(REQUEST_CUR, Control, &Resolution, sizeof(CHAR), NULL);
					ParameterBlock->Treble.Resolution = SGN_16X16(Resolution);

					CHAR Current = 0;						
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(CHAR), NULL);
					ParameterBlock->Treble.Current = SGN_16X16(Current);
				}
				else
				{
					CHAR Current = SGN_6X2(ParameterBlock->Treble.Current);						
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(CHAR));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_GRAPHIC_EQ:
		{
			ParameterBlock->GraphicEQ.Support = Support;

			if (Support)
			{
				struct
				{
					ULONG	bmBandsPresent;
					CHAR	bBand[32];
				} GraphicEQ;
				
				if (Read)
				{
					ULONG i=0, N = 0;

					GetRequest(REQUEST_MIN, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);
					for (i=0, N=0; i<32; i++)
					{
						if (GraphicEQ.bmBandsPresent & (1<<i))
						{
							ParameterBlock->GraphicEQ.Levels[i].Minimum = SGN_16X16(GraphicEQ.bBand[N]); N++;

							ParameterBlock->GraphicEQ.Levels[i].Support = TRUE;
						}
						else
						{
							ParameterBlock->GraphicEQ.Levels[i].Support = FALSE;
						}
					}

					GetRequest(REQUEST_MAX, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);
					for (i=0, N=0; i<32; i++)
					{
						if (GraphicEQ.bmBandsPresent & (1<<i))
						{
							ParameterBlock->GraphicEQ.Levels[i].Maximum = SGN_16X16(GraphicEQ.bBand[N]); N++;
						}
					}

					GetRequest(REQUEST_RES, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);
					for (i=0, N=0; i<32; i++)
					{
						if (GraphicEQ.bmBandsPresent & (1<<i))
						{
							ParameterBlock->GraphicEQ.Levels[i].Resolution = SGN_16X16(GraphicEQ.bBand[N]); N++;
						}
					}

					GetRequest(REQUEST_CUR, Control, &GraphicEQ, sizeof(GraphicEQ), NULL);
					
					for (i=0, N=0; i<32; i++)
					{
						if (GraphicEQ.bmBandsPresent & (1<<i))
						{
							ParameterBlock->GraphicEQ.Levels[i].Current = SGN_16X16(GraphicEQ.bBand[N]); N++;
						}
					}
					ParameterBlock->GraphicEQ.BandsPresent = GraphicEQ.bmBandsPresent;
					ParameterBlock->GraphicEQ.NumberOfBands = N;
				}
				else
				{
					ULONG N = 0;

					for (ULONG i=0; i<32; i++)
					{
						if (ParameterBlock->GraphicEQ.BandsPresent & (1<<i))
						{
							GraphicEQ.bBand[N] = SGN_6X2(ParameterBlock->GraphicEQ.Levels[i].Current); N++;
						}
					}
					SetRequest(REQUEST_CUR, Control, &GraphicEQ, sizeof(ULONG)+N*sizeof(CHAR));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_AUTOMATIC_GAIN:
		{
			ParameterBlock->AGC.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->AGC.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->AGC.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_DELAY:
		{					
			ParameterBlock->Delay.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;					
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->Delay.Minimum.Time = GTI_MILLISECONDS(Minimum) / 64;
					ParameterBlock->Delay.Minimum.Numerator = 1;
					ParameterBlock->Delay.Minimum.Denominator = 1;

					USHORT Maximum = 0;					
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->Delay.Maximum.Time = GTI_MILLISECONDS(Maximum) / 64;
					ParameterBlock->Delay.Maximum.Numerator = 1;
					ParameterBlock->Delay.Maximum.Denominator = 1;

					USHORT Resolution = 0;					
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->Delay.Resolution.Time = GTI_MILLISECONDS(Resolution) / 64;
					ParameterBlock->Delay.Resolution.Numerator = 1;
					ParameterBlock->Delay.Resolution.Denominator = 1;

					USHORT Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->Delay.Current.Time = GTI_MILLISECONDS(Current) / 64;
					ParameterBlock->Delay.Current.Numerator = 1;
					ParameterBlock->Delay.Current.Denominator = 1;
				}
				else
				{
					USHORT Current = USHORT((64 * ParameterBlock->Delay.Current.Time * ParameterBlock->Delay.Current.Numerator / ParameterBlock->Delay.Current.Denominator) / GTI_MILLISECONDS(1));
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_BASS_BOOST:
		{
			ParameterBlock->BassBoost.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;						
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->BassBoost.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->BassBoost.Current ? 1 : 0;						
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_FU_CONTROL_LOUDNESS:
		{
			ParameterBlock->Loudness.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;						
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Loudness.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Loudness.Current ? 1 : 0;						
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
			break;
	}

	return ntStatus;
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
	PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR ProcessDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(m_UnitDescriptor);

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
	UCHAR iProcessor = *(PUCHAR(m_ProcessorUnitDescriptor) + USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(m_ProcessorUnitDescriptor->bControlSize));

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
 * CProcessorUnit::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
CProcessorUnit::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	OutClusterDescriptor->bNrChannels = m_ProcessorUnitDescriptor->bNrChannels;
	OutClusterDescriptor->wChannelConfig = m_ProcessorUnitDescriptor->wChannelConfig;
	OutClusterDescriptor->iChannelNames = m_ProcessorUnitDescriptor->iChannelNames;
	
	return TRUE;
}

/*****************************************************************************
 * CProcessorUnit::NumberOfChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CProcessorUnit::
NumberOfChannels
(
	IN		BOOL	Direction
)
{
	PAGED_CODE();

	USHORT NumChannels = 0;

	if (Direction == 0) // Output
	{
		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

		if (FindAudioChannelCluster(&ClusterDescriptor))
		{
			NumChannels = ClusterDescriptor.bNrChannels;
		}
	}
	else // Input
	{
		PENTITY Entity = NULL;

		if (m_AudioTopology->FindEntity(m_ProcessorUnitDescriptor->bSourceID, &Entity))
		{
			switch (Entity->DescriptorSubtype())
			{
				case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
				case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
				{
					CTerminal * Terminal = (CTerminal*)Entity;

					USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
	
					if (Terminal->FindAudioChannelCluster(&ClusterDescriptor))
					{
						NumChannels += ClusterDescriptor.bNrChannels;
					}				
				}
				break;

				case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
				{
					CUnit * Unit = (CUnit*)Entity;

					USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
	
					if (Unit->FindAudioChannelCluster(&ClusterDescriptor))
					{
						NumChannels += ClusterDescriptor.bNrChannels;
					}				
				}
				break;
			}
		}
	}

	return NumChannels;
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

	PUCHAR bmControls = PUCHAR(m_ProcessorUnitDescriptor) + USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET;

	UCHAR ByteOffset = Index / 8;

	UCHAR BitMask = 0x01 << (Index % 8);

	if (bmControls[ByteOffset] & BitMask)
	{
		Found = TRUE;
	}

	*OutControlSelector = Index+1;

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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CUpDownMixUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_UpDownMixUnitDescriptor = PUSB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUpDownMixUnit::NumberOfModes()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CUpDownMixUnit::
NumberOfModes
(
	IN OUT	PLONG	ChannelConfigs
)
{
	PAGED_CODE();

	ULONG NumberOfModes = *((PUCHAR(m_UpDownMixUnitDescriptor) + USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_BNRMODES_OFFSET(m_UpDownMixUnitDescriptor->bControlSize)));

	if (ChannelConfigs)
	{
		PUSHORT Modes = PUSHORT(PUCHAR(m_UpDownMixUnitDescriptor) + USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_WAMODES_OFFSET(m_UpDownMixUnitDescriptor->bControlSize));

		for (ULONG i=0; i<NumberOfModes; i++)
		{
			ChannelConfigs[i] = LONG(Modes[i]);
		}
	}

	return NumberOfModes;
}

/*****************************************************************************
 * CUpDownMixUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CUpDownMixUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_UD_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_UD_CONTROL_MODE_SELECT:
		{
			if (m_ParameterBlock.Mode.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Mode = *(PULONG(ParameterBlock));

						USHORT Mode_ = USHORT(Mode);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Mode_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Mode.Current = Mode;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Mode.Minimum = Mode;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Mode.Maximum = Mode;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Mode.Resolution = Mode;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_UD_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock.Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_UD_CONTROL_MODE_SELECT:
		{
			if (m_ParameterBlock.Mode.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Mode = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Mode = m_ParameterBlock.Mode.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Mode = m_ParameterBlock.Mode.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Mode = m_ParameterBlock.Mode.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Mode = m_ParameterBlock.Mode.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
 * CUpDownMixUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUpDownMixUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	// Initialize all controls on it.
	if (ParameterBlock && (ParameterBlockSize == sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_UD_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PUP_DOWNMIX_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_UD_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUpDownMixUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUpDownMixUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUpDownMixUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CUpDownMixUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(UP_DOWNMIX_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CUpDownMixUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CUpDownMixUnit::
_RestoreParameterBlock
(
	IN		UCHAR								ControlSelector,
	IN		BOOL								Support,
	IN		PUP_DOWNMIX_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL								Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_UD_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_UD_CONTROL_MODE_SELECT:
		{
			ParameterBlock->Mode.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 1;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Minimum = ULONG(Minimum);
					
					UCHAR Maximum = 1;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Maximum = ULONG(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Resolution = ULONG(Resolution);

					UCHAR Current = 1;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Current = ULONG(Current);
				}
				else
				{
					UCHAR Current = UCHAR(ParameterBlock->Mode.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CDolbyPrologicUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_PrologicUnitDescriptor = PUSB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CDolbyPrologicUnit::NumberOfModes()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CDolbyPrologicUnit::
NumberOfModes
(
	IN OUT	PLONG	ChannelConfigs
)
{
	PAGED_CODE();

	ULONG NumberOfModes = *((PUCHAR(m_PrologicUnitDescriptor) + USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_BNRMODES_OFFSET(m_PrologicUnitDescriptor->bControlSize)));

	if (ChannelConfigs)
	{
		PUSHORT Modes = PUSHORT(PUCHAR(m_PrologicUnitDescriptor) + USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_WAMODES_OFFSET(m_PrologicUnitDescriptor->bControlSize));

		for (ULONG i=0; i<NumberOfModes; i++)
		{
			ChannelConfigs[i] = LONG(Modes[i]);
		}
	}

	return NumberOfModes;
}

/*****************************************************************************
 * CDolbyPrologicUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CDolbyPrologicUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_DP_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DP_CONTROL_MODE_SELECT:
		{
			if (m_ParameterBlock.Mode.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Mode = *(PULONG(ParameterBlock));

						UCHAR Mode_ = UCHAR(Mode);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Mode_, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Mode.Current = Mode;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Mode.Minimum = Mode;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Mode.Maximum = Mode;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Mode.Resolution = Mode;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_DP_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock.Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DP_CONTROL_MODE_SELECT:
		{
			if (m_ParameterBlock.Mode.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Mode = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Mode = m_ParameterBlock.Mode.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Mode = m_ParameterBlock.Mode.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Mode = m_ParameterBlock.Mode.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Mode = m_ParameterBlock.Mode.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
 * CDolbyPrologicUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDolbyPrologicUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	// Initialize all controls on it.
	if (ParameterBlock && (ParameterBlockSize == sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_DP_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PDOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_DP_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CDolbyPrologicUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDolbyPrologicUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CDolbyPrologicUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CDolbyPrologicUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CDolbyPrologicUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDolbyPrologicUnit::
_RestoreParameterBlock
(
	IN		UCHAR									ControlSelector,
	IN		BOOL									Support,
	IN		PDOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL									Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_DP_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_DP_CONTROL_MODE_SELECT:
		{
			ParameterBlock->Mode.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 1;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Minimum = ULONG(Minimum);
					
					UCHAR Maximum = 1;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Maximum = ULONG(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Resolution = ULONG(Resolution);

					UCHAR Current = 1;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Mode.Current = ULONG(Current);
				}
				else
				{
					UCHAR Current = UCHAR(ParameterBlock->Mode.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * C3dStereoExtenderUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_3dExtenderUnitDescriptor = PUSB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
C3dStereoExtenderUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_3D_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_3D_CONTROL_SPACIOUSNESS:
		{
			if (m_ParameterBlock.Spaciousness.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Spaciousness = *(PULONG(ParameterBlock));

						UCHAR Spaciousness_ = PCT_8X0(Spaciousness);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Spaciousness_, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Spaciousness.Current = Spaciousness;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Spaciousness.Minimum = Spaciousness;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Spaciousness.Maximum = Spaciousness;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Spaciousness.Resolution = Spaciousness;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_3D_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock.Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_3D_CONTROL_SPACIOUSNESS:
		{
			if (m_ParameterBlock.Spaciousness.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Spaciousness = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Spaciousness = m_ParameterBlock.Spaciousness.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Spaciousness = m_ParameterBlock.Spaciousness.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Spaciousness = m_ParameterBlock.Spaciousness.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Spaciousness = m_ParameterBlock.Spaciousness.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
 * C3dStereoExtenderUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
C3dStereoExtenderUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	// Initialize all controls on it.
	if (ParameterBlock && (ParameterBlockSize == sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_3D_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PSTEREO_EXTENDER_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_3D_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
C3dStereoExtenderUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
C3dStereoExtenderUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(STEREO_EXTENDER_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * C3dStereoExtenderUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
C3dStereoExtenderUnit::
_RestoreParameterBlock
(
	IN		UCHAR									ControlSelector,
	IN		BOOL									Support,
	IN		PSTEREO_EXTENDER_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL									Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_3D_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_3D_CONTROL_SPACIOUSNESS:
		{
			ParameterBlock->Spaciousness.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Spaciousness.Minimum = PCT_16X16(Minimum);
					
					UCHAR Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Spaciousness.Maximum = PCT_16X16(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Spaciousness.Resolution = PCT_16X16(Resolution);

					UCHAR Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Spaciousness.Current = PCT_16X16(Current);
				}
				else
				{
					UCHAR Current = PCT_8X0(ParameterBlock->Spaciousness.Current);
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CReverberationUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_ReveberationUnitDescriptor = PUSB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CReverberationUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CReverberationUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_RV_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_TYPE:
		{
			if (m_ParameterBlock.Type.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Type = *(PULONG(ParameterBlock));

						UCHAR Type_ = UCHAR(Type);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Type_, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Type.Current = Type;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Type.Minimum = Type;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Type.Maximum = Type;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Type.Resolution = Type;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_LEVEL:
		{
			if (m_ParameterBlock.Level.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Level = *(PULONG(ParameterBlock));

						UCHAR Level_ = PCT_8X0(Level);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Level.Current = Level;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Level.Minimum = Level;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Level.Maximum = Level;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Level.Resolution = Level;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_TIME:
		{
			if (m_ParameterBlock.Time.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Time = *(PULONG(ParameterBlock));

						USHORT Time_ = SEC_8X8(Time);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Time_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Time.Current = Time;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Time.Minimum = Time;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Time.Maximum = Time;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Time.Resolution = Time;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_FEEDBACK:
		{
			if (m_ParameterBlock.Feedback.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Feedback = *(PULONG(ParameterBlock));

						UCHAR Feedback_ = PCT_8X0(Feedback);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Feedback_, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Feedback.Current = Feedback;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Feedback.Minimum = Feedback;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Feedback.Maximum = Feedback;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Feedback.Resolution = Feedback;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_RV_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock.Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_TYPE:
		{
			if (m_ParameterBlock.Type.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Type = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Type = m_ParameterBlock.Type.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Type = m_ParameterBlock.Type.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Type = m_ParameterBlock.Type.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Type = m_ParameterBlock.Type.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_LEVEL:
		{
			if (m_ParameterBlock.Level.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Level = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Level = m_ParameterBlock.Level.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Level = m_ParameterBlock.Level.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Level = m_ParameterBlock.Level.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Level = m_ParameterBlock.Level.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_TIME:
		{
			if (m_ParameterBlock.Time.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Time = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Time = m_ParameterBlock.Time.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Time = m_ParameterBlock.Time.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Time = m_ParameterBlock.Time.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Time = m_ParameterBlock.Time.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_FEEDBACK:
		{
			if (m_ParameterBlock.Feedback.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Feedback = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Feedback = m_ParameterBlock.Feedback.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Feedback = m_ParameterBlock.Feedback.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Feedback = m_ParameterBlock.Feedback.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Feedback = m_ParameterBlock.Feedback.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
 * CReverberationUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CReverberationUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	// Initialize all controls on it.
	if (ParameterBlock && (ParameterBlockSize == sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_RV_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PREVERBERATION_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_RV_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CReverberationUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CReverberationUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CReverberationUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CReverberationUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(REVERBERATION_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CReverberationUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CReverberationUnit::
_RestoreParameterBlock
(
	IN		UCHAR								ControlSelector,
	IN		BOOL								Support,
	IN		PREVERBERATION_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL								Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_RV_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_TYPE:
		{
			ParameterBlock->Type.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Type.Minimum = ULONG(Minimum);
					
					UCHAR Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Type.Maximum = ULONG(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Type.Resolution = ULONG(Resolution);

					UCHAR Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Type.Current = ULONG(Current);
				}
				else
				{
					UCHAR Current = UCHAR(ParameterBlock->Type.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_LEVEL:
		{
			ParameterBlock->Level.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Minimum = PCT_16X16(Minimum);
					
					UCHAR Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Maximum = PCT_16X16(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Resolution = PCT_16X16(Resolution);

					UCHAR Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Current = PCT_16X16(Current);
				}
				else
				{
					UCHAR Current = PCT_8X0(ParameterBlock->Level.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_TIME:
		{
			ParameterBlock->Time.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->Time.Minimum = SEC_16X16(Minimum);
					
					USHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->Time.Maximum = SEC_16X16(Maximum);

					USHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->Time.Resolution = SEC_16X16(Resolution);

					USHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->Time.Current = SEC_16X16(Current);
				}
				else
				{
					USHORT Current = SEC_8X8(ParameterBlock->Time.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		case USB_AUDIO_RV_CONTROL_FEEDBACK:
		{
			ParameterBlock->Feedback.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Feedback.Minimum = PCT_16X16(Minimum);
					
					UCHAR Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Feedback.Maximum = PCT_16X16(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Feedback.Resolution = PCT_16X16(Resolution);

					UCHAR Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Feedback.Current = PCT_16X16(Current);
				}
				else
				{
					UCHAR Current = PCT_8X0(ParameterBlock->Feedback.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CChorusUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_ChorusUnitDescriptor = PUSB_AUDIO_CHORUS_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CChorusUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CChorusUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(CHORUS_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_CH_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_LEVEL:
		{
			if (m_ParameterBlock.Level.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Level = *(PULONG(ParameterBlock));

						UCHAR Level_ = PCT_8X0(Level);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Level_, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Level.Current = Level;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Level.Minimum = Level;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Level.Maximum = Level;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Level.Resolution = Level;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_RATE:
		{
			if (m_ParameterBlock.Rate.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Rate = *(PULONG(ParameterBlock));

						USHORT Rate_ = HZ_8X8(Rate);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Rate_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Rate.Current = Rate;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Rate.Minimum = Rate;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Rate.Maximum = Rate;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Rate.Resolution = Rate;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_DEPTH:
		{
			if (m_ParameterBlock.Depth.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG Depth = *(PULONG(ParameterBlock));

						USHORT Depth_ = MS_8X8(Depth);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Depth_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Depth.Current = Depth;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Depth.Minimum = Depth;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Depth.Maximum = Depth;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Depth.Resolution = Depth;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_CH_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock.Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_LEVEL:
		{
			if (m_ParameterBlock.Level.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Level = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Level = m_ParameterBlock.Level.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Level = m_ParameterBlock.Level.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Level = m_ParameterBlock.Level.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Level = m_ParameterBlock.Level.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_RATE:
		{
			if (m_ParameterBlock.Rate.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Rate = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Rate = m_ParameterBlock.Rate.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Rate = m_ParameterBlock.Rate.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Rate = m_ParameterBlock.Rate.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Rate = m_ParameterBlock.Rate.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_DEPTH:
		{
			if (m_ParameterBlock.Depth.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG Depth = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Depth = m_ParameterBlock.Depth.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Depth = m_ParameterBlock.Depth.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Depth = m_ParameterBlock.Depth.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Depth = m_ParameterBlock.Depth.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
 * CChorusUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CChorusUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	// Initialize all controls on it.
	if (ParameterBlock && (ParameterBlockSize == sizeof(CHORUS_UNIT_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_CH_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PCHORUS_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(CHORUS_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_CH_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CChorusUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CChorusUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(CHORUS_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(CHORUS_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(CHORUS_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CChorusUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CChorusUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(CHORUS_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CChorusUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CChorusUnit::
_RestoreParameterBlock
(
	IN		UCHAR							ControlSelector,
	IN		BOOL							Support,
	IN		PCHORUS_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL							Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_CH_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_LEVEL:
		{
			ParameterBlock->Level.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Minimum = PCT_16X16(Minimum);
					
					UCHAR Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Maximum = PCT_16X16(Maximum);

					UCHAR Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Resolution = PCT_16X16(Resolution);

					UCHAR Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Level.Current = PCT_16X16(Current);
				}
				else
				{
					UCHAR Current = PCT_8X0(ParameterBlock->Level.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_RATE:
		{
			ParameterBlock->Rate.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->Rate.Minimum = HZ_16X16(Minimum);
					
					USHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->Rate.Maximum = HZ_16X16(Maximum);

					USHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->Rate.Resolution = HZ_16X16(Resolution);

					USHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->Rate.Current = HZ_16X16(Current);
				}
				else
				{
					USHORT Current = HZ_8X8(ParameterBlock->Rate.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		case USB_AUDIO_CH_CONTROL_DEPTH:
		{
			ParameterBlock->Depth.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->Depth.Minimum = MS_16X16(Minimum);
					
					USHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->Depth.Maximum = MS_16X16(Maximum);

					USHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->Depth.Resolution = MS_16X16(Resolution);

					USHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->Depth.Current = MS_16X16(Current);
				}
				else
				{
					USHORT Current = MS_8X8(ParameterBlock->Depth.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
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

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_DrcUnitDescriptor = PUSB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR(UnitDescriptor);

	m_ProcessorUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CDynamicRangeCompressionUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(&m_ParameterBlock, sizeof(DRC_UNIT_PARAMETER_BLOCK));
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_DR_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_COMPRESSION_RATIO:
		{
			if (m_ParameterBlock.CompressionRatio.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG CompressionRatio = *(PULONG(ParameterBlock));

						USHORT CompressionRatio_ = HZ_8X8(CompressionRatio);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &CompressionRatio_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.CompressionRatio.Current = CompressionRatio;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.CompressionRatio.Minimum = CompressionRatio;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.CompressionRatio.Maximum = CompressionRatio;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.CompressionRatio.Resolution = CompressionRatio;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_MAX_AMPLITUDE:
		{
			if (m_ParameterBlock.MaxAmplitude.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						LONG MaxAmplitude = *(PLONG(ParameterBlock));

						SHORT MaxAmplitude_ = SGN_8X8(MaxAmplitude);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &MaxAmplitude_, sizeof(SHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.MaxAmplitude.Current = MaxAmplitude;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.MaxAmplitude.Minimum = MaxAmplitude;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.MaxAmplitude.Maximum = MaxAmplitude;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.MaxAmplitude.Resolution = MaxAmplitude;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_THRESHOLD:
		{
			if (m_ParameterBlock.Threshold.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						LONG Threshold = *(PLONG(ParameterBlock));

						SHORT Threshold_ = SGN_8X8(Threshold);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Threshold_, sizeof(SHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.Threshold.Current = Threshold;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.Threshold.Minimum = Threshold;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.Threshold.Maximum = Threshold;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.Threshold.Resolution = Threshold;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_ATTACK_TIME:
		{
			if (m_ParameterBlock.AttackTime.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG AttackTime = *(PULONG(ParameterBlock));

						USHORT AttackTime_ = MS_8X8(AttackTime);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &AttackTime_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.AttackTime.Current = AttackTime;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.AttackTime.Minimum = AttackTime;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.AttackTime.Maximum = AttackTime;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.AttackTime.Resolution = AttackTime;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_RELEASE_TIME:
		{
			if (m_ParameterBlock.ReleaseTime.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG ReleaseTime = *(PULONG(ParameterBlock));

						USHORT ReleaseTime_ = MS_8X8(ReleaseTime);

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &ReleaseTime_, sizeof(USHORT));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								if (RequestCode == REQUEST_CUR)
								{
									m_ParameterBlock.ReleaseTime.Current = ReleaseTime;
								}
								else if (RequestCode == REQUEST_MIN)
								{
									m_ParameterBlock.ReleaseTime.Minimum = ReleaseTime;
								}
								else if (RequestCode == REQUEST_MAX)
								{
									m_ParameterBlock.ReleaseTime.Maximum = ReleaseTime;
								}
								else //if (RequestCode == REQUEST_RES)
								{
									m_ParameterBlock.ReleaseTime.Resolution = ReleaseTime;
								}
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_DR_CONTROL_ENABLE:
		{
			if (m_ParameterBlock.Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock.Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_COMPRESSION_RATIO:
		{
			if (m_ParameterBlock.CompressionRatio.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG CompressionRatio = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*CompressionRatio = m_ParameterBlock.CompressionRatio.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*CompressionRatio = m_ParameterBlock.CompressionRatio.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*CompressionRatio = m_ParameterBlock.CompressionRatio.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*CompressionRatio = m_ParameterBlock.CompressionRatio.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_MAX_AMPLITUDE:
		{
			if (m_ParameterBlock.MaxAmplitude.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PLONG MaxAmplitude = PLONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*MaxAmplitude = m_ParameterBlock.MaxAmplitude.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*MaxAmplitude = m_ParameterBlock.MaxAmplitude.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*MaxAmplitude = m_ParameterBlock.MaxAmplitude.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*MaxAmplitude = m_ParameterBlock.MaxAmplitude.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(LONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_THRESHOLD:
		{
			if (m_ParameterBlock.Threshold.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PLONG Threshold = PLONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*Threshold = m_ParameterBlock.Threshold.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*Threshold = m_ParameterBlock.Threshold.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*Threshold = m_ParameterBlock.Threshold.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*Threshold = m_ParameterBlock.Threshold.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(LONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_ATTACK_TIME:
		{
			if (m_ParameterBlock.AttackTime.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG AttackTime = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*AttackTime = m_ParameterBlock.AttackTime.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*AttackTime = m_ParameterBlock.AttackTime.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*AttackTime = m_ParameterBlock.AttackTime.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*AttackTime = m_ParameterBlock.AttackTime.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_RELEASE_TIME:
		{
			if (m_ParameterBlock.ReleaseTime.Support)
			{
				if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
					(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG ReleaseTime = PULONG(ParameterBlock);

						if (RequestCode == REQUEST_CUR)
						{
							*ReleaseTime = m_ParameterBlock.ReleaseTime.Current;
						}
						else if (RequestCode == REQUEST_MIN)
						{
							*ReleaseTime = m_ParameterBlock.ReleaseTime.Minimum;
						}
						else if (RequestCode == REQUEST_MAX)
						{
							*ReleaseTime = m_ParameterBlock.ReleaseTime.Maximum;
						}
						else //if (RequestCode == REQUEST_RES)
						{
							*ReleaseTime = m_ParameterBlock.ReleaseTime.Resolution;
						}

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
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
 * CDynamicRangeCompressionUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDynamicRangeCompressionUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	// Initialize all controls on it.
	if (ParameterBlock && (ParameterBlockSize == sizeof(DRC_UNIT_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_DR_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PDRC_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(DRC_UNIT_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<(m_ProcessorUnitDescriptor->bControlSize*8); i++)
		{
			UCHAR ControlSelector = USB_AUDIO_DR_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDynamicRangeCompressionUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(DRC_UNIT_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(DRC_UNIT_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(DRC_UNIT_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CDynamicRangeCompressionUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(DRC_UNIT_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CDynamicRangeCompressionUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CDynamicRangeCompressionUnit::
_RestoreParameterBlock
(
	IN		UCHAR						ControlSelector,
	IN		BOOL						Support,
	IN		PDRC_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL						Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_DR_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_COMPRESSION_RATIO:
		{
			ParameterBlock->CompressionRatio.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->CompressionRatio.Minimum = HZ_16X16(Minimum);
					
					USHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->CompressionRatio.Maximum = HZ_16X16(Maximum);

					USHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->CompressionRatio.Resolution = HZ_16X16(Resolution);

					USHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->CompressionRatio.Current = HZ_16X16(Current);
				}
				else
				{
					USHORT Current = HZ_8X8(ParameterBlock->CompressionRatio.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_MAX_AMPLITUDE:
		{
			ParameterBlock->MaxAmplitude.Support = Support;

			if (Support)
			{
				if (Read)
				{
					SHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(SHORT), NULL);
					ParameterBlock->MaxAmplitude.Minimum = SGN_16X16(Minimum);
					
					SHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(SHORT), NULL);
					ParameterBlock->MaxAmplitude.Maximum = SGN_16X16(Maximum);

					SHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(SHORT), NULL);
					ParameterBlock->MaxAmplitude.Resolution = SGN_16X16(Resolution);

					SHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT), NULL);
					ParameterBlock->MaxAmplitude.Current = SGN_16X16(Current);
				}
				else
				{
					SHORT Current = SGN_8X8(ParameterBlock->MaxAmplitude.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT));
				}
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_THRESHOLD:
		{
			ParameterBlock->Threshold.Support = Support;

			if (Support)
			{
				if (Read)
				{
					SHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(SHORT), NULL);
					ParameterBlock->Threshold.Minimum = SGN_16X16(Minimum);
					
					SHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(SHORT), NULL);
					ParameterBlock->Threshold.Maximum = SGN_16X16(Maximum);

					SHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(SHORT), NULL);
					ParameterBlock->Threshold.Resolution = SGN_16X16(Resolution);

					SHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT), NULL);
					ParameterBlock->Threshold.Current = SGN_16X16(Current);
				}
				else
				{
					SHORT Current = SGN_8X8(ParameterBlock->Threshold.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(SHORT));
				}
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_ATTACK_TIME:
		{
			ParameterBlock->AttackTime.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->AttackTime.Minimum = MS_16X16(Minimum);
					
					USHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->AttackTime.Maximum = MS_16X16(Maximum);

					USHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->AttackTime.Resolution = MS_16X16(Resolution);

					USHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->AttackTime.Current = MS_16X16(Current);
				}
				else
				{
					USHORT Current = MS_8X8(ParameterBlock->AttackTime.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		case USB_AUDIO_DR_CONTROL_RELEASE_TIME:
		{
			ParameterBlock->ReleaseTime.Support = Support;

			if (Support)
			{
				if (Read)
				{
					USHORT Minimum = 0;				
					GetRequest(REQUEST_MIN, Control, &Minimum, sizeof(USHORT), NULL);
					ParameterBlock->ReleaseTime.Minimum = MS_16X16(Minimum);
					
					USHORT Maximum = 0;				
					GetRequest(REQUEST_MAX, Control, &Maximum, sizeof(USHORT), NULL);
					ParameterBlock->ReleaseTime.Maximum = MS_16X16(Maximum);

					USHORT Resolution = 1;				
					GetRequest(REQUEST_RES, Control, &Resolution, sizeof(USHORT), NULL);
					ParameterBlock->ReleaseTime.Resolution = MS_16X16(Resolution);

					USHORT Current = 0;				
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT), NULL);
					ParameterBlock->ReleaseTime.Current = MS_16X16(Current);
				}
				else
				{
					USHORT Current = MS_8X8(ParameterBlock->ReleaseTime.Current);				
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(USHORT));
				}
			}
		}
		break;

		default:
			ntStatus = STATUS_INVALID_PARAMETER;
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

	if (m_ParameterBlock)
	{
		ExFreePool(m_ParameterBlock);
	}

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CExtensionUnit::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the unit.
 * @param
 * Device Pointer to the topology device object.
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
	IN		CAudioTopology *					AudioTopology,
	IN		PUSB_DEVICE							UsbDevice,
	IN		UCHAR								InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = UnitDescriptor->bDescriptorSubtype;

	m_EntityID = UnitDescriptor->bUnitID;

	m_ExtensionUnitDescriptor = PUSB_AUDIO_EXTENSION_UNIT_DESCRIPTOR(UnitDescriptor);

	m_UnitDescriptor = UnitDescriptor;

	m_PowerState = PowerDeviceD0;

	m_ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));

	m_ParameterBlockSize = sizeof(EXTENSION_UNIT_PARAMETER_BLOCK) + m_ControlSize * 8 * sizeof(BOOL);

	m_ParameterBlock = PEXTENSION_UNIT_PARAMETER_BLOCK(ExAllocatePoolWithTag(NonPagedPool, m_ParameterBlockSize, 'mdW'));

	if (m_ParameterBlock)
	{
		RtlZeroMemory(m_ParameterBlock, m_ParameterBlockSize);

		RestoreParameterBlock();
	}
	else
	{
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}

	return ntStatus;
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
	UCHAR ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));

	UCHAR iExtension = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_IEXTENSION_OFFSET(m_ExtensionUnitDescriptor->bNrInPins, ControlSize));

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
			PUCHAR baSourceID = PUCHAR(m_ExtensionUnitDescriptor)+USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

			*OutSourceID = baSourceID[i];

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CExtensionUnit::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
CExtensionUnit::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	PUSB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor = PUSB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR(PUCHAR(m_ExtensionUnitDescriptor)+USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_CLUSTER_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));

	OutClusterDescriptor->bNrChannels = ClusterDescriptor->bNrChannels;
	OutClusterDescriptor->wChannelConfig = ClusterDescriptor->wChannelConfig;
	OutClusterDescriptor->iChannelNames = ClusterDescriptor->iChannelNames;

	return TRUE;
}

/*****************************************************************************
 * CExtensionUnit::NumberOfChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CExtensionUnit::
NumberOfChannels
(
	IN		BOOL	Direction
)
{
	PAGED_CODE();

	USHORT NumChannels = 0;

	if (Direction == 0) // Output
	{
		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

		if (FindAudioChannelCluster(&ClusterDescriptor))
		{
			NumChannels = ClusterDescriptor.bNrChannels;
		}
	}
	else // Input
	{
		PUCHAR baSourceID = PUCHAR(m_ExtensionUnitDescriptor)+USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BASOURCEID_OFFSET;

		for (UCHAR i=0; i<m_ExtensionUnitDescriptor->bNrInPins; i++)
		{
			PENTITY Entity = NULL;

			if (m_AudioTopology->FindEntity(baSourceID[i], &Entity))
			{
				switch (Entity->DescriptorSubtype())
				{
					case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
					case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
					{
						CTerminal * Terminal = (CTerminal*)Entity;

						USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
		
						if (Terminal->FindAudioChannelCluster(&ClusterDescriptor))
						{
							NumChannels += ClusterDescriptor.bNrChannels;
						}				
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
					case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
					{
						CUnit * Unit = (CUnit*)Entity;

						USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;
		
						if (Unit->FindAudioChannelCluster(&ClusterDescriptor))
						{
							NumChannels += ClusterDescriptor.bNrChannels;
						}				
					}
					break;
				}
			}
		}
	}

	return NumChannels;
}

/*****************************************************************************
 * CExtensionUnit::ExtensionDetails()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CExtensionUnit::
ExtensionDetails
(
	OUT		EXTENSION_UNIT_DETAILS *	OutDetails
)
{
	PUSB_DEVICE_DESCRIPTOR Descriptor = NULL;

	m_UsbDevice->GetDeviceDescriptor(&Descriptor);

	OutDetails->VendorID = Descriptor->idVendor;
	OutDetails->ProductID = Descriptor->idProduct;
	OutDetails->ExtensionCode = m_ExtensionUnitDescriptor->wExtensionCode;
	OutDetails->ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CExtensionUnit::PowerStateChange()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns STATUS_SUCCESS if the power state change is successful.
 */
NTSTATUS
CExtensionUnit::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up
			RestoreParameterBlock(m_ParameterBlock, m_ParameterBlockSize);
		}
		else
		{
			UpdateParameterBlock();
		}

		m_PowerState = NewState;
	}

    return STATUS_SUCCESS;
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_XU_CONTROL_ENABLE:
		{
			if (m_ParameterBlock->Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL Enable = *(PBOOL(ParameterBlock));

						UCHAR Current = Enable ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock->Enable.Current = Enable;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		default:
		{
			ULONG ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));

			if (ControlSelector <= (ControlSize*8))
			{
				if (m_ParameterBlock->Parameters[ControlSelector-1].Support)
				{
					if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
						(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
					{
						ntStatus = SetRequest(RequestCode, Control, ParameterBlock, ParameterBlockSize);
					}
				}
				else
				{
					ntStatus = STATUS_NOT_SUPPORTED;
				}
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
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	UpdateParameterBlock();

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_XU_CONTROL_ENABLE:
		{
			if (m_ParameterBlock->Enable.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL Enable = PBOOL(ParameterBlock);
						
						*Enable = m_ParameterBlock->Enable.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		default:
		{
			ULONG ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));
			
			if (ControlSelector <= (ControlSize*8))
			{
				if (m_ParameterBlock->Parameters[ControlSelector-1].Support)
				{
					if ((RequestCode == REQUEST_CUR) || (RequestCode == REQUEST_MIN) ||
						(RequestCode == REQUEST_MAX) || (RequestCode == REQUEST_RES))
					{
						ntStatus = GetRequest(RequestCode, Control, ParameterBlock, ParameterBlockSize, OutParameterBlockSize);
					}
				}
				else
				{
					ntStatus = STATUS_NOT_SUPPORTED;
				}
			}
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

	PUCHAR bmControls = PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(m_ExtensionUnitDescriptor->bNrInPins);

	UCHAR ByteOffset = Index / 8;

	UCHAR BitMask = 0x01 << (Index % 8);

	if (bmControls[ByteOffset] & BitMask)
	{
		Found = TRUE;
	}

	*OutControlSelector = Index+1;

	return Found;
}

/*****************************************************************************
 * CExtensionUnit::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CExtensionUnit::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	if (m_ParameterBlock)
	{
		// Initialize all controls on it.
		if (ParameterBlock && (ParameterBlockSize == m_ParameterBlockSize))
		{
			for (UCHAR i=0; i<(m_ControlSize*8); i++)
			{
				UCHAR ControlSelector = USB_AUDIO_UD_CONTROL_UNDEFINED;

				BOOL Support = _FindControl(i, &ControlSelector);

				_RestoreParameterBlock(ControlSelector, Support, PEXTENSION_UNIT_PARAMETER_BLOCK(ParameterBlock), FALSE);
			}

			RtlCopyMemory(m_ParameterBlock, ParameterBlock, m_ParameterBlockSize);
		}
		else
		{
			for (UCHAR i=0; i<(m_ControlSize*8); i++)
			{
				UCHAR ControlSelector = USB_AUDIO_UD_CONTROL_UNDEFINED;

				BOOL Support = _FindControl(i, &ControlSelector);

				_RestoreParameterBlock(ControlSelector, Support, m_ParameterBlock, TRUE);
			}
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CExtensionUnit::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CExtensionUnit::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= m_ParameterBlockSize))
	{
		RtlCopyMemory(ParameterBlock, m_ParameterBlock, m_ParameterBlockSize);

		*OutParameterBlockSize = m_ParameterBlockSize;

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CExtensionUnit::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CExtensionUnit::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = m_ParameterBlockSize;

	return ParameterBlockSize;
}

/*****************************************************************************
 * CExtensionUnit::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CExtensionUnit::
_RestoreParameterBlock
(
	IN		UCHAR							ControlSelector,
	IN		BOOL							Support,
	IN		PEXTENSION_UNIT_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL							Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_XU_CONTROL_ENABLE:
		{
			ParameterBlock->Enable.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->Enable.Current = Current ? TRUE: FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->Enable.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
		{
			ULONG ControlSize = *(PUCHAR(m_ExtensionUnitDescriptor) + USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(m_ExtensionUnitDescriptor->bNrInPins));
			
			if (ControlSelector <= (ControlSize*8))
			{
				ParameterBlock->Parameters[ControlSelector-1].Support = Support;
			}
		}
		break;
	}

	return ntStatus;
}

#pragma code_seg()

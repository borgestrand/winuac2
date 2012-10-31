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
 * @file	   Terminal.cpp
 * @brief	   Terminal implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Terminal.h"
#include "Audio.h"

#define STR_MODULENAME "Terminal: "

#pragma code_seg()

/*****************************************************************************
 * CTerminal::TerminalID()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CTerminal::
TerminalID
(	void
)
{
	return m_TerminalDescriptor->bTerminalID;
}

/*****************************************************************************
 * CTerminal::TerminalType()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
USHORT
CTerminal::
TerminalType
(	void
)
{
	return m_TerminalDescriptor->wTerminalType;
}

/*****************************************************************************
 * CTerminal::NumberOfChannels()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG
CTerminal::
NumberOfChannels
(	void
)
{
	USHORT NumChannels = 0;

	USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

	if (FindAudioChannelCluster(&ClusterDescriptor))
	{
		NumChannels = ClusterDescriptor.bNrChannels;
	}

	return NumChannels;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CTerminal::PowerStateChange()
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
CTerminal::
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
			RestoreParameterBlock(&m_ParameterBlock, sizeof(TERMINAL_PARAMETER_BLOCK));
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
 * CTerminal::WriteParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
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
		case USB_AUDIO_TE_CONTROL_COPY_PROTECT:
		{
			if (m_ParameterBlock.CopyProtect.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG CopyProtect = *(PULONG(ParameterBlock));

						UCHAR Current = UCHAR(CopyProtect);

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
								m_ParameterBlock.CopyProtect.Current = CopyProtect;
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
 * CTerminal::ReadParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
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
		case USB_AUDIO_TE_CONTROL_COPY_PROTECT:
		{
			if (m_ParameterBlock.CopyProtect.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG CopyProtect = PULONG(ParameterBlock);
						
						*CopyProtect = m_ParameterBlock.CopyProtect.Current;

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
 * CTerminal::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    PAGED_CODE();

	if (ParameterBlock && (ParameterBlockSize == sizeof(TERMINAL_PARAMETER_BLOCK)))
	{
		if ((m_TerminalDescriptor->wTerminalType == USB_TERMINAL_EXTERNAL_DIGITAL_AUDIO_INTERFACE) ||
			(m_TerminalDescriptor->wTerminalType == USB_TERMINAL_EXTERNAL_SPDIF_INTERFACE))
		{
			// FIXME: Is there any other way to determine if a terminal has copy protection control ??
			_RestoreParameterBlock(USB_AUDIO_TE_CONTROL_COPY_PROTECT, TRUE, PTERMINAL_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}
		else
		{
			_RestoreParameterBlock(USB_AUDIO_TE_CONTROL_COPY_PROTECT, FALSE, PTERMINAL_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(TERMINAL_PARAMETER_BLOCK));
	}
	else
	{
		if ((m_TerminalDescriptor->wTerminalType == USB_TERMINAL_EXTERNAL_DIGITAL_AUDIO_INTERFACE) ||
			(m_TerminalDescriptor->wTerminalType == USB_TERMINAL_EXTERNAL_SPDIF_INTERFACE))
		{
			// FIXME: Is there any other way to determine if a terminal has copy protection control ??
			_RestoreParameterBlock(USB_AUDIO_TE_CONTROL_COPY_PROTECT, TRUE, &m_ParameterBlock, TRUE);
		}
		else
		{
			_RestoreParameterBlock(USB_AUDIO_TE_CONTROL_COPY_PROTECT, FALSE, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CTerminal::SaveParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
SaveParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ParameterBlock && (ParameterBlockSize >= sizeof(TERMINAL_PARAMETER_BLOCK)))
	{
		RtlCopyMemory(ParameterBlock, &m_ParameterBlock, sizeof(TERMINAL_PARAMETER_BLOCK));

		*OutParameterBlockSize = sizeof(TERMINAL_PARAMETER_BLOCK);

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CTerminal::GetParameterBlockSize()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
ULONG 
CTerminal::
GetParameterBlockSize
(	void
)
{
    PAGED_CODE();

	ULONG ParameterBlockSize = sizeof(TERMINAL_PARAMETER_BLOCK);

	return ParameterBlockSize;
}

/*****************************************************************************
 * CTerminal::UpdateParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
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
 * CTerminal::InvalidateParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
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
 * CTerminal::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CTerminal::
_RestoreParameterBlock
(
	IN		UCHAR						ControlSelector,
	IN		BOOL						Support,
	IN		PTERMINAL_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL						Read
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_TE_CONTROL_COPY_PROTECT:
		{
			ParameterBlock->CopyProtect.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->CopyProtect.Current = ULONG(Current);
				}
				else
				{
					UCHAR Current = UCHAR(ParameterBlock->CopyProtect.Current);					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
		}
		break;

		default:
		{
			ntStatus = STATUS_INVALID_PARAMETER;
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CInputTerminal::~CInputTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
CInputTerminal::
~CInputTerminal
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
 * CInputTerminal::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the terminal.
 * @param
 * Device Pointer to the topology device object.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Terminal's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CInputTerminal::
Init
(
	IN		CAudioTopology *						AudioTopology,
	IN		PUSB_DEVICE								UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_TERMINAL_DESCRIPTOR	TerminalDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = TerminalDescriptor->bDescriptorSubtype;

	m_EntityID = TerminalDescriptor->bTerminalID;

	m_InputTerminalDescriptor = PUSB_AUDIO_INPUT_TERMINAL_DESCRIPTOR(TerminalDescriptor);

	m_TerminalDescriptor = TerminalDescriptor;

	m_PowerState = PowerDeviceD0;

	//edit yuanfen 
	//	Change InputTerminal Type to show Microphone in Control Panel
	PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor = NULL;

	m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

	if (UsbDeviceDescriptor)
	{
		if((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F04))
		{
			if((TerminalDescriptor->wTerminalType ==  USB_TERMINAL_EXTERNAL_DIGITAL_AUDIO_INTERFACE))
				TerminalDescriptor->wTerminalType = USB_TERMINAL_INPUT_MICROPHONE;

		}

		if((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F02))
		{
			if((TerminalDescriptor->wTerminalType ==  USB_TERMINAL_EXTERNAL_ANALOG_CONNECTOR))
				TerminalDescriptor->wTerminalType = USB_TERMINAL_INPUT_MICROPHONE;

		}

	} 
	//end edit

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CInputTerminal::iTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CInputTerminal::
iTerminal
(	void
)
{
	return m_InputTerminalDescriptor->iTerminal;
}

/*****************************************************************************
 * CInputTerminal::ParseSources()
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
CInputTerminal::
ParseSources
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutSourceID
)
{
	PAGED_CODE();

	// No sources connected to the input terminal.
	return FALSE;
}

/*****************************************************************************
 * CInputTerminal::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
CInputTerminal::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	OutClusterDescriptor->bNrChannels = m_InputTerminalDescriptor->bNrChannels;
	OutClusterDescriptor->wChannelConfig = m_InputTerminalDescriptor->wChannelConfig;
	OutClusterDescriptor->iChannelNames = m_InputTerminalDescriptor->iChannelNames;

	return TRUE;
}

/*****************************************************************************
 * COutputTerminal::~COutputTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Destructor.
 */
COutputTerminal::
~COutputTerminal
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
 * COutputTerminal::Init()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Initialize the terminal.
 * @param
 * Device Pointer to the topology device object.
 * @param
 * InterfaceNumber Interface number.
 * @param
 * Descriptor Terminal's descriptor.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
COutputTerminal::
Init
(
	IN		CAudioTopology *						AudioTopology,
	IN		PUSB_DEVICE								UsbDevice,
	IN		UCHAR									InterfaceNumber,
	IN		PUSB_AUDIO_COMMON_TERMINAL_DESCRIPTOR	TerminalDescriptor
)
{
	PAGED_CODE();

	m_AudioTopology = AudioTopology;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_DescriptorSubtype = TerminalDescriptor->bDescriptorSubtype;

	m_EntityID = TerminalDescriptor->bTerminalID;

	m_OutputTerminalDescriptor = PUSB_AUDIO_OUTPUT_TERMINAL_DESCRIPTOR(TerminalDescriptor);

	m_TerminalDescriptor = TerminalDescriptor;

	m_PowerState = PowerDeviceD0;

	//edit yuanfen
	//Reserve proper output terminal  
	PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor = NULL;

	m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

	if (UsbDeviceDescriptor)
	{
		if((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F04))
		{
			BOOL   bIsVista = TRUE;
			bIsVista = IoIsWdmVersionAvailable(6,0); //Vista OS
			
			if (bIsVista)
			{
				//Reserve Digital output
				if((TerminalDescriptor->wTerminalType ==  USB_TERMINAL_OUTPUT_SPEAKER) ||
			       (TerminalDescriptor->wTerminalType ==  USB_TERMINAL_OUTPUT_HEADPHONES))	
					return STATUS_INVALID_PARAMETER;
			}
			else 
			{
				//Reserve Speaker output
				if((TerminalDescriptor->wTerminalType ==  USB_TERMINAL_EXTERNAL_DIGITAL_AUDIO_INTERFACE) ||
			     (TerminalDescriptor->wTerminalType ==  USB_TERMINAL_OUTPUT_HEADPHONES))
					return STATUS_INVALID_PARAMETER;
			}
		}

		if((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F02))
		{
			if(TerminalDescriptor->wTerminalType ==  USB_TERMINAL_OUTPUT_HEADPHONES)
				return STATUS_INVALID_PARAMETER;
		}

	}
	//end edit

	RestoreParameterBlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * COutputTerminal::iTerminal()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
COutputTerminal::
iTerminal
(	void
)
{
	return m_OutputTerminalDescriptor->iTerminal;
}

/*****************************************************************************
 * COutputTerminal::ParseSources()
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
COutputTerminal::
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
		*OutSourceID = m_OutputTerminalDescriptor->bSourceID;

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * COutputTerminal::FindAudioChannelCluster()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL 
COutputTerminal::
FindAudioChannelCluster
(
	OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	PENTITY Entity = NULL;
	
	if (m_AudioTopology->FindEntity(m_OutputTerminalDescriptor->bSourceID, &Entity))
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

#pragma code_seg()

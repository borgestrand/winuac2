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
 * @file       Descriptor.cpp
 * @brief      MIDI Topology descriptor implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Descriptor.h"
#include "Filter.h"
#include "Pin.h"
#include "ExtProp.h"
#include "Tables.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "MidiTopology: "

#pragma code_seg("PAGE")


/*****************************************************************************
 * CWireDescriptor::CWireDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CWireDescriptor::
CWireDescriptor
(   void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CWireDescriptor::~CWireDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CWireDescriptor::
~CWireDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CWireDescriptor::~CWireDescriptor]"));
}

#pragma code_seg()

/*****************************************************************************
 * CWireDescriptor::ConnectFrom()
 *****************************************************************************
 *//*!
 * @brief
 * Connects the pin to the input side of the wire.
 * @param
 * Pin Pointer to the pin descriptor.
 * @return
 * None
 */
void
CWireDescriptor::
ConnectFrom
(
    IN      CPinDescriptor * Pin
)
{
    m_Pin1 = Pin;
}

/*****************************************************************************
 * CWireDescriptor::ConnectTo()
 *****************************************************************************
 *//*!
 * @brief
 * Connects the pin to the output side of the wire.
 * @param
 * Pin Pointer to the pin descriptor.
 * @return
 * None
 */
void
CWireDescriptor::
ConnectTo
(
    IN      CPinDescriptor * Pin
)
{
    m_Pin0 = Pin;
}

/*****************************************************************************
 * CWireDescriptor::FromPin()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the pin which is connected to the input side of the wire.
 * @param
 * None
 * @return
 * Returns a pointer to the pin descriptor.
 */
CPinDescriptor *
CWireDescriptor::
FromPin
(   void
)
{
    return m_Pin1;
}

/*****************************************************************************
 * CWireDescriptor::ToPin()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the pin which is connected to the output side of the wire.
 * @param
 * None
 * @return
 * Returns a pointer to the pin descriptor.
 */
CPinDescriptor *
CWireDescriptor::
ToPin
(   void
)
{
    return m_Pin0;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CPinDescriptor::CPinDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CPinDescriptor::
CPinDescriptor
(   void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CPinDescriptor::~CPinDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CPinDescriptor::
~CPinDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CPinDescriptor::~CPinDescriptor]"));
}

/*****************************************************************************
 * CPinDescriptor::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the pin descriptor.
 * @param
 * Node Pointer to the node descriptor.
 * @param
 * DataFlow Direction of the data flow.
 * @param
 * PinId Identifier of the pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CPinDescriptor::
Init
(
    IN      CNodeDescriptor *   Node,
    IN      ULONG               DataFlow,
    IN      ULONG               PinId
)
{
    PAGED_CODE();

    m_DataFlow = DataFlow;

    m_Node = Node;

    m_PinId = PinId;

    return STATUS_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CPinDescriptor::PinId()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the pin identifier.
 * @param
 * PinId Identifier of the pin.
 * @return
 * None
 */
void
CPinDescriptor::
PinId
(
    IN      ULONG   PinId
)
{
    m_PinId = PinId;
}

/*****************************************************************************
 * CPinDescriptor::PinId()
 *****************************************************************************
 *//*!
 * @overload
 * @brief
 * Gets the pin identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the pin.
 */
ULONG
CPinDescriptor::
PinId
(   void
)
{
    return (m_PinId);
}

/*****************************************************************************
 * CPinDescriptor::Node()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the node descriptor.
 * @param
 * None
 * @return
 * Returns the pointer to the node descriptor.
 */
CNodeDescriptor *
CPinDescriptor::
Node
(   void
)
{
    return m_Node;
}

/*****************************************************************************
 * CPinDescriptor::Connect()
 *****************************************************************************
 *//*!
 * @brief
 * Connects a wire to the pin.
 * @param
 * Wire Pointer to the wire descriptor.
 * @return
 * None
 */
void
CPinDescriptor::
Connect
(
    IN      CWireDescriptor *   Wire
)
{
    m_Wire = Wire;
}

/*****************************************************************************
 * CPinDescriptor::Wire()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the wire connected to the pin.
 * @param
 * None.
 * @return
 * Returns the pointer to the wire descriptor.
 */
CWireDescriptor *
CPinDescriptor::
Wire
(   void
)
{
    return m_Wire;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CNodeDescriptor::CNodeDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CNodeDescriptor::
CNodeDescriptor
(   void
)
{
    PAGED_CODE();
}

/*****************************************************************************
 * CNodeDescriptor::~CNodeDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CNodeDescriptor::
~CNodeDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CNodeDescriptor::~CNodeDescriptor]"));
}

/*****************************************************************************
 * CNodeDescriptor::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the node descriptor.
 * @param
 * FilterPin Pointer to the filter pin descriptor.
 * @param
 * NodeId Identifier of the node.
 * @param
 * Unit	Pointer to the topology unit.
 * @param
 * ControlSelector If the unit is a feature unit, then it is the selector to 
 * the control that this node is tied to. Otherwise, zero.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CNodeDescriptor::
Init
(
    IN      ULONG			NodeId,
	IN		PMIDI_JACK		Jack,
	IN		PMIDI_ELEMENT	Element,
	IN		ULONG			Flags,
	IN		ULONG			MaximumNumberOfInputPins,
	IN		ULONG			MaximumNumberOfOutputPins
)
{
    PAGED_CODE();

    m_NodeId = NodeId;
	
	m_Jack = Jack;

	m_JackID = (m_Jack) ? m_Jack->JackID() : 0;

	m_Element = Element;

	m_ElementID = (m_Element) ? m_Element->ElementID() : 0;

	m_MaximumNumberOfInputPins = MaximumNumberOfInputPins;

	m_MaximumNumberOfOutputPins = MaximumNumberOfOutputPins;

	_InitializeNodeDescriptor(Flags);

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CNodeDescriptor::_InitializeNodeDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the portcls node descriptor.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CNodeDescriptor::
_InitializeNodeDescriptor
(
	IN		ULONG	Flags
)
{
	m_KsNodeDescriptor.Name = NULL;

	if (m_Element)
	{
		m_KsNodeDescriptor.AutomationTable = &MidiElementAutomationTable;
		m_KsNodeDescriptor.Type = &KSNODETYPE_MIDI_ELEMENT;
	}
	else if (m_Jack)
	{
		switch (Flags)
		{
			case 0:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &KSNODETYPE_MIDI_JACK;
			}
			break;
	
			#ifdef ENABLE_DIRECTMUSIC_SUPPORT
			case 1:
			{
				m_KsNodeDescriptor.AutomationTable = &MidiSynthAutomationTable;
				m_KsNodeDescriptor.Type = &KSNODETYPE_SYNTHESIZER;
			}
			break;
			#endif // ENABLE_DIRECTMUSIC_SUPPORT
		}
	}
	else
	{
		m_KsNodeDescriptor.AutomationTable = NULL;
		m_KsNodeDescriptor.Type = &GUID_NULL;
	}

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CNodeDescriptor::AddPin()
 *****************************************************************************
 *//*!
 * @brief
 * Add a pin to the node.
 * @details
 * The type of pin (input or output) is indicated by the @em DataFlow parameter.
 * @param
 * DataFlow Direction of the data flow.
 * @return
 * Returns the pointer to the pin descriptor.
 */
CPinDescriptor *
CNodeDescriptor::
AddPin
(
    IN      ULONG               DataFlow
)
{
    PAGED_CODE();

    CPinDescriptor * Pin = new(NonPagedPool) CPinDescriptor();

    if (Pin)
    {
        Pin->Init(this, DataFlow, (DataFlow == PIN_DATAFLOW_IN) ? m_InputPinList.Count()+1 : 0);

        if (DataFlow == PIN_DATAFLOW_IN)
        {
            m_InputPinList.Put(Pin);
        }
        else
        {
            m_OutputPinList.Put(Pin);
        }
    }

    return Pin;
}

#pragma code_seg()

/*****************************************************************************
 * CNodeDescriptor::PinCount()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of pins (input & output) that this node has.
 * @param
 * None
 * @return
 * Returns the total pin count.
 */
ULONG
CNodeDescriptor::
PinCount
(   void
)
{
    return m_InputPinList.Count()+m_OutputPinList.Count();
}

/*****************************************************************************
 * CNodeDescriptor::GetDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the KS node descriptor.
 * @param
 * OutDescriptor Pointer to the KS node descriptor structure.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CNodeDescriptor::
GetDescriptor
(
    OUT     KSNODE_DESCRIPTOR * OutDescriptor
)
{
    ASSERT(OutDescriptor);

	*OutDescriptor = m_KsNodeDescriptor;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CNodeDescriptor::NodeId()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the node identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the node.
 */
ULONG
CNodeDescriptor::
NodeId
(   void
)
{
    return m_NodeId;
}

/*****************************************************************************
 * CNodeDescriptor::Element()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the element associated with this node.
 * @param
 * None
 * @return
 * Returns the element.
 */
PMIDI_ELEMENT
CNodeDescriptor::
Element
(   void
)
{
    return m_Element;
}

/*****************************************************************************
 * CNodeDescriptor::ElementID()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the element identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the element.
 */
UCHAR
CNodeDescriptor::
ElementID
(   void
)
{
    return m_ElementID;
}

/*****************************************************************************
 * CNodeDescriptor::Jack()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the jack associated with this node.
 * @param
 * None
 * @return
 * Returns the jack.
 */
PMIDI_JACK
CNodeDescriptor::
Jack
(   void
)
{
    return m_Jack;
}

/*****************************************************************************
 * CNodeDescriptor::JackID()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the element identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the element.
 */
UCHAR
CNodeDescriptor::
JackID
(   void
)
{
    return m_JackID;
}

/*****************************************************************************
 * CNodeDescriptor::IsConnectionPossible()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
CNodeDescriptor::
IsConnectionPossible
(
	IN		ULONG	DataFlow
)
{
	BOOL Possible;
	
	if (DataFlow == PIN_DATAFLOW_IN)
	{
		Possible = m_InputPinList.Count() < m_MaximumNumberOfInputPins;
	}
	else
	{
		Possible = m_OutputPinList.Count() < m_MaximumNumberOfOutputPins;
	}

	return Possible;
}

/*****************************************************************************
 * CNodeDescriptor::SupportCapability()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
CNodeDescriptor::
SupportCapability
(
	IN		UCHAR	Capability
)
{
	BOOL Supported = FALSE;

	if (m_Element)
	{
		UCHAR Index = Capability - 1;

		if (m_Element->ParseCapabilities(Index, &Capability))
		{
			if (Capability == (Index + 1))
			{
				Supported = TRUE;
			}
		}
	}

	return Supported;
}

/*****************************************************************************
 * CNodeDescriptor::WriteParameter()
 *****************************************************************************
 *//*!
 * @brief
 * Writes a parameter value.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CNodeDescriptor::
WriteParameter
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	RequestValueH,
	IN		UCHAR	RequestValueL,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_Element)
	{
		ntStatus = m_Element->WriteParameterBlock
						(
							RequestCode, 
							RequestValueH, 
							RequestValueL, 
							ParameterBlock, 
							ParameterBlockSize
						);
	}

    return ntStatus;
}

/*****************************************************************************
 * CNodeDescriptor::ReadParameter()
 *****************************************************************************
 *//*!
 * @brief
 * Reads a parameter value.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CNodeDescriptor::
ReadParameter
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	RequestValueH,
	IN		UCHAR	RequestValueL,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_Element)
	{
		ntStatus = m_Element->ReadParameterBlock
						(
							RequestCode, 
							RequestValueH, 
							RequestValueL, 
							ParameterBlock, 
							ParameterBlockSize,
							OutParameterBlockSize
						);
	}

    return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CFilterPinDescriptor::CFilterPinDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CFilterPinDescriptor::
CFilterPinDescriptor
(   void
)
:   CPinDescriptor()
{
    PAGED_CODE();
}

/*****************************************************************************
 * CFilterPinDescriptor::~CFilterPinDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CFilterPinDescriptor::
~CFilterPinDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::~CFilterPinDescriptor]"));

	if (m_PinDataRanges)
	{
		ExFreePool(m_PinDataRanges);
	}

	if (m_PinDataRangePointers)
	{
		ExFreePool(m_PinDataRangePointers);
	}
}

/*****************************************************************************
 * CFilterPinDescriptor::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the filter pin descriptor.
 * @param
 * Filter Pointer to the filter descriptor.
 * @param
 * Terminal Pointer to the topology terminal.
 * @param
 * PinId Identifier of the pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CFilterPinDescriptor::
Init
(
    IN      CFilterDescriptor * Filter,
	IN		PMIDI_JACK			Jack,
    IN      ULONG               PinId
)
{
    PAGED_CODE();

    m_Filter = Filter;

	m_Jack = Jack;

	m_JackType = Jack->JackType();
	
	m_JackID = Jack->JackID();

	m_iJack = Jack->iJack();

	m_ConnectionEnable = TRUE;

	m_IsSource = (Jack->DescriptorSubtype() == USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK);

	m_IsEmbedded = (m_JackType == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED);

    CPinDescriptor::Init(Filter, (m_IsSource) ? PIN_DATAFLOW_IN : PIN_DATAFLOW_OUT, PinId);

	if (m_JackType == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
	{
		// For MIDI streaming jacks, represent it with:
		m_Category = KSCATEGORY_WDMAUD_USE_PIN_NAME;
	}
	else
	{
		m_Category = KSCATEGORY_AUDIO;
	}

	m_PinDataRangesCount = 0;
	m_PinDataRanges = NULL;
	m_PinDataRangePointers = NULL;

	m_PinAllocatorFraming = CMidiPin::AllocatorFraming;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CFilterPinDescriptor::BuildDataRanges()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CFilterPinDescriptor::
BuildDataRanges
(
	IN		PMIDI_CABLE	MidiCable,
	IN		BOOL		DirectMusicCapable
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::BuildDataRanges] - %p", MidiCable));

	if (MidiCable)
	{
		if (MidiCable->AssociatedJackID() == m_JackID)
		{
			_AllocInitPinDataRangesStream
			(
				MidiCable->InterfaceNumber(), 
				MidiCable->EndpointAddress(), 
				MidiCable->CableNumber(),
				KSMUSIC_TECHNOLOGY_PORT, //FIXME: This should be derived ??
				0, 0, 0xFFFF,			 //FIXME: This should be derived ??
				KSDATAFORMAT_TYPE_MUSIC, 
				KSDATAFORMAT_SUBTYPE_MIDI, 
				KSDATAFORMAT_SPECIFIER_NONE
			);

			#ifdef ENABLE_DIRECTMUSIC_SUPPORT
			if (DirectMusicCapable)
			{
				_AllocInitPinDataRangesStream
				(
					MidiCable->InterfaceNumber(), 
					MidiCable->EndpointAddress(), 
					MidiCable->CableNumber(),
					KSMUSIC_TECHNOLOGY_PORT, //FIXME: This should be derived ??
					0, 0, 0xFFFF,			 //FIXME: This should be derived ??
					KSDATAFORMAT_TYPE_MUSIC, 
					KSDATAFORMAT_SUBTYPE_DIRECTMUSIC, 
					KSDATAFORMAT_SPECIFIER_NONE
				);
			}
			#endif // ENABLE_DIRECTMUSIC_SUPPORT
		}
	}
	else
	{
		_AllocInitPinDataRangesBridge
		(
			KSDATAFORMAT_TYPE_MUSIC, 
			KSDATAFORMAT_SUBTYPE_MIDI_BUS, 
			KSDATAFORMAT_SPECIFIER_NONE
		);
	}
	
	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CFilterPinDescriptor::_AllocInitPinDataRangesStream()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CFilterPinDescriptor::
_AllocInitPinDataRangesStream
(
	IN		UCHAR	InterfaceNumber,
	IN		UCHAR	EndpointAddress,
	IN		UCHAR	CableNumber,
	IN		GUID	Technology,
	IN		ULONG	Channels,
	IN		ULONG	Notes,
	IN		ULONG	ChannelMask,
	IN		GUID	MajorFormat,
	IN		GUID	SubFormat,
	IN		GUID	Specifier
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	ULONG PinDataRangesCount = m_PinDataRangesCount + 1;
	
	PKSDATARANGE_MUSIC_EX PinDataRanges = (PKSDATARANGE_MUSIC_EX)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(KSDATARANGE_MUSIC_EX), 'mdW');

	PKSDATARANGE * PinDataRangePointers = (PKSDATARANGE *)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(PKSDATARANGE), 'mdW');
	
	if (PinDataRanges && PinDataRangePointers)
	{
		if (m_PinDataRangesCount)
		{
			// There was a previously allocated data ranges. Append it to the new data ranges.
			ASSERT(m_PinDataRanges);

			RtlCopyMemory(PinDataRanges, m_PinDataRanges, m_PinDataRangesCount * sizeof(KSDATARANGE_MUSIC_EX));
			
			ExFreePool(m_PinDataRanges);

			m_PinDataRanges = NULL;

			ASSERT(m_PinDataRangePointers);

			ExFreePool(m_PinDataRangePointers);

			m_PinDataRangePointers = NULL;
		}

		ULONG Offset = m_PinDataRangesCount;

		PinDataRanges[Offset].Ex.DataRange.FormatSize = sizeof(KSDATARANGE_MUSIC);
		PinDataRanges[Offset].Ex.DataRange.Flags = 0;
		PinDataRanges[Offset].Ex.DataRange.SampleSize = 0;
		PinDataRanges[Offset].Ex.DataRange.Reserved = 0;
		PinDataRanges[Offset].Ex.DataRange.MajorFormat = MajorFormat;
		PinDataRanges[Offset].Ex.DataRange.SubFormat = SubFormat;
		PinDataRanges[Offset].Ex.DataRange.Specifier = Specifier;
		PinDataRanges[Offset].Ex.Technology = Technology;
		PinDataRanges[Offset].Ex.Channels = Channels;
		PinDataRanges[Offset].Ex.Notes = Notes;
		PinDataRanges[Offset].Ex.ChannelMask = ChannelMask;
		PinDataRanges[Offset].InterfaceNumber = InterfaceNumber;
		PinDataRanges[Offset].EndpointAddress = EndpointAddress;
		PinDataRanges[Offset].CableNumber = CableNumber;

		for (ULONG i=0; i<PinDataRangesCount; i++)
		{
			PinDataRangePointers[i] = (PKSDATARANGE)&PinDataRanges[i];
		}

		m_PinDataRangesCount = PinDataRangesCount;
		m_PinDataRanges = PinDataRanges;
		m_PinDataRangePointers = PinDataRangePointers;
	}
	else
	{
		if (PinDataRanges)
		{
			ExFreePool(PinDataRanges);
		}

		if (PinDataRangePointers)
		{
			ExFreePool(PinDataRangePointers);
		}

		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}

	return ntStatus;
}

/*****************************************************************************
 * CFilterPinDescriptor::_AllocInitPinDataRangesBridge()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CFilterPinDescriptor::
_AllocInitPinDataRangesBridge
(
	IN		GUID	MajorFormat,
	IN		GUID	SubFormat,
	IN		GUID	Specifier
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	ULONG PinDataRangesCount = m_PinDataRangesCount + 1;

	PKSDATARANGE_MUSIC_EX PinDataRanges = (PKSDATARANGE_MUSIC_EX)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(KSDATARANGE_MUSIC_EX), 'mdW');

	PKSDATARANGE * PinDataRangePointers = (PKSDATARANGE *)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(PKSDATARANGE), 'mdW');

	if (PinDataRanges && PinDataRangePointers)
	{
		if (m_PinDataRangesCount)
		{
			// There was a previously allocated data ranges. Append it to the new data ranges.
			ASSERT(m_PinDataRanges);

			RtlCopyMemory(PinDataRanges, m_PinDataRanges, m_PinDataRangesCount * sizeof(KSDATARANGE_MUSIC_EX));
			
			ExFreePool(m_PinDataRanges);

			m_PinDataRanges = NULL;

			ASSERT(m_PinDataRangePointers);

			ExFreePool(m_PinDataRangePointers);

			m_PinDataRangePointers = NULL;
		}

		ULONG Offset = m_PinDataRangesCount;

		PinDataRanges[Offset].Ex.DataRange.FormatSize = sizeof(KSDATARANGE);
		PinDataRanges[Offset].Ex.DataRange.Flags = 0;
		PinDataRanges[Offset].Ex.DataRange.SampleSize = 0;
		PinDataRanges[Offset].Ex.DataRange.Reserved = 0;
		PinDataRanges[Offset].Ex.DataRange.MajorFormat = MajorFormat;
		PinDataRanges[Offset].Ex.DataRange.SubFormat = SubFormat;
		PinDataRanges[Offset].Ex.DataRange.Specifier = Specifier;
		PinDataRanges[Offset].InterfaceNumber = 0;
		PinDataRanges[Offset].EndpointAddress = 0;
		PinDataRanges[Offset].CableNumber = 0;

		for (ULONG i=0; i<PinDataRangesCount; i++)
		{
			PinDataRangePointers[i] = (PKSDATARANGE)&PinDataRanges[i];
		}

		m_PinDataRangesCount = PinDataRangesCount;
		m_PinDataRanges = PinDataRanges;
		m_PinDataRangePointers = PinDataRangePointers;
	}
	else
	{
		if (PinDataRanges)
		{
			ExFreePool(PinDataRanges);
		}

		if (PinDataRangePointers)
		{
			ExFreePool(PinDataRangePointers);
		}

		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CFilterPinDescriptor::GetDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the portcls pin descriptor.
 * @param
 * OutDescriptor Pointer to the KS pin descriptor structure.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CFilterPinDescriptor::
GetDescriptor
(
    OUT     KSPIN_DESCRIPTOR_EX *	OutDescriptor
)
{
    ASSERT(OutDescriptor);

	if (m_JackType == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
	{
		OutDescriptor->Dispatch = &CMidiPin::DispatchTable;
		OutDescriptor->Flags = KSPIN_FLAG_ASYNCHRONOUS_PROCESSING | KSPIN_FLAG_CRITICAL_PROCESSING | KSPIN_FLAG_ENFORCE_FIFO;
		OutDescriptor->InstancesPossible = KSINSTANCE_INDETERMINATE;
		OutDescriptor->InstancesNecessary = 0;
		OutDescriptor->AllocatorFraming = &m_PinAllocatorFraming;
		OutDescriptor->IntersectHandler = NULL;
	}
	else
	{
		OutDescriptor->Dispatch = NULL;
		OutDescriptor->Flags = 0;
		OutDescriptor->InstancesPossible = 0;
		OutDescriptor->InstancesNecessary = 0;
		OutDescriptor->AllocatorFraming = NULL;
		OutDescriptor->IntersectHandler = NULL;
	}

	//FIXME: Figure out if we need a pin automation table ?
	OutDescriptor->AutomationTable = NULL;

	OutDescriptor->PinDescriptor.InterfacesCount = 0;
	OutDescriptor->PinDescriptor.Interfaces = NULL;
	OutDescriptor->PinDescriptor.MediumsCount = 0;
	OutDescriptor->PinDescriptor.Mediums = NULL;
	OutDescriptor->PinDescriptor.DataRangesCount = m_PinDataRangesCount;
	OutDescriptor->PinDescriptor.DataRanges = m_PinDataRangePointers;

	if (m_JackType == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
	{
		OutDescriptor->PinDescriptor.Communication = KSPIN_COMMUNICATION_SINK;// : KSPIN_COMMUNICATION_BOTH;
	}
	else
	{
		OutDescriptor->PinDescriptor.Communication = KSPIN_COMMUNICATION_NONE;
	}

	OutDescriptor->PinDescriptor.DataFlow = m_IsSource ? KSPIN_DATAFLOW_IN : KSPIN_DATAFLOW_OUT;
	OutDescriptor->PinDescriptor.Category = &m_Category;
	OutDescriptor->PinDescriptor.Name = NULL;//&m_Name;
	OutDescriptor->PinDescriptor.ConstrainedDataRangesCount = 0;
	OutDescriptor->PinDescriptor.ConstrainedDataRanges = NULL;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CFilterPinDescriptor::EnableConnection()
 *****************************************************************************
 *//*!
 * @brief
 * Enable/disable the filter pin connection.
 * @param
 * Flag TRUE to enable direction to the pin, FALSE to disable it.
 * @return
 * <None>
 */
void 
CFilterPinDescriptor::
EnableConnection
(
	IN		BOOL	Flag
)
{
    m_ConnectionEnable = Flag;
}

/*****************************************************************************
 * CFilterPinDescriptor::IsConnectionPossible()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the filter pin connection capability.
 * @param
 * <None>
 * @return
 * Returns TRUE if a connection can be made to the pin. Otherwise FALSE.
 */
BOOL 
CFilterPinDescriptor::
IsConnectionPossible
(	void
)
{
	return m_ConnectionEnable;
}

/*****************************************************************************
 * CFilterPinDescriptor::Name()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the filter pin name.
 * @param
 * None
 * @return
 * Returns the GUID representing the name of the filter pin.
 */
GUID *
CFilterPinDescriptor::
Name
(   void
)
{
    return &m_Name;
}

/*****************************************************************************
 * CFilterPinDescriptor::IsSource()
 *****************************************************************************
 *//*!
 * @brief
 * Determine if the filter pin is a source pin.
 * @param
 * None
 * @return
 * Returns TRUE if the filter pin is a source pin, otherwise FALSE.
 */
BOOL
CFilterPinDescriptor::
IsSource
(   void
)
{
    return m_IsSource;
}

/*****************************************************************************
 * CFilterPinDescriptor::IsEmbedded()
 *****************************************************************************
 *//*!
 * @brief
 * Determine if the filter pin is embedded.
 * @param
 * None
 * @return
 * Returns TRUE if the filter pin is embedded, otherwise FALSE.
 */
BOOL
CFilterPinDescriptor::
IsEmbedded
(   void
)
{
    return m_IsEmbedded;
}

/*****************************************************************************
 * CNodeDescriptor::Jack()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the MIDI jack.
 * @param
 * None
 * @return
 * Returns the MIDI jack.
 */
PMIDI_JACK
CFilterPinDescriptor::
Jack
(   void
)
{
    return m_Jack;
}

/*****************************************************************************
 * CNodeDescriptor::JackID()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the MIDI jack identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the MIDI jack.
 */
UCHAR
CFilterPinDescriptor::
JackID
(   void
)
{
    return m_JackID;
}

/*****************************************************************************
 * CFilterPinDescriptor::JackType()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the MIDI jack type.
 * @param
 * None
 * @return
 * Returns the MIDI jack type.
 */
UCHAR
CFilterPinDescriptor::
JackType
(   void
)
{
    return m_JackType;
}

/*****************************************************************************
 * CFilterPinDescriptor::iJack()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the MIDI jack string index.
 * @param
 * None
 * @return
 * Returns the MIDI jack string index.
 */
UCHAR
CFilterPinDescriptor::
iJack
(   void
)
{
    return m_iJack;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CFilterDescriptor::CFilterDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CFilterDescriptor::
CFilterDescriptor
(   void
)
:   CNodeDescriptor()
{
    PAGED_CODE();

    m_NodeId = KSFILTER_NODE;
}

/*****************************************************************************
 * CFilterDescriptor::~CFilterDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CFilterDescriptor::
~CFilterDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterDescriptor::~CFilterDescriptor]"));
}

/*****************************************************************************
 * CFilterDescriptor::AddPin()
 *****************************************************************************
 *//*!
 * @brief
 * Add a pin to the filter.
 * @param
 * Jack A MIDI jack.
 * @return
 * Returns the pointer of the filter pin descriptor.
 */
CFilterPinDescriptor *
CFilterDescriptor::
AddPin
(
	IN      PMIDI_JACK	Jack
)
{
    PAGED_CODE();

    CFilterPinDescriptor * Pin = new(NonPagedPool) CFilterPinDescriptor();

    if (Pin)
    {
        Pin->Init(this, Jack, PinCount());

        if (Pin->IsSource())
        {
            m_InputPinList.Put(Pin);
        }
        else
        {
            m_OutputPinList.Put(Pin);
        }
    }

    return Pin;
}

/*****************************************************************************
 * CFilterDescriptor::AddNode()
 *****************************************************************************
 *//*!
 * @brief
 * Add a node to the filter.
 * @return
 * Returns the pointer of the node descriptor.
 */
CNodeDescriptor *
CFilterDescriptor::
AddNode
(
	IN      PMIDI_JACK		Jack,
	IN		PMIDI_ELEMENT	Element,
	IN		ULONG			Flags,
	IN		ULONG			MaximumNumberOfInputPins,
	IN		ULONG			MaximumNumberOfOutputPins
)
{
    PAGED_CODE();

    CNodeDescriptor * Node = new(NonPagedPool) CNodeDescriptor();

    if (Node)
    {
        Node->Init(m_NodeList.Count(), Jack, Element, Flags, MaximumNumberOfInputPins, MaximumNumberOfOutputPins);

        m_NodeList.Put(Node);
    }

    return Node;
}

/*****************************************************************************
 * CFilterDescriptor::Connect()
 *****************************************************************************
 *//*!
 * @brief
 * Connects two pin together with a wire.
 * @param
 * FromPin Pointer to the FROM pin descriptor.
 * @param
 * FromPin Pointer to the TO pin descriptor.
 * @return
 * Returns the pointer to the wire descriptor.
 */
CWireDescriptor *
CFilterDescriptor::
Connect
(
    IN      CPinDescriptor *    FromPin,
    IN      CPinDescriptor *    ToPin
)
{
    PAGED_CODE();

    CWireDescriptor * Wire = new(NonPagedPool) CWireDescriptor();

    if (Wire)
    {
        m_WireList.Put(Wire);
        Wire->ConnectFrom(FromPin);
        Wire->ConnectTo(ToPin);
        FromPin->Connect(Wire);
        ToPin->Connect(Wire);
    }

    return Wire;
}

#pragma code_seg()

/*****************************************************************************
 * CFilterDescriptor::AddPinToEntity()
 *****************************************************************************
 *//*!
 * @brief
 */
CPinDescriptor *
CFilterDescriptor::
AddPinToEntity
(
	IN		UCHAR	EntityID,
	IN		ULONG	DataFlow
)
{
	CPinDescriptor * Pin = NULL;

	// Search the node to find the source.
	for (CNodeDescriptor * Node = (CNodeDescriptor *)m_NodeList.First(); Node != NULL; Node = (CNodeDescriptor *)m_NodeList.Next(Node))
	{
		if ((Node->ElementID() == EntityID) && (Node->IsConnectionPossible(DataFlow)))
		{
			Pin = Node->AddPin(DataFlow);
			break;
		}
		if ((Node->JackID() == EntityID) && (Node->IsConnectionPossible(DataFlow)))
		{
			Pin = Node->AddPin(DataFlow);
			break;
		}
	}

	if (!Pin)
	{
		// Search the output filter pins to find the source.
		for (CFilterPinDescriptor * FilterPin = (CFilterPinDescriptor *)m_OutputPinList.First(); FilterPin != NULL; FilterPin = (CFilterPinDescriptor *)m_OutputPinList.Next(FilterPin))
		{
			if (FilterPin->JackID() == EntityID)
			{
				if (!FilterPin->IsConnectionPossible())
				{
					CWireDescriptor * Wire = FilterPin->Wire();

					if (Wire)
					{
						CNodeDescriptor * Node = Wire->FromPin()->Node();

						if (Node)
						{
							if (Node->IsConnectionPossible(PIN_DATAFLOW_IN))
							{
								Pin = Node->AddPin(PIN_DATAFLOW_IN);
							}
						}
					}
				}
				else
				{
					Pin = FilterPin;
				}
			}

			if (Pin)
			{
				break;
			}
		}
	}

	if (!Pin)
	{
		// Search the input filter pins to find the source.
		for (CFilterPinDescriptor * FilterPin = (CFilterPinDescriptor *)m_InputPinList.First(); FilterPin != NULL; FilterPin = (CFilterPinDescriptor *)m_InputPinList.Next(FilterPin))
		{
			if (FilterPin->JackID() == EntityID)
			{
				if (!FilterPin->IsConnectionPossible())
				{
					CWireDescriptor * Wire = FilterPin->Wire();

					if (Wire)
					{
						CNodeDescriptor * Node = Wire->ToPin()->Node();

						if (Node)
						{
							if (Node->IsConnectionPossible(PIN_DATAFLOW_OUT))
							{
								Pin = Node->AddPin(PIN_DATAFLOW_OUT);
							}
						}
					}
				}
				else
				{
					Pin = FilterPin;
				}
			}

			if (Pin)
			{
				break;
			}
		}
	}

	return Pin;
}

/*****************************************************************************
 * CFilterDescriptor::PinCount()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of pins (input & output) that this filter has.
 * @param
 * None
 * @return
 * Returns the total pin count.
 */
ULONG
CFilterDescriptor::
PinCount
(   void
)
{
    return m_InputPinList.Count()+m_OutputPinList.Count();
}

/*****************************************************************************
 * CFilterDescriptor::Node()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of nodes that this filter has.
 * @param
 * None
 * @return
 * Returns the total node count.
 */
ULONG
CFilterDescriptor::
NodeCount
(   void
)
{
    return m_NodeList.Count();
}

/*****************************************************************************
 * CFilterDescriptor::ConnectionCount()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of connections that this filter has.
 * @param
 * None
 * @return
 * Returns the total connection count.
 */
ULONG
CFilterDescriptor::
ConnectionCount
(   void
)
{
    return m_WireList.Count();
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiFilterDescriptor::CMidiFilterDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CMidiFilterDescriptor::
CMidiFilterDescriptor
(   void
)
:   CFilterDescriptor()
{
    PAGED_CODE();
}

/*****************************************************************************
 * CMidiFilterDescriptor::~CMidiFilterDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CMidiFilterDescriptor::
~CMidiFilterDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterDescriptor::~CMidiFilterDescriptor]"));

	if (m_MidiDevice)
	{
		m_MidiDevice->Release();
	}

	if (m_KsPins)
    {
        ExFreePool(m_KsPins);
    }
    if (m_KsNodes)
    {
        ExFreePool(m_KsNodes);
    }
    if (m_KsTopologyConnections)
    {
        ExFreePool(m_KsTopologyConnections);
    }
}

/*****************************************************************************
 * CMidiFilterDescriptor::Initialize()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the MIDI filter descriptor.
 * @details
 * This method figures out the audio hardware topology by enumerating thru
 * the terminals/units to construct the pins & nodes. The pin and nodes are 
 * connected to each other (both pins and nodes) via wires. The pins, nodes 
 * and wires are then translated PortCls pins, nodes and connections structures.
 * @param
 * TopologyDevice Pointer to the topology device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiFilterDescriptor::
Initialize
(
    IN      PMIDI_DEVICE	MidiDevice,
	IN		PMIDI_TOPOLOGY	MidiTopology,
	IN		PMIDI_CABLE		MidiCable,
	IN		BOOL			DirectMusicCapable
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterDescriptor::Initialize]"));

	m_MidiDevice = MidiDevice;
	m_MidiDevice->AddRef();

	m_MidiTopology = MidiTopology;

	m_MidiCable = MidiCable;

    _ParseAndBuildPins(DirectMusicCapable);

    _ParseAndBuildNodes();

	if (PinCount())
	{
	    m_KsPins = (KSPIN_DESCRIPTOR_EX *)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSPIN_DESCRIPTOR_EX) * PinCount(), 'mdW');
	}

	if (NodeCount())
	{
		m_KsNodes = (KSNODE_DESCRIPTOR *)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSNODE_DESCRIPTOR) * NodeCount(), 'mdW');
    }

	if (ConnectionCount())
	{
		m_KsTopologyConnections = (KSTOPOLOGY_CONNECTION *)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSTOPOLOGY_CONNECTION) * ConnectionCount(), 'mdW');
	}

    // Fill in the pins...
	CFilterPinDescriptor * pin;

    for (pin = (CFilterPinDescriptor *)m_InputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_InputPinList.Next(pin))
    {
        pin->GetDescriptor(&m_KsPins[pin->PinId()]);
    }

    for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
        pin->GetDescriptor(&m_KsPins[pin->PinId()]);
    }

    // Fill in the nodes...
    for (CNodeDescriptor * node = m_NodeList.First(); node != NULL; node = m_NodeList.Next(node))
    {
        node->GetDescriptor(&m_KsNodes[node->NodeId()]);
    }

    // Build connections table.
    ULONG i = 0;
    for (CWireDescriptor * wire = m_WireList.First(); wire != NULL; wire = m_WireList.Next(wire))
    {
        m_KsTopologyConnections[i].FromNode     = wire->FromPin()->Node()->NodeId();
        m_KsTopologyConnections[i].FromNodePin  = wire->FromPin()->PinId();
        m_KsTopologyConnections[i].ToNode       = wire->ToPin()->Node()->NodeId();
        m_KsTopologyConnections[i].ToNodePin    = wire->ToPin()->PinId();

        i++;
    }

	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor; m_MidiDevice->GetDeviceDescriptor(&DeviceDescriptor);

	// The KSCOMPONENTID structure is used by the MME subsystem to fill in the MIDIIN/MIDIOUT caps
	// structure. So use INIT_MMREG_xxx macros instead of INIT_USBAUDIO_xxx.
	//INIT_USBAUDIO_MID(&m_KsComponentId.Manufacturer, DeviceDescriptor->idVendor);
	//INIT_USBAUDIO_PID(&m_KsComponentId.Product, DeviceDescriptor->idProduct);
	#define MM_EMU_USBMIDI	100
	if (DeviceDescriptor->idVendor == 0x041E/*Creative*/)
	{
		INIT_MMREG_MID(&m_KsComponentId.Manufacturer, MM_EMU);
		INIT_MMREG_PID(&m_KsComponentId.Product, MM_EMU_USBMIDI);
	}
	else
	{
		INIT_MMREG_MID(&m_KsComponentId.Manufacturer, MM_UNMAPPED);
		INIT_MMREG_PID(&m_KsComponentId.Product, MM_PID_UNMAPPED);
	}

	INIT_USBAUDIO_PRODUCT_NAME(&m_KsComponentId.Name, DeviceDescriptor->idVendor, DeviceDescriptor->idProduct, DeviceDescriptor->iProduct);

	m_KsComponentId.Component = KSCOMPONENTID_USBMIDI_10;
	m_KsComponentId.Version   = 1;
	m_KsComponentId.Revision  = 0;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CMidiFilterDescriptor::FindEntity()
 *****************************************************************************
 *//*!
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
CMidiFilterDescriptor::
FindEntity
(
	IN		UCHAR		EntityID,
	OUT		PENTITY *	OutEntity
)
{
	PAGED_CODE();

	return m_MidiTopology->FindEntity(EntityID, OutEntity);
}

#pragma code_seg()

/*****************************************************************************
 * CMidiFilterDescriptor::PinCount()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of pins (input & output) that this filter has.
 * @param
 * None
 * @return
 * Returns the total pin count.
 */
ULONG
CMidiFilterDescriptor::
PinCount
(   void
)
{
    return CFilterDescriptor::PinCount();
}

/*****************************************************************************
 * CMidiFilterDescriptor::Node()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of nodes that this filter has.
 * @param
 * None
 * @return
 * Returns the total node count.
 */
ULONG
CMidiFilterDescriptor::
NodeCount
(   void
)
{
    return CFilterDescriptor::NodeCount();
}

/*****************************************************************************
 * CMidiFilterDescriptor::ConnectionCount()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of connections that this filter has.
 * @param
 * None
 * @return
 * Returns the total connection count.
 */
ULONG
CMidiFilterDescriptor::
ConnectionCount
(   void
)
{
    return CFilterDescriptor::ConnectionCount();
}

/*****************************************************************************
 * CMidiFilterDescriptor::CategoryCount()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number of category that this filter has.
 * @param
 * None
 * @return
 * Returns the total category count.
 */
ULONG
CMidiFilterDescriptor::
CategoryCount
(   void
)
{
    return SIZEOF_ARRAY(CMidiFilter::Categories);
}

/*****************************************************************************
 * CMidiFilterDescriptor::Pins()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the KS pin descriptors.
 * @param
 * None
 * @return
 * Returns the pointer to the PortCls pin descriptors.
 */
KSPIN_DESCRIPTOR_EX *
CMidiFilterDescriptor::
Pins
(   void
)
{
    return m_KsPins;
}

/*****************************************************************************
 * CMidiFilterDescriptor::Nodes()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the KS node descriptors.
 * @param
 * None
 * @return
 * Returns the pointer to the PortCls node descriptors.
 */
KSNODE_DESCRIPTOR *
CMidiFilterDescriptor::
Nodes
(   void
)
{
    return m_KsNodes;
}

/*****************************************************************************
 * CMidiFilterDescriptor::Connections()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the KS topology connection descriptors.
 * @param
 * None
 * @return
 * Returns the pointer to the PortCls connection descriptors.
 */
KSTOPOLOGY_CONNECTION *
CMidiFilterDescriptor::
Connections
(   void
)
{
    return m_KsTopologyConnections;
}

/*****************************************************************************
 * CMidiFilterDescriptor::Categories()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the miniport's KS categories.
 * @param
 * None
 * @return
 * Returns the pointer to the miniport categories.
 */
GUID *
CMidiFilterDescriptor::
Categories
(   void
)
{
    return CMidiFilter::Categories;
}

/*****************************************************************************
 * CMidiFilterDescriptor::ComponentId()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the PortCls connection descriptors.
 * @param
 * None
 * @return
 * Returns the pointer to the PortCls connection descriptors.
 */
KSCOMPONENTID *
CMidiFilterDescriptor::
ComponentId
(   void
)
{
    return &m_KsComponentId;
}

/*****************************************************************************
 * CMidiFilterDescriptor::FindFilterPin()
 *****************************************************************************
 *//*!
 * @brief
 */
CFilterPinDescriptor *
CMidiFilterDescriptor::
FindFilterPin
(
	IN		ULONG	PinId
)
{
    // Find the appropriate pin
	CFilterPinDescriptor * pin;

    for (pin = m_InputPinList.First(); pin != NULL; pin = m_InputPinList.Next(pin))
    {
        if (pin->PinId() == PinId) break;
    }

	if (!pin)
	{
		for (pin = m_OutputPinList.First(); pin != NULL; pin = m_OutputPinList.Next(pin))
		{
			if (pin->PinId() == PinId) break;
		}
	}

	return pin;
}

/*****************************************************************************
 * CMidiFilterDescriptor::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
CNodeDescriptor *
CMidiFilterDescriptor::
FindNode
(
	IN		ULONG	NodeId
)
{
    // Find the appropriate node
    CNodeDescriptor * node;

	for (node = m_NodeList.First(); node != NULL; node = m_NodeList.Next(node))
    {
        if (node->NodeId() == NodeId) break;
    }

	return node;
}

/*****************************************************************************
 * CMidiFilterDescriptor::_ParseAndBuildPins()
 *****************************************************************************
 *//*!
 * @brief
 * Discovers the pins that is exposed by the audio topology.
 * @details
 * Enumerate thru the MIDI topology, and group these jacks into pin 
 * descriptors.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiFilterDescriptor::
_ParseAndBuildPins
(
	IN		BOOL	DirectMusicCapable
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterDescriptor::_ParseAndBuildPins]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	if (m_MidiTopology)
	{
		for (ULONG i=0; ; i++)
		{
			PMIDI_JACK Jack = NULL;

			if (m_MidiTopology->ParseJacks(i, &Jack))
			{
				if (m_MidiCable)
				{
					// If we are enumerating on a per-cable basis, then make
					// sure either the jack is the associated jack or it is in
					// some way connected to the associated jack.
					UCHAR AssociatedJackID = m_MidiCable->AssociatedJackID();

					UCHAR JackID = Jack->JackID();

					if ((JackID == AssociatedJackID) || m_MidiTopology->FindConnection(JackID, AssociatedJackID) || m_MidiTopology->FindConnection(AssociatedJackID, JackID))
					{
						AddPin(Jack);
					}
				}
				else
				{
					AddPin(Jack);
				}
			}
			else
			{
				break;
			}
		}
	}

	// Build the data ranges...
	CFilterPinDescriptor * pin;
    for (pin = (CFilterPinDescriptor *)m_InputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_InputPinList.Next(pin))
    {
        if (pin->JackType() == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
        {
			PMIDI_CABLE Cable = NULL;
			
			for (UCHAR i=0; ; i++)
			{
				// One of the cable got to match the jack...
				if (m_MidiDevice->ParseCables(i, &Cable))
				{
					pin->BuildDataRanges(Cable, DirectMusicCapable);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			pin->BuildDataRanges(NULL, FALSE);
		}
    }

    for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
        if (pin->JackType() == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
        {
			PMIDI_CABLE Cable = NULL;
			
			for (UCHAR i=0; ; i++)
			{
				// One of the cable got to match the jack...
				if (m_MidiDevice->ParseCables(i, &Cable))
				{
					pin->BuildDataRanges(Cable, DirectMusicCapable);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			pin->BuildDataRanges(NULL, FALSE);
		}
    }

	for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
		// According to MS UAA documentation, MIDI jacks are exposed as a node with 
		// KSNODETYPE_MIDI_JACK for identification. Any control request can
		// be sent to the the jack node, and data can be sent to the pin.
		#ifdef ENABLE_DIRECTMUSIC_SUPPORT
		if ((DirectMusicCapable)&& (pin->JackType() == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED))
		{
			CNodeDescriptor * Node0 = AddNode(pin->Jack(), NULL, 1/*synth*/, 1, 1);
			CPinDescriptor * FromPin = Node0->AddPin(PIN_DATAFLOW_OUT);
			Connect(FromPin, pin);

			CNodeDescriptor * Node1 = AddNode(pin->Jack(), NULL, 0/*jack*/, -1, 1);
			FromPin = Node1->AddPin(PIN_DATAFLOW_OUT);
			CPinDescriptor * ToPin = Node0->AddPin(PIN_DATAFLOW_IN);
			Connect(FromPin, ToPin);
		}
		else
		#endif // ENABLE_DIRECTMUSIC_SUPPORT
		{
			CNodeDescriptor * FromNode = AddNode(pin->Jack(), NULL, 0/*jack*/, -1, 1);
			CPinDescriptor * FromPin = FromNode->AddPin(PIN_DATAFLOW_OUT);
			Connect(FromPin, pin);
		}

		pin->EnableConnection(FALSE);
    }

    for (pin = (CFilterPinDescriptor *)m_InputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_InputPinList.Next(pin))
    {
		// According to MS UAA documentation, MIDI jacks are exposed as a node with 
		// KSNODETYPE_MIDI_JACK for identification. Any control request can
		// be sent to the the jack node, and data can be sent to the pin.
		#ifdef ENABLE_DIRECTMUSIC_SUPPORT
		if ((DirectMusicCapable) && (pin->JackType() == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED))
		{
			CNodeDescriptor * Node0 = AddNode(pin->Jack(), NULL, 1/*synth*/, 1, 1);
			CPinDescriptor * ToPin = Node0->AddPin(PIN_DATAFLOW_IN);
			Connect(pin, ToPin);

			CNodeDescriptor * Node1 = AddNode(pin->Jack(), NULL, 0/*jack*/, 1, -1);
			CPinDescriptor * FromPin = Node0->AddPin(PIN_DATAFLOW_OUT);
			ToPin = Node1->AddPin(PIN_DATAFLOW_IN);
			Connect(FromPin, ToPin);
		}
		else
		#endif // ENABLE_DIRECTMUSIC_SUPPORT
		{
			CNodeDescriptor * ToNode = AddNode(pin->Jack(), NULL, 0/*jack*/, 1, -1);
			CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);
			Connect(pin, ToPin);
		}

		pin->EnableConnection(FALSE);
    }

	return ntStatus;
}

/*****************************************************************************
 * CMidiFilterDescriptor::_ParseAndBuildNodes()
 *****************************************************************************
 *//*!
 * @brief
 * Discovers the nodes that are in the MIDI filter.
 * @details
 * Enumerate the MIDI topology elements, and find all the nodes that connected
 * to the pins. Also the connections between the nodes & pins are established
 * using wire descriptors.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiFilterDescriptor::
_ParseAndBuildNodes
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterDescriptor::_ParseAndBuildNodes]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	if (m_MidiTopology)
	{
		for (UCHAR i=0; ; i++)
		{
			PMIDI_ELEMENT Element = NULL;

			if (m_MidiTopology->ParseElements(i, &Element))
			{
				if (m_MidiCable)
				{
					// If we are enumerating on a per-cable basis, then make
					// sure either the element in some way connected to the 
					// associated jack.
					UCHAR AssociatedJackID = m_MidiCable->AssociatedJackID();

					UCHAR ElementID = Element->ElementID();

					if (m_MidiTopology->FindConnection(ElementID, AssociatedJackID) || m_MidiTopology->FindConnection(AssociatedJackID, ElementID))
					{
						AddNode(NULL, Element, 0, -1, -1);
					}
				}
				else
				{
					AddNode(NULL, Element, 0, -1, -1);
				}
			}
			else
			{
				break;
			}
		}
	}

    // Search for the pin/nodes connected to the output pins.
    for (CFilterPinDescriptor * FilterPin = (CFilterPinDescriptor *)m_OutputPinList.First(); FilterPin != NULL; FilterPin = (CFilterPinDescriptor *)m_OutputPinList.Next(FilterPin))
    {
		PMIDI_JACK Jack = FilterPin->Jack();

		CPinDescriptor * ToPin;

		CWireDescriptor * Wire = FilterPin->Wire();

		if (Wire)
		{
			ToPin = AddPinToEntity(FilterPin->JackID(), PIN_DATAFLOW_IN);
		}
		else
		{
			ToPin = FilterPin;
		}

		for (UCHAR i=0; ; i++)
		{
			UCHAR SourceID = 0;
			UCHAR SourcePin = 0;

			if (Jack->ParseSources(i, &SourceID, &SourcePin))
			{
				CPinDescriptor * FromPin = AddPinToEntity(SourceID, PIN_DATAFLOW_OUT);

				if (FromPin)
				{
					Connect(FromPin, ToPin);
				}
			}
			else
			{
				break;
			}
		}
    }

    // Parse the nodes to establish connections between them.
    for (CNodeDescriptor * ToNode = (CNodeDescriptor *)m_NodeList.First(); ToNode != NULL; ToNode = (CNodeDescriptor *)m_NodeList.Next(ToNode))
    {
		PMIDI_ELEMENT Element = ToNode->Element();

		if (Element)
		{
			if (ToNode->IsConnectionPossible(PIN_DATAFLOW_IN))
			{
				for (UCHAR i=0; ; i++)
				{
					UCHAR SourceID = 0;
					UCHAR SourcePin = 0;

					if (Element->ParseSources(i, &SourceID, &SourcePin))
					{
						CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);

						CPinDescriptor * FromPin = AddPinToEntity(SourceID, PIN_DATAFLOW_OUT);

						if (FromPin)
						{
							Connect(FromPin, ToPin);
						}
					}
					else
					{
						break;
					}
				}
			}
		}
    }

	return ntStatus;
}

#pragma code_seg()


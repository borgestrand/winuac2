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
*//*
 *****************************************************************************
 *//*!
 * @file       Descriptor.cpp
 * @brief      Audioo topology descriptor implementation.
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
#define STR_MODULENAME "Topology: "

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

	if (m_KsNodeDescriptor.AutomationTable)
	{
		_DestroyAutomationTable(PKSAUTOMATION_TABLE(m_KsNodeDescriptor.AutomationTable));
	}
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
    IN      ULONG       NodeId,
	IN		PTERMINAL	Terminal,
	IN		PUNIT		Unit,
	IN		UCHAR		ControlSelector,
	IN		ULONG		MaximumNumberOfInputPins,
	IN		ULONG		MaximumNumberOfOutputPins
)
{
    PAGED_CODE();

    m_NodeId = NodeId;
	
	m_Terminal = Terminal;

	m_Unit = Unit;

	if (m_Unit)
	{
		m_UnitID = m_Unit->UnitID();

		if (m_Unit->DescriptorSubtype() == USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT)
		{
			CMixerUnit * MixerUnit = (CMixerUnit*)(Unit);

			// The control selector in the mixer unit is re-used as the index to the input plug.
			if (ControlSelector)
			{
				// Per input plug channel count.
				m_NumberOfInputChannels = MixerUnit->NumberOfInputChannels(ControlSelector-1, &m_InputChannelOffset);
			}
			else
			{
				m_NumberOfInputChannels = m_Unit->NumberOfChannels(1);
				m_InputChannelOffset = 0;
			}

			m_NumberOfOutputChannels = m_Unit->NumberOfChannels(0);
			m_OutputChannelOffset = 0;
		}
		else
		{
			m_NumberOfInputChannels = m_Unit->NumberOfChannels(1);
			m_NumberOfOutputChannels = m_Unit->NumberOfChannels(0);
			m_InputChannelOffset = 0;
			m_OutputChannelOffset = 0;
		}
	}
	else
	{
		m_UnitID = 0;
		m_NumberOfInputChannels = 0;
		m_NumberOfOutputChannels = 0;
		m_InputChannelOffset = 0;
		m_OutputChannelOffset = 0;
	}

	m_ControlSelector = ControlSelector;

	m_MaximumNumberOfInputPins = MaximumNumberOfInputPins;

	m_MaximumNumberOfOutputPins = MaximumNumberOfOutputPins;

	_InitializeNodeDescriptor();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CNodeDescriptor::_BuildAutomationTable()
 *****************************************************************************
 *//*!
 * @brief
 */
PKSAUTOMATION_TABLE
CNodeDescriptor::
_BuildAutomationTable
(
	IN		PKSAUTOMATION_TABLE	Template
)
{
	ULONG i;
	ULONG SizeOfStruct = sizeof(KSAUTOMATION_TABLE);
	
	SizeOfStruct += Template->PropertySetsCount * sizeof(KSPROPERTY_SET);
	SizeOfStruct += Template->MethodSetsCount * sizeof(KSMETHOD_SET);
	SizeOfStruct += Template->EventSetsCount * sizeof(KSEVENT_SET);
	
	for (i=0; i<Template->PropertySetsCount; i++)
	{
		SizeOfStruct += Template->PropertySets[i].PropertiesCount * sizeof(KSPROPERTY_ITEM);
	}

	for (i=0; i<Template->MethodSetsCount; i++)
	{
		SizeOfStruct +=Template->MethodSets[i].MethodsCount * sizeof(KSMETHOD_ITEM);
	}

	for (i=0; i<Template->EventSetsCount; i++)
	{
		SizeOfStruct += Template->EventSets[i].EventsCount * sizeof(KSEVENT_ITEM);
	}

	PKSAUTOMATION_TABLE AutomationTable = PKSAUTOMATION_TABLE(ExAllocatePoolWithTag(NonPagedPool, SizeOfStruct, 'mdW'));

	if (AutomationTable)
	{
		RtlZeroMemory(AutomationTable, SizeOfStruct);

		PVOID Pointer = PVOID(AutomationTable + 1);

		AutomationTable->PropertySetsCount = Template->PropertySetsCount;
		AutomationTable->PropertyItemSize  = Template->PropertyItemSize;

		if (AutomationTable->PropertySetsCount)
		{
			AutomationTable->PropertySets = PKSPROPERTY_SET(Pointer);
			Pointer = PVOID(AutomationTable->PropertySets + AutomationTable->PropertySetsCount);
		}

		AutomationTable->MethodSetsCount = Template->MethodSetsCount;
		AutomationTable->MethodItemSize  = Template->MethodItemSize;

		if (AutomationTable->MethodSetsCount)
		{
			AutomationTable->MethodSets = PKSMETHOD_SET(Pointer);
			Pointer = PVOID(AutomationTable->MethodSets + AutomationTable->MethodSetsCount);
		}

		AutomationTable->EventSetsCount = Template->EventSetsCount;
		AutomationTable->EventItemSize  = Template->EventItemSize;

		if (AutomationTable->EventSetsCount)
		{
			AutomationTable->EventSets = PKSEVENT_SET(Pointer);
			Pointer = PVOID(AutomationTable->EventSets + AutomationTable->EventSetsCount);
		}

		for (i=0; i<AutomationTable->PropertySetsCount; i++)
		{
			PKSPROPERTY_SET PropertySet = PKSPROPERTY_SET(&AutomationTable->PropertySets[i]);

			PropertySet->Set = Template->PropertySets[i].Set;
			PropertySet->PropertiesCount = Template->PropertySets[i].PropertiesCount;

			if (PropertySet->PropertiesCount)
			{
				PropertySet->PropertyItem = PKSPROPERTY_ITEM(Pointer);			
				RtlCopyMemory(PVOID(PropertySet->PropertyItem), Template->PropertySets[i].PropertyItem, PropertySet->PropertiesCount * sizeof(KSPROPERTY_ITEM));
				Pointer = PVOID(PropertySet->PropertyItem + PropertySet->PropertiesCount);
			}
		}

		for (i=0; i<AutomationTable->MethodSetsCount; i++)
		{
			PKSMETHOD_SET MethodSet = PKSMETHOD_SET(&AutomationTable->MethodSets[i]);

			MethodSet->Set = Template->MethodSets[i].Set;
			MethodSet->MethodsCount = Template->MethodSets[i].MethodsCount;

			if (MethodSet->MethodsCount)
			{
				MethodSet->MethodItem = PKSMETHOD_ITEM(Pointer);			
				RtlCopyMemory(PVOID(MethodSet->MethodItem), Template->MethodSets[i].MethodItem, MethodSet->MethodsCount * sizeof(KSMETHOD_ITEM));
				Pointer = PVOID(MethodSet->MethodItem + MethodSet->MethodsCount);
			}
		}

		for (i=0; i<AutomationTable->EventSetsCount; i++)
		{
			PKSEVENT_SET EventSet = PKSEVENT_SET(&AutomationTable->EventSets[i]);

			EventSet->Set = Template->EventSets[i].Set;
			EventSet->EventsCount = Template->EventSets[i].EventsCount;

			if (EventSet->EventsCount)
			{
				EventSet->EventItem = PKSEVENT_ITEM(Pointer);			
				RtlCopyMemory(PVOID(EventSet->EventItem), Template->EventSets[i].EventItem, EventSet->EventsCount * sizeof(KSEVENT_ITEM));
				Pointer = PVOID(EventSet->EventItem + EventSet->EventsCount);
			}
		}
	}

	return AutomationTable;
}

/*****************************************************************************
 * CNodeDescriptor::_BuildExtensionAutomationTable()
 *****************************************************************************
 *//*!
 * @brief
 */
PKSAUTOMATION_TABLE
CNodeDescriptor::
_BuildExtensionAutomationTable
(
	IN		GUID *	ExtensionPropSet,
	IN		ULONG	ExtensionPropertiesCount
)
{
	ULONG SizeOfStruct = sizeof(KSAUTOMATION_TABLE);
	
	// 3 property sets
	SizeOfStruct += 3 * sizeof(KSPROPERTY_SET);

	SizeOfStruct += (2 + ExtensionPropertiesCount) * sizeof(KSPROPERTY_ITEM);

	// 1 event set
	SizeOfStruct += sizeof(KSEVENT_SET);

	SizeOfStruct += sizeof(KSEVENT_ITEM);

	PKSAUTOMATION_TABLE AutomationTable = PKSAUTOMATION_TABLE(ExAllocatePoolWithTag(NonPagedPool, SizeOfStruct, 'mdW'));

	if (AutomationTable)
	{
		RtlZeroMemory(AutomationTable, SizeOfStruct);

		PVOID Pointer = PVOID(AutomationTable + 1);

		AutomationTable->PropertySetsCount = 3;
		AutomationTable->PropertyItemSize  = sizeof(KSPROPERTY_ITEM);
		AutomationTable->PropertySets = PKSPROPERTY_SET(Pointer);
		Pointer = PVOID(AutomationTable->PropertySets + AutomationTable->PropertySetsCount);

		AutomationTable->EventSetsCount = 1;
		AutomationTable->EventItemSize  = sizeof(KSEVENT_ITEM);
		AutomationTable->EventSets = PKSEVENT_SET(Pointer);
		Pointer = PVOID(AutomationTable->EventSets + AutomationTable->EventSetsCount);

		// Property sets...
		PKSPROPERTY_SET PropertySet = PKSPROPERTY_SET(&AutomationTable->PropertySets[0]);

		PropertySet->Set = &KSPROPSETID_TopologyNode;
		PropertySet->PropertiesCount = 1;
		PropertySet->PropertyItem = PKSPROPERTY_ITEM(Pointer);	

		PKSPROPERTY_ITEM PropertyItem = PKSPROPERTY_ITEM(PropertySet->PropertyItem);
		
		PropertyItem->PropertyId = KSPROPERTY_TOPOLOGYNODE_ENABLE;
		PropertyItem->GetPropertyHandler = PFNKSHANDLER(CAudioFilter::GetEnableExtensionControl);
		PropertyItem->MinProperty = sizeof(KSNODEPROPERTY);
		PropertyItem->MinData = sizeof(BOOL);
		PropertyItem->SetPropertyHandler = PFNKSHANDLER(CAudioFilter::SetEnableExtensionControl);
		
		Pointer = PVOID(PropertySet->PropertyItem + PropertySet->PropertiesCount);

		PropertySet = PKSPROPERTY_SET(&AutomationTable->PropertySets[1]);

		PropertySet->Set = &KSPROPSETID_Audio;
		PropertySet->PropertiesCount = 1;
		PropertySet->PropertyItem = PKSPROPERTY_ITEM(Pointer);			

		PropertyItem = PKSPROPERTY_ITEM(PropertySet->PropertyItem);
		
		PropertyItem->PropertyId = KSPROPERTY_AUDIO_CPU_RESOURCES;
		PropertyItem->GetPropertyHandler = PFNKSHANDLER(CAudioFilter::GetCpuResources);
		PropertyItem->MinProperty = sizeof(KSNODEPROPERTY);
		PropertyItem->MinData = sizeof(ULONG);
		PropertyItem->SetPropertyHandler = NULL;

		Pointer = PVOID(PropertySet->PropertyItem + PropertySet->PropertiesCount);

		if (ExtensionPropertiesCount)
		{
			PropertySet = PKSPROPERTY_SET(&AutomationTable->PropertySets[2]);

			PropertySet->Set = ExtensionPropSet;
			PropertySet->PropertiesCount = ExtensionPropertiesCount;
			PropertySet->PropertyItem = PKSPROPERTY_ITEM(Pointer);

			for (ULONG i=0; i<PropertySet->PropertiesCount; i++)
			{
				PropertyItem = PKSPROPERTY_ITEM(&PropertySet->PropertyItem[i]);

				PropertyItem->PropertyId = i; // D0->...
				PropertyItem->GetPropertyHandler = PFNKSHANDLER(CAudioFilter::GetExtensionControl);
				PropertyItem->MinProperty = sizeof(KSNODEPROPERTY);
				PropertyItem->MinData = 0;
				PropertyItem->SetPropertyHandler = PFNKSHANDLER(CAudioFilter::SetExtensionControl);
			}

			Pointer = PVOID(PropertySet->PropertyItem + PropertySet->PropertiesCount);
		}

		// Event sets...
		PKSEVENT_SET EventSet = PKSEVENT_SET(&AutomationTable->EventSets[0]);

		EventSet->Set = &KSEVENTSETID_AudioControlChange;
		EventSet->EventsCount = 1;
		EventSet->EventItem = PKSEVENT_ITEM(Pointer);	

		PKSEVENT_ITEM EventItem = PKSEVENT_ITEM(EventSet->EventItem);
		
		EventItem->EventId = KSEVENT_CONTROL_CHANGE;
		EventItem->DataInput = sizeof(KSEVENTDATA);
		EventItem->ExtraEntryData = sizeof(ULONG) * 2;
		EventItem->AddHandler =CAudioFilter::AddControlEvent;
		EventItem->RemoveHandler = CAudioFilter::RemoveControlEvent;
		EventItem->SupportHandler = NULL;
		
		Pointer = PVOID(EventSet->EventItem + EventSet->EventsCount);
	}

	return AutomationTable;
}

/*****************************************************************************
 * CNodeDescriptor::_DestroyAutomationTable()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CNodeDescriptor::
_DestroyAutomationTable
(
	IN		PKSAUTOMATION_TABLE	AutomationTable
)
{
	if (AutomationTable)
	{
		ExFreePool(AutomationTable);
	}
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
(	void
)
{
	m_KsNodeDescriptor.Name = NULL;

	if (m_Unit)
	{
		switch (m_Unit->DescriptorSubtype())
		{
			case  USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
			{
				if (m_ControlSelector == 0)
				{
					m_KsNodeDescriptor.AutomationTable = NULL;
					m_KsNodeDescriptor.Type = &KSNODETYPE_SUM;
				}
				else
				{
					// Non zero control selector in the mixer unit is re-used as the index to the input plug.
					m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&MixAutomationTable));
					m_KsNodeDescriptor.Type = &KSNODETYPE_SUPERMIX;
				}
			}
			break;

			case  USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
			{
				m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&MuxAutomationTable));
				m_KsNodeDescriptor.Type = &KSNODETYPE_MUX;
			}
			break;

			case  USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
			{
				switch (m_ControlSelector)
				{
					case USB_AUDIO_FU_CONTROL_MUTE:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&MuteAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_MUTE;
					}
					break;

					case USB_AUDIO_FU_CONTROL_VOLUME:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&VolumeLevelAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_VOLUME;
					}
					break;

					case USB_AUDIO_FU_CONTROL_BASS:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&BassAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_TONE;
					}
					break;

					case USB_AUDIO_FU_CONTROL_MID:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&MidAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_TONE;
					}
					break;

					case USB_AUDIO_FU_CONTROL_TREBLE:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&TrebleAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_TONE;
					}
					break;

					case USB_AUDIO_FU_CONTROL_GRAPHIC_EQ:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&EqAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_EQUALIZER;
					}
					break;

					case USB_AUDIO_FU_CONTROL_AUTOMATIC_GAIN:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&AgcAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_AGC;
					}
					break;

					case USB_AUDIO_FU_CONTROL_DELAY:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&DelayAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_DELAY;
					}
					break;

					case USB_AUDIO_FU_CONTROL_BASS_BOOST:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&BassBoostAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_TONE;
					}
					break;

					case USB_AUDIO_FU_CONTROL_LOUDNESS:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&LoudnessAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_LOUDNESS;
					}
					break;

					default:
					{
						m_KsNodeDescriptor.AutomationTable = NULL;
						m_KsNodeDescriptor.Type = &GUID_NULL;
					}
					break;
				}
			}
			break;

			case  USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
			{
				PPROCESSOR_UNIT ProcessorUnit = PPROCESSOR_UNIT(m_Unit);

				switch (ProcessorUnit->ProcessType())
				{
					case USB_AUDIO_PROCESS_UPMIX_DOWNMIX:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&UpDownMixAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_UPDOWN_MIX;
					}
					break;

					case USB_AUDIO_PROCESS_DOLBY_PROLOGIC:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&DolbyPrologicAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_PROLOGIC_DECODER;
					}
					break;

					case USB_AUDIO_PROCESS_3D_STEREO_EXTENDER:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&StereoExtenderAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_STEREO_WIDE;
					}
					break;

					case USB_AUDIO_PROCESS_REVERBERATION:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&ReverbAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_REVERB;
					}
					break;

					case USB_AUDIO_PROCESS_CHORUS:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&ChorusAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_CHORUS;
					}
					break;

					case USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION:
					{
						m_KsNodeDescriptor.AutomationTable = _BuildAutomationTable(PKSAUTOMATION_TABLE(&DrcAutomationTable));
						m_KsNodeDescriptor.Type = &KSNODETYPE_DYN_RANGE_COMPRESSOR;
					}
					break;

					default:
					{
						m_KsNodeDescriptor.AutomationTable = NULL;
						m_KsNodeDescriptor.Type = &GUID_NULL;
					}
					break;
				}

			}
			break;

			case  USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
			{
				// Setup the variable-sized extension property set.
				PEXTENSION_UNIT ExtensionUnit = PEXTENSION_UNIT(m_Unit);

				EXTENSION_UNIT_DETAILS Details;	ExtensionUnit->ExtensionDetails(&Details);

				INIT_XU_PROPSETID(&m_ExtensionPropSetId, Details.VendorID, Details.ProductID, Details.ExtensionCode);

				ULONG NumExtensionProperties = Details.ControlSize * 8;
				
				m_KsNodeDescriptor.AutomationTable = _BuildExtensionAutomationTable(&m_ExtensionPropSetId, NumExtensionProperties);
				m_KsNodeDescriptor.Type = &KSNODETYPE_DEV_SPECIFIC;
			}
			break;

			default:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &GUID_NULL;
			}
			break;
		}
	}
	else if (m_Terminal)
	{
		switch (m_ControlSelector)
		{
			case 0:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &KSNODETYPE_DAC;
			}
			break;

			case 1:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &KSNODETYPE_ADC;
			}
			break;

			case 2:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &KSNODETYPE_SRC;
			}
			break;

			case 3:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &KSNODETYPE_SPDIF_INTERFACE;
			}
			break;

			default:
			{
				m_KsNodeDescriptor.AutomationTable = NULL;
				m_KsNodeDescriptor.Type = &GUID_NULL;
			}
			break;
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
 * Gets the portcls node descriptor.
 * @param
 * OutDescriptor Pointer to the portcls node descriptor structure.
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
 * CNodeDescriptor::Unit()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the unit.
 * @param
 * None
 * @return
 * Returns the unit.
 */
PUNIT
CNodeDescriptor::
Unit
(   void
)
{
    return m_Unit;
}

/*****************************************************************************
 * CNodeDescriptor::UnitID()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the unit identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the unit.
 */
UCHAR
CNodeDescriptor::
UnitID
(   void
)
{
    return m_UnitID;
}

/*****************************************************************************
 * CNodeDescriptor::ControlSelector()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the unit associated control selector.
 * @param
 * None
 * @return
 * Returns the unit associated control selector.
 */
UCHAR
CNodeDescriptor::
ControlSelector
(   void
)
{
    return m_ControlSelector;
}

/*****************************************************************************
 * CNodeDescriptor::ProcessType()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the process type of the unit.
 * @param
 * None
 * @return
 * Returns the process type of the unit.
 */
USHORT
CNodeDescriptor::
ProcessType
(   void
)
{
	USHORT ProcessType = 0;

	if (m_Unit)
	{
		if (m_Unit->DescriptorSubtype() == USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT)
		{
			PPROCESSOR_UNIT ProcessorUnit = PPROCESSOR_UNIT(m_Unit);

			ProcessType = ProcessorUnit->ProcessType();
		}
	}

	return ProcessType;
}

/*****************************************************************************
 * CNodeDescriptor::NumberOfChannels()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
CNodeDescriptor::
NumberOfChannels
(
	IN		BOOL	Direction,
	OUT		PULONG	OutChannelOffset
)
{
	if (Direction == 0)
	{
		if (OutChannelOffset)
		{
			*OutChannelOffset = m_OutputChannelOffset;
		}

		return m_NumberOfOutputChannels;
	}
	else
	{
		if (OutChannelOffset)
		{
			*OutChannelOffset = m_InputChannelOffset;
		}

		return m_NumberOfInputChannels;
	}
}

/*****************************************************************************
 * CNodeDescriptor::ChannelConfig()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG
CNodeDescriptor::
NumberOfModes
(
	IN OUT	PLONG	ActiveSpeakerPositions
)
{
	ULONG NumChannelConfig = 0;

	if (m_Unit)
	{
		if (m_Unit->DescriptorSubtype() == USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT)
		{
			PPROCESSOR_UNIT ProcessorUnit = PPROCESSOR_UNIT(m_Unit);

			USHORT ProcessType = ProcessorUnit->ProcessType();

			if (ProcessType == USB_AUDIO_PROCESS_UPMIX_DOWNMIX)
			{
				PUPDOWN_MIX_UNIT UpDownMixUnit = PUPDOWN_MIX_UNIT(m_Unit);
			
				NumChannelConfig = UpDownMixUnit->NumberOfModes(ActiveSpeakerPositions);
			}
			else if (ProcessType == USB_AUDIO_PROCESS_DOLBY_PROLOGIC)
			{
				PDOLBY_PROLOGIC_UNIT DolbyPrologicUnit = PDOLBY_PROLOGIC_UNIT(m_Unit);

				NumChannelConfig = DolbyPrologicUnit->NumberOfModes(ActiveSpeakerPositions);
			}
		}
	}
	else if (m_Terminal)
	{
		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

		if (m_Terminal->FindAudioChannelCluster(&ClusterDescriptor))
		{
			if (ActiveSpeakerPositions)
			{
				*ActiveSpeakerPositions = LONG(ClusterDescriptor.wChannelConfig);
			}

			NumChannelConfig = 1;
		}
	}

    return NumChannelConfig;
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

	if (m_Unit)
	{
		ULONG Flags = (m_DrmReferenceCount > 0) ? PARAMETER_BLOCK_FLAGS_IO_SOFTWARE : PARAMETER_BLOCK_FLAGS_IO_BOTH;

		ntStatus = m_Unit->WriteParameterBlock
						(
							RequestCode, 
							RequestValueH, 
							RequestValueL, 
							ParameterBlock, 
							ParameterBlockSize,
							Flags
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

	if (m_Unit)
	{
		ntStatus = m_Unit->ReadParameterBlock
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
 * CNodeDescriptor::EnforceDrmProtection()
 *****************************************************************************
 *//*!
 * @brief
 * Enforce DRM protection scheme.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CNodeDescriptor::
EnforceDrmProtection
(
	IN		BOOL	OnOff
)
{
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_Unit)
	{
		switch (m_Unit->DescriptorSubtype())
		{
			case  USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
			{
				if (m_ControlSelector) // SuperMix
				{
					if (OnOff) // On
					{
						if (InterlockedIncrement(&m_DrmReferenceCount) == 1)
						{
							ULONG InputChannelOffset = 0;

							ULONG InputChannels = NumberOfChannels(1, &InputChannelOffset);

							ULONG OutputChannelOffset = 0;

							ULONG OutputChannels = NumberOfChannels(0, &OutputChannelOffset);

							for (ULONG i=InputChannelOffset, k=0; i<(InputChannelOffset+InputChannels); i++)
							{
								for (ULONG j=OutputChannelOffset; j<(OutputChannelOffset+OutputChannels); j++, k++)
								{
									LONG Level = INFINITY * dB;
									
									m_Unit->WriteParameterBlock(REQUEST_CUR, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), PARAMETER_BLOCK_FLAGS_IO_HARDWARE);
								}
							}
						}
					}
					else // Off
					{
						if (InterlockedDecrement(&m_DrmReferenceCount) == 0)
						{
							ULONG InputChannelOffset = 0;

							ULONG InputChannels = NumberOfChannels(1, &InputChannelOffset);

							ULONG OutputChannelOffset = 0;

							ULONG OutputChannels = NumberOfChannels(0, &OutputChannelOffset);

							for (ULONG i=InputChannelOffset, k=0; i<(InputChannelOffset+InputChannels); i++)
							{
								for (ULONG j=OutputChannelOffset; j<(OutputChannelOffset+OutputChannels); j++, k++)
								{
									LONG Level = 0;
									
									m_Unit->ReadParameterBlock(REQUEST_CUR, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), NULL);
									
									m_Unit->WriteParameterBlock(REQUEST_CUR, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), PARAMETER_BLOCK_FLAGS_IO_HARDWARE);
								}
							}
						}
					}

					ntStatus = STATUS_SUCCESS;
				}
			}
			break;

			case  USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
			{
				if (m_ControlSelector == USB_AUDIO_FU_CONTROL_MUTE)
				{
					if (OnOff) // On
					{
						if (InterlockedIncrement(&m_DrmReferenceCount) == 1)
						{
							UCHAR CS = USB_AUDIO_FU_CONTROL_MUTE;

							ULONG Channels = NumberOfChannels(0);

							for (UCHAR CN=0; CN<=Channels; CN++)
							{
								BOOL Mute = TRUE;
	
								m_Unit->WriteParameterBlock(REQUEST_CUR, CS, CN, &Mute, sizeof(BOOL), PARAMETER_BLOCK_FLAGS_IO_HARDWARE);
							}
						}
					}
					else // Off
					{
						if (InterlockedDecrement(&m_DrmReferenceCount) == 0)
						{
							UCHAR CS = USB_AUDIO_FU_CONTROL_MUTE;

							ULONG Channels = NumberOfChannels(0);

							for (UCHAR CN=0; CN<=Channels; CN++)
							{
								BOOL Mute = FALSE;

								m_Unit->ReadParameterBlock(REQUEST_CUR, CS, CN, &Mute, sizeof(BOOL), NULL);

								m_Unit->WriteParameterBlock(REQUEST_CUR, CS, CN, &Mute, sizeof(BOOL), PARAMETER_BLOCK_FLAGS_IO_HARDWARE);
							}
						}
					}

					ntStatus = STATUS_SUCCESS;
				}
			}
			break;

			case  USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
			{
				PPROCESSOR_UNIT ProcessorUnit = PPROCESSOR_UNIT(m_Unit);

				if (ProcessorUnit->ProcessType() == USB_AUDIO_PROCESS_UPMIX_DOWNMIX)
				{
					if (OnOff) // On
					{
						if (InterlockedIncrement(&m_DrmReferenceCount) == 1)
						{
							UCHAR CS = USB_AUDIO_UD_CONTROL_MODE_SELECT;

							ULONG ModesCount = NumberOfModes(NULL);

							PLONG Modes = PLONG(ExAllocatePoolWithTag(PagedPool, ModesCount * sizeof(LONG), 'mdW'));

							if (Modes)
							{
								ntStatus = STATUS_NOT_IMPLEMENTED;

								NumberOfModes(Modes);

								for (ULONG i=0; i<ModesCount; i++)
								{
									if (Modes[i] == 0)
									{
										ULONG Mode = i+1;									
										
										ntStatus = m_Unit->WriteParameterBlock(REQUEST_CUR, CS, 0, &Mode, sizeof(ULONG), PARAMETER_BLOCK_FLAGS_IO_HARDWARE);

										break;
									}
								}

								ExFreePool(Modes);

							}
							else
							{
								ntStatus = STATUS_INSUFFICIENT_RESOURCES;
							}
						}
						else
						{
							ntStatus = STATUS_SUCCESS;
						}
					}
					else // Off
					{
						if (InterlockedDecrement(&m_DrmReferenceCount) == 0)
						{
							UCHAR CS = USB_AUDIO_UD_CONTROL_MODE_SELECT;

							ULONG Mode = 1;

							m_Unit->ReadParameterBlock(REQUEST_CUR, CS, 0, &Mode, sizeof(ULONG), NULL);

							m_Unit->WriteParameterBlock(REQUEST_CUR, CS, 0, &Mode, sizeof(ULONG), PARAMETER_BLOCK_FLAGS_IO_HARDWARE);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
		}
	}

    return ntStatus;
}

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
	IN		PTERMINAL			Terminal,
    IN      ULONG               PinId,
	IN		ULONG				FormatSpecifier
)
{
    PAGED_CODE();

    m_Filter = Filter;

	m_Terminal = Terminal;

	m_TerminalType = Terminal->TerminalType();
	m_DescriptorSubtype = Terminal->DescriptorSubtype();
	m_TerminalID = Terminal->TerminalID();
	m_iTerminal = Terminal->iTerminal();

    m_NumberOfChannels = m_Terminal->NumberOfChannels();

	m_ConnectionEnable = TRUE;

	m_IsSource = (m_DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL);

	m_IsDigital = (m_TerminalType == USB_TERMINAL_USB_STREAMING) ||
				  (m_TerminalType == USB_TERMINAL_EXTERNAL_DIGITAL_AUDIO_INTERFACE) ||	
				  (m_TerminalType == USB_TERMINAL_EXTERNAL_SPDIF_INTERFACE) ||	
				  (m_TerminalType == USB_TERMINAL_EXTERNAL_1394_DA_STREAM) ||	
				  (m_TerminalType == USB_TERMINAL_EXTERNAL_1394_DV_STREAM_SOUNDTRACK);	
	
    CPinDescriptor::Init(Filter, (m_IsSource) ? PIN_DATAFLOW_IN : PIN_DATAFLOW_OUT, PinId);

	m_FormatSpecifier = FormatSpecifier;

	if (m_TerminalType == USB_TERMINAL_USB_STREAMING)
	{
		// For USB streaming terminals, represent it with:
		// render: KSCATEGORY_AUDIO
		// capture: PINNAME_CAPTURE.
		m_Category = m_IsSource ? KSCATEGORY_AUDIO : PINNAME_CAPTURE;
	}
	else
	{
		INIT_USB_TERMINAL(&m_Category, m_TerminalType);
	}

	m_PinDataRangesCount = 0;
	m_PinDataRanges = NULL;
	m_PinDataRangePointers = NULL;

	m_PinAllocatorFraming = CAudioPin::AllocatorFraming;

	INIT_USB_TERMINAL(&m_Name, Terminal->TerminalType());

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
	IN		PAUDIO_INTERFACE	Interface
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::BuildDataRanges] - %p", Interface));

	if (Interface)
	{
		UCHAR InterfaceNumber = Interface->InterfaceNumber();

		_DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::BuildDataRanges] - InterfaceNumber : 0x%x", InterfaceNumber));

		for (UCHAR AlternateSetting=1; ;AlternateSetting++)
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::BuildDataRanges] - AlternateSetting : 0x%x", AlternateSetting));

			USHORT FormatTag = USB_AUDIO_FORMAT_TYPE_I_UNDEFINED;

			if (Interface->ParseSupportedFormat(AlternateSetting, &FormatTag))
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::BuildDataRanges] - FormatTag : 0x%02x", FormatTag));

				if (Interface->TerminalLink(AlternateSetting) == m_TerminalID)
				{
					//BEGIN_HACK
					if (Interface->HasBadDescriptors())
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = Interface->GetDataEndpointDescriptor(AlternateSetting);

						if (EndpointDescriptor->bInterval == 4/*1ms == 8 micro-frames. 2^(bInterval-1)*/)
						{
							if (_FindSimilarAlternateSetting(Interface, AlternateSetting))
							{
								// Skip this alternate setting.
								continue;
							}
						}
					}
					//END_HACK

					_DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::BuildDataRanges] - TerminalLink : 0x%02x", m_TerminalID));

					PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor = Interface->GetFormatTypeDescriptor(AlternateSetting);

					if (FormatTypeDescriptor)
					{
						switch (FormatTag)
						{
							case USB_AUDIO_FORMAT_TYPE_I_PCM:
							case USB_AUDIO_FORMAT_TYPE_I_PCM8:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_I)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_PCM, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_PCM, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							case USB_AUDIO_FORMAT_TYPE_I_IEEE_FLOAT:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_I)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							case USB_AUDIO_FORMAT_TYPE_I_ALAW:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_I)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_ALAW, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_ALAW, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							case USB_AUDIO_FORMAT_TYPE_I_MULAW:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_I)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_MULAW, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_MULAW, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							case USB_AUDIO_FORMAT_TYPE_II_MPEG:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_II)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_MPEG, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_MPEG, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							case USB_AUDIO_FORMAT_TYPE_II_AC3:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_II)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_AC3_AUDIO, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_AC3_AUDIO, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							case USB_AUDIO_FORMAT_TYPE_III_IEC1937_AC3:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_III)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF, KSDATAFORMAT_SPECIFIER_DSOUND);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							//FIXME: Determine if usbaudio.sys support which of these as MP3 formats.
							case USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER_1:
							case USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER_23/*USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_NOEXT*/:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_III)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_STANDARD_MPEG1_AUDIO, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_STANDARD_MPEG1_AUDIO, KSDATAFORMAT_SPECIFIER_DSOUND);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;

							//FIXME: Not supported ???
							case USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_EXT:
							case USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER_1_LS:
							case USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER_23_LS:
							{
								if (m_FormatSpecifier & FORMAT_SPECIFIER_TYPE_III)
								{
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_MPEG2_AUDIO, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_MPEG2_AUDIO, KSDATAFORMAT_SPECIFIER_DSOUND);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
									_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF, KSDATAFORMAT_SPECIFIER_DSOUND);
								}
							}
							break;
								
							default:
							{
								// Unsupported or unknown format. Ignore it.
								//_AllocInitPinDataRangesStream(InterfaceNumber, AlternateSetting, FormatTypeDescriptor, KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_UNKNOWN, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX);
							}
							break;
						}
					}
				}
			}
			else
			{
				// No more formats...
				break;
			}
		}
	}
	else
	{
		_AllocInitPinDataRangesBridge(KSDATAFORMAT_TYPE_AUDIO, KSDATAFORMAT_SUBTYPE_ANALOG, KSDATAFORMAT_SPECIFIER_NONE);
	}
	
	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CFilterPinDescriptor::_FindSimilarAlternateSetting()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CFilterPinDescriptor::
_FindSimilarAlternateSetting
(
	IN		PAUDIO_INTERFACE	Interface,
	IN		UCHAR				AlternateSetting
)
{
    PAGED_CODE();

	BOOL FoundSimilarDescriptor = FALSE;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::_FindSimilarAlternateSetting]"));

	if (Interface)
	{
		USHORT FormatTag = USB_AUDIO_FORMAT_TYPE_I_UNDEFINED;

		Interface->ParseSupportedFormat(AlternateSetting, &FormatTag);

		UCHAR TerminalID = Interface->TerminalLink(AlternateSetting);
	
		PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor = Interface->GetFormatTypeDescriptor(AlternateSetting);

		if (FormatTypeDescriptor)
		{
			for (UCHAR altsetting=1; ;altsetting++)
			{
				if (altsetting != AlternateSetting)
				{
					USHORT SimilarFormatTag = USB_AUDIO_FORMAT_TYPE_I_UNDEFINED;

					if (Interface->ParseSupportedFormat(altsetting, &SimilarFormatTag))
					{
						if (SimilarFormatTag == FormatTag)
						{
							if (Interface->TerminalLink(altsetting) == TerminalID)
							{
								PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR SimilarFormatTypeDescriptor = Interface->GetFormatTypeDescriptor(altsetting);

								if (SimilarFormatTypeDescriptor)
								{
									if (SimilarFormatTypeDescriptor->bLength == FormatTypeDescriptor->bLength)
									{
										if (RtlCompareMemory(SimilarFormatTypeDescriptor, FormatTypeDescriptor, FormatTypeDescriptor->bLength) == FormatTypeDescriptor->bLength)
										{
											FoundSimilarDescriptor = TRUE;
											break;
										}
									}
								}
							}
						}
					}
					else
					{
						// No more formats...
						break;
					}
				}
			}
		}
	}
	
	return FoundSimilarDescriptor;
}

#define SAMFREQ(tSamFreq) ((ULONG(tSamFreq[2])<<16) | (ULONG(tSamFreq[1])<<8) | ULONG(tSamFreq[0]))

/*****************************************************************************
 * CFilterPinDescriptor::_AllocInitPinDataRangesTypeI()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CFilterPinDescriptor::
_AllocInitPinDataRangesStream
(
	IN		UCHAR										InterfaceNumber,
	IN		UCHAR										AlternateSetting,
	IN		PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR	FormatTypeDescriptor_,
	IN		GUID										MajorFormat,
	IN		GUID										SubFormat,
	IN		GUID										Specifier
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	if ((FormatTypeDescriptor_->bFormatType == USB_AUDIO_FORMAT_TYPE_I) || (FormatTypeDescriptor_->bFormatType == USB_AUDIO_FORMAT_TYPE_III))
	{
		// The format type descriptor is the same for TYPE_I and TYPE_III.
		PUSB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR FormatTypeDescriptor = PUSB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR(FormatTypeDescriptor_);

		ULONG PinDataRangesCount = m_PinDataRangesCount + (FormatTypeDescriptor->bSamFreqType ? FormatTypeDescriptor->bSamFreqType : 1);

		PKSDATARANGE_AUDIO_EX PinDataRanges = (PKSDATARANGE_AUDIO_EX)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(KSDATARANGE_AUDIO_EX), 'mdW');

		PKSDATARANGE * PinDataRangePointers = (PKSDATARANGE *)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(PKSDATARANGE), 'mdW');
		
		if (PinDataRanges && PinDataRangePointers)
		{
			if (m_PinDataRangesCount)
			{
				// There was a previously allocated data ranges. Append it to the new data ranges.
				ASSERT(m_PinDataRanges);

				RtlCopyMemory(PinDataRanges, m_PinDataRanges, m_PinDataRangesCount * sizeof(KSDATARANGE_AUDIO_EX));
				
				ExFreePool(m_PinDataRanges);

				m_PinDataRanges = NULL;

				ASSERT(m_PinDataRangePointers);

				ExFreePool(m_PinDataRangePointers);

				m_PinDataRangePointers = NULL;
			}

			ULONG Offset = m_PinDataRangesCount;

			if (FormatTypeDescriptor->bSamFreqType == 0)
			{
				// Continuous sample rate.
				PinDataRanges[Offset].Ex.DataRange.FormatSize = sizeof(KSDATARANGE_AUDIO);
				PinDataRanges[Offset].Ex.DataRange.Flags = 0;
				PinDataRanges[Offset].Ex.DataRange.SampleSize = 0;
				PinDataRanges[Offset].Ex.DataRange.Reserved = 0;
				PinDataRanges[Offset].Ex.DataRange.MajorFormat = MajorFormat;
				PinDataRanges[Offset].Ex.DataRange.SubFormat = SubFormat;
				PinDataRanges[Offset].Ex.DataRange.Specifier = Specifier;
				PinDataRanges[Offset].Ex.MaximumChannels = FormatTypeDescriptor->bNrChannels;
				PinDataRanges[Offset].Ex.MinimumBitsPerSample = FormatTypeDescriptor->bBitResolution;
				PinDataRanges[Offset].Ex.MaximumBitsPerSample = FormatTypeDescriptor->bBitResolution;
				PinDataRanges[Offset].Ex.MinimumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[0]); // lower bound
				PinDataRanges[Offset].Ex.MaximumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[1]); // upper bound
				PinDataRanges[Offset].InterfaceNumber = InterfaceNumber;
				PinDataRanges[Offset].AlternateSetting = AlternateSetting;
				PinDataRanges[Offset].BitResolution = FormatTypeDescriptor->bBitResolution;

				if ((FormatTypeDescriptor->bBitResolution >= 16) && IsEqualGUIDAligned(SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
				{
					// Support conversion down to 16-bit data type.
					PinDataRanges[Offset].Ex.MinimumBitsPerSample = 16;
					// Support conversion up to 32-bit data type.
					PinDataRanges[Offset].Ex.MaximumBitsPerSample = 32;
				}
			}
			else
			{
				// Discrete sampling rates.
				for (ULONG i=Offset; i<PinDataRangesCount; i++)
				{
					PinDataRanges[i].Ex.DataRange.FormatSize = sizeof(KSDATARANGE_AUDIO);
					PinDataRanges[i].Ex.DataRange.Flags = 0;
					PinDataRanges[i].Ex.DataRange.SampleSize = 0;
					PinDataRanges[i].Ex.DataRange.Reserved = 0;
					PinDataRanges[i].Ex.DataRange.MajorFormat = MajorFormat;
					PinDataRanges[i].Ex.DataRange.SubFormat = SubFormat;
					PinDataRanges[i].Ex.DataRange.Specifier = Specifier;
					PinDataRanges[i].Ex.MaximumChannels = FormatTypeDescriptor->bNrChannels;
					PinDataRanges[i].Ex.MinimumBitsPerSample = FormatTypeDescriptor->bBitResolution;
					PinDataRanges[i].Ex.MaximumBitsPerSample = FormatTypeDescriptor->bBitResolution;
					PinDataRanges[i].Ex.MinimumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[i-Offset]);
					PinDataRanges[i].Ex.MaximumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[i-Offset]);
					PinDataRanges[i].InterfaceNumber = InterfaceNumber;
					PinDataRanges[i].AlternateSetting = AlternateSetting;
					PinDataRanges[i].BitResolution = FormatTypeDescriptor->bBitResolution;

					if ((FormatTypeDescriptor->bBitResolution >= 16) && IsEqualGUIDAligned(SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
					{
						// Support conversion down to 16-bit data type.
						PinDataRanges[i].Ex.MinimumBitsPerSample = 16;
						// Support conversion up to 32-bit data type.
						PinDataRanges[i].Ex.MaximumBitsPerSample = 32;
					}
				}
			}

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
	}
	else
	{
		PUSB_AUDIO_TYPE_II_FORMAT_DESCRIPTOR FormatTypeDescriptor = PUSB_AUDIO_TYPE_II_FORMAT_DESCRIPTOR(FormatTypeDescriptor_);

		ULONG PinDataRangesCount = m_PinDataRangesCount + (FormatTypeDescriptor->bSamFreqType ? FormatTypeDescriptor->bSamFreqType : 1);

		PKSDATARANGE_AUDIO_EX PinDataRanges = (PKSDATARANGE_AUDIO_EX)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(KSDATARANGE_AUDIO_EX), 'mdW');

		PKSDATARANGE * PinDataRangePointers = (PKSDATARANGE *)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(PKSDATARANGE), 'mdW');
		
		if (PinDataRanges && PinDataRangePointers)
		{
			if (m_PinDataRangesCount)
			{
				// There was a previously allocated data ranges. Append it to the new data ranges.
				ASSERT(m_PinDataRanges);

				RtlCopyMemory(PinDataRanges, m_PinDataRanges, m_PinDataRangesCount * sizeof(KSDATARANGE_AUDIO_EX));
				
				ExFreePool(m_PinDataRanges);

				m_PinDataRanges = NULL;

				ASSERT(m_PinDataRangePointers);

				ExFreePool(m_PinDataRangePointers);

				m_PinDataRangePointers = NULL;
			}

			ULONG Offset = m_PinDataRangesCount;

			if (FormatTypeDescriptor->bSamFreqType == 0)
			{
				// Continuous sample rate.
				PinDataRanges[Offset].Ex.DataRange.FormatSize = sizeof(KSDATARANGE_AUDIO);
				PinDataRanges[Offset].Ex.DataRange.Flags = 0;
				PinDataRanges[Offset].Ex.DataRange.SampleSize = 0;
				PinDataRanges[Offset].Ex.DataRange.Reserved = 0;
				PinDataRanges[Offset].Ex.DataRange.MajorFormat = MajorFormat;
				PinDataRanges[Offset].Ex.DataRange.SubFormat = SubFormat;
				PinDataRanges[Offset].Ex.DataRange.Specifier = Specifier;
				PinDataRanges[Offset].Ex.MaximumChannels = 6; // FIXME: is this correct ?
				PinDataRanges[Offset].Ex.MinimumBitsPerSample = 0;
				PinDataRanges[Offset].Ex.MaximumBitsPerSample = 0;
				PinDataRanges[Offset].Ex.MinimumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[0]); // lower bound
				PinDataRanges[Offset].Ex.MaximumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[1]); // upper bound
				PinDataRanges[Offset].InterfaceNumber = InterfaceNumber;
				PinDataRanges[Offset].AlternateSetting = AlternateSetting;
				PinDataRanges[Offset].BitResolution = 0;
			}
			else
			{
				// Discrete sampling rates.
				for (ULONG i=Offset; i<PinDataRangesCount; i++)
				{
					PinDataRanges[i].Ex.DataRange.FormatSize = sizeof(KSDATARANGE_AUDIO);
					PinDataRanges[i].Ex.DataRange.Flags = 0;
					PinDataRanges[i].Ex.DataRange.SampleSize = 0;
					PinDataRanges[i].Ex.DataRange.Reserved = 0;
					PinDataRanges[i].Ex.DataRange.MajorFormat = MajorFormat;
					PinDataRanges[i].Ex.DataRange.SubFormat = SubFormat;
					PinDataRanges[i].Ex.DataRange.Specifier = Specifier;
					PinDataRanges[i].Ex.MaximumChannels = 6; // FIXME: is this correct ?
					PinDataRanges[i].Ex.MinimumBitsPerSample = 0;
					PinDataRanges[i].Ex.MaximumBitsPerSample = 0;
					PinDataRanges[i].Ex.MinimumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[i-Offset]);
					PinDataRanges[i].Ex.MaximumSampleFrequency = SAMFREQ(FormatTypeDescriptor->tSamFreq[i-Offset]);
					PinDataRanges[i].InterfaceNumber = InterfaceNumber;
					PinDataRanges[i].AlternateSetting = AlternateSetting;
					PinDataRanges[i].BitResolution = 0;
				}
			}

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

	PKSDATARANGE_AUDIO_EX PinDataRanges = (PKSDATARANGE_AUDIO_EX)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(KSDATARANGE_AUDIO_EX), 'mdW');

	PKSDATARANGE * PinDataRangePointers = (PKSDATARANGE *)ExAllocatePoolWithTag(NonPagedPool, PinDataRangesCount * sizeof(PKSDATARANGE), 'mdW');

	if (PinDataRanges && PinDataRangePointers)
	{
		if (m_PinDataRangesCount)
		{
			// There was a previously allocated data ranges. Append it to the new data ranges.
			ASSERT(m_PinDataRanges);

			RtlCopyMemory(PinDataRanges, m_PinDataRanges, m_PinDataRangesCount * sizeof(KSDATARANGE_AUDIO_EX));
			
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

	if (m_TerminalType == USB_TERMINAL_USB_STREAMING)
	{
		OutDescriptor->Dispatch = &CAudioPin::DispatchTable;
		OutDescriptor->Flags = KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING | 
							   KSPIN_FLAG_INITIATE_PROCESSING_ON_EVERY_ARRIVAL |
							   KSPIN_FLAG_HYPERCRITICAL_PROCESSING |
							   KSPIN_FLAG_ASYNCHRONOUS_PROCESSING;
		//if (!m_IsSource) OutDescriptor->Flags |= KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY;
		OutDescriptor->InstancesPossible = 1;
		OutDescriptor->InstancesNecessary = 0;
		OutDescriptor->AllocatorFraming = &m_PinAllocatorFraming;
		OutDescriptor->IntersectHandler = (PFNKSINTERSECTHANDLEREX)CAudioPin::IntersectHandler;
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

	if (m_TerminalType == USB_TERMINAL_USB_STREAMING)
	{
		OutDescriptor->AutomationTable = &CAudioPin::AutomationTable;
	}
	else
	{
		OutDescriptor->AutomationTable = NULL;
	}

	OutDescriptor->PinDescriptor.InterfacesCount = SIZEOF_ARRAY(CAudioPin::Interfaces);
	OutDescriptor->PinDescriptor.Interfaces = CAudioPin::Interfaces;
	OutDescriptor->PinDescriptor.MediumsCount = 0;
	OutDescriptor->PinDescriptor.Mediums = NULL;
	OutDescriptor->PinDescriptor.DataRangesCount = m_PinDataRangesCount;
	OutDescriptor->PinDescriptor.DataRanges = m_PinDataRangePointers;

	if (m_TerminalType == USB_TERMINAL_USB_STREAMING)
	{
		OutDescriptor->PinDescriptor.Communication = m_IsSource ? KSPIN_COMMUNICATION_SINK : KSPIN_COMMUNICATION_BOTH;
	}
	else
	{
		OutDescriptor->PinDescriptor.Communication = KSPIN_COMMUNICATION_BRIDGE;
	}

	OutDescriptor->PinDescriptor.DataFlow = m_IsSource ? KSPIN_DATAFLOW_IN : KSPIN_DATAFLOW_OUT;
	OutDescriptor->PinDescriptor.Category = &m_Category;
	OutDescriptor->PinDescriptor.Name = &m_Name;
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
 * CFilterPinDescriptor::IsDigital()
 *****************************************************************************
 *//*!
 * @brief
 * Determine if the filter pin is a digital or analog pin.
 * @param
 * None
 * @return
 * Returns TRUE if the filter pin is a digital pin, otherwise FALSE.
 */
BOOL
CFilterPinDescriptor::
IsDigital
(   void
)
{
    return m_IsDigital;
}

/*****************************************************************************
 * CFilterPinDescriptor::NumberOfChannels()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the number channels that the filter pin supports.
 * @param
 * None
 * @return
 * Returns the number of channels that the filter pin supports.
 */
LONG
CFilterPinDescriptor::
NumberOfChannels
(   void
)
{
    return m_NumberOfChannels;
}

/*****************************************************************************
 * CFilterPinDescriptor::ChannelConfig()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the channel configuration that the filter pin supports.
 * @param
 * None
 * @return
 * Returns the channel configuration that the filter pin supports.
 */
ULONG
CFilterPinDescriptor::
ChannelConfig
(	void
)
{
	ULONG ChannelConfig = (1<<m_NumberOfChannels)-1;

	USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR ClusterDescriptor;

	if (m_Terminal->FindAudioChannelCluster(&ClusterDescriptor))
	{
		ChannelConfig = ClusterDescriptor.wChannelConfig;
	}

    return ChannelConfig;
}

/*****************************************************************************
 * CFilterPinDescriptor::Terminal()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the terminal.
 * @param
 * None
 * @return
 * Returns the terminal.
 */
PTERMINAL
CFilterPinDescriptor::
Terminal
(   void
)
{
    return m_Terminal;
}

/*****************************************************************************
 * CFilterPinDescriptor::TerminalID()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the terminal identifier.
 * @param
 * None
 * @return
 * Returns the identifier of the terminal.
 */
UCHAR
CFilterPinDescriptor::
TerminalID
(   void
)
{
    return m_TerminalID;
}

/*****************************************************************************
 * CFilterPinDescriptor::TerminalType()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the pin/terminal type.
 * @param
 * None
 * @return
 * Returns the the pin/terminal type..
 */
USHORT
CFilterPinDescriptor::
TerminalType
(   void
)
{
    return m_TerminalType;
}

/*****************************************************************************
 * CFilterPinDescriptor::iTerminal()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the terminal string index.
 * @param
 * None
 * @return
 * Returns the string index of the terminal.
 */
UCHAR
CFilterPinDescriptor::
iTerminal
(   void
)
{
    return m_iTerminal;
}

/*****************************************************************************
 * CFilterPinDescriptor::FormatSpecifier()
 *****************************************************************************
 *//*!
 * @brief
 * @param
 * None
 * @return
 */
ULONG
CFilterPinDescriptor::
FormatSpecifier
(   void
)
{
    return m_FormatSpecifier;
}

/*****************************************************************************
 * CFilterPinDescriptor::WriteParameter()
 *****************************************************************************
 *//*!
 * @brief
 * Writes a parameter value.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CFilterPinDescriptor::
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

	if (m_Terminal)
	{
		ntStatus = m_Terminal->WriteParameterBlock
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
 * CFilterPinDescriptor::ReadParameter()
 *****************************************************************************
 *//*!
 * @brief
 * Reads a parameter value.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CFilterPinDescriptor::
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

	if (m_Terminal)
	{
		ntStatus = m_Terminal->ReadParameterBlock
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
 * Terminal An input/output terminal.
 * @return
 * Returns the pointer of the filter pin descriptor.
 */
CFilterPinDescriptor *
CFilterDescriptor::
AddPin
(
	IN      PTERMINAL	Terminal,
	IN		ULONG		FormatSpecifier
)
{
    PAGED_CODE();

    CFilterPinDescriptor * Pin = new(NonPagedPool) CFilterPinDescriptor();

    if (Pin)
    {
        Pin->Init(this, Terminal, PinCount(), FormatSpecifier);

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
	IN		PTERMINAL	Terminal,
	IN		PUNIT		Unit,
    IN      UCHAR		ControlSelector,
	IN		ULONG		MaximumNumberOfInputPins,
	IN		ULONG		MaximumNumberOfOutputPins
)
{
    PAGED_CODE();

    CNodeDescriptor * Node = new(NonPagedPool) CNodeDescriptor();

    if (Node)
    {
        Node->Init(m_NodeList.Count(), Terminal, Unit, ControlSelector, MaximumNumberOfInputPins, MaximumNumberOfOutputPins);

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
		if ((Node->UnitID() == EntityID) && (Node->IsConnectionPossible(DataFlow)))
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
			if (FilterPin->TerminalID() == EntityID)
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
			if (FilterPin->TerminalID() == EntityID)
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
 * CAudioFilterDescriptor::CAudioFilterDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CAudioFilterDescriptor::
CAudioFilterDescriptor
(   void
)
:   CFilterDescriptor()
{
    PAGED_CODE();
}

/*****************************************************************************
 * CAudioFilterDescriptor::~CAudioFilterDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CAudioFilterDescriptor::
~CAudioFilterDescriptor
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterDescriptor::~CAudioFilterDescriptor]"));

	if (m_AudioDevice)
	{
		//m_AudioDevice->SetInterruptHandler(NULL, NULL);

		m_AudioDevice->Release();
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
 * CAudioFilterDescriptor::Initialize()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the audio filter descriptor.
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
CAudioFilterDescriptor::
Initialize
(
    IN      PAUDIO_DEVICE	AudioDevice
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterDescriptor::Initialize]"));

	CFilterPinDescriptor * pin;

	m_AudioDevice = AudioDevice;
	m_AudioDevice->AddRef();

	m_AudioDevice->ParseTopology(0, &m_AudioTopology);

    _ParseAndBuildPins();

    _ParseAndBuildNodes();

	if (PinCount())
	{
	    m_KsPins = (KSPIN_DESCRIPTOR_EX *)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSPIN_DESCRIPTOR_EX) * PinCount(), 'mdW');
	}

	if (NodeCount())
	{
		m_KsNodes = (KSNODE_DESCRIPTOR *)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSNODE_DESCRIPTOR) * (NodeCount() + m_AudioDevice->m_NoOfSoftNode), 'mdW');
    }

	if (ConnectionCount())
	{
		m_KsTopologyConnections = (KSTOPOLOGY_CONNECTION *)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSTOPOLOGY_CONNECTION) * ConnectionCount(), 'mdW');
	}

    // Fill in the pins...
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
		
		//edit yuanfen 
#if 0
		//Hide hardware volume icon from Vista Control Panel 
		PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor = NULL;

		m_AudioDevice->m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

		if (UsbDeviceDescriptor)
		{
			if ((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) &&
				((UsbDeviceDescriptor->idProduct == 0x3F02/*MicroPod*/) || (UsbDeviceDescriptor->idProduct == 0x3F04/*HulaPod*/)))
			{
				if(m_KsNodes[node->NodeId()].Type == &KSNODETYPE_MUTE || m_KsNodes[node->NodeId()].Type == &KSNODETYPE_VOLUME)
				{
					m_KsNodes[node->NodeId()].AutomationTable = NULL;
				}

			}
			
		}
		//end edit
#endif
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

    if(m_AudioDevice->IsSoftwareMasterVolMute())
        AddMasterVolMuteNode();
	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor; m_AudioDevice->GetDeviceDescriptor(&DeviceDescriptor);

	//FIXME: Write names to MediaCategories...
	INIT_USBAUDIO_MID(&m_KsComponentId.Manufacturer, DeviceDescriptor->idVendor);
	INIT_USBAUDIO_PID(&m_KsComponentId.Product, DeviceDescriptor->idProduct);
	INIT_USBAUDIO_PRODUCT_NAME(&m_KsComponentId.Name, DeviceDescriptor->idVendor, DeviceDescriptor->idProduct, DeviceDescriptor->iProduct);

	m_KsComponentId.Component = KSCOMPONENTID_USBAUDIO_10;
	m_KsComponentId.Version   = 1;
	m_KsComponentId.Revision  = 0;

	return STATUS_SUCCESS;
}

void
CAudioFilterDescriptor::
AddMasterVolMuteNode(void)
{
    ULONG   NoOfSoftNode = m_AudioDevice->m_NoOfSoftNode;
	
    // add Software node
	{
		//edit yuanfen
		//Add the SoftNode to NodeList
		for (ULONG i=0;i< NoOfSoftNode;i++)
		{
			CNodeDescriptor * Node = AddNode(NULL,NULL,0,-1,-1);
		}
		
        // 1st node : volume node
        m_KsNodes[NodeCount()-NoOfSoftNode].AutomationTable     = &SwVolumeLevelAutomationTable;
        m_KsNodes[NodeCount()-NoOfSoftNode].Name                = NULL;
        m_KsNodes[NodeCount()-NoOfSoftNode].Type                = &KSNODETYPE_VOLUME;

        // 2nd node : mute node
        m_KsNodes[NodeCount()-NoOfSoftNode+1].AutomationTable   = &SwMuteAutomationTable;
        m_KsNodes[NodeCount()-NoOfSoftNode+1].Name              = NULL;
        m_KsNodes[NodeCount()-NoOfSoftNode+1].Type              = &KSNODETYPE_MUTE;

    }

//    for (CNodeDescriptor * node = m_NodeList.First(); node != NULL; node = m_NodeList.Next(node))
//    {
//		KSNODE_DESCRIPTOR KsNodeDescriptor;
		
//        node->GetDescriptor(&KsNodeDescriptor);

//        if (IsEqualGUIDAligned(*(KsNodeDescriptor.Type), KSNODETYPE_SUM))
//		{
			//edit yuanfen
			// Got the SUM node that we are looking for
			CNodeDescriptor * node;
			node = FindSumNodeatPlaypath();
			//end edit

            ULONG i = 0;
            for (CWireDescriptor * wire = m_WireList.First(); wire != NULL; wire = m_WireList.Next(wire))
            {
                if(node->NodeId() == wire->FromPin()->Node()->NodeId())
                {
                    //this is mixer node wireList

                    // cached this ToNode for mute node to connect to
                    ULONG ToNode = m_KsTopologyConnections[i].ToNode;

                    //connect this mixer node to vol node
                    m_KsTopologyConnections[i].ToNode       = NodeCount()-NoOfSoftNode; //vol node id
//                    DbgPrint("connect this mixer node to FU node=%x NodeId=%x\n",node->m_NodeId,NodeCount()-NoOfSoftNode);

                    //connect vol node to mute node
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].FromNode = NodeCount()-NoOfSoftNode; //vol node id
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].FromNodePin = 0;
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].ToNode = NodeCount()-NoOfSoftNode+1; //mute node id
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].ToNodePin = 1;
//                    DbgPrint("%x 2nd Last added fromNode=%x FromPin=%x ToNode=%x ToPin=%x\n", (ConnectionCount()-NoOfSoftNode),m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].FromNode,m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].FromNodePin,m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].ToNode,m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode].ToNodePin);

                    //connect mute node to original node that is connected from mixer node
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].FromNode = NodeCount()-NoOfSoftNode+1; //mute node id
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].FromNodePin = 0;
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].ToNode = ToNode;
                    m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].ToNodePin = 1;
//                    DbgPrint("%x Last added fromNode=%x FromPin=%x ToNode=%x ToPin=%x\n", (ConnectionCount()-NoOfSoftNode+1),m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].FromNode,m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].FromNodePin,m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].ToNode,m_KsTopologyConnections[ConnectionCount()-NoOfSoftNode+1].ToNodePin);

                    break;
                }                    
                i++;
            }
//edit yuanfen
//			break; 
//		}
//    }
}

/*****************************************************************************
 * CAudioFilterDescriptor::FindEntity()
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
CAudioFilterDescriptor::
FindEntity
(
	IN		UCHAR		EntityID,
	OUT		PENTITY *	OutEntity
)
{
	PAGED_CODE();

	return m_AudioTopology->FindEntity(EntityID, OutEntity);
}

#pragma code_seg()

/*****************************************************************************
 * CAudioFilterDescriptor::PinCount()
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
CAudioFilterDescriptor::
PinCount
(   void
)
{
    return CFilterDescriptor::PinCount();
}

/*****************************************************************************
 * CAudioFilterDescriptor::Node()
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
CAudioFilterDescriptor::
NodeCount
(   void
)
{
//    return CFilterDescriptor::NodeCount()+ m_AudioDevice->m_NoOfSoftNode;
	//edit yuanfen
	//SoftNode was added to Nodelist
	  return CFilterDescriptor::NodeCount();
}

/*****************************************************************************
 * CAudioFilterDescriptor::ConnectionCount()
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
CAudioFilterDescriptor::
ConnectionCount
(   void
)
{
    return CFilterDescriptor::ConnectionCount()+ m_AudioDevice->m_NoOfSoftNode;
}

/*****************************************************************************
 * CAudioFilterDescriptor::CategoryCount()
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
CAudioFilterDescriptor::
CategoryCount
(   void
)
{
    return SIZEOF_ARRAY(CAudioFilter::Categories);
}

/*****************************************************************************
 * CAudioFilterDescriptor::Pins()
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
CAudioFilterDescriptor::
Pins
(   void
)
{
    return m_KsPins;
}

/*****************************************************************************
 * CAudioFilterDescriptor::Nodes()
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
CAudioFilterDescriptor::
Nodes
(   void
)
{
    return m_KsNodes;
}

/*****************************************************************************
 * CAudioFilterDescriptor::Connections()
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
CAudioFilterDescriptor::
Connections
(   void
)
{
    return m_KsTopologyConnections;
}

/*****************************************************************************
 * CAudioFilterDescriptor::Categories()
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
CAudioFilterDescriptor::
Categories
(   void
)
{
    return CAudioFilter::Categories;
}

/*****************************************************************************
 * CAudioFilterDescriptor::ComponentId()
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
CAudioFilterDescriptor::
ComponentId
(   void
)
{
    return &m_KsComponentId;
}

/*****************************************************************************
 * CAudioFilterDescriptor::FindFilterPin()
 *****************************************************************************
 *//*!
 * @brief
 */
CFilterPinDescriptor *
CAudioFilterDescriptor::
FindFilterPin
(
	IN		ULONG	PinId
)
{
	CFilterPinDescriptor * pin;

    // Find the appropriate pin
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
 * CAudioFilterDescriptor::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
CNodeDescriptor *
CAudioFilterDescriptor::
FindNode
(
	IN		ULONG	NodeId
)
{
	CNodeDescriptor * node;

    // Find the appropriate node
    for (node = m_NodeList.First(); node != NULL; node = m_NodeList.Next(node))
    {
        if (node->NodeId() == NodeId) break;
    }

	return node;
}

//edit yuanfen
/*****************************************************************************
 * CAudioFilterDescriptor::FindSumNodeatPlaypath()
 *****************************************************************************
 *//*!
 * @brief
 */
CNodeDescriptor * 
CAudioFilterDescriptor::
FindSumNodeatPlaypath
( 
	 void 
)
{
	CNodeDescriptor * SumNode;
	for (CFilterPinDescriptor * OutputPin = (CFilterPinDescriptor *)m_OutputPinList.First(); OutputPin != NULL; OutputPin = (CFilterPinDescriptor *)m_OutputPinList.Next(OutputPin))
	{
		KSPIN_DESCRIPTOR_EX KsPinDescriptor;

		OutputPin->GetDescriptor(&KsPinDescriptor);


		if (IsEqualGUIDAligned(*KsPinDescriptor.PinDescriptor.Category, KSNODETYPE_SPEAKER) ||
			IsEqualGUIDAligned(*KsPinDescriptor.PinDescriptor.Category, KSNODETYPE_DIGITAL_AUDIO_INTERFACE))
		{
			// This is a analog output interface pin.
			ULONG PinId = OutputPin->PinId();
			ULONG NodeId = KSFILTER_NODE;

			// Walk the wire connections to find the first sum node from the digital output 
			// interface pin, if there is one.
			KSTOPOLOGY_CONNECTION Connection;

			while (_FindTopologyConnectionTo(NodeId, PinId, &Connection))
			{
				CNodeDescriptor * Node = FindNode(Connection.FromNode);

				if (Node)
				{
					KSNODE_DESCRIPTOR KsNodeDescriptor;

					Node->GetDescriptor(&KsNodeDescriptor);

					if (IsEqualGUIDAligned(*KsNodeDescriptor.Type, KSNODETYPE_SUM))
					{
						// Got the SUM node that we are looking for... phew!!!
						SumNode = Node;
						break;
					}
				}

				NodeId = Connection.FromNode;
				PinId = 1; // first input pin.
			}
		}

		if (SumNode) break;
	}

	return SumNode;
}
//end edit

/*****************************************************************************
 * CAudioFilterDescriptor::OnEvent()
 *****************************************************************************
 *//*!
 * @brief
 * Audio events handler.
 * @param
 * Event Audio event.
 * @param
 * Argument Argument for the audio event.
 * @param
 * Context Context information passed to the Advise function.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
STDMETHODIMP
CAudioFilterDescriptor::
OnEvent
(
    IN      ULONG		Event,
    IN      ULONG       Argument,
    IN      PVOID       Context
)
{

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioFilterDescriptor::_FindFormatSpecifier()
 *****************************************************************************
 *//*!
 * @brief
 * Finds the format types supported by a terminal.
 */
ULONG
CAudioFilterDescriptor::
_FindFormatSpecifier
(
	IN		PTERMINAL	Terminal	
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::_FindFormatSpecifier]"));

	ULONG FormatSpecifier = 0;

	if (Terminal->TerminalType() == USB_TERMINAL_USB_STREAMING)
	{
		PAUDIO_INTERFACE Interface = NULL;
		
		for (UCHAR i=0; ; i++)
		{
			if (m_AudioDevice->ParseInterfaces(i, &Interface))
			{
				if (Interface)
				{
					for (UCHAR AlternateSetting=1; ;AlternateSetting++)
					{
						PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor = Interface->GetFormatTypeDescriptor(AlternateSetting);

						if (FormatTypeDescriptor)
						{
							if (Interface->TerminalLink(AlternateSetting) == Terminal->TerminalID())
							{
								switch (FormatTypeDescriptor->bFormatType)
								{
									case USB_AUDIO_FORMAT_TYPE_I:
									{
										FormatSpecifier |= FORMAT_SPECIFIER_TYPE_I;
									}
									break;

									case USB_AUDIO_FORMAT_TYPE_II:
									{
										FormatSpecifier |= FORMAT_SPECIFIER_TYPE_II;
									}
									break;

									case USB_AUDIO_FORMAT_TYPE_III:
									{
										FormatSpecifier |= FORMAT_SPECIFIER_TYPE_III;
									}
									break;
								}
							}
						}
						else
						{
							// No more formats...
							break;
						}
					}
				}
			}
			else
			{
				break;
			}
		}

		_DbgPrintF(DEBUGLVL_VERBOSE,("[CFilterPinDescriptor::_FindFormatSpecifier] - TerminalLink : 0x%02x, FormatSpecifier: 0x%x", Terminal->TerminalID(), FormatSpecifier));
	}

	return FormatSpecifier;
}

/*****************************************************************************
 * CAudioFilterDescriptor::_FindTopologyConnectionTo()
 *****************************************************************************
 *//*!
 */
BOOL
CAudioFilterDescriptor::
_FindTopologyConnectionTo
(
	IN		ULONG					ToNode,
	IN		ULONG					ToNodePin,
	OUT		KSTOPOLOGY_CONNECTION *	OutConnection
)
{
	BOOL Found = FALSE;

	// Walk the wire connections to find the connection to the specified node & pin.
	for (CWireDescriptor * wire = m_WireList.First(); wire != NULL; wire = m_WireList.Next(wire))
	{
		KSTOPOLOGY_CONNECTION Connection; 

		Connection.FromNode     = wire->FromPin()->Node()->NodeId();
		Connection.FromNodePin  = wire->FromPin()->PinId();
		Connection.ToNode       = wire->ToPin()->Node()->NodeId();
		Connection.ToNodePin    = wire->ToPin()->PinId();

		if ((Connection.ToNode == ToNode) && (Connection.ToNodePin == ToNodePin))
		{
			// Got a match.
			*OutConnection = Connection;

			Found = TRUE;

			break;
		}
	}					

	return Found;
}

/*****************************************************************************
 * CAudioFilterDescriptor::_ParseAndBuildPins()
 *****************************************************************************
 *//*!
 * @brief
 * Discovers the pins that is exposed by the audio topology.
 * @details
 * Enumerate thru the audio topology, and group these terminals into pin 
 * descriptors.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioFilterDescriptor::
_ParseAndBuildPins
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterDescriptor::_ParseAndBuildPins]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	if (m_AudioTopology)
	{
		for (ULONG i=0; ; i++)
		{
			PTERMINAL Terminal = NULL;

			if (m_AudioTopology->ParseTerminals(i, &Terminal))
			{
		        if (Terminal->TerminalType() == USB_TERMINAL_USB_STREAMING)
				{
					ULONG FormatSpecifier = _FindFormatSpecifier(Terminal);

					if (Terminal->DescriptorSubtype() == USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL)
					{
						// If there is a terminal with both PCM and AC3 capabilities, then split them into 2 separate pins.
						if ((FormatSpecifier & FORMAT_SPECIFIER_TYPE_I) && (FormatSpecifier & (FORMAT_SPECIFIER_TYPE_II | FORMAT_SPECIFIER_TYPE_III)))
						{
							AddPin(Terminal, FORMAT_SPECIFIER_TYPE_I);
							AddPin(Terminal, (FormatSpecifier & (FORMAT_SPECIFIER_TYPE_II | FORMAT_SPECIFIER_TYPE_III)) | 0x80000000/*Funky pin.*/);
						}
						else
						{
							AddPin(Terminal, FormatSpecifier);
						}
					}
					else
					{
						AddPin(Terminal, FormatSpecifier);
					}				
				}
				else
				{
					AddPin(Terminal, 0);
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
        if (pin->TerminalType() == USB_TERMINAL_USB_STREAMING)
        {
			PAUDIO_INTERFACE Interface = NULL;
			
			for (UCHAR i=0; ; i++)
			{
				if (m_AudioDevice->ParseInterfaces(i, &Interface))
				{
					pin->BuildDataRanges(Interface);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			pin->BuildDataRanges(NULL);
		}
    }

    for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
        if (pin->TerminalType() == USB_TERMINAL_USB_STREAMING)
        {
			PAUDIO_INTERFACE Interface = NULL;
			
			for (UCHAR i=0; ; i++)
			{
				if (m_AudioDevice->ParseInterfaces(i, &Interface))
				{
					pin->BuildDataRanges(Interface);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			pin->BuildDataRanges(NULL);
		}
    }

    // Go thru the pins and rearrange them in order.
    ULONG i = 0;

	// record pins first...
    for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
        if (pin->TerminalType() == USB_TERMINAL_USB_STREAMING)
        {
            pin->PinId(i);
            i++;
        }
    }

	// wave out...
    for (pin = (CFilterPinDescriptor *)m_InputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_InputPinList.Next(pin))
    {
        if (pin->TerminalType() == USB_TERMINAL_USB_STREAMING)
        {
            pin->PinId(i);
            i++;
        }
    }

	// other source pins...
    for (pin = (CFilterPinDescriptor *)m_InputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_InputPinList.Next(pin))
    {
        if (pin->TerminalType() != USB_TERMINAL_USB_STREAMING)
		{
			pin->PinId(i);
			i++;
		}
    }

	// destination pins...
    for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
        if (pin->TerminalType() != USB_TERMINAL_USB_STREAMING)
        {
            pin->PinId(i);
            i++;
        }
    }

	for (pin = (CFilterPinDescriptor *)m_OutputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_OutputPinList.Next(pin))
    {
		if (pin->TerminalType() == USB_TERMINAL_USB_STREAMING)
		{
			// Add a SRC node. Whether it is supported will be determined in the 
			// propertyset handler.
			CNodeDescriptor * FromNode = AddNode(pin->Terminal(), NULL, (pin->FormatSpecifier() & (FORMAT_SPECIFIER_TYPE_II | FORMAT_SPECIFIER_TYPE_III)) ? 3/*KSNODETYPE_SPDIF_INTERFACE*/: 2/*KSNODETYPE_SRC*/, -1, 1);
			CPinDescriptor * FromPin = FromNode->AddPin(PIN_DATAFLOW_OUT);
			Connect(FromPin, pin);
			pin->EnableConnection(FALSE);
		}
		else
		{
			// Add DAC nodes to the analog pins.
			//if (!pin->IsDigital())
			{
				CNodeDescriptor * FromNode = AddNode(pin->Terminal(), NULL, 0/*KSNODETYPE_DAC*/, -1, 1);
				CPinDescriptor * FromPin = FromNode->AddPin(PIN_DATAFLOW_OUT);
				Connect(FromPin, pin);
				pin->EnableConnection(FALSE);
			}
		}
    }

    for (pin = (CFilterPinDescriptor *)m_InputPinList.First(); pin != NULL; pin = (CFilterPinDescriptor *)m_InputPinList.Next(pin))
    {
		if (pin->TerminalType() == USB_TERMINAL_USB_STREAMING)
		{
			// Add a SRC node. Whether it is supported will be determined in the 
			// propertyset handler.
			CNodeDescriptor * ToNode = AddNode(pin->Terminal(), NULL, (pin->FormatSpecifier() & (FORMAT_SPECIFIER_TYPE_II | FORMAT_SPECIFIER_TYPE_III)) ? 3/*KSNODETYPE_SPDIF_INTERFACE*/: 2/*KSNODETYPE_SRC*/, 1, -1);
			CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);
			Connect(pin, ToPin);
			pin->EnableConnection(FALSE);
		}
		else
		{
			// Add ADC nodes to the analog pins.
			//if (!pin->IsDigital())
			{
				CNodeDescriptor * ToNode = AddNode(pin->Terminal(), NULL, 1/*KSNODETYPE_ADC*/, 1, -1);
				CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);
				Connect(pin, ToPin);
				pin->EnableConnection(FALSE);
			}
		}
    }

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilterDescriptor::_ParseAndBuildNodes()
 *****************************************************************************
 *//*!
 * @brief
 * Discovers the nodes that are in the audio filter.
 * @details
 * Enumerate the audio topology units, and find all the nodes that connected
 * to the pins. Also the connections between the nodes & pins are established
 * using wire descriptors.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioFilterDescriptor::
_ParseAndBuildNodes
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterDescriptor::_ParseAndBuildNodes]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	if (m_AudioTopology)
	{
		for (UCHAR i=0; ; i++)
		{
			PUNIT Unit = NULL;

			if (m_AudioTopology->ParseUnits(i, &Unit))
			{
				UCHAR DescriptorSubtype = Unit->DescriptorSubtype();

				if (DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT)
				{
					PFEATURE_UNIT FeatureUnit = PFEATURE_UNIT(Unit);

					CNodeDescriptor * FromNode = NULL;
					CPinDescriptor * FromPin = NULL; 
					CNodeDescriptor * ToNode = NULL;
					CPinDescriptor * ToPin = NULL;

					UCHAR NumControls = 0;

					for (UCHAR j=0; ; j++)
					{
						UCHAR ControlSelector = USB_AUDIO_FU_CONTROL_UNDEFINED;

						if (FeatureUnit->ParseControls(j, &ControlSelector))
						{
							if (ControlSelector)
							{
								// Control is defined.
								NumControls++;
							}
						}
						else
						{
							// No more controls
							break;
						}
					}

					for (UCHAR j=0, k=0; k<NumControls; j++)
					{
						UCHAR ControlSelector = USB_AUDIO_FU_CONTROL_UNDEFINED;

						if (FeatureUnit->ParseControls(j, &ControlSelector))
						{
							if (ControlSelector)
							{
								k++;

								// Control is defined.
								ToNode = AddNode(NULL, FeatureUnit, ControlSelector, 1,  (k < NumControls) ? 1 : -1);

								// Connect all the nodes within this unit together.
								if (FromNode)
								{
									FromPin = FromNode->AddPin(PIN_DATAFLOW_OUT);
								}

								if (FromPin)
								{
									ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);

									Connect(FromPin, ToPin);
								}

								FromNode = ToNode;
							}
						}
						else
						{
							// No more controls
							break;
						}
					}
				}
				else if (DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT)
				{
					// A mixer unit is represented as the combination of SUPERMIX nodes
					// and a SUM. The number of SUPERMIX nodes is determined by the 
					// number of input plugs on the unit.
					ULONG NumberOfSources = 0;

					for (UCHAR j=0; ; j++)
					{
						UCHAR SourceID = 0;

						if (Unit->ParseSources(j, &SourceID))
						{
							NumberOfSources++;
						}
						else
						{
							break;
						}
					}

					if (NumberOfSources)
					{
						// Add sum node.
						CNodeDescriptor * ToNode = AddNode(NULL, Unit, 0, NumberOfSources, -1);

						// Add supermix nodes.
						for (UCHAR j=0; j<NumberOfSources; j++)
						{
							// The control selector in the mixer unit is re-used as the index to the input plug.
							CNodeDescriptor * FromNode = AddNode(NULL, Unit, j+1, 1, 1);
							CPinDescriptor * FromPin = FromNode->AddPin(PIN_DATAFLOW_OUT);

							CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);

							// Connect all the nodes together.
							Connect(FromPin, ToPin);
						}
					}
				}
				else
				{
					AddNode(NULL, Unit, 0, -1, -1);
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
		PTERMINAL Terminal = FilterPin->Terminal();

		CPinDescriptor * ToPin;

		CWireDescriptor * Wire = FilterPin->Wire();

		if (Wire)
		{
			ToPin = Wire->FromPin()->Node()->AddPin(PIN_DATAFLOW_IN);
		}
		else
		{
			ToPin = FilterPin;
		}

		for (UCHAR i=0; ; i++)
		{
			UCHAR SourceID = 0;

			if (Terminal->ParseSources(i, &SourceID))
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
		PUNIT Unit = ToNode->Unit();

		if (Unit)
		{
			UCHAR DescriptorSubtype = Unit->DescriptorSubtype();

			if (DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT)
			{
				if (ToNode->IsConnectionPossible(PIN_DATAFLOW_IN))
				{
					// The control selector in the mixer unit is re-used as the index to the input plug.
					UCHAR SourceID = 0;

					if (Unit->ParseSources(ToNode->ControlSelector()-1, &SourceID))
					{
						CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);

						CPinDescriptor * FromPin = AddPinToEntity(SourceID, PIN_DATAFLOW_OUT);

						if (FromPin)
						{
							Connect(FromPin, ToPin);
						}
					}
				}
			}
			else
			{
				if (ToNode->IsConnectionPossible(PIN_DATAFLOW_IN))
				{
					for (UCHAR i=0; ; i++)
					{
						UCHAR SourceID = 0;

						if (Unit->ParseSources(i, &SourceID))
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
    }

	// Establish connection between the "funky" pins and the sum node. This is used for the virtual AC3 pin to connect 
	// to the first sum node from the digital output pin.
    for (CFilterPinDescriptor * InputPin = (CFilterPinDescriptor *)m_InputPinList.First(); InputPin != NULL; InputPin = (CFilterPinDescriptor *)m_InputPinList.Next(InputPin))
    {
		if (InputPin->TerminalType() == USB_TERMINAL_USB_STREAMING)
		{
			if (InputPin->FormatSpecifier() & 0x80000000) // Got a funky pin ?
			{		
				if (InputPin->FormatSpecifier() & (FORMAT_SPECIFIER_TYPE_II | FORMAT_SPECIFIER_TYPE_III))
				{
					CWireDescriptor * Wire = InputPin->Wire();

					if (Wire)
					{
						CNodeDescriptor * FromNode = Wire->ToPin()->Node();

						if (FromNode)
						{
							CNodeDescriptor * ToNode = NULL;

							// Find the digital output interface pin.
							for (CFilterPinDescriptor * OutputPin = (CFilterPinDescriptor *)m_OutputPinList.First(); OutputPin != NULL; OutputPin = (CFilterPinDescriptor *)m_OutputPinList.Next(OutputPin))
							{
								KSPIN_DESCRIPTOR_EX KsPinDescriptor;

								OutputPin->GetDescriptor(&KsPinDescriptor);

								//edit yuanfen
//								if (IsEqualGUIDAligned(*KsPinDescriptor.PinDescriptor.Category, KSNODETYPE_DIGITAL_AUDIO_INTERFACE))
								if (IsEqualGUIDAligned(*KsPinDescriptor.PinDescriptor.Category, KSNODETYPE_DIGITAL_AUDIO_INTERFACE) ||
									IsEqualGUIDAligned(*KsPinDescriptor.PinDescriptor.Category, KSNODETYPE_SPEAKER))
								{
									// This is a digital output interface pin.
									ULONG PinId = OutputPin->PinId();
									ULONG NodeId = KSFILTER_NODE;

									// Walk the wire connections to find the first sum node from the digital output 
									// interface pin, if there is one.
									KSTOPOLOGY_CONNECTION Connection;

									while (_FindTopologyConnectionTo(NodeId, PinId, &Connection))
									{
										CNodeDescriptor * Node = FindNode(Connection.FromNode);

										if (Node)
										{
											KSNODE_DESCRIPTOR KsNodeDescriptor;

											Node->GetDescriptor(&KsNodeDescriptor);

											if (IsEqualGUIDAligned(*KsNodeDescriptor.Type, KSNODETYPE_SUM))
											{
												// Got the SUM node that we are looking for... phew!!!
												ToNode = Node;
												break;
											}
										}

										NodeId = Connection.FromNode;
										PinId = 1; // first input pin.
									}
								}

								if (ToNode) break;
							}

							if (ToNode)
							{
								// Hook up the nodes together.
								CPinDescriptor * ToPin = ToNode->AddPin(PIN_DATAFLOW_IN);

								CPinDescriptor * FromPin = FromNode->AddPin(PIN_DATAFLOW_OUT);

								if (FromPin && ToPin)
								{
									Connect(FromPin, ToPin);
								}
							}
						}
					}
				}
			}
		}
    }

	return ntStatus;
}

#pragma code_seg()


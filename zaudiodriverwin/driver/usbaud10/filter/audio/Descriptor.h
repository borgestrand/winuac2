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
 * @brief      Audio topology descriptor definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _AUDIO_TOPOLOGY_DESCRIPTORS_H_
#define _AUDIO_TOPOLOGY_DESCRIPTORS_H_

#include "IKsAdapter.h"
#include "CList.h"

namespace AUDIO_TOPOLOGY
{
/*****************************************************************************
 * Defines
 */
//@{
/*! @brief Volume level and channel definitions. */
#define dB                  65536
#define INFINITY_DB			(-32768 * dB)
#define CHAN_MASTER         (-1)
#define CHAN_LEFT           0
#define CHAN_RIGHT          1
//@}

//@{
/*! @brief Format specifier codes. */
#define FORMAT_SPECIFIER_TYPE_I		0x00000001
#define FORMAT_SPECIFIER_TYPE_II	0x00000002
#define FORMAT_SPECIFIER_TYPE_III	0x00000004
//@}

typedef struct
{
	KSDATARANGE_AUDIO	Ex;
	UCHAR				InterfaceNumber;
	UCHAR				AlternateSetting;
	UCHAR				BitResolution;
} KSDATARANGE_AUDIO_EX, *PKSDATARANGE_AUDIO_EX;

/*****************************************************************************
 * Classes
 */
class CWireDescriptor;
class CPinDescriptor;
class CNodeDescriptor;
class CFilterDescriptor;
class CFilterPinDescriptor;

/*****************************************************************************
 *//*! @class CWireDescriptor
 *****************************************************************************
 * @ingroup MINIPORT_TOPOLOGY_GROUP
 * @brief
 * Wire descriptor object.
 * @details
 * Wires are object that connect pins. The ends of the wire are numbered as
 * followed:\n
 * @verbatim
 *
 *      (input) 1 >-----------< 0 (output)
 *
 * @endverbatim
 */
class CWireDescriptor
{
private:
    CWireDescriptor *   m_Next;     /*!< @brief Pointer to the next item. */
    CWireDescriptor *   m_Prev;     /*!< @brief Pointer to the previous item. */
    PVOID               m_Owner;    /*!< @brief Owner of this list item. */

    CPinDescriptor *    m_Pin1;     /*!< @brief Input pin. */
    CPinDescriptor *    m_Pin0;     /*!< @brief Output pin. */

protected:

public:

    CWireDescriptor();
    ~CWireDescriptor();
	void Destruct() { delete this; }

    void ConnectFrom
    (
        IN      CPinDescriptor * Pin
    );
    void ConnectTo
    (
        IN      CPinDescriptor * Pin
    );
    CPinDescriptor * FromPin
    (   void
    );
    CPinDescriptor * ToPin
    (   void
    );

    friend class CList<CWireDescriptor>;
};

typedef CWireDescriptor * PWIRE_DESCRIPTOR;

//@{
/*! @brief Data flow direction for the pin. */
#define PIN_DATAFLOW_NONE   0
#define PIN_DATAFLOW_IN     1
#define PIN_DATAFLOW_OUT    2
//@}

/*****************************************************************************
 *//*! @class CPinDescriptor
 *****************************************************************************
 * @ingroup MINIPORT_TOPOLOGY_GROUP
 * @brief
 * Pin descriptor object.
 * @details
 * Pins are the objects that nodes/filters exposes to the outside world. Pins
 * are connected to other pins thru wires.
 */
class CPinDescriptor
{
private:
    CPinDescriptor *    m_Next;     /*!< @brief Pointer to the next item. */
    CPinDescriptor *    m_Prev;     /*!< @brief Pointer to the previous item. */
    PVOID               m_Owner;    /*!< @brief Owner of this list item. */

protected:
    ULONG               m_PinId;    /*!< @brief Identifier for this pin. */
    ULONG               m_DataFlow; /*!< @brief Either input/output pin. */
    CWireDescriptor *   m_Wire;     /*!< @brief The wire which the pin is connected to. */
    CNodeDescriptor *   m_Node;     /*!< @brief The node that exposes this pin. */

public:

    CPinDescriptor();
    ~CPinDescriptor();
	void Destruct() { delete this; }

    NTSTATUS Init
    (
        IN      CNodeDescriptor *   Node,
        IN      ULONG               DataFlow,
        IN      ULONG               PinId
    );
    void PinId
    (
        IN      ULONG   PinId
    );
    ULONG PinId
    (   void
    );
    CNodeDescriptor * Node
    (   void
    );
    void Connect
    (
        IN      CWireDescriptor *   Wire
    );
    CWireDescriptor * Wire
    (   void
    );

    friend class CList<CPinDescriptor>;
};

typedef CPinDescriptor * PPIN_DESCRIPTOR;

/*****************************************************************************
 *//*! @class CNodeDescriptor
 *****************************************************************************
 * @ingroup MINIPORT_TOPOLOGY_GROUP
 * @brief
 * Node descriptor object.
 * @details
 * A node can have multiple input/output pins. Output pins always have id 0
 * while input pins are numbered from 1 onwards. A node does not contain
 * any node inside.
 */
class CNodeDescriptor
{
private:
    CNodeDescriptor *       m_Next;     /*!< @brief Pointer to the next item. */
    CNodeDescriptor *       m_Prev;     /*!< @brief Pointer to the previous item. */
    PVOID                   m_Owner;    /*!< @brief Owner of this list item. */

	PUNIT		m_Unit;
	UCHAR		m_UnitID;
	PTERMINAL	m_Terminal;
	UCHAR		m_ControlSelector; // 0 if not used.
	ULONG		m_NumberOfInputChannels;
	ULONG		m_NumberOfOutputChannels;
	ULONG		m_InputChannelOffset;
	ULONG		m_OutputChannelOffset;
	ULONG		m_MaximumNumberOfInputPins;
	ULONG		m_MaximumNumberOfOutputPins;

	KSNODE_DESCRIPTOR	m_KsNodeDescriptor;

	LONG		m_DrmReferenceCount;

	// These are for extensions only.
	GUID		m_ExtensionPropSetId;

	NTSTATUS _InitializeNodeDescriptor
	(	void
	);

	PKSAUTOMATION_TABLE _BuildAutomationTable
	(
		IN		PKSAUTOMATION_TABLE	Template
	);

	PKSAUTOMATION_TABLE _BuildExtensionAutomationTable
	(
		IN		GUID *	ExtensionPropSet,
		IN		ULONG	ExtensionPropertiesCount
	);

	VOID _DestroyAutomationTable
	(
		IN		PKSAUTOMATION_TABLE	AutomationTable
	);

protected:
    ULONG                   m_NodeId;           /*!< @brief Identifier for this node. */
    CList<CPinDescriptor>   m_InputPinList;     /*!< @brief List of input pins that this node expose. */
    CList<CPinDescriptor>   m_OutputPinList;    /*!< @brief List of output pins that this node expose. */

public:

    CNodeDescriptor();
    ~CNodeDescriptor();
	void Destruct() { delete this; }

    NTSTATUS Init
    (
        IN      ULONG       NodeId,
		IN		PTERMINAL	Terminal,
		IN		PUNIT		Unit,
		IN		UCHAR		ControlSelector,
		IN		ULONG		MaximumNumberOfInputPins,
		IN		ULONG		MaximumNumberOfOutputPins
    );
    CPinDescriptor * AddPin
    (
        IN      ULONG	DataFlow
    );
	ULONG PinCount
    (   void
    );
    NTSTATUS GetDescriptor
    (
        OUT     KSNODE_DESCRIPTOR * OutDescriptor
    );
	ULONG NodeId
	(   void
	);
	PUNIT Unit
	(   void
	);
	UCHAR UnitID
	(   void
	);
	UCHAR ControlSelector
	(	void
	);
	USHORT ProcessType
	(	void
	);
	ULONG NumberOfChannels
	(
		IN		BOOL	Direction,
		IN		PULONG	OutChannelOffset = NULL
	);
	ULONG NumberOfModes
	(
		IN OUT	PLONG	ActiveSpeakerPositions
	);	
	BOOL IsConnectionPossible
	(
		IN		ULONG	DataFlow
	);
	NTSTATUS WriteParameter
    (
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
    );
    NTSTATUS ReadParameter
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
    );
    NTSTATUS EnforceDrmProtection
	(
		IN		BOOL	OnOff
    );

    friend class CList<CNodeDescriptor>;
};

typedef CNodeDescriptor * PNODE_DESCRIPTOR;

/*****************************************************************************
 *//*! @class CFilterPinDescriptor
 *****************************************************************************
 * @ingroup MINIPORT_TOPOLOGY_GROUP
 * @brief
 * Filter pin descriptor object.
 * @details
 * Filter pins are the objects that filters exposes to the outside world. Pins
 * are connected to other pins thru wires.
 */
class CFilterPinDescriptor
:   public CPinDescriptor
{
private:
    CFilterPinDescriptor *      m_Next;                         /*!< @brief Pointer to the next item. */
    CFilterPinDescriptor *      m_Prev;                         /*!< @brief Pointer to the previous item. */
    PVOID                       m_Owner;                        /*!< @brief Owner of this list item. */

	PTERMINAL					m_Terminal;
    USHORT		                m_TerminalType;                 /*!< @brief Terminal type represented by this pin. */
	UCHAR						m_DescriptorSubtype;
	UCHAR						m_TerminalID;
	UCHAR						m_iTerminal;
	GUID						m_Category;						/*!< @brief Which category this filter pin is under. */
	GUID						m_Name;							/*!< @brief Name of the filter pin. */
	BOOL						m_IsSource;                     /*!< @brief TRUE if this pin is a source, FALSE if this pin is a destination. */
	BOOL						m_IsDigital;                    /*!< @brief TRUE if this pin is a digital pin, FALSE if this pin is an analog pin. */
    
	CFilterDescriptor *         m_Filter;                       /*!< @brief Pointer to the filter that own this pin. */
    LONG                        m_NumberOfChannels;             /*!< @brief Number of channels supported by this pin. */

	ULONG						m_FormatSpecifier;				/*!< @brief The format types that the pin is allowed to use. */

	BOOL						m_ConnectionEnable;

	ULONG						m_PinDataRangesCount;
	PKSDATARANGE_AUDIO_EX		m_PinDataRanges;
	PKSDATARANGE *				m_PinDataRangePointers;

	KSALLOCATOR_FRAMING_EX		m_PinAllocatorFraming;

	BOOL _FindSimilarAlternateSetting
	(
		IN		PAUDIO_INTERFACE	Interface,
		IN		UCHAR				AlternateSetting
	);

	NTSTATUS _AllocInitPinDataRangesStream
	(
		IN		UCHAR										InterfaceNumber,
		IN		UCHAR										AlternateSetting,
		IN		PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR	FormatTypeDescriptor_,
		IN		GUID										MajorFormat,
		IN		GUID										SubFormat,
		IN		GUID										Specifier
	);

	NTSTATUS _AllocInitPinDataRangesBridge
	(
		IN		GUID	MajorFormat,
		IN		GUID	SubFormat,
		IN		GUID	Specifier
	);

public:

    CFilterPinDescriptor();
    ~CFilterPinDescriptor();
	void Destruct() { delete this; }

    NTSTATUS Init
    (
        IN      CFilterDescriptor * Filter,
		IN		PTERMINAL			Terminal,
        IN      ULONG               PinId,
		IN		ULONG				FormatSpecifier
    );
	NTSTATUS BuildDataRanges
	(
		IN		PAUDIO_INTERFACE	Interface
	);
    NTSTATUS GetDescriptor
    (
        OUT     KSPIN_DESCRIPTOR_EX *	OutDescriptor
    );
	void EnableConnection
	(
		IN		BOOL	Flag
	);
    BOOL IsConnectionPossible
    (   void
    );
	GUID * Name
	(   void
	);
    BOOL IsSource
    (   void
    );
    BOOL IsDigital
    (   void
    );
	LONG NumberOfChannels
	(   void
	);
	ULONG ChannelConfig
	(	void
	);	
	PTERMINAL Terminal
	(   void
	);
	UCHAR TerminalID
	(   void
	);
	USHORT TerminalType
	(	void
	);
	UCHAR iTerminal
	(   void
	);
	ULONG FormatSpecifier
	(   void
	);
	NTSTATUS WriteParameter
    (
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
    );
    NTSTATUS ReadParameter
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
    );

    friend class CList<CFilterPinDescriptor>;
};

typedef CFilterPinDescriptor * PFILTER_PIN_DESCRIPTOR;

/*****************************************************************************
 *//*! @class CFilterDescriptor
 *****************************************************************************
 * @ingroup MINIPORT_TOPOLOGY_GROUP
 * @brief
 * Filter descriptor object.
 * @details
 * A filter is a special type of node. As with node, filter can have multiple
 * input/output pins. The main difference is that a filter can contain nodes.
 * Pins and nodes are connected together thru wires.
 */
class CFilterDescriptor
:   public  CNodeDescriptor
{
private:
protected:
    CList<CNodeDescriptor>      m_NodeList;         /*!< @brief List of internal nodes. */
    CList<CWireDescriptor>      m_WireList;         /*!< @brief List of wires that connect the pins and nodes internally. */
    CList<CFilterPinDescriptor> m_InputPinList;     /*!< @brief List of input pins that this filter expose. */
    CList<CFilterPinDescriptor> m_OutputPinList;    /*!< @brief List of output pins that this filter expose. */

public:

    CFilterDescriptor();
    ~CFilterDescriptor();
	void Destruct() { delete this; }

    CFilterPinDescriptor * AddPin
    (
		IN      PTERMINAL	Terminal,
		IN		ULONG		FormatSpecifier
    );
    CNodeDescriptor * AddNode
    (
		IN		PTERMINAL	Terminal,
		IN		PUNIT		Unit,
        IN      UCHAR		ControlSelector,
		IN		ULONG		MaximumNumberOfInputPins,
		IN		ULONG		MaximumNumberOfOutputPins
    );
    CWireDescriptor * Connect
    (
        IN      CPinDescriptor *    FromPin,
        IN      CPinDescriptor *    ToPin
    );
	CPinDescriptor * AddPinToEntity
	(
		IN		UCHAR	EntityID,
		IN		ULONG	DataFlow
	);
    ULONG PinCount
    (   void
    );
    ULONG NodeCount
    (   void
    );
    ULONG ConnectionCount
    (   void
    );
};

typedef CFilterDescriptor * PFILTER_DESCRIPTOR;

/*****************************************************************************
 *//*! @class CAudioFilterDescriptor
 *****************************************************************************
 * @ingroup MINIPORT_TOPOLOGYPIA_GROUP
 * @brief
 * Audio filter descriptor.
 * @details
 * A audio filter describes an audio topology, ie the list of filter pins,
 * node and the connections between the pins & nodes.
 */
class CAudioFilterDescriptor
:   protected  CFilterDescriptor
{
private:
	PAUDIO_DEVICE				m_AudioDevice;			/*!< @brief Pointer to the audio device interface. */
    PAUDIO_TOPOLOGY				m_AudioTopology;		/*!< @brief Pointer to the audio topology interface. */

    KSPIN_DESCRIPTOR_EX *       m_KsPins;				/*!< @brief KS pin descriptors. */
    KSNODE_DESCRIPTOR *         m_KsNodes;				/*!< @brief KS node descriptors. */
    KSTOPOLOGY_CONNECTION *		m_KsTopologyConnections;/*!< @brief KS topology connections. */
	KSCOMPONENTID				m_KsComponentId;		/*!< @brief KS component id. */

	ULONG _FindFormatSpecifier
	(
		IN		PTERMINAL	Terminal	
	);
	BOOL _FindTopologyConnectionTo
	(
		IN		ULONG					ToNode,
		IN		ULONG					ToPin,
		OUT		KSTOPOLOGY_CONNECTION *	OutConnection
	);
    NTSTATUS _ParseAndBuildPins
    (	void
	);
    NTSTATUS _ParseAndBuildNodes
    (   void
    );

public:

    CAudioFilterDescriptor();
    ~CAudioFilterDescriptor();
	void Destruct() { delete this; }

    NTSTATUS Initialize
    (
        IN      PAUDIO_DEVICE	AudioDevice
	);
    
    void AddMasterVolMuteNode(void);

	BOOL FindEntity
	(
		IN		UCHAR		EntityID,
		OUT		PENTITY *	OutEntity
	);
    ULONG PinCount
    (   void
    );
    ULONG NodeCount
    (   void
    );
    ULONG ConnectionCount
    (   void
    );
    ULONG CategoryCount
    (   void
    );
    KSPIN_DESCRIPTOR_EX * Pins
    (   void
    );
    KSNODE_DESCRIPTOR * Nodes
    (   void
    );
    KSTOPOLOGY_CONNECTION * Connections
    (   void
    );
    GUID * Categories
    (   void
    );
    KSCOMPONENTID * ComponentId
    (   void
    );
	CFilterPinDescriptor * FindFilterPin
	(
		IN		ULONG	PinId
	);
	CNodeDescriptor * FindNode
	(
		IN		ULONG	NodeId
	);
    STDMETHODIMP OnEvent
    (
        IN      ULONG		Event,
        IN      ULONG       Argument,
        IN      PVOID       Context
    );

	//edit yuanfen
	//find SUM node at playback path 
	CNodeDescriptor * FindSumNodeatPlaypath
	(
		void
	);
};

typedef CAudioFilterDescriptor * PAUDIO_FILTER_DESCRIPTOR;

} // namespace AUDIO_TOPOLOGY

#endif // _AUDIO_TOPOLOGY_DESCRIPTORS_H_

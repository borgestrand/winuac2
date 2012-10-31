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
 * @brief      MIDI Topology descriptor definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _MIDI_TOPOLOGY_DESCRIPTORS_H_
#define _MIDI_TOPOLOGY_DESCRIPTORS_H_

#include "IKsAdapter.h"
#include "CList.h"

#ifdef ENABLE_DIRECTMUSIC_SUPPORT
#include <portcls.h>
#include <dmusicks.h>
#endif // ENABLE_DIRECTMUSIC_SUPPORT

namespace MIDI_TOPOLOGY
{
/*****************************************************************************
 * Defines
 */
typedef struct
{
	KSDATARANGE_MUSIC	Ex;
	UCHAR				InterfaceNumber;
	UCHAR				EndpointAddress;
	UCHAR				CableNumber;
} KSDATARANGE_MUSIC_EX, *PKSDATARANGE_MUSIC_EX;

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
 * @ingroup MIDI_TOPOLOGY_GROUP
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
 * @ingroup MIDI_TOPOLOGY_GROUP
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
 * @ingroup MIDI_TOPOLOGY_GROUP
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

	PMIDI_ELEMENT			m_Element;
	UCHAR					m_ElementID;
	PMIDI_JACK				m_Jack;
	UCHAR					m_JackID;

	ULONG					m_MaximumNumberOfInputPins;
	ULONG					m_MaximumNumberOfOutputPins;

	KSNODE_DESCRIPTOR		m_KsNodeDescriptor;

	NTSTATUS _InitializeNodeDescriptor
	(
		IN		ULONG	Flags
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
        IN      ULONG			NodeId,
		IN		PMIDI_JACK		Jack,
		IN		PMIDI_ELEMENT	Element,
		IN		ULONG			Flags,
		IN		ULONG			MaximumNumberOfInputPins,
		IN		ULONG			MaximumNumberOfOutputPins
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
	PMIDI_ELEMENT Element
	(   void
	);
	UCHAR ElementID
	(   void
	);
	PMIDI_JACK Jack
	(	void
	);
	UCHAR JackID
	(   void
	);
	BOOL IsConnectionPossible
	(
		IN		ULONG	DataFlow
	);
	BOOL
	CNodeDescriptor::
	SupportCapability
	(
		IN		UCHAR	Capability
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

    friend class CList<CNodeDescriptor>;
};

typedef CNodeDescriptor * PNODE_DESCRIPTOR;

/*****************************************************************************
 *//*! @class CFilterPinDescriptor
 *****************************************************************************
 * @ingroup MIDI_TOPOLOGY_GROUP
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

	PMIDI_JACK					m_Jack;
	UCHAR						m_JackID;
    UCHAR		                m_JackType;						/*!< @brief Jack type represented by this pin. */
	UCHAR						m_iJack;
	GUID						m_Category;						/*!< @brief Which category this filter pin is under. */
	GUID						m_Name;							/*!< @brief Name of the filter pin. */
	BOOL						m_IsSource;                     /*!< @brief TRUE if this pin is a source, FALSE if this pin is a destination. */
	BOOL						m_IsEmbedded;					/*!< @brief TRUE if this pin is embedded, FALSE if this pin is external. */

	CFilterDescriptor *         m_Filter;                       /*!< @brief Pointer to the filter that own this pin. */

	BOOL						m_ConnectionEnable;

	ULONG						m_PinDataRangesCount;
	PKSDATARANGE_MUSIC_EX		m_PinDataRanges;
	PKSDATARANGE *				m_PinDataRangePointers;

	KSALLOCATOR_FRAMING_EX		m_PinAllocatorFraming;

	NTSTATUS _AllocInitPinDataRangesStream
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
		IN		PMIDI_JACK			Jack,
        IN      ULONG               PinId
    );
	NTSTATUS BuildDataRanges
	(
		IN		PMIDI_CABLE	MidiCable,
		IN		BOOL		DirectMusicCapable
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
    BOOL IsEmbedded
    (   void
    );
	PMIDI_JACK Jack
	(   void
	);
	UCHAR JackID
	(   void
	);
	UCHAR JackType
	(	void
	);
	UCHAR iJack
	(	void
	);

    friend class CList<CFilterPinDescriptor>;
};

typedef CFilterPinDescriptor * PFILTER_PIN_DESCRIPTOR;

/*****************************************************************************
 *//*! @class CFilterDescriptor
 *****************************************************************************
 * @ingroup MIDI_TOPOLOGY_GROUP
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
		IN      PMIDI_JACK	Jack
    );
    CNodeDescriptor * AddNode
    (
		IN      PMIDI_JACK		Jack,
		IN		PMIDI_ELEMENT	Element,
		IN		ULONG			Flags,
		IN		ULONG			MaximumNumberOfInputPins,
		IN		ULONG			MaximumNumberOfOutputPins
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
 *//*! @class CMidiFilterDescriptor
 *****************************************************************************
 * @ingroup MIDI_TOPOLOGY_GROUP
 * @brief
 * MIDI filter descriptor.
 * @details
 * A MIDI filter describes MIDI topology, ie the list of filter pins,
 * node and the connections between the pins & nodes.
 */
class CMidiFilterDescriptor
:   protected  CFilterDescriptor
{
private:
	PMIDI_DEVICE				m_MidiDevice;			/*!< @brief Pointer to the MIDI device interface. */
    PMIDI_TOPOLOGY				m_MidiTopology;			/*!< @brief Pointer to the MIDI topology interface. */
    PMIDI_CABLE					m_MidiCable;			/*!< @brief Pointer to the MIDI cable interface. */

    KSPIN_DESCRIPTOR_EX *       m_KsPins;				/*!< @brief KS pin descriptors. */
    KSNODE_DESCRIPTOR *         m_KsNodes;				/*!< @brief KS node descriptors. */
    KSTOPOLOGY_CONNECTION *		m_KsTopologyConnections;/*!< @brief KS topology connections. */
	KSCOMPONENTID				m_KsComponentId;		/*!< @brief KS component id. */

    NTSTATUS _ParseAndBuildPins
    (
		IN		BOOL	DirectMusicCapable
	);
    NTSTATUS _ParseAndBuildNodes
    (   void
    );

public:

    CMidiFilterDescriptor();
    ~CMidiFilterDescriptor();
	void Destruct() { delete this; }

    NTSTATUS Initialize
    (
        IN      PMIDI_DEVICE	MidiDevice,
		IN		PMIDI_TOPOLOGY	MidiTopology,
		IN		PMIDI_CABLE		MidiCable,
		IN		BOOL			DirectMusicCapable
	);
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
};

typedef CMidiFilterDescriptor * PMIDI_FILTER_DESCRIPTOR;

} // namespace MIDI_TOPOLOGY

#endif // _MIDI_TOPOLOGY_DESCRIPTORS_H_

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
 * @file       Filter.h
 * @brief      MIDI filter definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _MIDI_FILTER_PRIVATE_H_
#define _MIDI_FILTER_PRIVATE_H_

#include "IKsAdapter.h"
#include "Descriptor.h"
#include "PrvProp.h"

using namespace MIDI_TOPOLOGY;

/*****************************************************************************
 * Defines
 */
/*****************************************************************************
 * Classes
 */
class CMidiFilterFactory;

/*****************************************************************************
 *//*! @class CMidiFilter
 *****************************************************************************
 * @brief
 * Audio filter.
 * @details
 * This object is associated with the device and is created when the device
 * is started.  The class inherits CUnknown so it automatically gets reference 
 * counting and aggregation support.
 */
class CMidiFilter
:	public CUnknown
{
private:
    PKSFILTER				m_KsFilter;					/*!< @brief The AVStream filter we're associated with. */
    PKSADAPTER				m_KsAdapter;				/*!< @brief Pointer to the IKsAdapter interface. */
	PKSFILTER_DESCRIPTOR	m_KsFilterDescriptor;		/*!< @brief KS filter descriptor. */

	PUSB_DEVICE				m_UsbDevice;				/*!< @brief Pointer to the USB device object. */
	PMIDI_DEVICE			m_MidiDevice;				/*!< @brief Pointer to the audio device object. */

	CMidiFilterFactory * 	m_FilterFactory;

public:

    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in FILTER.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in FILTER.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
	DEFINE_STD_CONSTRUCTOR(CMidiFilter);
    ~CMidiFilter();

    /*************************************************************************
     * CMidiFilter methods
     */
	NTSTATUS Init
	(
		IN		PKSFILTER	KsFilter
	);

	NTSTATUS GetDescription
	(
		OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
	);

    NTSTATUS ValidateFormat
    (   
		IN      ULONG           PinId,
        IN      BOOLEAN         Capture,
        IN      PKSDATAFORMAT   Format
    );

	PFILTER_PIN_DESCRIPTOR FindPin
	(
		IN		ULONG	PinId
	);

	PNODE_DESCRIPTOR FindNode
	(
		IN		ULONG	NodeId
	);

	NTSTATUS FindPinName
	(
		IN		ULONG		PinId,
		IN OUT	PVOID		Buffer,
		IN		ULONG		BufferSize,
		OUT		ULONG *		OutPinNameLength	OPTIONAL
	);

	/*************************************************************************
     * Static
     */
	static
	KSFILTER_DISPATCH DispatchTable;

    static
    NTSTATUS DispatchCreate 
	(
        IN		PKSFILTER	KsFilter,
		IN		PIRP		Irp
    );

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	static const
	KSPROPERTY_ITEM PinPropertyTable[];

	static const
	KSPROPERTY_ITEM ControlPropertyTable[];

	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	static const
	KSPROPERTY_ITEM SynthClockPropertyTable[];
	#endif // ENABLE_DIRECTMUSIC_SUPPORT

	static const
	KSPROPERTY_SET PropertySetTable[];

	static const
	KSAUTOMATION_TABLE AutomationTable;

	static
	GUID Categories[4];

	static
	NTSTATUS SupportPinName
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetPinName
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetDeviceControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetDeviceControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportMidiElementCapability
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetMidiElementCapability
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	static
	NTSTATUS GetSynthCaps
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetSynthPortParameters
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetSynthChannelGroups
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetSynthChannelGroups
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetSynthLatencyClock
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportSynthMasterClock
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetSynthMasterClock
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);
	#endif // ENABLE_DIRECTMUSIC_SUPPORT

	static
	NTSTATUS SupportCpuResources
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetCpuResources
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	/*************************************************************************
     * Friends
     */
    friend class CMidiPin;
};

#endif // _MIDI_FILTER_PRIVATE_H_

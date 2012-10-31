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
 * @file       Factory.h
 * @brief      MIDI filter factory private definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _MIDI_FACTORY_PRIVATE_H_
#define _MIDI_FACTORY_PRIVATE_H_

#include "IKsAdapter.h"
#include "Descriptor.h"
#include "Profile.h"

using namespace MIDI_TOPOLOGY;

/*****************************************************************************
 * Defines
 */
/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CMidiFilterFactory
 *****************************************************************************
 * @brief
 * MIDI filter factory.
 * @details
 * This object is associated with the device and is created when the device 
 * is started.  The class inherits CUnknown so it automatically gets reference 
 * counting and aggregation support.
 */
class CMidiFilterFactory
:	public CUnknown
{
private:
    PKSDEVICE				m_KsDevice;				/*!< @brief The AVStream device we're associated with. */
    PKSADAPTER				m_KsAdapter;			/*!< @brief Pointer to the IKsAdapter interface. */
	KSFILTER_DESCRIPTOR		m_KsFilterDescriptor;
    DEVICE_POWER_STATE		m_DevicePowerState;		/*!< @brief Device power state. */

	PUSB_DEVICE				m_UsbDevice;			/*!< @brief Pointer to the USB device object. */
	PMIDI_DEVICE			m_MidiDevice;			/*!< @brief Pointer to the MIDI device object. */
	PMIDI_TOPOLOGY			m_MidiTopology;			/*!< @brief Pointer to the MIDI topology object. */
	PMIDI_CABLE				m_MidiCable;			/*!< @brief Pointer to the MIDI cable object. */

	BOOL					m_DirectMusicCapable;	/*!< @brief TRUE if the filter is DirectMusic capable. */

	CMidiFilterDescriptor 	m_MidiFilterDescriptor;

	/*************************************************************************
     * CMidiFilterFactory private methods
     */
	static
	VOID SetupFriendlyName
	(
		IN		PKSADAPTER				KsAdapter,
		IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
		IN		PWCHAR					RefString,
		IN		PVOID					Parameter1,
		IN		PVOID					Parameter2
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in FACTORY.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in FACTORY.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
	DEFINE_STD_CONSTRUCTOR(CMidiFilterFactory);
    ~CMidiFilterFactory();

    /*************************************************************************
     * CMidiFilterFactory public methods
     */
	NTSTATUS Init
	(
		IN		PKSDEVICE	KsDevice,
		IN		PVOID		Parameter1,
		IN		PVOID		Parameter2
	);

	NTSTATUS GetFilterDescription
	(
		OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
	);

	VOID PowerChangeNotify
	(
		IN      DEVICE_POWER_STATE	NewPowerState
	);

	PFILTER_PIN_DESCRIPTOR FindPin
	(
		IN		ULONG	PinId
	);

	PNODE_DESCRIPTOR FindNode
	(
		IN		ULONG	NodeId
	);

	BOOL IsDirectMusicCapable
	(	void
	);

	/*************************************************************************
     * Static
     */
	static
	VOID SleepCallback 
	(
		IN		PKSFILTERFACTORY	KsFilterFactory,
		IN		DEVICE_POWER_STATE	PowerState
	);

	static
	VOID WakeCallback 
	(
		IN		PKSFILTERFACTORY	KsFilterFactory,
		IN		DEVICE_POWER_STATE	PowerState
	);

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	/*************************************************************************
     * Friends
     */
	friend
	NTSTATUS CreateMidiFilterFactory
	(
		IN		PKSDEVICE	KsDevice,
		IN		PWCHAR		RefString,
		IN		PVOID		Parameter1,
		IN		PVOID		Parameter2
	);
};

#endif  //  _MIDI_FACTORY_PRIVATE_H_

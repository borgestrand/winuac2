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
 * @file       Pin.h
 * @brief      MIDI pin definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _MIDI_PIN_PRIVATE_H_
#define _MIDI_PIN_PRIVATE_H_

#include "Filter.h"

using namespace MIDI_TOPOLOGY;

/*****************************************************************************
 * Defines
 */
/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CMidiPin
 *****************************************************************************
 * @brief
 * Audio pin.
 * @details
 * This object is associated with a streaming pin and is created when a pin
 * is created on the filter.  The class inherits CUnknown so it automatically 
 * gets reference counting and aggregation support.
 */
class CMidiPin
:	public CUnknown
{
private:
	PKSPIN						m_KsPin;

    CMidiFilter *				m_MidiFilter;			/*!< @brief Filter that created us. */
    ULONG						m_PinId;				/*!< @brief Pin identifier. */
    KSSTATE						m_State;				/*!< @brief State (RUN/PAUSE/ACQUIRE/STOP). */
    BOOLEAN						m_Capture;				/*!< @brief TRUE for capture, FALSE for render. */

	PMIDI_DEVICE				m_MidiDevice;			/*!< @brief Pointer to the MIDI device object. */
	PMIDI_CLIENT				m_MidiClient;			/*!< @brief Pointer to the MIDI client instance. */

	UCHAR						m_InterfaceNumber;		/*!< @brief USB interface number. */
	UCHAR						m_EndpointAddress;		/*!< @brief USB endpoint address. */
	UCHAR						m_CableNumber;			/*!< @brief Number assignment of the Embedded MIDI Jack associated with the endpoint that is transferring data. */

	BOOL						m_SynchronousMode;

	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	BOOL						m_DirectMusicFormat;
	#endif // ENABLE_DIRECTMUSIC_SUPPORT

	LONGLONG					m_StartTimeStampCounter;

	LONGLONG					m_TimeStampFrequency;

	PIKSREFERENCECLOCK			m_ReferenceClock;

	PKSSTREAM_POINTER			m_PreviousClonePointer;

	BOOL						m_PendingIo;

    /*************************************************************************
     * CMidiPin methods
     *
     * These are private member functions used internally by the object.
     */
	NTSTATUS _FindMidiCable
	(
		IN      ULONG	PinId
	);    
	NTSTATUS _AllocateResources
    (   void
    );
    NTSTATUS _FreeResources
    (   void
    );
	NTSTATUS _FreeClonePointers
	(   void
	);
    NTSTATUS _Run
    (   void
    );
    NTSTATUS _Pause
    (   void
    );
    NTSTATUS _Stop
    (   void
    );
    NTSTATUS _Reset
    (   void
    );
	NTSTATUS _ProcessKsMusicFormat
	(	void
	);
	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	NTSTATUS _ProcessDirectMusicFormat
	(	void
	);
	#endif // ENABLE_DIRECTMUSIC_SUPPORT

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
    DEFINE_STD_CONSTRUCTOR(CMidiPin);
    ~CMidiPin();

    /*************************************************************************
     * CMidiPin methods
     */
    NTSTATUS Init
    (
		IN		PKSPIN	KsPin
    );

	NTSTATUS SetFormat
	(
		IN      PKSDATAFORMAT   Format
	);

	NTSTATUS SetState
	(
		IN      KSSTATE     NewState
	);

	NTSTATUS Process
	(	void
	);

	void IoCompletion 
	(
		IN		ULONG	BytesCount
	);

	/*************************************************************************
     * Static methods
     */
	static
	KSPIN_DISPATCH DispatchTable; 

    static
    NTSTATUS DispatchCreate 
	(
		IN		PKSPIN		KsPin,
		IN		PIRP		Irp
    );

    static
    NTSTATUS DispatchSetFormat 
	(
		IN		PKSPIN						KsPin,
		IN		PKSDATAFORMAT				OldFormat			OPTIONAL,
		IN		PKSMULTIPLE_ITEM			OldAttributeList	OPTIONAL,
		IN		const KSDATARANGE *			DataRange,
		IN		const KSATTRIBUTE_LIST *	AttributeRange		OPTIONAL
    );

    static
    NTSTATUS DispatchSetState 
	(
		IN		PKSPIN		KsPin,
		IN		KSSTATE		ToState,
		IN		KSSTATE		FromState
    );

	static 
    NTSTATUS DispatchProcess 
	(
        IN		PKSPIN	KsPin
    );

	static const
	KSALLOCATOR_FRAMING_EX AllocatorFraming;

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	static
	VOID MidiCallbackRoutine
	(
		IN		PVOID	Context,
		IN		ULONG	BytesCount
	);
};

#endif // _MIDI_PIN_PRIVATE_H_

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
 * @file       Pin.cpp
 * @brief      Audio pin implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "Pin.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "MIDI_PIN: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiPin::DispatchTable
 *****************************************************************************
 *//*!
 * @brief
 * This is the dispatch table for the audio pin.  It provides notifications
 * about creation, closure, processing, data formats, etc...
 */
KSPIN_DISPATCH 
CMidiPin::DispatchTable = 
{
    CMidiPin::DispatchCreate,				// Pin Create
    NULL,                                   // Pin Close
    CMidiPin::DispatchProcess,				// Pin Process
    NULL,                                   // Pin Reset
    CMidiPin::DispatchSetFormat,			// Pin Set Data Format
    CMidiPin::DispatchSetState,				// Pin Set Device State
    NULL,                                   // Pin Connect
    NULL,                                   // Pin Disconnect
    NULL,                                   // Clock Dispatch
    NULL                                    // Allocator Dispatch
};

/*****************************************************************************
 * CMidiPin::DispatchCreate()
 *****************************************************************************
 *//*!
 * @brief
 * This is the Create dispatch for the pin.  It creates the CMidiPin object 
 * and associates it with the AVStream object, bagging it in the process.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiPin::
DispatchCreate 
(
    IN		PKSPIN		KsPin,
	IN		PIRP		Irp
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchCreate]"));

	NTSTATUS ntStatus = STATUS_DEVICE_NOT_CONNECTED;

    CMidiFilter * MidiFilter = (CMidiFilter *)(KsPin->Context); // only at init.

	if (MidiFilter->m_KsAdapter->IsReadyForIO())
	{
		CMidiPin * Pin = new(NonPagedPool,'aChS') CMidiPin(NULL);

		if (Pin)
		{
			Pin->AddRef();

			ntStatus = Pin->Init(KsPin);

			if (NT_SUCCESS(ntStatus))
			{
				//
				// Add the item to the object bag if we were successful. Whenever the device goes 
				// away, the bag is cleaned up and we will be freed.
				//
				// For backwards compatibility with DirectX 8.0, we must grab the device mutex 
				// before doing this.  For Windows XP, this is not required, but it is still safe.
				//
				KsPinAcquireControl(KsPin);

				ntStatus = KsAddItemToObjectBag(KsPin->Bag, Pin, (PFNKSFREE)CMidiPin::Destruct);

				KsPinReleaseControl(KsPin);
			}

			if (NT_SUCCESS(ntStatus)) 
			{
				// Keeping this object...
				Pin->AddRef();

				KsPin->Context = PVOID(Pin);
			}

			// Release the private reference.
			Pin->Release();
		}
		else
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::DispatchSetState()
 *****************************************************************************
 *//*!
 * @brief
 * This is the set device state dispatch for the pin.  The routine bridges
 * to SetState() in the context of the CMidiPin.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiPin::
DispatchSetState 
(
	IN		PKSPIN		KsPin,
    IN		KSSTATE		ToState,
    IN		KSSTATE		FromState
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetState]"));

	CMidiPin * Pin = (CMidiPin *)(KsPin->Context);

	NTSTATUS ntStatus = Pin->SetState(ToState);

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::DispatchSetFormat()
 *****************************************************************************
 *//*!
 * @brief
 * This is the set data format dispatch for the pin.  This will be called
 * BEFORE pin creation to validate that a data format selected is a match
 * for the range pulled out of our range list.  It will also be called
 * for format changes.
 *
 * If OldFormat is NULL, this is an indication that it's the initial
 * call and not a format change.  Even fixed format pins get this call
 * once.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiPin::
DispatchSetFormat 
(
    IN		PKSPIN						KsPin,
    IN		PKSDATAFORMAT				OldFormat			OPTIONAL,
    IN		PKSMULTIPLE_ITEM			OldAttributeList	OPTIONAL,
    IN		const KSDATARANGE *			DataRange,
    IN		const KSATTRIBUTE_LIST *	AttributeRange		OPTIONAL
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat]"));

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - Context: %p, OldFormat: %p", KsPin->Context, OldFormat));

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->FormatSize: %d", DataRange->FormatSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->Flags: %d", DataRange->Flags));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->SampleSize: %d", DataRange->SampleSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->Reserved: %d", DataRange->Reserved));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->MajorFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								DataRange->MajorFormat.Data1, DataRange->MajorFormat.Data2, DataRange->MajorFormat.Data3,
								DataRange->MajorFormat.Data4[0], DataRange->MajorFormat.Data4[1], DataRange->MajorFormat.Data4[2], DataRange->MajorFormat.Data4[3],
								DataRange->MajorFormat.Data4[4], DataRange->MajorFormat.Data4[5], DataRange->MajorFormat.Data4[6], DataRange->MajorFormat.Data4[7],
								DataRange->MajorFormat.Data4[8], DataRange->MajorFormat.Data4[9], DataRange->MajorFormat.Data4[10], DataRange->MajorFormat.Data4[11],
								DataRange->MajorFormat.Data4[12], DataRange->MajorFormat.Data4[13], DataRange->MajorFormat.Data4[14], DataRange->MajorFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->SubFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								DataRange->SubFormat.Data1, DataRange->SubFormat.Data2, DataRange->SubFormat.Data3,
								DataRange->SubFormat.Data4[0], DataRange->SubFormat.Data4[1], DataRange->SubFormat.Data4[2], DataRange->SubFormat.Data4[3],
								DataRange->SubFormat.Data4[4], DataRange->SubFormat.Data4[5], DataRange->SubFormat.Data4[6], DataRange->SubFormat.Data4[7],
								DataRange->SubFormat.Data4[8], DataRange->SubFormat.Data4[9], DataRange->SubFormat.Data4[10], DataRange->SubFormat.Data4[11],
								DataRange->SubFormat.Data4[12], DataRange->SubFormat.Data4[13], DataRange->SubFormat.Data4[14], DataRange->SubFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - DataRange->Specifier: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								DataRange->Specifier.Data1, DataRange->Specifier.Data2, DataRange->Specifier.Data3,
								DataRange->Specifier.Data4[0], DataRange->Specifier.Data4[1], DataRange->Specifier.Data4[2], DataRange->Specifier.Data4[3],
								DataRange->Specifier.Data4[4], DataRange->Specifier.Data4[5], DataRange->Specifier.Data4[6], DataRange->Specifier.Data4[7],
								DataRange->Specifier.Data4[8], DataRange->Specifier.Data4[9], DataRange->Specifier.Data4[10], DataRange->Specifier.Data4[11],
								DataRange->Specifier.Data4[12], DataRange->Specifier.Data4[13], DataRange->Specifier.Data4[14], DataRange->Specifier.Data4[15]
								));

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->FormatSize: %d", KsPin->ConnectionFormat->FormatSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->Flags: %d", KsPin->ConnectionFormat->Flags));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->SampleSize: %d", KsPin->ConnectionFormat->SampleSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->Reserved: %d", KsPin->ConnectionFormat->Reserved));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->MajorFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								KsPin->ConnectionFormat->MajorFormat.Data1, KsPin->ConnectionFormat->MajorFormat.Data2, KsPin->ConnectionFormat->MajorFormat.Data3,
								KsPin->ConnectionFormat->MajorFormat.Data4[0], KsPin->ConnectionFormat->MajorFormat.Data4[1], KsPin->ConnectionFormat->MajorFormat.Data4[2], KsPin->ConnectionFormat->MajorFormat.Data4[3],
								KsPin->ConnectionFormat->MajorFormat.Data4[4], KsPin->ConnectionFormat->MajorFormat.Data4[5], KsPin->ConnectionFormat->MajorFormat.Data4[6], KsPin->ConnectionFormat->MajorFormat.Data4[7],
								KsPin->ConnectionFormat->MajorFormat.Data4[8], KsPin->ConnectionFormat->MajorFormat.Data4[9], KsPin->ConnectionFormat->MajorFormat.Data4[10], KsPin->ConnectionFormat->MajorFormat.Data4[11],
								KsPin->ConnectionFormat->MajorFormat.Data4[12], KsPin->ConnectionFormat->MajorFormat.Data4[13], KsPin->ConnectionFormat->MajorFormat.Data4[14], KsPin->ConnectionFormat->MajorFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->SubFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								KsPin->ConnectionFormat->SubFormat.Data1, KsPin->ConnectionFormat->SubFormat.Data2, KsPin->ConnectionFormat->SubFormat.Data3,
								KsPin->ConnectionFormat->SubFormat.Data4[0], KsPin->ConnectionFormat->SubFormat.Data4[1], KsPin->ConnectionFormat->SubFormat.Data4[2], KsPin->ConnectionFormat->SubFormat.Data4[3],
								KsPin->ConnectionFormat->SubFormat.Data4[4], KsPin->ConnectionFormat->SubFormat.Data4[5], KsPin->ConnectionFormat->SubFormat.Data4[6], KsPin->ConnectionFormat->SubFormat.Data4[7],
								KsPin->ConnectionFormat->SubFormat.Data4[8], KsPin->ConnectionFormat->SubFormat.Data4[9], KsPin->ConnectionFormat->SubFormat.Data4[10], KsPin->ConnectionFormat->SubFormat.Data4[11],
								KsPin->ConnectionFormat->SubFormat.Data4[12], KsPin->ConnectionFormat->SubFormat.Data4[13], KsPin->ConnectionFormat->SubFormat.Data4[14], KsPin->ConnectionFormat->SubFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchSetFormat] - KsPin->ConnectionFormat->Specifier: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								KsPin->ConnectionFormat->Specifier.Data1, KsPin->ConnectionFormat->Specifier.Data2, DataRange->Specifier.Data3,
								KsPin->ConnectionFormat->Specifier.Data4[0], KsPin->ConnectionFormat->Specifier.Data4[1], KsPin->ConnectionFormat->Specifier.Data4[2], KsPin->ConnectionFormat->Specifier.Data4[3],
								KsPin->ConnectionFormat->Specifier.Data4[4], KsPin->ConnectionFormat->Specifier.Data4[5], KsPin->ConnectionFormat->Specifier.Data4[6], KsPin->ConnectionFormat->Specifier.Data4[7],
								KsPin->ConnectionFormat->Specifier.Data4[8], KsPin->ConnectionFormat->Specifier.Data4[9], KsPin->ConnectionFormat->Specifier.Data4[10], KsPin->ConnectionFormat->Specifier.Data4[11],
								KsPin->ConnectionFormat->Specifier.Data4[12], KsPin->ConnectionFormat->Specifier.Data4[13], KsPin->ConnectionFormat->Specifier.Data4[14], KsPin->ConnectionFormat->Specifier.Data4[15]
								));

	NTSTATUS ntStatus;

	if (OldFormat)
	{
		// DispatchCreate is already done.
		CMidiPin * Pin = (CMidiPin*)(KsPin->Context);

		ntStatus = Pin->SetFormat(PKSDATAFORMAT(KsPin->ConnectionFormat));
	}
	else
	{
		CMidiFilter * Filter = (CMidiFilter*)(KsPin->Context);

		ntStatus = Filter->ValidateFormat(KsPin->Id, (KsPin->DataFlow == KSPIN_DATAFLOW_OUT), PKSDATAFORMAT(KsPin->ConnectionFormat));
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiPin::DispatchProcess()
 *****************************************************************************
 *//*!
 * @brief
 * This is the processing dispatch for the audio pin.  The routine 
 * bridges to Process() in the context of the CMidiPin.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiPin::
DispatchProcess 
(
	IN		PKSPIN	KsPin
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::DispatchProcess]"));

	CMidiPin * Pin = (CMidiPin *)(KsPin->Context);

	NTSTATUS ntStatus = Pin->Process();

	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiPin::AllocatorFraming
 *****************************************************************************
 *//*!
 * @brief
 * This is the simple framing structure for the audio pin.  Note that this
 * will be modified via KsEdit when the actual format is determined.
 */
DECLARE_SIMPLE_FRAMING_EX
(
	CMidiPin::AllocatorFraming,
	STATICGUIDOF(KSMEMORY_TYPE_KERNEL_NONPAGED),
	KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
	8,
	FILE_BYTE_ALIGNMENT,
	1,
	0xFFFFFFFF
);

/*****************************************************************************
 * CMidiPin::Destruct()
 *****************************************************************************
 *//*!
 * @brief
 * This is the free callback for the bagged filter.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CMidiPin object.
 * @return
 * None.
 */
VOID
CMidiPin::
Destruct 
(
	IN		PVOID	Self
)
{
    PAGED_CODE();

	CMidiPin * Pin = (CMidiPin *)(Self);

	Pin->Release();
}

/*****************************************************************************
 * CMidiPin::NonDelegatingQueryInterface()
 *****************************************************************************
 *//*!
 * @brief
 * Obtains an interface.
 * @details
 * This function works just like a COM QueryInterface call and is used if
 * the object is not being aggregated.
 * @param
 * Interface The GUID of the interface to be retrieved.
 * @param
 * Object Pointer to the location to store the retrieved interface object.
 * @return
 * Returns STATUS_SUCCESS if the interface is found. Otherwise, returns
 * STATUS_INVALID_PARAMETER.
 */
STDMETHODIMP
CMidiPin::
NonDelegatingQueryInterface
(
    IN      REFIID  Interface,
    OUT     PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::NonDelegatingQueryInterface]"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

/*****************************************************************************
 * CMidiPin::~CMidiPin()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CMidiPin::
~CMidiPin
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::~CMidiPin]"));

	// In case things didn't goes as planned, put the stream into proper state 
	// before destructing...
	if (m_State == KSSTATE_RUN)
    {
        SetState(KSSTATE_PAUSE);
    }

	if (m_State == KSSTATE_PAUSE)
    {
        SetState(KSSTATE_ACQUIRE);
    }

	if (m_State == KSSTATE_ACQUIRE)
    {
        SetState(KSSTATE_STOP);
    }

	if (m_MidiClient)
	{
		m_MidiDevice->Close(m_MidiClient);
	}

	if (m_MidiFilter)
    {
		m_MidiFilter->m_KsAdapter->DereferenceDevice();

        m_MidiFilter->Release();
    }
}

/*****************************************************************************
 * CMidiPin::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initializes a stream.
 * @details
 * The caller of @b Init should run at IRQL_PASSIVE_LEVEL.
 * @param
 * AudioFilter Pointer to the filter that created us.
 * @param
 * PinId Identifier of the pin.
 * @param
 * Capture TRUE for an input (capture) channel, and FALSE for an output (playback) channel.
 * @param
 * DataFormat Pointer to a KSDATAFORMAT structure indicating the format to use for this instance.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
Init
(
	IN		PKSPIN	KsPin
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::Init]"));

    ASSERT(KsPin);

    ASSERT(KsPin->ConnectionFormat);

	m_KsPin = KsPin;

    m_MidiFilter = (CMidiFilter *)(KsPin->Context); // only at init.
    m_MidiFilter->AddRef();

    m_PinId = KsPin->Id;

    m_State = KSSTATE_STOP;
    
	m_Capture = (KsPin->DataFlow == KSPIN_DATAFLOW_OUT); // Out from the filter into the host.

	m_MidiDevice = m_MidiFilter->m_MidiDevice;

	m_MidiFilter->m_KsAdapter->ReferenceDevice();

	m_SynchronousMode = TRUE;

	NTSTATUS ntStatus = _FindMidiCable(m_PinId);
	
	if (NT_SUCCESS(ntStatus))
	{
		// Open the hardware instance.
		ntStatus = m_MidiDevice->Open(m_InterfaceNumber, m_EndpointAddress, m_CableNumber, MidiCallbackRoutine, this, &m_MidiClient);

		if (NT_SUCCESS(ntStatus))
		{
			if (!m_Capture)
			{
				ULONG SysExTimeOutPeriod = 1000; // Default 1s time out.

				// HKR,Settings,SysExTimeOutPeriod,0x00010001,1000
				if (NT_SUCCESS(m_MidiFilter->m_KsAdapter->RegistryReadFromDriverSubKey(L"Settings", L"SysExTimeOutPeriod", &SysExTimeOutPeriod, sizeof(ULONG), NULL, NULL)))
				{
					m_MidiClient->SetSysExTimeOutPeriod(SysExTimeOutPeriod);
				}

				// HKR,Settings,Synchronous,0x00010001,0
				m_MidiFilter->m_KsAdapter->RegistryReadFromDriverSubKey(L"Settings", L"Synchronous", &m_SynchronousMode, sizeof(BOOL), NULL, NULL);
			}
		}
	}

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = SetFormat(KsPin->ConnectionFormat);
	}

    if (!NT_SUCCESS(ntStatus))
    {
        // Clean up the mess
		m_MidiFilter->m_KsAdapter->DereferenceDevice();

		if (m_MidiClient)
		{
			m_MidiDevice->Close(m_MidiClient);
			m_MidiClient = NULL;
		}

        m_MidiFilter->Release();
        m_MidiFilter = NULL;
    }

    return ntStatus;
}

/*****************************************************************************
 * CMidiPin::SetFormat()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the format of the stream.
 * @param
 * Format Pointer to a KSDATAFORMAT structure indicating the format to use for
 * this instance.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
SetFormat
(
    IN      PKSDATAFORMAT   Format
)
{
    PAGED_CODE();

    ASSERT(Format);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetFormat]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	m_DirectMusicFormat = IsEqualGUIDAligned(Format->SubFormat, KSDATAFORMAT_SUBTYPE_DIRECTMUSIC);
	#endif // ENABLE_DIRECTMUSIC_SUPPORT

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::SetState()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the stream's transport state to a new state.
 * @details
 * The new state can be one of the following:
 * - KSSTATE_RUN
 * Data transport in the current audio filter graph is running and
 * functioning as normal.
 * - KSSTATE_ACQUIRE
 * This is a transitional state that helps to manage the transition between
 * KSSTATE_RUN and KSSTATE_STOP.
 * - KSSTATE_PAUSE
 * This is a transitional state that helps to manage the transition between
 * KSSTATE_RUN and KSSTATE_STOP.
 * - KSSTATE_STOP
 * Data transport is stopped in the current audio filter graph.
 *
 * For most filters, KSSTATE_ACQUIRE and KSSTATE_PAUSE are
 * indistinguishable.
 *
 * Transitions always occur in one of the following two sequences:
 * - STOP -> ACQUIRE -> PAUSE -> RUN
 * - RUN -> PAUSE -> ACQUIRE -> STOP
 *
 * The initial state of the stream is KSSTATE_STOP.
 * @param
 * NewState KSSTATE enumeration value that indicates the new state that the
 * stream is to be set to.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
SetState
(
    IN      KSSTATE     NewState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState]"));

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	switch (m_State)
    {
        case KSSTATE_STOP:
        {
            switch (NewState)
            {
                case KSSTATE_STOP:
                {
                    // No state change.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_STOP->KSSTATE_STOP"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // Transition from KSSTATE_STOP to KSSTATE_ACQUIRE
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_STOP->KSSTATE_ACQUIRE"));

                    // Acquire the resources.
                    ntStatus = _AllocateResources();

					if (NT_SUCCESS(ntStatus))
                    {
                        m_State = NewState;
                    }
                }
                break;

                case KSSTATE_PAUSE:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_STOP->KSSTATE_PAUSE is INVALID"));
                }
                break;

                case KSSTATE_RUN:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_STOP->KSSTATE_RUN is INVALID"));
                }
                break;
            }
        }
        break;

        case KSSTATE_ACQUIRE:
        {
            switch (NewState)
            {
                case KSSTATE_STOP:
                {
                    // Transition from KSSTATE_STOP to KSSTATE_ACQUIRE
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_STOP"));

					// Reset the hardware.
					_Reset();

                    // Stop the hardware first (if necessary).
                    _Stop();

					// Free the clone pointers.
					_FreeClonePointers();

                    // Release the resources.
                    _FreeResources();

                    // Update state.
                    m_State = NewState;

					ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // No state change.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_ACQUIRE"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_PAUSE:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_PAUSE"));

					// Update state.
                    m_State = NewState;

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_RUN:
                {
                    // Only make this a valid state transition because we
                    // could handle it, and to fix ACPI problem on Win98SE
                    // which give us a bad state transition from
                    // STOP->PAUSE->ACQUIRE->RUN upon power transition
                    // D3->D0.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_RUN"));

					if (m_MidiFilter->m_KsAdapter->IsReadyForIO())
					{
						ntStatus = _Run();

						if (NT_SUCCESS(ntStatus))
						{
							m_State = NewState;
						}
					}
					else
					{
						ntStatus = STATUS_DEVICE_NOT_CONNECTED;
					}
                }
                break;
            }
        }
        break;

        case KSSTATE_PAUSE:
        {
            switch (NewState)
            {
                case KSSTATE_STOP:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_PAUSE->KSSTATE_STOP is INVALID"));
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_PAUSE->KSSTATE_ACQUIRE"));

					// The below is done because on DirectX 8.0, when the pin gets
					// a message to stop, the queue is inaccessible.  The reset 
					// which comes on every stop happens after this (at which time
					// the queue is inaccessible also).  So, for compatibility with
					// DirectX 8.0, I am stopping the "fake" hardware at this
					// point and cleaning up all references we have on frames.  See
					// the comments above regarding the CleanupReferences call.
					//
					// If this sample were targeting XP only, the below code would
					// not be here.  Again, I only do this so the sample does not
					// hang when it is stopped running on a configuration such as
					// Win2K + DX8.

					// Reset the hardware.
					_Reset();

					// Stop the hardware first (if necessary).
                    _Stop();

					// Free the clone pointers.
					_FreeClonePointers();

                    // Update state.
                    m_State = NewState;

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_PAUSE:
                {
                    // No state change.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_PAUSE->KSSTATE_PAUSE"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_RUN:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_PAUSE->KSSTATE_RUN"));

                    // Start the hardware.
					if (m_MidiFilter->m_KsAdapter->IsReadyForIO())
					{
						ntStatus = _Run();

						if (NT_SUCCESS(ntStatus))
						{
							m_State = NewState;
						}
					}
					else
					{
						ntStatus = STATUS_DEVICE_NOT_CONNECTED;
					}
                }
                break;
            }
        }
        break;

        case KSSTATE_RUN:
        {
            switch (NewState)
            {
                case KSSTATE_STOP:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_RUN->KSSTATE_STOP is INVALID"));
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_RUN->KSSTATE_ACQUIRE is INVALID"));
                }
                break;

                case KSSTATE_PAUSE:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_RUN->KSSTATE_PAUSE"));

					// Pause the hardware.
					ntStatus = _Pause();

                    // Update state.
                    m_State = NewState;

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_RUN:
                {
                    // No state change.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::SetState] : KSSTATE_RUN->KSSTATE_RUN"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;
            }
        }
        break;

        default:
            break;
    }

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiPin::Process()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CMidiPin::
Process
(	void
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::Process]"));

	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
    if (m_DirectMusicFormat)
	{
		return _ProcessDirectMusicFormat();
	}
	else
	#endif // ENABLE_DIRECTMUSIC_SUPPORT
	{
		return _ProcessKsMusicFormat();
	}
}

/*****************************************************************************
 * CMidiPin::MidiCallbackRoutine()
 *****************************************************************************
 *//*!
 * @brief
 * MIDI input callback routine.
 * @param
 * @param
 * @return
 * <None>
 */
VOID
CMidiPin::
MidiCallbackRoutine
(
	IN		PVOID	Context,
	IN		ULONG	BytesCount
)
{
    CMidiPin * that = (CMidiPin *)(Context);

	if (that->m_State == KSSTATE_RUN)
	{
		that->IoCompletion(BytesCount);
	}
}

/*****************************************************************************
 * CMidiPin::IoCompletion()
 *****************************************************************************
 *//*!
 * @brief
 * Called to notify the pin that a given number of data packets have completed.  
 * Let the buffers go if possible. We're called at DPC.
 * @return
 * None
 */
void
CMidiPin::
IoCompletion 
(
	IN		ULONG	BytesCount
)
{
    //
    // Kick processing to happen again.
    //
    KsPinAttemptProcessing(m_KsPin, TRUE);
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiPin::_FindMidiCable()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CMidiPin::
_FindMidiCable
(
    IN      ULONG	PinId
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_FindMidiCable]"));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    PKSFILTER_DESCRIPTOR FilterDescriptor; m_MidiFilter->GetDescription(&FilterDescriptor);

    if (PinId < FilterDescriptor->PinDescriptorsCount)
    {
        PKSPIN_DESCRIPTOR_EX Pin = PKSPIN_DESCRIPTOR_EX(&FilterDescriptor->PinDescriptors[PinId]);

        // Validate data flows...
        PKSDATARANGE * DataRanges = (PKSDATARANGE *)Pin->PinDescriptor.DataRanges;

        if (DataRanges)
        {
			// The cable information is the same across all data ranges, so the first one
			// will do fine.
			PKSDATARANGE_MUSIC_EX DataRangeMusicEx = PKSDATARANGE_MUSIC_EX(DataRanges[0]);

			m_InterfaceNumber = DataRangeMusicEx->InterfaceNumber;
			m_EndpointAddress = DataRangeMusicEx->EndpointAddress;
			m_CableNumber     = DataRangeMusicEx->CableNumber;

			ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_AllocateResources()
 *****************************************************************************
 *//*!
 * @brief
 * Allocate resources for the stream.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_AllocateResources
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_AllocateResources]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	// Attempt to get an interface to the master clock. This will fail if one 
	// has not been assigned.  Since one must be assigned while the pin is still in 
    // KSSTATE_STOP, this is a guranteed method of getting the clock should one 
	// be assigned.
    if (!NT_SUCCESS(KsPinGetReferenceClockInterface(m_KsPin, &m_ReferenceClock))) 
	{
        // If we could not get an interface to the clock, don't use one.  
        m_ReferenceClock = NULL;
    }

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::_AllocateResources] - ReferenceClock: %p", m_ReferenceClock));

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_FreeResources()
 *****************************************************************************
 *//*!
 * @brief
 * Free resources allocated to the stream.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_FreeResources
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_FreeResources]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_ReferenceClock) 
	{
        m_ReferenceClock->Release();
        m_ReferenceClock = NULL;
    }

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_FreeClonePointers()
 *****************************************************************************
 *//*!
 * @brief
 * Free the clone pointers.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_FreeClonePointers
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_FreeClonePointers]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	KsPinAcquireProcessingMutex(m_KsPin);

	// Walk through the clones, deleting them, and setting DataUsed to
    // zero since we didn't use any data!
    PKSSTREAM_POINTER ClonePointer = KsPinGetFirstCloneStreamPointer(m_KsPin);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_FreeClonePointers] - FirstClone: %p", ClonePointer));

    while (ClonePointer) 
	{
        PKSSTREAM_POINTER NextClonePointer = KsStreamPointerGetNextClone(ClonePointer);

		if (m_Capture)
		{
			ClonePointer->StreamHeader->DataUsed = 0;
		}

		KsStreamPointerSetStatusCode(ClonePointer, STATUS_CANCELLED);

        KsStreamPointerDelete(ClonePointer);

        ClonePointer = NextClonePointer;
    }

	m_PreviousClonePointer = NULL;

	KsPinReleaseProcessingMutex(m_KsPin);

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_Run()
 *****************************************************************************
 *//*!
 * @brief
 * Start the stream render/capture operation.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_Run
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_Run]"));
	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_MidiClient)
	{
		ntStatus = m_MidiClient->Start(&m_StartTimeStampCounter, &m_TimeStampFrequency);
	}
    
	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_Pause()
 *****************************************************************************
 *//*!
 * @brief
 * Pause the stream render/capture operation.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_Pause
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_Pause]"));

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_MidiClient)
	{
		ntStatus = m_MidiClient->Pause();
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_Stop()
 *****************************************************************************
 *//*!
 * @brief
 * Stop the stream render/capture operation.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_Stop
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_Stop]"));

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_MidiClient)
	{
		ntStatus = m_MidiClient->Stop();
	}

	if (m_Capture)
	{
		#ifdef ENABLE_DIRECTMUSIC_SUPPORT
		if (m_DirectMusicFormat)
		{
			PKSSTREAM_POINTER LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);

			while (LeadingEdge) 
			{
				LPDMUS_EVENTHEADER EventHdr = (LPDMUS_EVENTHEADER)(LeadingEdge->StreamHeader->Data);

				EventHdr->cbEvent = 0; EventHdr->dwChannelGroup = 0;
				EventHdr->rtDelta = 0; EventHdr->dwFlags = 0;

				// Incomplete stream of data.
				LeadingEdge->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;

				KsStreamPointerAdvanceOffsetsAndUnlock(LeadingEdge, 0, DMUS_EVENT_SIZE(EventHdr->cbEvent), TRUE);

				LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);
			}
		}
		else
		#endif // ENABLE_DIRECTMUSIC_SUPPORT
		{
			PKSSTREAM_POINTER LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);

			while (LeadingEdge) 
			{
				PKSMUSICFORMAT Format = PKSMUSICFORMAT(LeadingEdge->StreamHeader->Data);

				// Incomplete stream of data.
				LeadingEdge->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;

				KsStreamPointerAdvanceOffsetsAndUnlock(LeadingEdge, 0, sizeof(KSMUSICFORMAT) + ((Format->ByteCount + 3) / 4) * 4/*DWORD_ALIGNED*/, TRUE);

				LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);
			}
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiPin::_Reset()
 *****************************************************************************
 *//*!
 * @brief
 * Reset the stream render/capture operation.
 * @param
 * None
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiPin::
_Reset
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiPin::_Reset]"));
	
	if (m_MidiClient)
	{
		m_MidiClient->Reset();
	}

	return STATUS_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiPin::_ProcessKsMusicFormat()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CMidiPin::
_ProcessKsMusicFormat
(	void
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::_ProcessKsMusicFormat]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	PKSSTREAM_POINTER LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);

	if (m_Capture)
	{
		while (NT_SUCCESS(ntStatus) && LeadingEdge) 
		{
			PKSMUSICFORMAT Format = PKSMUSICFORMAT(LeadingEdge->StreamHeader->Data);

			//
			// Validate the data header.
			//
			if (LeadingEdge->Offset->Count <= sizeof(KSMUSICFORMAT))
			{
				ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, 0, TRUE);
				continue;
			}

			PUCHAR Buffer = LeadingEdge->Offset->Data + sizeof(KSMUSICFORMAT);

			ULONG BufferLength = LeadingEdge->Offset->Count - sizeof(KSMUSICFORMAT);  
			
			// Limit buffer length to 3 bytes. HCT's MIDI Driver Test cases 16.10.19 and
			// 16.10.22 failed if it is > 3 bytes due to timing issue/Microsoft MIDI parser
			// implementation.
			if (BufferLength > 3) BufferLength = 3; 

			LONGLONG TimeStampCounter;

			//
			// Read the buffer.
			//
			ULONG BytesRead = m_MidiClient->ReadBuffer(Buffer, BufferLength, &TimeStampCounter,	NULL);

			_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::Process] - BytesRead: %d", BytesRead));
			
			if (BytesRead)
			{
				Format->TimeDeltaMs = ULONG((TimeStampCounter > m_StartTimeStampCounter) ? (DOUBLE(TimeStampCounter - m_StartTimeStampCounter) * 1000 / DOUBLE(m_TimeStampFrequency)) : 0); // ms;

				Format->ByteCount = BytesRead;

				//
				// Advance the leading edge. If we run off the end of the queue, 
				// ntStatus will be STATUS_DEVICE_NOT_READY.  Otherwise, the leading 
				// edge will point to a new frame.
				//
				KsStreamPointerAdvanceOffsetsAndUnlock(LeadingEdge, 0, sizeof(KSMUSICFORMAT) + ((Format->ByteCount + 3) / 4) * 4/*DWORD_ALIGNED*/, TRUE);

				LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);
			}
			else
			{
				//
				// The hardware was incapable of providing more bytes.  The FIFO 
				// buffer is empty.
				//
				ntStatus = STATUS_PENDING;
				break;
			}
		}
	}
	else
	{
		while (NT_SUCCESS(ntStatus) && LeadingEdge) 
		{
			//
			// KSMUSICFORMAT structure used to send information about MIDI data.
			//
			PKSMUSICFORMAT Format = PKSMUSICFORMAT(LeadingEdge->StreamHeader->Data);

			//
			// For optimization sake in this particular sample, I will only keep
			// one clone stream pointer per frame.  This complicates the logic
			// here but simplifies the completions.
			//
			//
			// First thing we need to do is clone the leading edge.  This allows
			// us to keep reference on the frames while they're in FIFO buffer.
			//
			PKSSTREAM_POINTER ClonePointer = NULL;

			if (m_PreviousClonePointer == NULL)
			{
				ntStatus = KsStreamPointerClone(LeadingEdge, NULL, sizeof(ULONG), &ClonePointer);

				//
				// If the clone failed, likely we're out of resources.  Break out
				// of the loop for now.  We may end up starving FIFO buffer.
				//
				if (!NT_SUCCESS(ntStatus)) 
				{
					KsStreamPointerUnlock(LeadingEdge, FALSE);
					break;
				}

				//
				// Validate the data header.
				//
				if (LeadingEdge->StreamHeader->FrameExtent >= sizeof(KSMUSICFORMAT))
				{
					if (LeadingEdge->Offset->Count >= (sizeof(KSMUSICFORMAT) + Format->ByteCount))
					{
						// Past the KSMUSICFORMAT header...
						KsStreamPointerAdvanceOffsets(LeadingEdge, sizeof(KSMUSICFORMAT), 0, FALSE);

						KsStreamPointerAdvanceOffsets(ClonePointer, sizeof(KSMUSICFORMAT), 0, FALSE);

						*(PULONG(ClonePointer->Context)) = Format->ByteCount;
					}
					else
					{
						// An invalid packet... goto next frame.
						ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, 0, TRUE);

						KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);

						KsStreamPointerDelete(ClonePointer);
						continue;
					}
				}
				else
				{
					// An invalid packet... goto next frame.
					ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, 0, TRUE);

					KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);

					KsStreamPointerDelete(ClonePointer);
					continue;
				}
			}
			else
			{
				ClonePointer = m_PreviousClonePointer;

				ntStatus = STATUS_SUCCESS;
			}

			PULONG BytesAvailable = PULONG(ClonePointer->Context);

			ULONG BytesToWrite = *BytesAvailable;

			LONGLONG TimeStampCounter = m_StartTimeStampCounter + LONGLONG(DOUBLE(Format->TimeDeltaMs) / 1000 * DOUBLE(m_TimeStampFrequency));

			//
			// Write to the hardware buffer.  I would use ClonePointer->Offset.*, 
			// but because of the optimization of one stream pointer per frame, it
			// doesn't make complete sense.
			//
			ULONG BytesWritten = BytesToWrite ? m_MidiClient->WriteBuffer(LeadingEdge->Offset->Data, BytesToWrite, TimeStampCounter, m_SynchronousMode) : 0;

			_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::Process] - BytesWritten: %d", BytesWritten));
			
			//
			// In order to keep one clone per frame and simplify the logic, make a 
			// check to see if we completely used the mappings in the leading edge.  
			// Set a flag.
			//
			if (BytesWritten == BytesToWrite) 
			{
				m_PreviousClonePointer = NULL;

				// Finish and done with this buffer... so advance by the remaining bytes left
				// in the leading edge.
				ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, LeadingEdge->Offset->Remaining, 0, FALSE);

				KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);
				//
				// All of the bytes in this clone have been completed. 
				// Delete the clone. 
				//
				KsStreamPointerDelete(ClonePointer);
			} 
			else 
			{
				m_PreviousClonePointer = ClonePointer;

				if (BytesWritten)
				{
					//
					// Advance the leading edge. If we run off the end of the queue, 
					// ntStatus will be STATUS_DEVICE_NOT_READY.  Otherwise, the leading 
					// edge will point to a new frame.  The previous one will not have been
					// dismissed (unless it is completed) since there's a clone pointer 
					// referencing the frames.
					//
					ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, BytesWritten, 0, FALSE);

					KsStreamPointerAdvanceOffsets(ClonePointer, BytesWritten, 0, FALSE);

					*BytesAvailable -= BytesWritten;
				}
				else
				{
					//
					// The hardware was incapable of accepting more bytes.  The FIFO 
					// buffer is full.
					//
					ntStatus = STATUS_PENDING;
					break;
				}
			}
		}
	}

    //
    // If the leading edge failed to lock (this is always possible, remember
    // that locking CAN occassionally fail), don't blow up passing NULL
    // into KsStreamPointerUnlock.  Also, set m_PendIo to kick us later...
    //
    if (!LeadingEdge) 
	{
        m_PendingIo = TRUE;

        //
        // If the lock failed, there's no point in getting called back 
        // immediately.  The lock could fail due to insufficient memory,
        // etc...  In this case, we don't want to get called back immediately.
        // Return pending.  The m_PendingIo flag will cause us to get kicked
        // later.
        //
        ntStatus = STATUS_PENDING;
    }

    //
    // If we didn't run the leading edge off the end of the queue, unlock it.
    //
    if (NT_SUCCESS(ntStatus) && LeadingEdge) 
	{
        KsStreamPointerUnlock(LeadingEdge, FALSE);
    } 
	else 
	{
        //
        // DEVICE_NOT_READY indicates that the advancement ran off the end
        // of the queue.  We couldn't lock the leading edge.
        //
        if (ntStatus == STATUS_DEVICE_NOT_READY) 
		{
			ntStatus = STATUS_SUCCESS;
		}
    }

    //
    // If we failed with something that requires pending, set the pending I/O
    // flag so we know we need to start it again in a completion DPC.
    //
    if (!NT_SUCCESS(ntStatus) || (ntStatus == STATUS_PENDING)) 
	{
        m_PendingIo = TRUE;
    }

	return ntStatus;
}

#ifdef ENABLE_DIRECTMUSIC_SUPPORT
/*****************************************************************************
 * CMidiPin::_ProcessDirectMusicFormat()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CMidiPin::
_ProcessDirectMusicFormat
(	void
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::_ProcessDirectMusicFormat]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	PKSSTREAM_POINTER LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);

	if (m_Capture)
	{
		while (NT_SUCCESS(ntStatus) && LeadingEdge) 
		{
			LPDMUS_EVENTHEADER EventHdr = (LPDMUS_EVENTHEADER)(LeadingEdge->StreamHeader->Data);

			//
			// Validate the data header.
			//
			if (LeadingEdge->Offset->Count <= sizeof(DMUS_EVENTHEADER))
			{
				ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, 0, TRUE);
				continue;
			}

			PUCHAR Buffer = LeadingEdge->Offset->Data + sizeof(DMUS_EVENTHEADER);

			ULONG BufferLength = LeadingEdge->Offset->Count - sizeof(DMUS_EVENTHEADER);

			LONGLONG TimeStampCounter;

			ULONG Status = 0;

			//
			// Read the buffer.
			//
			ULONG BytesRead = m_MidiClient->ReadBuffer(Buffer, BufferLength, &TimeStampCounter,	&Status);

			_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::Process] - BytesRead: %d", BytesRead));
			
			if (BytesRead)
			{
				// You might wonder why there is a delay here for non-structured messages (ie SysEx).
				// Normally, the SysEx messages is packaged in 44-bytes packet (why 44-bytes ? Go read
				// the documentation at http://earthvegaconnection.com/evc/products/miditest/developers.html).
				// This is to avoid the DirectMusic DLL repackaging the MIDI data as the time between 2 
				// 44-bytes packets is usually sufficient enough for the packets to be delivered to application.
				// If the time difference is insuffficient and 2 packets arrived in close proximity to one
				// another, DirectMusic DLL will try to repackage them, leading to swapped packet order or 
				// corrupted MIDI packets.
				// What the code below do is to check if <= 3 bytes are received, it add an explicit delay of
				// 1ms so that the current packet will not arrived together with the previous packet.
				if ((Status & MESSAGE_STATUS_STRUCTURED) == 0)
				{
					if ((BufferLength > 3) && (BytesRead <= 3))
					{
						KEVENT Event; KeInitializeEvent(&Event, NotificationEvent, FALSE);

						LARGE_INTEGER TimeOut; TimeOut.QuadPart = -1*10000; // 1ms
						
						KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeOut);
					}
				}

				EventHdr->cbEvent = BytesRead;

				EventHdr->dwChannelGroup = 0;

				EventHdr->rtDelta = 0;
				
				EventHdr->dwFlags = (Status & MESSAGE_STATUS_STRUCTURED) ? DMUS_EVENT_STRUCTURED : 0;

				LeadingEdge->StreamHeader->PresentationTime.Time = LONGLONG(DOUBLE(TimeStampCounter) * 10000000 / DOUBLE(m_TimeStampFrequency)); // 100ns;
				LeadingEdge->StreamHeader->PresentationTime.Numerator = 1;
				LeadingEdge->StreamHeader->PresentationTime.Denominator = 1;
				LeadingEdge->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID;

				//
				// Advance the leading edge. If we run off the end of the queue, 
				// ntStatus will be STATUS_DEVICE_NOT_READY.  Otherwise, the leading 
				// edge will point to a new frame.
				//
				KsStreamPointerAdvanceOffsetsAndUnlock(LeadingEdge, 0, DMUS_EVENT_SIZE(EventHdr->cbEvent), TRUE);

				LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);
			}
			else
			{
				//
				// The hardware was incapable of providing more bytes.  The FIFO 
				// buffer is empty.
				//
				ntStatus = STATUS_PENDING;
				break;
			}
		}
	}
	else
	{
		while (NT_SUCCESS(ntStatus) && LeadingEdge) 
		{
			//
			// DMUS_EVENTHEADER structure used to send information about MIDI data.
			//
			LPDMUS_EVENTHEADER EventHdr = (LPDMUS_EVENTHEADER)(LeadingEdge->StreamHeader->Data);

			//
			// For optimization sake in this particular sample, I will only keep
			// one clone stream pointer per frame.  This complicates the logic
			// here but simplifies the completions.
			//
			//
			// First thing we need to do is clone the leading edge.  This allows
			// us to keep reference on the frames while they're in FIFO buffer.
			//
			PKSSTREAM_POINTER ClonePointer = NULL;

			if (m_PreviousClonePointer == NULL)
			{
				ntStatus = KsStreamPointerClone(LeadingEdge, NULL, sizeof(ULONG), &ClonePointer);

				//
				// If the clone failed, likely we're out of resources.  Break out
				// of the loop for now.  We may end up starving FIFO buffer.
				//
				if (!NT_SUCCESS(ntStatus)) 
				{
					KsStreamPointerUnlock(LeadingEdge, FALSE);
					break;
				}

				//
				// Validate the data header.
				//
				if (LeadingEdge->StreamHeader->FrameExtent >= sizeof(DMUS_EVENTHEADER))
				{
					if (LeadingEdge->Offset->Count >= (sizeof(DMUS_EVENTHEADER) + EventHdr->cbEvent))
					{
						// Past the DMUS_EVENTHEADER header...
						KsStreamPointerAdvanceOffsets(LeadingEdge, sizeof(DMUS_EVENTHEADER), 0, FALSE);

						KsStreamPointerAdvanceOffsets(ClonePointer, sizeof(DMUS_EVENTHEADER), 0, FALSE);

						*(PULONG(ClonePointer->Context)) = EventHdr->cbEvent;
					}
					else
					{
						// An invalid packet... goto next frame.
						ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, 0, TRUE);

						KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);

						KsStreamPointerDelete(ClonePointer);
						continue;
					}
				}
				else
				{
					// An invalid packet... goto next frame.
					ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, 0, TRUE);

					KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);

					KsStreamPointerDelete(ClonePointer);
					continue;
				}
			}
			else
			{
				ClonePointer = m_PreviousClonePointer;

				ntStatus = STATUS_SUCCESS;
			}

			PULONG BytesAvailable = PULONG(ClonePointer->Context);

			ULONG BytesToWrite = *BytesAvailable;

			LONGLONG TimeStampCounter = LeadingEdge->StreamHeader->PresentationTime.Time + LONGLONG(DOUBLE(EventHdr->rtDelta) / 10000000 * DOUBLE(m_TimeStampFrequency));

			//
			// Write to the hardware buffer.  I would use ClonePointer->Offset.*, 
			// but because of the optimization of one stream pointer per frame, it
			// doesn't make complete sense.
			//
			ULONG BytesWritten = BytesToWrite ? m_MidiClient->WriteBuffer(LeadingEdge->Offset->Data, BytesToWrite, TimeStampCounter, m_SynchronousMode) : 0;

			_DbgPrintF(DEBUGLVL_BLAB,("[CMidiPin::Process] - BytesWritten: %d", BytesWritten));
			
			//
			// In order to keep one clone per frame and simplify the logic, make a 
			// check to see if we completely used the mappings in the leading edge.  
			// Set a flag.
			//
			if (BytesWritten == BytesToWrite) 
			{
				m_PreviousClonePointer = NULL;

				// Finish and done with this buffer... so advance by the remaining bytes left
				// in the leading edge.
				ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, LeadingEdge->Offset->Remaining, 0, FALSE);

				KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);
				//
				// All of the bytes in this clone have been completed. 
				// Delete the clone. 
				//
				KsStreamPointerDelete(ClonePointer);
			} 
			else 
			{
				m_PreviousClonePointer = ClonePointer;

				if (BytesWritten)
				{
					//
					// Advance the leading edge. If we run off the end of the queue, 
					// ntStatus will be STATUS_DEVICE_NOT_READY.  Otherwise, the leading 
					// edge will point to a new frame.  The previous one will not have been
					// dismissed (unless it is completed) since there's a clone pointer 
					// referencing the frames.
					//
					ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, BytesWritten, 0, FALSE);

					KsStreamPointerAdvanceOffsets(ClonePointer, BytesWritten, 0, FALSE);

					*BytesAvailable -= BytesWritten;
				}
				else
				{
					//
					// The hardware was incapable of accepting more bytes.  The FIFO 
					// buffer is full.
					//
					ntStatus = STATUS_PENDING;
					break;
				}
			}
		}
	}

    //
    // If the leading edge failed to lock (this is always possible, remember
    // that locking CAN occassionally fail), don't blow up passing NULL
    // into KsStreamPointerUnlock.  Also, set m_PendIo to kick us later...
    //
    if (!LeadingEdge) 
	{
        m_PendingIo = TRUE;

        //
        // If the lock failed, there's no point in getting called back 
        // immediately.  The lock could fail due to insufficient memory,
        // etc...  In this case, we don't want to get called back immediately.
        // Return pending.  The m_PendingIo flag will cause us to get kicked
        // later.
        //
        ntStatus = STATUS_PENDING;
    }

    //
    // If we didn't run the leading edge off the end of the queue, unlock it.
    //
    if (NT_SUCCESS(ntStatus) && LeadingEdge) 
	{
        KsStreamPointerUnlock(LeadingEdge, FALSE);
    } 
	else 
	{
        //
        // DEVICE_NOT_READY indicates that the advancement ran off the end
        // of the queue.  We couldn't lock the leading edge.
        //
        if (ntStatus == STATUS_DEVICE_NOT_READY) 
		{
			ntStatus = STATUS_SUCCESS;
		}
    }

    //
    // If we failed with something that requires pending, set the pending I/O
    // flag so we know we need to start it again in a completion DPC.
    //
    if (!NT_SUCCESS(ntStatus) || (ntStatus == STATUS_PENDING)) 
	{
        m_PendingIo = TRUE;
    }

	return ntStatus;
}
#endif // ENABLE_DIRECTMUSIC_SUPPORT

#pragma code_seg()

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
 * @file       Pin.cpp
 * @brief      Audio pin implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "Pin.h"

//#include "dbgbuffer.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "AUDIO_PIN: "

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::DispatchTable
 *****************************************************************************
 *//*!
 * @brief
 * This is the dispatch table for the audio pin.  It provides notifications
 * about creation, closure, processing, data formats, etc...
 */
KSPIN_DISPATCH
CAudioPin::DispatchTable =
{
    CAudioPin::DispatchCreate,				// Pin Create
    NULL,                                   // Pin Close
    CAudioPin::DispatchProcess,				// Pin Process
    NULL,                                   // Pin Reset
    CAudioPin::DispatchSetFormat,			// Pin Set Data Format
    CAudioPin::DispatchSetState,			// Pin Set Device State
    NULL,                                   // Pin Connect
    NULL,                                   // Pin Disconnect
    NULL,                                   // Clock Dispatch
    NULL                                    // Allocator Dispatch
};

/*****************************************************************************
 * CAudioPin::DispatchCreate()
 *****************************************************************************
 *//*!
 * @brief
 * This is the Create dispatch for the pin.  It creates the CAudioPin object
 * and associates it with the AVStream object, bagging it in the process.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioPin::
DispatchCreate
(
    IN		PKSPIN		KsPin,
	IN		PIRP		Irp
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchCreate]"));

	NTSTATUS ntStatus = STATUS_DEVICE_NOT_CONNECTED;

    CAudioFilter * AudioFilter = (CAudioFilter *)(KsPin->Context); // only at init.

	if (AudioFilter->m_KsAdapter->IsReadyForIO())
	{
		CAudioPin * Pin = new(NonPagedPool,'aChS') CAudioPin(NULL);

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

				ntStatus = KsAddItemToObjectBag(KsPin->Bag, Pin, (PFNKSFREE)CAudioPin::Destruct);

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
 * CAudioPin::DispatchSetState()
 *****************************************************************************
 *//*!
 * @brief
 * This is the set device state dispatch for the pin.  The routine bridges
 * to SetState() in the context of the CAudioPin.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioPin::
DispatchSetState
(
	IN		PKSPIN		KsPin,
    IN		KSSTATE		ToState,
    IN		KSSTATE		FromState
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetState]"));

	CAudioPin * Pin = (CAudioPin *)(KsPin->Context);

	NTSTATUS ntStatus = Pin->SetState(ToState);

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::DispatchSetFormat()
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
CAudioPin::
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

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - Context: %p, OldFormat: %p", KsPin->Context, OldFormat));

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->FormatSize: %d", DataRange->FormatSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->Flags: %d", DataRange->Flags));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->SampleSize: %d", DataRange->SampleSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->Reserved: %d", DataRange->Reserved));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->MajorFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								DataRange->MajorFormat.Data1, DataRange->MajorFormat.Data2, DataRange->MajorFormat.Data3,
								DataRange->MajorFormat.Data4[0], DataRange->MajorFormat.Data4[1], DataRange->MajorFormat.Data4[2], DataRange->MajorFormat.Data4[3],
								DataRange->MajorFormat.Data4[4], DataRange->MajorFormat.Data4[5], DataRange->MajorFormat.Data4[6], DataRange->MajorFormat.Data4[7],
								DataRange->MajorFormat.Data4[8], DataRange->MajorFormat.Data4[9], DataRange->MajorFormat.Data4[10], DataRange->MajorFormat.Data4[11],
								DataRange->MajorFormat.Data4[12], DataRange->MajorFormat.Data4[13], DataRange->MajorFormat.Data4[14], DataRange->MajorFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->SubFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								DataRange->SubFormat.Data1, DataRange->SubFormat.Data2, DataRange->SubFormat.Data3,
								DataRange->SubFormat.Data4[0], DataRange->SubFormat.Data4[1], DataRange->SubFormat.Data4[2], DataRange->SubFormat.Data4[3],
								DataRange->SubFormat.Data4[4], DataRange->SubFormat.Data4[5], DataRange->SubFormat.Data4[6], DataRange->SubFormat.Data4[7],
								DataRange->SubFormat.Data4[8], DataRange->SubFormat.Data4[9], DataRange->SubFormat.Data4[10], DataRange->SubFormat.Data4[11],
								DataRange->SubFormat.Data4[12], DataRange->SubFormat.Data4[13], DataRange->SubFormat.Data4[14], DataRange->SubFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - DataRange->Specifier: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								DataRange->Specifier.Data1, DataRange->Specifier.Data2, DataRange->Specifier.Data3,
								DataRange->Specifier.Data4[0], DataRange->Specifier.Data4[1], DataRange->Specifier.Data4[2], DataRange->Specifier.Data4[3],
								DataRange->Specifier.Data4[4], DataRange->Specifier.Data4[5], DataRange->Specifier.Data4[6], DataRange->Specifier.Data4[7],
								DataRange->Specifier.Data4[8], DataRange->Specifier.Data4[9], DataRange->Specifier.Data4[10], DataRange->Specifier.Data4[11],
								DataRange->Specifier.Data4[12], DataRange->Specifier.Data4[13], DataRange->Specifier.Data4[14], DataRange->Specifier.Data4[15]
								));

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->FormatSize: %d", KsPin->ConnectionFormat->FormatSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->Flags: %d", KsPin->ConnectionFormat->Flags));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->SampleSize: %d", KsPin->ConnectionFormat->SampleSize));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->Reserved: %d", KsPin->ConnectionFormat->Reserved));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->MajorFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								KsPin->ConnectionFormat->MajorFormat.Data1, KsPin->ConnectionFormat->MajorFormat.Data2, KsPin->ConnectionFormat->MajorFormat.Data3,
								KsPin->ConnectionFormat->MajorFormat.Data4[0], KsPin->ConnectionFormat->MajorFormat.Data4[1], KsPin->ConnectionFormat->MajorFormat.Data4[2], KsPin->ConnectionFormat->MajorFormat.Data4[3],
								KsPin->ConnectionFormat->MajorFormat.Data4[4], KsPin->ConnectionFormat->MajorFormat.Data4[5], KsPin->ConnectionFormat->MajorFormat.Data4[6], KsPin->ConnectionFormat->MajorFormat.Data4[7],
								KsPin->ConnectionFormat->MajorFormat.Data4[8], KsPin->ConnectionFormat->MajorFormat.Data4[9], KsPin->ConnectionFormat->MajorFormat.Data4[10], KsPin->ConnectionFormat->MajorFormat.Data4[11],
								KsPin->ConnectionFormat->MajorFormat.Data4[12], KsPin->ConnectionFormat->MajorFormat.Data4[13], KsPin->ConnectionFormat->MajorFormat.Data4[14], KsPin->ConnectionFormat->MajorFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->SubFormat: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								KsPin->ConnectionFormat->SubFormat.Data1, KsPin->ConnectionFormat->SubFormat.Data2, KsPin->ConnectionFormat->SubFormat.Data3,
								KsPin->ConnectionFormat->SubFormat.Data4[0], KsPin->ConnectionFormat->SubFormat.Data4[1], KsPin->ConnectionFormat->SubFormat.Data4[2], KsPin->ConnectionFormat->SubFormat.Data4[3],
								KsPin->ConnectionFormat->SubFormat.Data4[4], KsPin->ConnectionFormat->SubFormat.Data4[5], KsPin->ConnectionFormat->SubFormat.Data4[6], KsPin->ConnectionFormat->SubFormat.Data4[7],
								KsPin->ConnectionFormat->SubFormat.Data4[8], KsPin->ConnectionFormat->SubFormat.Data4[9], KsPin->ConnectionFormat->SubFormat.Data4[10], KsPin->ConnectionFormat->SubFormat.Data4[11],
								KsPin->ConnectionFormat->SubFormat.Data4[12], KsPin->ConnectionFormat->SubFormat.Data4[13], KsPin->ConnectionFormat->SubFormat.Data4[14], KsPin->ConnectionFormat->SubFormat.Data4[15]
								));
	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchSetFormat] - KsPin->ConnectionFormat->Specifier: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
								KsPin->ConnectionFormat->Specifier.Data1, KsPin->ConnectionFormat->Specifier.Data2, DataRange->Specifier.Data3,
								KsPin->ConnectionFormat->Specifier.Data4[0], KsPin->ConnectionFormat->Specifier.Data4[1], KsPin->ConnectionFormat->Specifier.Data4[2], KsPin->ConnectionFormat->Specifier.Data4[3],
								KsPin->ConnectionFormat->Specifier.Data4[4], KsPin->ConnectionFormat->Specifier.Data4[5], KsPin->ConnectionFormat->Specifier.Data4[6], KsPin->ConnectionFormat->Specifier.Data4[7],
								KsPin->ConnectionFormat->Specifier.Data4[8], KsPin->ConnectionFormat->Specifier.Data4[9], KsPin->ConnectionFormat->Specifier.Data4[10], KsPin->ConnectionFormat->Specifier.Data4[11],
								KsPin->ConnectionFormat->Specifier.Data4[12], KsPin->ConnectionFormat->Specifier.Data4[13], KsPin->ConnectionFormat->Specifier.Data4[14], KsPin->ConnectionFormat->Specifier.Data4[15]
								));

    if ((!IsEqualGUIDAligned(DataRange->MajorFormat, KsPin->ConnectionFormat->MajorFormat) &&
         !IsEqualGUIDAligned(DataRange->MajorFormat, KSDATAFORMAT_TYPE_WILDCARD)) ||
		(!IsEqualGUIDAligned(DataRange->SubFormat, KsPin->ConnectionFormat->SubFormat) &&
		 !IsEqualGUIDAligned(DataRange->SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD)) ||
		(!IsEqualGUIDAligned(DataRange->Specifier, KsPin->ConnectionFormat->Specifier) &&
		 !IsEqualGUIDAligned(DataRange->Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD)))
    {
        ntStatus = STATUS_NO_MATCH;
    }

	if (NT_SUCCESS(ntStatus))
	{
		if (OldFormat)
		{
			// DispatchCreate is already done.
			CAudioPin * Pin = (CAudioPin*)(KsPin->Context);

			ntStatus = Pin->SetFormat(PKSDATAFORMAT(KsPin->ConnectionFormat));
		}
		else
		{
			CAudioFilter * Filter = (CAudioFilter*)(KsPin->Context);

			ntStatus = Filter->ValidateFormat(KsPin->Id, (KsPin->DataFlow == KSPIN_DATAFLOW_OUT), PKSDATAFORMAT(KsPin->ConnectionFormat));
		}
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioPin::DispatchProcess()
 *****************************************************************************
 *//*!
 * @brief
 * This is the processing dispatch for the audio pin.  The routine
 * bridges to Process() in the context of the CAudioPin.
 * @param
 * KsPin Pointer to the KSPIN structure representing the AVStream pin.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioPin::
DispatchProcess
(
	IN		PKSPIN	KsPin
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::DispatchProcess]"));

	CAudioPin * Pin = (CAudioPin *)(KsPin->Context);

	NTSTATUS ntStatus = Pin->Process();

	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::IntersectHandler()
 *****************************************************************************
 *//*!
 * @brief
 * This is the data intersection handler for the audio pin.  This
 * determines an optimal format in the intersection of two ranges,
 * one local and one possibly foreign.  If there is no compatible format,
 * STATUS_NO_MATCH is returned.
 * @param
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioPin::
IntersectHandler
(
    IN		PKSFILTER		KsFilter,
    IN		PIRP			Irp,
    IN		PKSP_PIN		PinInstance,
    IN		PKSDATARANGE	CallerDataRange,
    IN		PKSDATARANGE	DescriptorDataRange,
    IN		ULONG			BufferSize,
    OUT		PVOID			Data OPTIONAL,
    OUT		PULONG			DataSize
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::IntersectHandler]"));

	CAudioFilter * Filter = (CAudioFilter*)(KsFilter->Context);

	NTSTATUS ntStatus = Filter->DataRangeIntersection
						(
							PinInstance->PinId,
							CallerDataRange,
							DescriptorDataRange,
							BufferSize,
							Data,
							DataSize
						);

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SleepCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioPin::
SleepCallback 
(
	IN		PKSPIN				KsPin,
	IN		DEVICE_POWER_STATE	PowerState
)
{
	PAGED_CODE();

	CAudioPin * Pin = (CAudioPin *)(KsPin->Context);

	Pin->PowerChangeNotify(PowerState);
}

/*****************************************************************************
 * CAudioPin::WakeCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioPin::
WakeCallback 
(
	IN		PKSPIN				KsPin,
	IN		DEVICE_POWER_STATE	PowerState
)
{
	PAGED_CODE();

	CAudioPin * Pin = (CAudioPin *)(KsPin->Context);

	Pin->PowerChangeNotify(PowerState);
}

/*****************************************************************************
 * CAudioPin::AllocatorFraming
 *****************************************************************************
 *//*!
 * @brief
 * This is the simple framing structure for the audio pin.  Note that this
 * will be modified via KsEdit when the actual format is determined.
 */
DECLARE_SIMPLE_FRAMING_EX
(
	CAudioPin::AllocatorFraming,
	STATICGUIDOF(KSMEMORY_TYPE_KERNEL_NONPAGED),
	KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
	8,
	FILE_LONG_ALIGNMENT,
	2 * PAGE_SIZE,
	2 * PAGE_SIZE
);


/*****************************************************************************
 * CAudioPin::Interfaces[]
 *****************************************************************************
 *//*!
 * @brief
 */
KSPIN_INTERFACE
CAudioPin::
Interfaces[2] =
{
	{ 
		STATICGUIDOF(KSINTERFACESETID_Standard), 
		KSINTERFACE_STANDARD_STREAMING, 
		0 
	},
	{ 
		STATICGUIDOF(KSINTERFACESETID_Standard),
		KSINTERFACE_STANDARD_LOOPED_STREAMING, 
		0 
	}
};

/*****************************************************************************
 * CAudioPin::Destruct()
 *****************************************************************************
 *//*!
 * @brief
 * This is the free callback for the bagged filter.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CAudioPin object.
 * @return
 * None.
 */
VOID
CAudioPin::
Destruct
(
	IN		PVOID	Self
)
{
    PAGED_CODE();

	CAudioPin * Pin = (CAudioPin *)(Self);

	Pin->Release();
}

/*****************************************************************************
 * CAudioPin::NonDelegatingQueryInterface()
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
CAudioPin::
NonDelegatingQueryInterface
(
    IN      REFIID  Interface,
    OUT     PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::NonDelegatingQueryInterface]"));

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
 * CAudioPin::~CAudioPin()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CAudioPin::
~CAudioPin
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::~CAudioPin]"));

	// Restore the default DRM rights if DRM was asserted.
	if (m_DrmRights.CopyProtect || m_DrmRights.DigitalOutputDisable)
	{
		// Get the filter where the pin is implemented.
		PKSFILTER KsFilter = KsPinGetParentFilter(m_KsPin);

		KsFilterAcquireControl(KsFilter);

		// Default DRM rights.
		DEFINE_DRMRIGHTS_DEFAULT(DefaultDrmRights);

		_EnforceDrmRights(KsFilter, m_DrmRights, DefaultDrmRights);

		KsFilterReleaseControl(KsFilter);
	}

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

	ExSetTimerResolution(10000, FALSE);

	if (m_AudioClient)
	{
		m_AudioDevice->Close(m_AudioClient);
	}

	if (m_AudioFilter)
    {
		m_AudioFilter->m_KsAdapter->DereferenceDevice();

        m_AudioFilter->Release();
    }

	//spewdbgbuffer();
}

/*****************************************************************************
 * CAudioPin::Init()
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
CAudioPin::
Init
(
	IN		PKSPIN	KsPin
)
{
    PAGED_CODE();

	//resetdbgbuffer();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::Init]"));

    ASSERT(KsPin);

    ASSERT(KsPin->ConnectionFormat);

	m_KsPin = KsPin;

    m_AudioFilter = (CAudioFilter *)(KsPin->Context); // only at init.
    m_AudioFilter->AddRef();

	ASSERT(NT_SUCCESS(m_AudioFilter->ValidateFormat(KsPin->Id, (KsPin->DataFlow == KSPIN_DATAFLOW_OUT), KsPin->ConnectionFormat)));

    m_PinId = KsPin->Id;

    m_State = KSSTATE_STOP;

	m_Capture = (KsPin->DataFlow == KSPIN_DATAFLOW_OUT); // Out from the filter into the host.

	m_DevicePowerState = PowerDeviceD0;

	m_AudioDevice = m_AudioFilter->m_AudioDevice;

	m_AudioFilter->m_KsAdapter->ReferenceDevice();

	KeInitializeSpinLock(&m_ProcessingLock);

	// Currently, arbitrary select 10ms as the frame target size.
	m_NotificationInterval = 10;

	ExSetTimerResolution(10000, TRUE); // 1ms

	NTSTATUS ntStatus = m_AudioDevice->Open(NotificationRoutine, this, &m_AudioClient);

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = SetFormat(KsPin->ConnectionFormat);
	}

	if (NT_SUCCESS(ntStatus))
	{
		KsPinRegisterPowerCallbacks(m_KsPin, SleepCallback, WakeCallback);
	}

	if (!NT_SUCCESS(ntStatus))
    {
        // Clean up the mess
		m_AudioFilter->m_KsAdapter->DereferenceDevice();

		if (m_AudioClient)
		{
			m_AudioDevice->Close(m_AudioClient);
			m_AudioClient = NULL;
		}

        m_AudioFilter->Release();
        m_AudioFilter = NULL;
    }

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SetFormat()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the wave format of the stream.
 * @param
 * Format Pointer to a KSDATAFORMAT structure indicating the format to use for
 * this instance.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
SetFormat
(
    IN      PKSDATAFORMAT   Format
)
{
    PAGED_CODE();

    ASSERT(Format);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetFormat]"));

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    if (m_State != KSSTATE_RUN)
    {
        ntStatus = m_AudioFilter->ValidateFormat(m_PinId, m_Capture, Format);

        PWAVEFORMATEX waveFormat = NULL;

        if (NT_SUCCESS(ntStatus))
        {
            if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
            {
                waveFormat = PWAVEFORMATEX(Format +1);

                ntStatus = STATUS_SUCCESS;
            }
            else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
            {
                PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format + 1);

                waveFormat = &BufferDesc->WaveFormatEx;

                ntStatus = STATUS_SUCCESS;
            }
            else
            {
                ntStatus = STATUS_INVALID_PARAMETER;
            }
        }

		ULONG PreferredSampleRate = m_AudioFilter->GetPreferredSampleRate();

		if (NT_SUCCESS(ntStatus))
		{
			ULONG BitResolution = 0;

			ntStatus = _FindAudioBitResolution(m_PinId, m_Capture, Format, &BitResolution);

			if (NT_SUCCESS(ntStatus))
			{
				ntStatus = _FindAudioInterface(m_PinId, m_Capture, Format, BitResolution);
			}
		}

		if (NT_SUCCESS(ntStatus))
		{
			if (PreferredSampleRate != waveFormat->nSamplesPerSec)
			{
				m_AudioFilter->TriggerEvent(0); // Indicate that clock rate has changed.
			}
		}

		if (NT_SUCCESS(ntStatus)) 
		{        
			//
			// We need to edit the descriptor to ensure we don't mess up any other
			// pins using the descriptor or touch read-only memory.
			//
			ntStatus = KsEdit(m_KsPin, &m_KsPin->Descriptor, 'aChS');

			if (NT_SUCCESS(ntStatus)) 
			{
				ntStatus = KsEdit(m_KsPin, &m_KsPin->Descriptor->AllocatorFraming, 'aChS');
			}

			//
			// If the edits proceeded without running out of memory, adjust 
			// the framing based on the new format.
			//
			if (NT_SUCCESS(ntStatus)) 
			{
				//
				// We've KsEdit'ed this...  I'm safe to cast away constness as
				// long as the edit succeeded.
				//
				PKSALLOCATOR_FRAMING_EX Framing = PKSALLOCATOR_FRAMING_EX(m_KsPin->Descriptor->AllocatorFraming);

				Framing->FramingItem[0].Frames = 2;

				_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetFormat] - FramingItem[0].Flags: 0x%x", Framing->FramingItem[0].Flags));
				//
				// The physical and optimal ranges must be size of latency.  We only
				// support one frame size, precisely the size of the latency.
				//
				// Currently, arbitrary select 10ms as the frame target size.
				//
				Framing->FramingItem[0].PhysicalRange.MinFrameSize = 0;
				Framing->FramingItem[0].PhysicalRange.MaxFrameSize = 0xFFFFFFFF;
				Framing->FramingItem[0].PhysicalRange.Stepping = 1;

				Framing->FramingItem[0].FramingRange.Range.MinFrameSize =
				Framing->FramingItem[0].FramingRange.Range.MaxFrameSize =
					m_NotificationInterval * waveFormat->nSamplesPerSec * waveFormat->nChannels * (waveFormat->wBitsPerSample / 8) / 1000;	
				Framing->FramingItem[0].FramingRange.Range.Stepping = waveFormat->nChannels * (waveFormat->wBitsPerSample / 8);
			}
		}

		if (NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("  SampleRate: %d",waveFormat->nSamplesPerSec));

            m_FormatChannels    = waveFormat->nChannels;
            m_SampleSize        = waveFormat->wBitsPerSample;
            m_SamplingFrequency = waveFormat->nSamplesPerSec;

            // Find the actual subformat if it is extensible.
            GUID SubFormat = Format->SubFormat;
            
            if (IsEqualGUIDAligned(Format->SubFormat, KSDATAFORMAT_SUBTYPE_EXTENSIBLE))
            {
                if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
                {
                    PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)(Format + 1);
                    
                    SubFormat = WaveFormatExt->SubFormat;
                }
                else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
                {
                    PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format+1);
                    
                    PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;
                    
                    SubFormat = WaveFormatExt->SubFormat;
                }
            }

            m_NonPcmFormat = !IsEqualGUIDAligned(SubFormat, KSDATAFORMAT_SUBTYPE_PCM);
        }
    }

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::PowerChangeNotify()
 *****************************************************************************
 *//*!
 * @brief
 * Change power state for the device.
 * @details
 * The code for this method should reside in paged memory.
 * @param
 * NewPowerState Current power state. 
 * @return
 * None
 */
VOID
CAudioPin::
PowerChangeNotify
(
    IN      DEVICE_POWER_STATE	NewPowerState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::PowerChangeNotify]"));

    if (m_DevicePowerState != NewPowerState)
    {
        switch (m_DevicePowerState)
        {
            case PowerDeviceD0:
            {
                switch (NewPowerState)
                {
                    case PowerDeviceD1:
                    case PowerDeviceD2:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D0->D1/D2"));
                        // Power State Transition
						if (m_State == KSSTATE_RUN)
						{
							PKSGATE KsGate = KsPinGetAndGate(m_KsPin);
							
							if (KsGate)
							{
								KsGateTurnInputOff(KsGate);
							}

							_Stop();

							_FreeClonePointers();
						}
						// Restore the default DRM rights if DRM was asserted.
						if (m_DrmRights.CopyProtect || m_DrmRights.DigitalOutputDisable)
						{
							// Get the filter where the pin is implemented.
							PKSFILTER KsFilter = KsPinGetParentFilter(m_KsPin);

							// Default DRM rights.
							DEFINE_DRMRIGHTS_DEFAULT(DefaultDrmRights);

							_EnforceDrmRights(KsFilter, m_DrmRights, DefaultDrmRights);
						}
                        m_DevicePowerState = NewPowerState;
                        break;
                    case PowerDeviceD3:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D0->D3"));
                        // Power State Transition
						if (m_State == KSSTATE_RUN)
						{
							PKSGATE KsGate = KsPinGetAndGate(m_KsPin);

							if (KsGate)
							{
								KsGateTurnInputOff(KsGate);
							}
							
							_Stop();
							
							_FreeClonePointers();
						}
						// Restore the default DRM rights if DRM was asserted.
						if (m_DrmRights.CopyProtect || m_DrmRights.DigitalOutputDisable)
						{
							// Get the filter where the pin is implemented.
							PKSFILTER KsFilter = KsPinGetParentFilter(m_KsPin);

							// Default DRM rights.
							DEFINE_DRMRIGHTS_DEFAULT(DefaultDrmRights);

							_EnforceDrmRights(KsFilter, m_DrmRights, DefaultDrmRights);
						}
                        m_DevicePowerState = NewPowerState;
                        break;
                    default:
                        break;
                }
            }
            break;
            case PowerDeviceD1:
            case PowerDeviceD2:
            {
                switch (NewPowerState)
                {
                    case PowerDeviceD0:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D1/D2->D0"));
                        // Power State Transition
						// Restore the the current DRM rights if DRM was asserted.
						if (m_DrmRights.CopyProtect || m_DrmRights.DigitalOutputDisable)
						{
							// Get the filter where the pin is implemented.
							PKSFILTER KsFilter = KsPinGetParentFilter(m_KsPin);

							// Default DRM rights.
							DEFINE_DRMRIGHTS_DEFAULT(DefaultDrmRights);

							_EnforceDrmRights(KsFilter, DefaultDrmRights, m_DrmRights);
						}
						if (m_State == KSSTATE_RUN)
						{
							PKSGATE KsGate = KsPinGetAndGate(m_KsPin);

							if (KsGate)
							{
								KsGateTurnInputOn(KsGate);
							}
							
							if (m_AudioFilter->m_SynchronizeStart)
							{
								m_AudioFilter->m_UsbDevice->GetCurrentFrameNumber(&m_AudioFilter->m_StartFrameNumber);

								m_AudioFilter->m_StartFrameNumber += MIN_AUDIO_START_FRAME_OFFSET;
							}

							_Run();
						    
							KsPinAttemptProcessing(m_KsPin, TRUE);
						}
                        m_DevicePowerState = NewPowerState;
                        break;
                    default:
                        break;
                }
            }
            break;
            case PowerDeviceD3:
            {
                switch (NewPowerState)
                {
                    case PowerDeviceD0:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D3->D0"));
                        // Power State Transition
						// Restore the the current DRM rights if DRM was asserted.
						if (m_DrmRights.CopyProtect || m_DrmRights.DigitalOutputDisable)
						{
							// Get the filter where the pin is implemented.
							PKSFILTER KsFilter = KsPinGetParentFilter(m_KsPin);

							// Default DRM rights.
							DEFINE_DRMRIGHTS_DEFAULT(DefaultDrmRights);

							_EnforceDrmRights(KsFilter, DefaultDrmRights, m_DrmRights);
						}
						if (m_State == KSSTATE_RUN)
						{
							PKSGATE KsGate = KsPinGetAndGate(m_KsPin);

							if (KsGate)
							{
								KsGateTurnInputOn(KsGate);
							}
							
							if (m_AudioFilter->m_SynchronizeStart)
							{
								m_AudioFilter->m_UsbDevice->GetCurrentFrameNumber(&m_AudioFilter->m_StartFrameNumber);

								m_AudioFilter->m_StartFrameNumber += MIN_AUDIO_START_FRAME_OFFSET;
							}
							
							_Run();
						    
							KsPinAttemptProcessing(m_KsPin, TRUE);
						}
                        m_DevicePowerState = NewPowerState;
                        break;
                    default:
                        break;
                }
            }
            break;
            default:
                _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : Unknown current device power state"));
                break;
        }
    }
}

#pragma code_seg()

/*****************************************************************************
 * CAudioPin::GetAudioPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the current position of the stream.
 * @param
 * Position Pointer to a location to which the method outputs the current byte
 * position of the stream.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
GetAudioPosition
(
    OUT     PKSAUDIO_POSITION	OutPosition
)
{
    // Not PAGED_CODE().  May be called at dispatch level.

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::GetAudioPosition]"));

	ASSERT(OutPosition);

	NTSTATUS ntStatus = _GetPosition(&OutPosition->PlayOffset, &OutPosition->WriteOffset);

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SetAudioPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the current position of the stream.
 * @param
 * Position Position of the stream.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
SetAudioPosition
(
	IN		KSAUDIO_POSITION	Position
)
{
    // Not PAGED_CODE().  May be called at dispatch level.

	//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetAudioPosition]"));
	
	NTSTATUS ntStatus = _SetPosition(Position.PlayOffset, Position.WriteOffset);

    return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::GetLatency()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
GetLatency
(
    OUT     PKSTIME	OutLatency
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::GetLatency]"));

	if (OutLatency)
	{
		m_AudioFilter->GetLatency(m_PinId, m_SamplingFrequency, OutLatency);
	}

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioPin::SetDrmContentId()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
SetDrmContentId
(
	IN		ULONG		ContentId,
	IN		DRMRIGHTS	DrmRights
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetDrmContentId]"));

	// HACK: This is to disable DRM support for Hula & MicroPod.
	{
	PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor;

	m_AudioFilter->m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

	if ((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) && 
		((UsbDeviceDescriptor->idProduct == 0x3F02/*MicroPod*/) || 
		(UsbDeviceDescriptor->idProduct == 0x3F04/*HulaPod*/) ||
		(UsbDeviceDescriptor->idProduct == 0x3F0B/*Itey*/) || 
		(UsbDeviceDescriptor->idProduct == 0x3F0A/*MicroPre*/)))
	{
		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
	}
	}
	// ENDHACK.

	// This property should only be called on a sink pin. Check this out.
	if ((m_KsPin->Communication != KSPIN_COMMUNICATION_SINK) && 
		(m_KsPin->Communication != KSPIN_COMMUNICATION_BOTH))
	{
		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
	}

	if (m_KsPin->Communication != KSPIN_DATAFLOW_IN)
	{
		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
	}

	if (NT_SUCCESS(ntStatus))
	{
		// Get the filter where the pin is implemented.
		PKSFILTER KsFilter = KsPinGetParentFilter(m_KsPin);

		// Go thru the list of pins to get the source pins. To do it
		// safely, the control mutex need to be acquired.
		KsFilterAcquireControl(KsFilter);

		#if 0
		//TODO: Does the driver need to forward the DRM content id ? 
		//If it does, then the following need to be uncommented. Also drmk.sys
		//only available on 32-bit Windows platform, not 64-bit ones, so make sure
		//it is compiled for the 32-bit only if required.
		for (ULONG i=0; i<KsFilter->Descriptor->PinDescriptorsCount; i++)
		{
			// Look for source pins attached to the sink pin.
			PKSPIN KsPin = KsFilterGetFirstChildPin(KsFilter, i);

			while (KsPin)
			{
				if ((KsPin->Communication == KSPIN_DATAFLOW_OUT) &&
					((KsPin->Communication == KSPIN_COMMUNICATION_SOURCE) ||
					 (KsPin->Communication == KSPIN_COMMUNICATION_BOTH)))
				{
					// If the sink pin is connected to this pin, then forward the content to the 
					// lower attached device object.
					if (_IsPinsConnected(KsFilter, KSFILTER_NODE, m_PinId, KSFILTER_NODE, KsPin->PinId))
					{
						// Forward the content Id to the device object attached to the source pin.
						PFILE_OBJECT FileObject = KsPinGetConnectedPinFileObject(KsPin);
						PDEVICE_OBJECT DeviceObject = KsPinGetConnectedPinDeviceObject(KsPin);

						if (FileObject && DeviceObject)
						{
							DRMFORWARD DrmForward;

							DrmForward.Flags = 0;
							DrmForward.DeviceObject = DeviceObject;
							DrmForward.FileObject = FileObject;
							DrmForward.Context = PVOID(FileObject);

							//ntStatus = DrmForwardContentToDeviceObject(ContentId, NULL, &DrmForward);
						}
					}
				}

				if (!NT_SUCCESS(ntStatus))
				{
					break;
				}

				KsPin = KsPinGetNextSiblingPin(KsPin);
			}

			if (!NT_SUCCESS(ntStatus))
			{
				break;
			}
		}
		#endif // 0

		if (NT_SUCCESS(ntStatus))
		{
			// Try to honor the DRM rights bits.
			ntStatus = _EnforceDrmRights(KsFilter, m_DrmRights, DrmRights);

			if (NT_SUCCESS(ntStatus))
			{
				// New DRM rights asserted.
				m_DrmRights = DrmRights;
			}
			else
			{
				// Restore the previous DRM rights.
				_EnforceDrmRights(KsFilter, DrmRights, m_DrmRights);
			}
		}

		KsFilterReleaseControl(KsFilter);
	}

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SetState()
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
CAudioPin::
SetState
(
    IN      KSSTATE     NewState
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState]"));

	//dbgprintf("[CAudioPin::SetState] - %d\n", NewState);

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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_STOP->KSSTATE_STOP"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // Transition from KSSTATE_STOP to KSSTATE_ACQUIRE
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_STOP->KSSTATE_ACQUIRE"));

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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_STOP->KSSTATE_PAUSE is INVALID"));
                }
                break;

                case KSSTATE_RUN:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_STOP->KSSTATE_RUN is INVALID"));
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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_STOP"));

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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_ACQUIRE"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_PAUSE:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_PAUSE"));

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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_ACQUIRE->KSSTATE_RUN"));

					if (m_AudioFilter->m_KsAdapter->IsReadyForIO())
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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_PAUSE->KSSTATE_STOP is INVALID"));
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_PAUSE->KSSTATE_ACQUIRE"));

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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_PAUSE->KSSTATE_PAUSE"));

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_RUN:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_PAUSE->KSSTATE_RUN"));

                    // Start the hardware.
					if (m_AudioFilter->m_KsAdapter->IsReadyForIO())
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
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_RUN->KSSTATE_STOP is INVALID"));
                }
                break;

                case KSSTATE_ACQUIRE:
                {
                    // Invalid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_RUN->KSSTATE_ACQUIRE is INVALID"));
                }
                break;

                case KSSTATE_PAUSE:
                {
                    // Valid state transition.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_RUN->KSSTATE_PAUSE"));

                    // Pause the hardware.
                    _Pause(); //_Stop();

					// Free the clone pointers.
					_FreeClonePointers();

					// Update state.
                    m_State = NewState;

                    ntStatus = STATUS_SUCCESS;
                }
                break;

                case KSSTATE_RUN:
                {
                    // No state change.
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetState] : KSSTATE_RUN->KSSTATE_RUN"));

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
 * CAudioPin::Process()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
Process
(	void
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::Process]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	KIRQL OldIrql;

	KeAcquireSpinLock(&m_ProcessingLock, &OldIrql);

	PKSSTREAM_POINTER LeadingEdge = KsPinGetLeadingEdgeStreamPointer(m_KsPin, KSSTREAM_POINTER_STATE_LOCKED);

	if (m_Capture)
	{
		while (NT_SUCCESS(ntStatus) && LeadingEdge) 
		{
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

			if (!m_PreviousClonePointer)
			{
				ntStatus = KsStreamPointerClone(LeadingEdge, NULL, 0, &ClonePointer);

			}
			else
			{
				ClonePointer = m_PreviousClonePointer;

				ntStatus = STATUS_SUCCESS;
			}
			
			//
			// If the clone failed, likely we're out of resources.  Break out
			// of the loop for now.  We may end up starving FIFO buffer.
			//
			if (!NT_SUCCESS(ntStatus)) 
			{
				KsStreamPointerUnlock(LeadingEdge, FALSE);
				break;
			}

			// "Queue" the buffer.
			ULONG BytesQueued = m_AudioClient->QueueBuffer(LeadingEdge->Offset->Data, LeadingEdge->Offset->Remaining);

			//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::Process] - BytesQueued: %d", BytesQueued));
			
			//
			// In order to keep one clone per frame and simplify the logic, make a 
			// check to see if we completely used the mappings in the leading edge.  
			// Set a flag.
			//
			if (BytesQueued == LeadingEdge->Offset->Remaining) 
			{
				m_PreviousClonePointer = NULL;

				// Finish and done with this buffer... so advance by the remaining bytes left
				// in the leading edge.
				ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, LeadingEdge->Offset->Remaining, FALSE);

				// The clone pointer will be deleted later in the I/O completion routine.
			} 
			else 
			{
				m_PreviousClonePointer = ClonePointer;

				if (BytesQueued)
				{
					//
					// Advance the leading edge. If we run off the end of the queue, 
					// ntStatus will be STATUS_DEVICE_NOT_READY.  Otherwise, the leading 
					// edge will point to a new frame.  The previous one will not have been
					// dismissed (unless it is completed) since there's a clone pointer 
					// referencing the frames.
					//
					ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, 0, BytesQueued, FALSE);
				}
				else
				{
					//
					// The hardware was incapable of accepting more bytes.  The queue is full.
					//
					ntStatus = STATUS_PENDING;
					break;
				}
			}
		}
	}
	else
	{
		while (NT_SUCCESS(ntStatus) && LeadingEdge) 
		{
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

				if (NT_SUCCESS(ntStatus))
				{
					PULONG LoopOffset = PULONG(ClonePointer->Context); *LoopOffset = 0;
				}
			}
			else
			{
				ClonePointer = m_PreviousClonePointer;

				ntStatus = STATUS_SUCCESS;
			}

			//
			// If the clone failed, likely we're out of resources.  Break out
			// of the loop for now.  We may end up starving FIFO buffer.
			//
			if (!NT_SUCCESS(ntStatus)) 
			{
				KsStreamPointerUnlock(LeadingEdge, FALSE);
				break;
			}

			if ((LeadingEdge->Pin->ConnectionInterface.Id == KSINTERFACE_STANDARD_LOOPED_STREAMING) && (LeadingEdge->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA))
			{
				//
				// Looped streaming.
				//
				if (m_LoopedBufferSize == 0) 
				{
					m_LoopedBufferSize = LeadingEdge->StreamHeader->DataUsed;
				}

				PULONG LoopOffset = PULONG(ClonePointer->Context);

				//
				// Write to the hardware buffer.
				//
				ULONG BytesWritten = m_AudioClient->WriteBuffer(PUCHAR(LeadingEdge->Offset->Data) + *LoopOffset, LeadingEdge->StreamHeader->DataUsed - *LoopOffset);

				//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::Process] - BytesWritten: %d", BytesWritten));
				
				if (BytesWritten == (LeadingEdge->StreamHeader->DataUsed - *LoopOffset))
				{
					// Finished with this clone.
					m_PreviousClonePointer = NULL;

					// Wrap around.
					*LoopOffset = 0;
				}
				else
				{
					m_PreviousClonePointer = ClonePointer;
		
					if (BytesWritten)
					{
						// Advance the loop offset.
						*LoopOffset += BytesWritten;
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
			else
			{
				//
				// Standard streaming.
				//
				if (m_LoopedBufferSize) 
				{
					m_LoopedBufferSize = 0;
				}

				//
				// Write to the hardware buffer.  I would use ClonePointer->Offset.*, 
				// but because of the optimization of one stream pointer per frame, it
				// doesn't make complete sense.
				//
				ULONG BytesWritten = m_AudioClient->WriteBuffer(LeadingEdge->Offset->Data, LeadingEdge->Offset->Remaining);

				//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::Process] - BytesWritten: %d", BytesWritten));
				
				//
				// In order to keep one clone per frame and simplify the logic, make a 
				// check to see if we completely used the mappings in the leading edge.  
				// Set a flag.
				//
				if (BytesWritten == LeadingEdge->Offset->Remaining) 
				{
					m_PreviousClonePointer = NULL;

					// Finish and done with this buffer... so advance by the remaining bytes left
					// in the leading edge.
					ntStatus = KsStreamPointerAdvanceOffsets(LeadingEdge, LeadingEdge->Offset->Remaining, 0, FALSE);

					// The clone pointer will be deleted later in the I/O completion routine.
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

	KeReleaseSpinLock(&m_ProcessingLock, OldIrql);

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::NotificationRoutine()
 *****************************************************************************
 *//*!
 * @brief
 * Stream notification routine.
 * @param
 * Context Pointer to the pin.
 * @return
 * None
 */
void
CAudioPin::
NotificationRoutine
(
    IN      PVOID					Context,
	IN		ULONG					Reason,
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
    CAudioPin * that = (CAudioPin *)(Context);

	that->IoCompletion(FifoWorkItem);
}

/*****************************************************************************
 * CAudioPin::IoCompletion()
 *****************************************************************************
 *//*!
 * @brief
 * Called to notify the pin that a given number of data packets have completed.  
 * Let the buffers go if possible. We're called at DPC.
 * @return
 * None
 */
void
CAudioPin::
IoCompletion 
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	KIRQL OldIrql;

	KeAcquireSpinLock(&m_ProcessingLock, &OldIrql);

	// Get the processing gate.
	PKSGATE KsGate = KsPinGetAndGate(m_KsPin);

	if (KsGate)
	{
		// Turn the processing gate off to prevent the Process() being called when
		// the stream pointers are deleted.
		KsGateTurnInputOff(KsGate);

		// AVStream will adjust the StreamHeader->DataUsed field for us as we don't
		// specify the KSPIN_FLAG_GENERATE_MAPPINGS, so there is no need to update
		// it here.
		if (m_Capture)
		{
			ULONG BytesTransfered = FifoWorkItem->BytesInFifoBuffer;

			//dbgprintf("[IN] -  TS: %p\n", KeQueryInterruptTime());
			//if (BytesTransfered != 192)
			//dbgprintf("******************************[IN] -  BytesTransfered: %d\n", BytesTransfered);
			//
			// Walk through the clones list and delete clones whose time has come.
			// The list is guaranteed to be kept in the order they were cloned.
			//
			PKSSTREAM_POINTER ClonePointer = KsPinGetFirstCloneStreamPointer(m_KsPin);

			while (ClonePointer)
			{
				PKSSTREAM_POINTER NextClonePointer = KsStreamPointerGetNextClone(ClonePointer);

				//
				// Read the buffer.
				//
				ULONG BytesRead = m_AudioClient->ReadBuffer(ClonePointer->Offset->Data, ClonePointer->Offset->Remaining);

				//dbgprintf("[IN] -  Remaining: %d\n", ClonePointer->Offset->Remaining);
				//dbgprintf("[IN] -  BytesRead: %d\n", BytesRead);

				//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::IoCompletion] - BytesRead: %d", BytesRead));
				
				// 
				// If we have completed all remaining data in this clone, it
				// is an indication that the clone is ready to be deleted and the
				// buffer released.  Set anything required in the stream header which
				// has not yet been set.  If we have a clock, we can timestamp the
				// sample.
				//
				if (BytesRead == ClonePointer->Offset->Remaining)
				{
					//FIXME: Duration ??
					ClonePointer->StreamHeader->PresentationTime.Numerator = 1;

					ClonePointer->StreamHeader->PresentationTime.Denominator = 1;

					//
					// If a clock has been assigned, timestamp the packets with the
					// time shown on the clock. 
					//
					if (m_ReferenceClock) 
					{
						LONGLONG ClockTime = m_ReferenceClock->GetTime();

						ClonePointer->StreamHeader->PresentationTime.Time = ClockTime;

						ClonePointer->StreamHeader->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_TIMEVALID;// | KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
					} 
					else
					{
						//
						// If there is no clock, don't time stamp the packets.
						//
						ClonePointer->StreamHeader->PresentationTime.Time = 0;
					}

					// Advance it before deleting the clone pointer so the data is copied...
					KsStreamPointerAdvanceOffsets(ClonePointer, 0, BytesRead, FALSE);

					KsStreamPointerDelete(ClonePointer);
				}
				else
				{
					if (BytesRead)
					{
						//
						// If only part of the data in this clone have been completed,
						// update the pointers.  Since we're guaranteed this won't advance
						// to a new frame by the check above, it won't fail.
						//
						KsStreamPointerAdvanceOffsets(ClonePointer, 0, BytesRead, FALSE);
					}

					//
					// The hardware was incapable of providing more bytes.  The FIFO 
					// buffer is empty.
					//
					break;
				}

				//
				// Go to the next clone.
				//
				ClonePointer = NextClonePointer;
			}
		}
		else
		{
			ULONG BytesTransfered = FifoWorkItem->BytesInFifoBuffer;
			
			//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioPin::IoCompletion] - BytesTransfered : %d", BytesTransfered));

			//dbgprintf("[OUT] - TS: %p\n", KeQueryInterruptTime());
			//dbgprintf("[OUT] - BytesTransfered: %d\n", BytesTransfered);

			//
			// Walk through the clones list and delete clones whose time has come.
			// The list is guaranteed to be kept in the order they were cloned.
			//
			PKSSTREAM_POINTER ClonePointer = KsPinGetFirstCloneStreamPointer(m_KsPin);

			while (BytesTransfered && ClonePointer)
			{
				PKSSTREAM_POINTER NextClonePointer = KsStreamPointerGetNextClone(ClonePointer);

				// 
				// If we have completed all remaining data in this clone, it
				// is an indication that the clone is ready to be deleted and the
				// buffer released.  Set anything required in the stream header which
				// has not yet been set.  If we have a clock, we can timestamp the
				// sample.
				//
				if (BytesTransfered >= ClonePointer->Offset->Remaining) 
				{
					BytesTransfered -= ClonePointer->Offset->Remaining;

					KsStreamPointerAdvanceOffsets(ClonePointer, ClonePointer->Offset->Remaining, 0, FALSE);

					//
					// If all of the data in this clone have been completed,
					// delete the clone. 
					//
					KsStreamPointerDelete(ClonePointer);
				} 
				else 
				{
					//
					// If only part of the data in this clone have been completed,
					// update the pointers.  Since we're guaranteed this won't advance
					// to a new frame by the check above, it won't fail.
					//
					KsStreamPointerAdvanceOffsets(ClonePointer, BytesTransfered, 0, FALSE);

					BytesTransfered = 0;
				}

				//
				// Go to the next clone.
				//
				ClonePointer = NextClonePointer;
			}
		}

		// Turn the processing gate back on.
		KsGateTurnInputOn(KsGate);
	}

	KeReleaseSpinLock(&m_ProcessingLock, OldIrql);

	//
    // Kick processing to happen again.
    //
    KsPinAttemptProcessing(m_KsPin, TRUE);
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::QueryControlSupport()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
QueryControlSupport
(
	IN		UCHAR	ControlSelector
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = m_AudioClient->QueryControlSupport(ControlSelector);

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::WriteControl()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
WriteControl
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = m_AudioClient->WriteControl(RequestCode, ControlSelector, 0, ParameterBlock, ParameterBlockSize);

	if (NT_SUCCESS(ntStatus))
	{
		if (ControlSelector == USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY)
		{
			// sync local value with the changes.
			m_SamplingFrequency = *(PULONG(ParameterBlock));
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::ReadControl()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
ReadControl
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = m_AudioClient->ReadControl(RequestCode, ControlSelector, 0, ParameterBlock, ParameterBlockSize, OutParameterBlockSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_FindAudioBitResolution()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
_FindAudioBitResolution
(
    IN      ULONG           PinId,
    IN      BOOLEAN         Capture,
    IN      PKSDATAFORMAT   Format,
	OUT		ULONG *			OutBitResolution
)
{
    PAGED_CODE();

    ASSERT(Format);
	ASSERT(OutBitResolution);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FindAudioBitResolution]"));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    PKSFILTER_DESCRIPTOR FilterDescriptor; m_AudioFilter->GetDescription(&FilterDescriptor);

    if (PinId < FilterDescriptor->PinDescriptorsCount)
    {
        PKSPIN_DESCRIPTOR_EX Pin = PKSPIN_DESCRIPTOR_EX(&FilterDescriptor->PinDescriptors[PinId]);

        // Validate data flows...
        if (((Capture) && (Pin->PinDescriptor.DataFlow == KSPIN_DATAFLOW_OUT) &&
             ((Pin->PinDescriptor.Communication == KSPIN_COMMUNICATION_SINK) || 
			  (Pin->PinDescriptor.Communication == KSPIN_COMMUNICATION_BOTH))) ||
            ((!Capture) && (Pin->PinDescriptor.DataFlow == KSPIN_DATAFLOW_IN) &&
             (Pin->PinDescriptor.Communication == KSPIN_COMMUNICATION_SINK)))
        {
            PKSDATARANGE * DataRanges = (PKSDATARANGE *)Pin->PinDescriptor.DataRanges;

            if (DataRanges)
            {
				ULONG Delta0 = ULONG(-1);

                for (ULONG i = 0; i < Pin->PinDescriptor.DataRangesCount; i++)
                {
                    PKSDATARANGE_AUDIO DataRangeAudio = PKSDATARANGE_AUDIO(DataRanges[i]);

					// Find the actual subformat if it is extensible.
					GUID SubFormat = Format->SubFormat;

					if (IsEqualGUIDAligned(Format->SubFormat, KSDATAFORMAT_SUBTYPE_EXTENSIBLE))
					{
						if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
						{
							PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)(Format + 1);

							SubFormat = WaveFormatExt->SubFormat;
						}
						else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
						{
							PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format+1);

							PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;

							SubFormat = WaveFormatExt->SubFormat;
						}
					}

                    // KSDATAFORMAT contains three GUIDs to support extensible
                    // format.  The first two GUIDs identify the type of data.  The
                    // third indicates the type of specifier used to indicate format
                    // specifics.
                    if (IsEqualGUIDAligned(Format->MajorFormat, DataRangeAudio->DataRange.MajorFormat) &&
                        IsEqualGUIDAligned(SubFormat, DataRangeAudio->DataRange.SubFormat) &&
                        IS_VALID_WAVEFORMATEX_GUID(&Format->SubFormat) &&
                        IsEqualGUIDAligned(Format->Specifier, DataRangeAudio->DataRange.Specifier))
                    {
                        if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
                        {
                            if (Format->FormatSize >= sizeof(KSDATAFORMAT_WAVEFORMATEX))
                            {
                                PWAVEFORMATEX waveFormat = PWAVEFORMATEX(Format + 1);

                                if ((waveFormat->nChannels == DataRangeAudio->MaximumChannels) &&
                                    (waveFormat->wBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                                    (waveFormat->wBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                                    (waveFormat->nSamplesPerSec >= DataRangeAudio->MinimumSampleFrequency) &&
                                    (waveFormat->nSamplesPerSec <= DataRangeAudio->MaximumSampleFrequency))
                                {
									PKSDATARANGE_AUDIO_EX DataRangeAudioEx = PKSDATARANGE_AUDIO_EX(DataRangeAudio);

									// Byte-aligned bits.
									USHORT BitsPerSample = ((waveFormat->wBitsPerSample + 7) / 8) * 8;

									if (DataRangeAudioEx->BitResolution == BitsPerSample)
									{
										// Found exact match.
										*OutBitResolution = DataRangeAudioEx->BitResolution;

										Delta0 = 0; 
									}
									else if (DataRangeAudioEx->BitResolution > BitsPerSample)
									{
										ULONG Delta1 = DataRangeAudioEx->BitResolution - BitsPerSample;

										if (Delta1 <= Delta0)
										{
											*OutBitResolution = DataRangeAudioEx->BitResolution;

											Delta0 = Delta1;
										}
									}
									else // if (DataRangeAudioEx->BitResolution < BitsPerSample)
									{
										ULONG Delta1 = BitsPerSample - DataRangeAudioEx->BitResolution;

										if (Delta1 < Delta0)
										{
											*OutBitResolution = DataRangeAudioEx->BitResolution;

											Delta0 = Delta1;
										}
									}

									ntStatus = STATUS_SUCCESS;
                                }
                            }
                        }
                        else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
                        {
                            if (Format->FormatSize >= sizeof(KSDATAFORMAT_DSOUND))
                            {
                                PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format + 1);

                                PWAVEFORMATEX waveFormat = &BufferDesc->WaveFormatEx;

                                if ((waveFormat->nChannels == DataRangeAudio->MaximumChannels) &&
                                    (waveFormat->wBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                                    (waveFormat->wBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                                    (waveFormat->nSamplesPerSec >= DataRangeAudio->MinimumSampleFrequency) &&
                                    (waveFormat->nSamplesPerSec <= DataRangeAudio->MaximumSampleFrequency))
                                {
									PKSDATARANGE_AUDIO_EX DataRangeAudioEx = PKSDATARANGE_AUDIO_EX(DataRangeAudio);

									// Byte-aligned bits.
									USHORT BitsPerSample = ((waveFormat->wBitsPerSample + 7) / 8) * 8;

									if (DataRangeAudioEx->BitResolution == BitsPerSample)
									{
										// Found exact match.
										*OutBitResolution = DataRangeAudioEx->BitResolution;

										Delta0 = 0; 
									}
									else if (DataRangeAudioEx->BitResolution > BitsPerSample)
									{
										ULONG Delta1 = DataRangeAudioEx->BitResolution - BitsPerSample;

										if (Delta1 <= Delta0)
										{
											*OutBitResolution = DataRangeAudioEx->BitResolution;

											Delta0 = Delta1;
										}
									}
									else // if (DataRangeAudioEx->BitResolution < BitsPerSample)
									{
										ULONG Delta1 = BitsPerSample - DataRangeAudioEx->BitResolution;

										if (Delta1 < Delta0)
										{
											*OutBitResolution = DataRangeAudioEx->BitResolution;

											Delta0 = Delta1;
										}
									}

									ntStatus = STATUS_SUCCESS;
								}
                            }
                        }
					}

					if (Delta0 == 0) break;
                }
            }
        }
    }

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_FindAudioInterface()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
_FindAudioInterface
(
    IN      ULONG           PinId,
    IN      BOOLEAN         Capture,
    IN      PKSDATAFORMAT   Format,
	IN		ULONG 			BitResolution
)
{
    PAGED_CODE();

    ASSERT(Format);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FindAudioInterface]"));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    PKSFILTER_DESCRIPTOR FilterDescriptor; m_AudioFilter->GetDescription(&FilterDescriptor);

    if (PinId < FilterDescriptor->PinDescriptorsCount)
    {
        PKSPIN_DESCRIPTOR_EX Pin = PKSPIN_DESCRIPTOR_EX(&FilterDescriptor->PinDescriptors[PinId]);

        // Validate data flows...
        if (((Capture) && (Pin->PinDescriptor.DataFlow == KSPIN_DATAFLOW_OUT) &&
             ((Pin->PinDescriptor.Communication == KSPIN_COMMUNICATION_SINK) || 
			  (Pin->PinDescriptor.Communication == KSPIN_COMMUNICATION_BOTH))) ||
            ((!Capture) && (Pin->PinDescriptor.DataFlow == KSPIN_DATAFLOW_IN) &&
             (Pin->PinDescriptor.Communication == KSPIN_COMMUNICATION_SINK)))
        {
            PKSDATARANGE * DataRanges = (PKSDATARANGE *)Pin->PinDescriptor.DataRanges;

            if (DataRanges)
            {
                for (ULONG i = 0; i < Pin->PinDescriptor.DataRangesCount; i++)
                {
                    PKSDATARANGE_AUDIO DataRangeAudio = PKSDATARANGE_AUDIO(DataRanges[i]);

					// Find the actual subformat if it is extensible.
					GUID SubFormat = Format->SubFormat;

					if (IsEqualGUIDAligned(Format->SubFormat, KSDATAFORMAT_SUBTYPE_EXTENSIBLE))
					{
						if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
						{
							PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)(Format + 1);

							SubFormat = WaveFormatExt->SubFormat;
						}
						else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
						{
							PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format+1);

							PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;

							SubFormat = WaveFormatExt->SubFormat;
						}
					}

                    // KSDATAFORMAT contains three GUIDs to support extensible
                    // format.  The first two GUIDs identify the type of data.  The
                    // third indicates the type of specifier used to indicate format
                    // specifics.
                    if (IsEqualGUIDAligned(Format->MajorFormat, DataRangeAudio->DataRange.MajorFormat) &&
                        IsEqualGUIDAligned(SubFormat, DataRangeAudio->DataRange.SubFormat) &&
                        IS_VALID_WAVEFORMATEX_GUID(&Format->SubFormat) &&
                        IsEqualGUIDAligned(Format->Specifier, DataRangeAudio->DataRange.Specifier))
                    {
                        if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
                        {
                            if (Format->FormatSize >= sizeof(KSDATAFORMAT_WAVEFORMATEX))
                            {
                                PWAVEFORMATEX waveFormat = PWAVEFORMATEX(Format + 1);

                                if ((waveFormat->nChannels == DataRangeAudio->MaximumChannels) &&
                                    (waveFormat->wBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                                    (waveFormat->wBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                                    (waveFormat->nSamplesPerSec >= DataRangeAudio->MinimumSampleFrequency) &&
                                    (waveFormat->nSamplesPerSec <= DataRangeAudio->MaximumSampleFrequency))
                                {
									PKSDATARANGE_AUDIO_EX DataRangeAudioEx = PKSDATARANGE_AUDIO_EX(DataRangeAudio);

									if (DataRangeAudioEx->BitResolution == BitResolution)
									{
										ULONG Priority = AUDIO_PRIORITY_HIGH; // AC3/non-PCM clients

										BOOL ChangeClockRate = TRUE;

										if (IsEqualGUIDAligned(SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
										{
											ChangeClockRate = TRUE;

											if (m_AudioFilter->m_UsePreferredSampleRate)
											{
												// WDM/KMixer clients
												Priority = AUDIO_PRIORITY_LOW;
												//[NI Tao 20070626]disallow this kind of stream to change clock rate
												ChangeClockRate = FALSE;
												//end
											}
											else
											{
												// ASIO/DirectKS clients
												Priority = AUDIO_PRIORITY_NORMAL;
											}
										}

										//if (m_State != KSSTATE_STOP)
										{
											ntStatus = _AcquireAudioInterface(DataRangeAudioEx->InterfaceNumber, DataRangeAudioEx->AlternateSetting, waveFormat->nSamplesPerSec, waveFormat->nChannels, waveFormat->wBitsPerSample, Priority, ChangeClockRate);
										}
										//else
										{
											//ntStatus = STATUS_SUCCESS;
										}

										if (NT_SUCCESS(ntStatus))
										{
											m_InterfaceNumber = DataRangeAudioEx->InterfaceNumber;

											m_AlternateSetting = DataRangeAudioEx->AlternateSetting;
										}

										_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FindAudioInterface] - InterfaceNumber: %d, AlternateSetting: %d, ntStatus: 0x%x", DataRangeAudioEx->InterfaceNumber, DataRangeAudioEx->AlternateSetting, ntStatus));	
									}
                                }
                            }
                        }
                        else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
                        {
                            if (Format->FormatSize >= sizeof(KSDATAFORMAT_DSOUND))
                            {
                                PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format + 1);

                                PWAVEFORMATEX waveFormat = &BufferDesc->WaveFormatEx;

                                if ((waveFormat->nChannels == DataRangeAudio->MaximumChannels) &&
                                    (waveFormat->wBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                                    (waveFormat->wBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                                    (waveFormat->nSamplesPerSec >= DataRangeAudio->MinimumSampleFrequency) &&
                                    (waveFormat->nSamplesPerSec <= DataRangeAudio->MaximumSampleFrequency))
                                {
									PKSDATARANGE_AUDIO_EX DataRangeAudioEx = PKSDATARANGE_AUDIO_EX(DataRangeAudio);

									if (DataRangeAudioEx->BitResolution == BitResolution)
									{
										ULONG Priority = AUDIO_PRIORITY_HIGH; // AC3/non-PCM clients

										BOOL ChangeClockRate = TRUE;

										if (IsEqualGUIDAligned(SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
										{
											ChangeClockRate = TRUE;

											if (m_AudioFilter->m_UsePreferredSampleRate)
											{
												// WDM/KMixer clients
												Priority = AUDIO_PRIORITY_LOW;
											}
											else
											{
												// ASIO/DirectKS clients
												Priority = AUDIO_PRIORITY_NORMAL;
											}
										}

										//if (m_State != KSSTATE_STOP)
										{
											ntStatus = _AcquireAudioInterface(DataRangeAudioEx->InterfaceNumber, DataRangeAudioEx->AlternateSetting, waveFormat->nSamplesPerSec, waveFormat->nChannels, waveFormat->wBitsPerSample, Priority, ChangeClockRate);
										}
										//else
										{
											//ntStatus = STATUS_SUCCESS;
										}

										if (NT_SUCCESS(ntStatus))
										{
											m_InterfaceNumber = DataRangeAudioEx->InterfaceNumber;

											m_AlternateSetting = DataRangeAudioEx->AlternateSetting;
										}

										_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FindAudioInterface] - InterfaceNumber: %d, AlternateSetting: %d, ntStatus: 0x%x", DataRangeAudioEx->InterfaceNumber, DataRangeAudioEx->AlternateSetting, ntStatus));								
									}
								}
                            }
                        }
					}

					if (NT_SUCCESS(ntStatus)) break;
                }
            }
        }
    }

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_AcquireAudioInterface()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
_AcquireAudioInterface
(
	IN		UCHAR	InterfaceNumber,
	IN		UCHAR	AlternateSetting,
	IN		ULONG	SampleRate,
	IN		ULONG	FormatChannels,
	IN		ULONG	SampleSize,
	IN		ULONG	Priority,
	IN		BOOL	ChangeClockRate
)
{
	NTSTATUS ntStatus = m_AudioClient->SetInterfaceParameter(InterfaceNumber, AlternateSetting, Priority, ChangeClockRate ? SampleRate : 0);

	if (NT_SUCCESS(ntStatus))
	{
		if (NT_SUCCESS(m_AudioClient->QueryControlSupport(USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY)))
		{
			ntStatus = m_AudioClient->WriteControl(REQUEST_CUR, USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY, 0, &SampleRate, sizeof(ULONG));
		}
	}

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = m_AudioClient->SetupBuffer(SampleRate, FormatChannels, SampleSize, m_Capture, m_Capture ? m_AudioFilter->m_NumberOfFifoBuffers.Input : m_AudioFilter->m_NumberOfFifoBuffers.Output);
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_ReleaseAudioInterface()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioPin::
_ReleaseAudioInterface
(
	IN		UCHAR	InterfaceNumber
)
{
	NTSTATUS ntStatus = m_AudioClient->SetInterfaceParameter(InterfaceNumber, 0, AUDIO_PRIORITY_NONE, 0);

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_AllocateResources()
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
CAudioPin::
_AllocateResources
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_AllocateResources]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;//_AcquireAudioInterface(m_InterfaceNumber, m_AlternateSetting, m_SamplingFrequency, m_FormatChannels, m_SampleSize);
	
	// Attempt to get an interface to the master clock. This will fail if one 
	// has not been assigned.  Since one must be assigned while the pin is still in 
    // KSSTATE_STOP, this is a guranteed method of getting the clock should one 
	// be assigned.
    if (!NT_SUCCESS(KsPinGetReferenceClockInterface(m_KsPin, &m_ReferenceClock))) 
	{
        // If we could not get an interface to the clock, don't use one.  
        m_ReferenceClock = NULL;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_AllocateResources] - ReferenceClock: %p", m_ReferenceClock));

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_FreeResources()
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
CAudioPin::
_FreeResources
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FreeResources]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;//_ReleaseAudioInterface(m_InterfaceNumber);

    if (m_ReferenceClock) 
	{
        m_ReferenceClock->Release();
        m_ReferenceClock = NULL;
    }

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_FreeClonePointers()
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
CAudioPin::
_FreeClonePointers
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FreeClonePointers]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	KsPinAcquireProcessingMutex(m_KsPin);

	// Walk through the clones, deleting them, and setting DataUsed to
    // zero since we didn't use any data!
    PKSSTREAM_POINTER ClonePointer = KsPinGetFirstCloneStreamPointer(m_KsPin);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_FreeClonePointers] - FirstClone: %p", ClonePointer));

    while (ClonePointer) 
	{
        PKSSTREAM_POINTER NextClonePointer = KsStreamPointerGetNextClone(ClonePointer);

		if (m_Capture)
		{
			ClonePointer->StreamHeader->DataUsed = 0;
		}

        KsStreamPointerDelete(ClonePointer);

        ClonePointer = NextClonePointer;
    }

	m_PreviousClonePointer = NULL;

	KsPinReleaseProcessingMutex(m_KsPin);

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_Run()
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
CAudioPin::
_Run
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_Run]"));

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_AudioClient)
	{
		ntStatus = m_AudioClient->Start(m_AudioFilter->m_SynchronizeStart, m_AudioFilter->m_StartFrameNumber);
	}
    
	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_Pause()
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
CAudioPin::
_Pause
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_Pause]"));

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_AudioClient)
	{
		ntStatus = m_AudioClient->Pause();
	}

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_Stop()
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
CAudioPin::
_Stop
(   void
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_Stop]"));

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_AudioClient)
	{
		ntStatus = m_AudioClient->Stop();
	}

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioPin::_GetPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the current hardware running counter.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
_GetPosition
(
	OUT		ULONGLONG *	OutTransferPosition,
	OUT		ULONGLONG *	OutQueuePosition
)
{
    // Not PAGED_CODE(). May be called at dispatch level.
    NTSTATUS ntStatus = m_AudioClient->GetPosition(OutTransferPosition, OutQueuePosition);

	if (m_KsPin->ConnectionInterface.Id == KSINTERFACE_STANDARD_LOOPED_STREAMING)
	{
  		// Modulo the looped buffer size.
		if (m_LoopedBufferSize)
		{
			*OutTransferPosition %= m_LoopedBufferSize;

			*OutQueuePosition %= m_LoopedBufferSize;

            // A hack to keep PowerDVD happy...
            if ((m_NonPcmFormat) && (m_State != KSSTATE_RUN))
            {
                *OutQueuePosition = *OutTransferPosition = 0;
            }
		}
  	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_SetPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the current hardware running counter.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
_SetPosition
(
	IN		ULONGLONG 	TransferPosition,
	IN		ULONGLONG 	QueuePosition
)
{
    // Not PAGED_CODE(). May be called at dispatch level.
    NTSTATUS ntStatus = m_AudioClient->SetPosition(TransferPosition, QueuePosition);

	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::_IsPinsConnected()
 *****************************************************************************
 *//*!
 * @brief
 * Determine if the two pins on the specified nodes are connected to one
 * other.
 * @return
 * Returns TRUE if the pins are connected to one another. Otherwise, returns
 * FALSE.
 */
BOOL
CAudioPin::
_IsPinsConnected
(
	IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
	IN		ULONG					FromNode,
	IN		ULONG					FromNodePin,
	IN		ULONG					ToNode,
	IN		ULONG					ToNodePin
)
{
    PAGED_CODE();

	BOOL Connected = FALSE;

	for (ULONG i=0; i<KsFilterDescriptor->ConnectionsCount; i++)
	{
		if ((KsFilterDescriptor->Connections[i].ToNode == ToNode) && ((KsFilterDescriptor->Connections[i].ToNodePin == ToNodePin) || (ToNodePin == ULONG(-1)/*Any*/)))
		{
			if ((KsFilterDescriptor->Connections[i].FromNode == FromNode) && (KsFilterDescriptor->Connections[i].FromNodePin == FromNodePin))
			{
				// The nodes & pins are connected to each other.
				Connected = TRUE;
			}
			else if (KsFilterDescriptor->Connections[i].FromNode == KSFILTER_NODE)
			{
				// The nodes & pins are not connected.
				Connected = FALSE;
			}
			else
			{
				// Not done yet. Recurse to the next node.
				if (KsFilterDescriptor->Connections[i].FromNode < KsFilterDescriptor->NodeDescriptorsCount)
				{
					Connected = _IsPinsConnected(KsFilterDescriptor, FromNode, FromNodePin, KsFilterDescriptor->Connections[i].FromNode, ULONG(-1));
				}
				else
				{
					// Huh ?? Not connected :-P
					Connected = FALSE;
				}
			}

			if (Connected)
			{
				// Found the connection, so end this walk.
				break;
			}
			else
			{
				// Have not found the connection. If there is no more connections, end the walk.
				if (ToNodePin != ULONG(-1))/*Any*/
				{
					break;
				}
			}
		}
	}

	return Connected;
}

/*****************************************************************************
 * CAudioPin::_EnforceDrmRights()
 *****************************************************************************
 *//*!
 * @brief
 * Enforce the DRM rights.
 */
NTSTATUS
CAudioPin::
_EnforceDrmRights
(
	IN		PKSFILTER	KsFilter,
	IN		DRMRIGHTS	OldDrmRights,
	IN		DRMRIGHTS	NewDrmRights
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::_EnforceDrmRights]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	for (ULONG i=0; i<KsFilter->Descriptor->PinDescriptorsCount; i++)
	{
		PKSPIN_DESCRIPTOR KsPinDescriptor = PKSPIN_DESCRIPTOR(&KsFilter->Descriptor->PinDescriptors[i].PinDescriptor);

		if ((KsPinDescriptor->DataFlow == KSPIN_DATAFLOW_OUT) &&
			(KsPinDescriptor->Communication == KSPIN_COMMUNICATION_BRIDGE))
		{
			if ((IsEqualGUIDAligned(*(KsPinDescriptor->Category), KSNODETYPE_DIGITAL_AUDIO_INTERFACE)) || 
				(IsEqualGUIDAligned(*(KsPinDescriptor->Category), KSNODETYPE_SPDIF_INTERFACE)))
			{
				if (NewDrmRights.DigitalOutputDisable)
				{
					if (!OldDrmRights.DigitalOutputDisable)
					{
						// If the sink pin is connected to this pin, then enforce the DRM rights.
						if (_IsPinsConnected(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i))
						{
							NTSTATUS Status = STATUS_SUCCESS;

							_EnforceDrmDigitalOutputDisable(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i, NewDrmRights.DigitalOutputDisable, FALSE, &Status);

							if (!NT_SUCCESS(Status))
							{
								ntStatus = Status;
							}
						}
					}
				}
				else
				{
					if (OldDrmRights.DigitalOutputDisable)
					{
						// If the sink pin is connected to this pin, then enforce the DRM rights.
						if (_IsPinsConnected(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i))
						{
							NTSTATUS Status = STATUS_SUCCESS;

							_EnforceDrmDigitalOutputDisable(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i, NewDrmRights.DigitalOutputDisable, FALSE, &Status);

							if (!NT_SUCCESS(Status))
							{
								ntStatus = Status;
							}
						}
					}
				}
			}
		}
		else if ((KsPinDescriptor->DataFlow == KSPIN_DATAFLOW_OUT) &&
				 ((KsPinDescriptor->Communication == KSPIN_COMMUNICATION_SOURCE) ||
				  (KsPinDescriptor->Communication == KSPIN_COMMUNICATION_BOTH)))
		{
			if (IsEqualGUIDAligned(*(KsPinDescriptor->Category), PINNAME_CAPTURE))
			{
				if (NewDrmRights.CopyProtect)
				{
					if (!OldDrmRights.CopyProtect)
					{
						// If the sink pin is connected to this pin, then enforce the DRM rights.
						if (_IsPinsConnected(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i))
						{
							NTSTATUS Status = STATUS_SUCCESS;

							_EnforceDrmCopyProtect(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i, NewDrmRights.CopyProtect, FALSE, &Status);

							if (!NT_SUCCESS(Status))
							{
								ntStatus = Status;
							}
						}
					}
				}
				else
				{
					if (OldDrmRights.CopyProtect)
					{
						// If the sink pin is connected to this pin, then enforce the DRM rights.
						if (_IsPinsConnected(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i))
						{
							NTSTATUS Status = STATUS_SUCCESS;

							_EnforceDrmCopyProtect(PKSFILTER_DESCRIPTOR(KsFilter->Descriptor), KSFILTER_NODE, m_PinId, KSFILTER_NODE, i, NewDrmRights.CopyProtect, FALSE, &Status);

							if (!NT_SUCCESS(Status))
							{
								ntStatus = Status;
							}
						}
					}
				}
			}
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::_EnforceDrmCopyProtect()
 *****************************************************************************
 *//*!
 * @brief
 * Enforce the DRM rights.
 */
BOOL
CAudioPin::
_EnforceDrmCopyProtect
(
	IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
	IN		ULONG					FromNode,
	IN		ULONG					FromNodePin,
	IN		ULONG					ToNode,
	IN		ULONG					ToNodePin,
	IN		BOOL					CopyProtect,
	IN		BOOL					FoundSuperMixNode,
	OUT		NTSTATUS *				OutStatus
)
{
    PAGED_CODE();

	BOOL Connected = FALSE;

	for (ULONG i=0; i<KsFilterDescriptor->ConnectionsCount; i++)
	{
		if ((KsFilterDescriptor->Connections[i].ToNode == ToNode) && ((KsFilterDescriptor->Connections[i].ToNodePin == ToNodePin) || (ToNodePin == ULONG(-1)/*Any*/)))
		{
			if ((KsFilterDescriptor->Connections[i].FromNode == FromNode) && (KsFilterDescriptor->Connections[i].FromNodePin == FromNodePin))
			{
				// The nodes & pins are connected to each other.
				Connected = TRUE;
			}
			else if (KsFilterDescriptor->Connections[i].FromNode == KSFILTER_NODE)
			{
				// The nodes & pins are not connected.
				Connected = FALSE;
			}
			else
			{
				// Not done yet. Recurse to the next node.
				if (KsFilterDescriptor->Connections[i].FromNode < KsFilterDescriptor->NodeDescriptorsCount)
				{
					ULONG SuperMixNode = ULONG(-1);
					
					// The super mix node closest to the destination filter pin.
					if (!FoundSuperMixNode)
					{
						if (IsEqualGUIDAligned(*(KsFilterDescriptor->NodeDescriptors[KsFilterDescriptor->Connections[i].FromNode].Type), KSNODETYPE_SUPERMIX))
						{
							SuperMixNode = KsFilterDescriptor->Connections[i].FromNode;

							FoundSuperMixNode = (SuperMixNode != ULONG(-1));
						}
					}

					Connected = _EnforceDrmCopyProtect(KsFilterDescriptor, FromNode, FromNodePin, KsFilterDescriptor->Connections[i].FromNode, ULONG(-1), CopyProtect, FoundSuperMixNode, OutStatus);

					if (Connected)
					{
						if (FoundSuperMixNode && (SuperMixNode != ULONG(-1)))
						{
							if (m_AudioFilter)
							{
								PNODE_DESCRIPTOR Node = m_AudioFilter->FindNode(SuperMixNode);

								if (Node)
								{
									*OutStatus = Node->EnforceDrmProtection(CopyProtect);
								}
							}

							FoundSuperMixNode = FALSE;

							SuperMixNode = ULONG(-1);
						}
					}
				}
				else
				{
					// Huh ?? Not connected :-P
					Connected = FALSE;
				}
			}
		}
	}

	return Connected;
}

/*****************************************************************************
 * CAudioPin::_EnforceDrmDigitalOutputDisable()
 *****************************************************************************
 *//*!
 * @brief
 * Enforce the DRM rights.
 */
BOOL
CAudioPin::
_EnforceDrmDigitalOutputDisable
(
	IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
	IN		ULONG					FromNode,
	IN		ULONG					FromNodePin,
	IN		ULONG					ToNode,
	IN		ULONG					ToNodePin,
	IN		BOOL					DigitalOutputDisable,
	IN		BOOL					FoundMuteNode,
	OUT		NTSTATUS *				OutStatus
)
{
    PAGED_CODE();

	BOOL Connected = FALSE;

	for (ULONG i=0; i<KsFilterDescriptor->ConnectionsCount; i++)
	{
		if ((KsFilterDescriptor->Connections[i].ToNode == ToNode) && ((KsFilterDescriptor->Connections[i].ToNodePin == ToNodePin) || (ToNodePin == ULONG(-1)/*Any*/)))
		{
			if ((KsFilterDescriptor->Connections[i].FromNode == FromNode) && (KsFilterDescriptor->Connections[i].FromNodePin == FromNodePin))
			{
				// The nodes & pins are connected to each other.
				Connected = TRUE;
			}
			else if (KsFilterDescriptor->Connections[i].FromNode == KSFILTER_NODE)
			{
				// The nodes & pins are not connected.
				Connected = FALSE;
			}
			else
			{
				// Not done yet. Recurse to the next node.
				if (KsFilterDescriptor->Connections[i].FromNode < KsFilterDescriptor->NodeDescriptorsCount)
				{
					ULONG MuteNode = ULONG(-1);

					// The mute node closest to the destination filter pin.
					if (!FoundMuteNode)
					{
						if (IsEqualGUIDAligned(*(KsFilterDescriptor->NodeDescriptors[KsFilterDescriptor->Connections[i].FromNode].Type), KSNODETYPE_MUTE))
						{
							MuteNode = KsFilterDescriptor->Connections[i].FromNode;

							FoundMuteNode = (MuteNode != ULONG(-1));
						}
					}

					Connected = _EnforceDrmDigitalOutputDisable(KsFilterDescriptor, FromNode, FromNodePin, KsFilterDescriptor->Connections[i].FromNode, ULONG(-1), DigitalOutputDisable, FoundMuteNode, OutStatus);

					if (Connected)
					{
						if (FoundMuteNode && (MuteNode != ULONG(-1)))
						{
							if (m_AudioFilter)
							{
								PNODE_DESCRIPTOR Node = m_AudioFilter->FindNode(MuteNode);

								if (Node)
								{
									*OutStatus = Node->EnforceDrmProtection(DigitalOutputDisable);
								}
							}

							FoundMuteNode = FALSE;

							MuteNode = ULONG(-1);
						}
					}
				}
				else
				{
					// Huh ?? Not connected :-P
					Connected = FALSE;
				}
			}
		}
	}

	return Connected;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioPin::AudioPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Audio property items.
 */
DEFINE_KSPROPERTY_TABLE(CAudioPin::AudioPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_LATENCY,					// Id
		CAudioPin::GetLatencyControl,				// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),							// MinProperty
		sizeof(KSTIME),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		0											// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_POSITION,					// Id
		CAudioPin::GetAudioPositionControl,			// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),							// MinProperty
		sizeof(KSAUDIO_POSITION),					// MinData
		CAudioPin::SetAudioPositionControl,			// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		0											// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_SAMPLING_RATE,				// Id
		CAudioPin::GetSrcControl,					// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(ULONG),								// MinData
		CAudioPin::SetSrcControl,					// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CAudioPin::SupportSrcControl,				// SupportHandler
		0											// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE,		// Id
		CAudioPin::GetSrcControl,					// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(BOOL),								// MinData
		CAudioPin::SetSrcControl,					// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CAudioPin::SupportSrcControl,				// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * CAudioPin::DrmPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * DRM property items.
 */
DEFINE_KSPROPERTY_TABLE(CAudioPin::DrmPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DRMAUDIOSTREAM_CONTENTID,		// Id
		NULL,										// GetPropertyHandler or GetSupported
		sizeof(KSP_DRMAUDIOSTREAM_CONTENTID),		// MinProperty
		sizeof(KSDRMAUDIOSTREAM_CONTENTID),			// MinData
		CAudioPin::SetDrmControl,					// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * CAudioPin::PropertySetTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Pin property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(CAudioPin::PropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(CAudioPin::AudioPropertyTable),// PropertiesCount
		CAudioPin::AudioPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_DrmAudioStream,				// Set
		SIZEOF_ARRAY(CAudioPin::DrmPropertyTable),	// PropertiesCount
		CAudioPin::DrmPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * CAudioPin::AutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Pin automation table.
 */
DEFINE_KSAUTOMATION_TABLE(CAudioPin::AutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(CAudioPin::PropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::GetLatencyControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
GetLatencyControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::GetLatencyControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (AudioPin)
	{
		if (ValueSize >= sizeof(KSTIME))
		{
			PKSTIME Latency = (PKSTIME)(Value);

			ntStatus = AudioPin->GetLatency(Latency);
		}
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(KSTIME);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioPin::GetAudioPositionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
GetAudioPositionControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    ASSERT(Request);

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::GetAudioPositionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (AudioPin)
	{
		if (ValueSize >= sizeof(KSAUDIO_POSITION))
		{
			PKSAUDIO_POSITION Position = (PKSAUDIO_POSITION)(Value);

			ntStatus = AudioPin->GetAudioPosition(Position);
		}
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(KSAUDIO_POSITION);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SetAudioPositionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
SetAudioPositionControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    ASSERT(Request);

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetAudioPositionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (AudioPin)
	{
		if (ValueSize >= sizeof(KSAUDIO_POSITION))
		{
			KSAUDIO_POSITION Position = *(PKSAUDIO_POSITION(Value));

			ntStatus = AudioPin->SetAudioPosition(Position);
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioPin::SupportSrcControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
SupportSrcControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SupportSrcControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (AudioPin)
	{
		// service get request
		if (Request->Property.Id == KSPROPERTY_AUDIO_SAMPLING_RATE)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
											     KSPROPERTY_TYPE_GET |
											     KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_UI4;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = 0;
				Description->Reserved          = 0;

				// set the return value size
				ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

				ntStatus = STATUS_SUCCESS;
			}
			else if (ValueSize >= sizeof(ULONG))
			{
				// if return buffer can hold a ULONG, return the access flags
				PULONG AccessFlags = PULONG(Value);

				*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
							   KSPROPERTY_TYPE_GET |
							   KSPROPERTY_TYPE_SET;

				// set the return value size
				ValueSize = sizeof(ULONG);

				ntStatus = STATUS_SUCCESS;
			}
		}
		else if (Request->Property.Id == KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
											     KSPROPERTY_TYPE_GET |
											     KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_BOOL;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = 0;
				Description->Reserved          = 0;

				// set the return value size
				ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

				ntStatus = STATUS_SUCCESS;
			}
			else if (ValueSize >= sizeof(ULONG))
			{
				// if return buffer can hold a ULONG, return the access flags
				PULONG AccessFlags = PULONG(Value);

				*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
							   KSPROPERTY_TYPE_GET |
							   KSPROPERTY_TYPE_SET;

				// set the return value size
				ValueSize = sizeof(ULONG);

				ntStatus = STATUS_SUCCESS;
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::GetSrcControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
GetSrcControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::GetSrcControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (AudioPin)
	{
		// service get request
		if (Request->Property.Id == KSPROPERTY_AUDIO_SAMPLING_RATE)
		{
			if (ValueSize >= sizeof(ULONG))
			{
				ntStatus = AudioPin->ReadControl(REQUEST_CUR, USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY, 0, Value, sizeof(ULONG), NULL);
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(ULONG);
		}
		else if (Request->Property.Id == KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE)
		{
			if (ValueSize >= sizeof(BOOL))
			{
				ntStatus = AudioPin->ReadControl(REQUEST_CUR, USB_AUDIO_EP_CONTROL_PITCH, 0, Value, sizeof(BOOL), NULL);
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(BOOL);
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SetSrcControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
SetSrcControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetSrcControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (AudioPin)
	{
		// service set request
		if (Request->Property.Id == KSPROPERTY_AUDIO_SAMPLING_RATE)
		{
			if (ValueSize >= sizeof(ULONG))
			{
				ntStatus = AudioPin->WriteControl(REQUEST_CUR, USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY, 0, Value, sizeof(ULONG));
			}
		}
		else if (Request->Property.Id == KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE)
		{
			if (ValueSize >= sizeof(BOOL))
			{
				ntStatus = AudioPin->WriteControl(REQUEST_CUR, USB_AUDIO_EP_CONTROL_PITCH, 0, Value, sizeof(BOOL));
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioPin::SetDrmControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioPin::
SetDrmControl
(
	IN		PIRP							Irp,
	IN		PKSP_DRMAUDIOSTREAM_CONTENTID	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioPin::SetDrmControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioPin * AudioPin = KsGetPinFromIrp(Irp) ? (CAudioPin*)(KsGetPinFromIrp(Irp)->Context) : NULL;

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (Irp->RequestorMode == KernelMode)
	{
		if (AudioPin)
		{
			// service set request
			if (Request->Property.Id == KSPROPERTY_DRMAUDIOSTREAM_CONTENTID)
			{
				if (ValueSize >= sizeof(KSDRMAUDIOSTREAM_CONTENTID))
				{
					PKSDRMAUDIOSTREAM_CONTENTID DrmAudioStream = PKSDRMAUDIOSTREAM_CONTENTID(Value);

					ntStatus = AudioPin->SetDrmContentId(DrmAudioStream->ContentId, DrmAudioStream->DrmRights);
				}
			}
		}
	}
	else
	{
		// Property request did not originate from kernel mode, so fail it.
		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

#pragma code_seg()

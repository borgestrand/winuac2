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
 * @file       Filter.cpp
 * @brief      Audio filter implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "Filter.h"
#include "Factory.h"
#include "Profile.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "AUDIO_FILTER: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioFilter::DispatchTable
 *****************************************************************************
 *//*!
 * @brief
 * This is the dispatch table for the filter.  It provides notification
 * of creation, closure, processing for the filter.
 */
KSFILTER_DISPATCH 
CAudioFilter::DispatchTable = 
{
    CAudioFilter::DispatchCreate,		// Filter Create
    NULL,								// Filter Close
    NULL,                               // Filter Process
    NULL                                // Filter Reset
};

/*****************************************************************************
 * CAudioFilter::DispatchCreate()
 *****************************************************************************
 *//*!
 * @brief
 * This is the Create dispatch for the filter.  It creates the CAudioFilter 
 * and associates it with the KS filter via the bag.
 * @param
 * KsFilter Pointer to the KSFILTER structure representing the AVStream
 * filter.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioFilter::
DispatchCreate 
(
    IN		PKSFILTER	KsFilter,
	IN		PIRP		Irp
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilter::DispatchCreate]"));

	NTSTATUS ntStatus;

	CAudioFilter * Filter = new(NonPagedPool,'aChS') CAudioFilter(NULL);

	if (Filter)
	{
		Filter->AddRef();

		ntStatus = Filter->Init(KsFilter);

		if (NT_SUCCESS(ntStatus))
		{
			//
			// Add the item to the object bag if we were successful. Whenever the device goes 
			// away, the bag is cleaned up and we will be freed.
			//
			// For backwards compatibility with DirectX 8.0, we must grab the device mutex 
			// before doing this.  For Windows XP, this is not required, but it is still safe.
			//
			KsFilterAcquireControl(KsFilter);

			ntStatus = KsAddItemToObjectBag(KsFilter->Bag, Filter, (PFNKSFREE)CAudioFilter::Destruct);

			KsFilterReleaseControl(KsFilter);
		}

		if (NT_SUCCESS(ntStatus)) 
		{
			// Keeping this object...
			Filter->AddRef();

			KsFilter->Context = PVOID(Filter);
		}

		// Release the private reference.
		Filter->Release();
	}
	else
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilter::DispatchCreate] - ********************* Filter: %p", Filter));

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::Destruct()
 *****************************************************************************
 *//*!
 * @brief
 * This is the free callback for the bagged filter.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CAudioFilter object.
 * @return
 * None.
 */
VOID
CAudioFilter::
Destruct 
(
	IN		PVOID	Self
)
{
    PAGED_CODE();

	CAudioFilter * Filter = (CAudioFilter *)(Self);

	Filter->Release();
}

/*****************************************************************************
 * CAudioFilter::NonDelegatingQueryInterface()
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
CAudioFilter::
NonDelegatingQueryInterface
(
    IN      REFIID  Interface,
    OUT     PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::NonDelegatingQueryInterface]"));

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
        //
        // We reference the interface for the caller.
        //
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

/*****************************************************************************
 * CAudioFilter::~CAudioFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CAudioFilter::
~CAudioFilter
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::~CAudioFilter] - %p", this));

	if (m_EventHandle)
	{
		m_FilterFactory->RemoveEventHandler(m_EventHandle);
	}

    if (m_AudioDevice)
    {
        m_AudioDevice->Release();
    }

	// clean up UsbDevice
	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}

	if (m_KsAdapter)
    {
        m_KsAdapter->Release();
    }
}

/*****************************************************************************
 * CAudioFilter::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initializes a the filter.
 * @details
 * The caller of @b Init should run at IRQL_PASSIVE_LEVEL.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
Init
(
    IN		PKSFILTER	KsFilter
)
{
    PAGED_CODE();

    ASSERT(KsFilter);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::Init]"));

	m_KsFilter = KsFilter;

	m_FilterFactory = (CAudioFilterFactory *)(KsFilter->Context); // only on init.

	m_KsAdapter = PKSADAPTER(KsFilterGetDevice(KsFilter)->Context);
	m_KsAdapter->AddRef();

	m_UsbDevice = m_KsAdapter->GetUsbDevice();
	m_UsbDevice->AddRef();

	m_AudioDevice = m_KsAdapter->GetAudioDevice();
	m_AudioDevice->AddRef();

    // Initialize the speaker properties.
    m_ActiveSpeakerPositions = KSAUDIO_SPEAKER_STEREO;
    m_StereoSpeakerGeometry  = KSAUDIO_STEREO_SPEAKER_GEOMETRY_WIDE;

	NTSTATUS ntStatus = m_FilterFactory->AddEventHandler(EventCallbackRoutine, this, &m_EventHandle);

	if (NT_SUCCESS(ntStatus))
	{
		m_ClockRateExtension = _FindClockRateExtension();
	}

	if (!NT_SUCCESS(ntStatus))
    {
        //
        // clean up our mess
        //

		// clean up AudioDevice
		if (m_AudioDevice)
		{
			m_AudioDevice->Release();
			m_AudioDevice = NULL;
		}

		// clean up UsbDevice
		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}

		// clean up KsAdapter
        if (m_KsAdapter)
        {
            m_KsAdapter->Release();
            m_KsAdapter = NULL;
        }
    }

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetDescription()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the filter description.
 * @details
 * Gets a pointer to a filter description. It provides a location
 * to deposit a pointer in filter's description structure. This is the
 * placeholder for the FromNode or ToNode fields in connections which
 * describe connections to the filter's pins.
 * @param
 * OutKsFilterDescriptor Pointer to the filter description.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS 
CAudioFilter::
GetDescription
(
	OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutKsFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetDescription]"));

	NTSTATUS ntStatus = m_FilterFactory->GetFilterDescription(OutKsFilterDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::DataRangeIntersection()
 *****************************************************************************
 *//*!
 * @brief
 * Data range intersection handler.
 * @details
 * Tests a data range intersection. Determines the highest quality
 * intersection of two data ranges.
 * @param
 * PinId Pin for which the data intersection is being determined.
 * @param
 * DataRange Pointer to KSDATARANGE structure that contains the data range
 * submitted by client in the data-range intersection property request.
 * @param
 * MatchingDataRange Pin's data range to be compared with client's data
 * range.
 * @param
 * OutputBufferLength Size of the buffer pointed to by the resultant format
 * parameter.
 * @param
 * ResultantFormat Pointer to a location to which the method outputs the
 * resultant format.
 * @param
 * ResultantFormatLength Pointer to a location to which the method outputs
 * the length of the resultant format. This is the actual length of the
 * resultant format that is placed at ResultantFormat. This should be less
 * than or equal to OutputBufferLength.
 * @return
 * DataRangeIntersection returns STATUS_SUCCESS if the call was successful.
 * Otherwise, the method returns an appropriate error code. The table below
 * shows some of the possible error codes.
 * @retval
 * STATUS_NOMATCH There is no intersection.
 * @retval
 * STATUS_NOT_IMPLEMENTED Defers data-intersection handling to the port driver's default handler.
 */
NTSTATUS
CAudioFilter::
DataRangeIntersection
(
    IN      ULONG           PinId,
    IN      PKSDATARANGE    DataRange,
    IN      PKSDATARANGE    MatchingDataRange,
    IN      ULONG           OutputBufferLength,
    OUT     PVOID           ResultantFormat,
    OUT     PULONG          ResultantFormatLength
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::DataRangeIntersection]"));

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilter::DataRangeIntersection] - PinId: 0x%x", PinId));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	// Find the actual subformat if it is extensible.
	GUID SubFormat = DataRange->SubFormat;

	if (IsEqualGUIDAligned(DataRange->SubFormat, KSDATAFORMAT_SUBTYPE_EXTENSIBLE))
	{
		if (IsEqualGUIDAligned(DataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
		{
			PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)((PKSDATAFORMAT)DataRange + 1);

			SubFormat = WaveFormatExt->SubFormat;
		}
		else if (IsEqualGUIDAligned(DataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
		{
            PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC((PKSDATAFORMAT)DataRange+1);

            PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;

			SubFormat = WaveFormatExt->SubFormat;
		}
	}

    if ((!IsEqualGUIDAligned(DataRange->MajorFormat, MatchingDataRange->MajorFormat) &&
         !IsEqualGUIDAligned(DataRange->MajorFormat, KSDATAFORMAT_TYPE_WILDCARD)) ||
		(!IsEqualGUIDAligned(SubFormat, MatchingDataRange->SubFormat) &&
		 !IsEqualGUIDAligned(SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD)) ||
		(!IsEqualGUIDAligned(DataRange->Specifier, MatchingDataRange->Specifier) &&
		 !IsEqualGUIDAligned(DataRange->Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD)))
    {
        ntStatus = STATUS_NO_MATCH;
    }

	if (NT_SUCCESS(ntStatus))
	{
		if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_NONE))
		{
            // There was no specifier. Return only the KSDATAFORMAT structure.

			// Validate return buffer size, if the request is only for the
            // size of the resultant structure, return it now.
            if (!OutputBufferLength)
            {
                *ResultantFormatLength = sizeof(KSDATAFORMAT);

                ntStatus = STATUS_BUFFER_OVERFLOW;
            }
            else if (OutputBufferLength < sizeof(KSDATAFORMAT))
            {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                ntStatus = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(ntStatus))
            {
                // Copy the data format structure.
                RtlCopyMemory(ResultantFormat, MatchingDataRange, sizeof(KSDATAFORMAT));

                *ResultantFormatLength = sizeof(KSDATAFORMAT);
            }
		}
		else
		{
			if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
			{
		        if (IsEqualGUIDAligned(MatchingDataRange->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
				{
					// For KSDATAFORMAT_SUBTYPE_PCM, return KSDATAFORMAT with WAVEFORMATEXTENSIBLE.

					// Validate return buffer size, if the request is only for the
					// size of the resultant structure, return it now.
					if (!OutputBufferLength)
					{
						*ResultantFormatLength = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEXTENSIBLE);

						ntStatus = STATUS_BUFFER_OVERFLOW;
					}
					else if (OutputBufferLength < sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEXTENSIBLE))
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
					else
					{
						ntStatus = STATUS_SUCCESS;
					}

					if (NT_SUCCESS(ntStatus))
					{
						// Fill in the structure the datarange structure.
						// KSDATARANGE and KSDATAFORMAT are the same.
						*(PKSDATAFORMAT)ResultantFormat = *MatchingDataRange;

						// Modify the size of the data format structure
						// to fit the WAVEFORMATEXTENSIBLE structure.
						((PKSDATAFORMAT)ResultantFormat)->FormatSize = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEXTENSIBLE);

						// Append the WAVEFORMATEXTENSIBLE structure.
						PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)((PKSDATAFORMAT)ResultantFormat + 1);

						WaveFormatExt->Format.wFormatTag			= WAVE_FORMAT_EXTENSIBLE;
						WaveFormatExt->dwChannelMask				= FindPin(PinId)->ChannelConfig(); 
						ULONG PreferredSampleRate = GetPreferredSampleRate(); m_UsePreferredSampleRate = (PreferredSampleRate != 0);
						WaveFormatExt->Format.nSamplesPerSec		= (m_UsePreferredSampleRate) ? PreferredSampleRate : PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumSampleFrequency;
						WaveFormatExt->Format.nChannels				= (USHORT)_FindPreferredFormatChannels(PinId, MatchingDataRange, WaveFormatExt->Format.nSamplesPerSec);
						WaveFormatExt->Format.wBitsPerSample		= (USHORT)_FindPreferredFormatBitResolution(PinId, MatchingDataRange, WaveFormatExt->Format.nSamplesPerSec, WaveFormatExt->Format.nChannels);
						WaveFormatExt->Format.nBlockAlign			= (WaveFormatExt->Format.wBitsPerSample * WaveFormatExt->Format.nChannels) / 8;
						WaveFormatExt->Format.nAvgBytesPerSec		= (WaveFormatExt->Format.nSamplesPerSec * WaveFormatExt->Format.nBlockAlign);
						WaveFormatExt->Format.cbSize				= 22; // Take care of this value. Borrow from DDK.
						WaveFormatExt->Samples.wValidBitsPerSample	= WaveFormatExt->Format.wBitsPerSample;
						WaveFormatExt->SubFormat					= KSDATAFORMAT_SUBTYPE_PCM;

						// Overwrite the sample size in the KSDATAFORMAT structure.
						((PKSDATAFORMAT)ResultantFormat)->SampleSize = WaveFormatExt->Format.nBlockAlign;

						*ResultantFormatLength = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEXTENSIBLE);
					}
				}
				else
				{
					// For all other KSDATAFORMAT_SUBTYPE_XXX, return KSDATAFORMAT_WAVEFORMATEX.

					// Validate return buffer size, if the request is only for the
					// size of the resultant structure, return it now.
					if (!OutputBufferLength)
					{
						*ResultantFormatLength = sizeof(KSDATAFORMAT_WAVEFORMATEX);

						ntStatus = STATUS_BUFFER_OVERFLOW;
					}
					else if (OutputBufferLength < sizeof(KSDATAFORMAT_WAVEFORMATEX))
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
					else
					{
						ntStatus = STATUS_SUCCESS;
					}

					if (NT_SUCCESS(ntStatus))
					{
						// Fill in the structure the datarange structure.
						// KSDATARANGE and KSDATAFORMAT are the same.
						*(PKSDATAFORMAT)ResultantFormat = *MatchingDataRange;

						// Modify the size of the data format structure to fit the WAVEFORMATEX structure.
						((PKSDATAFORMAT)ResultantFormat)->FormatSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);

						// Append the WAVEFORMATEX structure.
						PWAVEFORMATEX WaveFormat = (PWAVEFORMATEX)((PKSDATAFORMAT)ResultantFormat + 1);

						WaveFormat->wFormatTag			= EXTRACT_WAVEFORMATEX_ID(&MatchingDataRange->SubFormat);
						WaveFormat->nSamplesPerSec		= PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumSampleFrequency;
						WaveFormat->nChannels			= (USHORT)_FindPreferredFormatChannels(PinId, MatchingDataRange, WaveFormat->nSamplesPerSec);
						WaveFormat->wBitsPerSample		= (USHORT)PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumBitsPerSample;
						WaveFormat->nBlockAlign			= (WaveFormat->wBitsPerSample * WaveFormat->nChannels) / 8;
						WaveFormat->nAvgBytesPerSec		= (WaveFormat->nSamplesPerSec * WaveFormat->nBlockAlign);
						WaveFormat->cbSize				= 0;

						// Overwrite the sample size in the KSDATAFORMAT structure.
						((PKSDATAFORMAT)ResultantFormat)->SampleSize = WaveFormat->nBlockAlign;

						*ResultantFormatLength = sizeof(KSDATAFORMAT_WAVEFORMATEX);
					}
				}
			}
			else if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
			{
		        if (IsEqualGUIDAligned(MatchingDataRange->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
				{
					// For KSDATAFORMAT_SUBTYPE_PCM, return KSDATAFORMAT_DSOUND with WAVEFORMATEXTENSIBLE.

					// Validate return buffer size, if the request is only for the
					// size of the resultant structure, return it now.
					if (!OutputBufferLength)
					{
						*ResultantFormatLength = sizeof(KSDATAFORMAT_DSOUND) - sizeof(WAVEFORMATEX) + sizeof(WAVEFORMATEXTENSIBLE);

						ntStatus = STATUS_BUFFER_OVERFLOW;
					}
					else if (OutputBufferLength < sizeof(KSDATAFORMAT_DSOUND) - sizeof(WAVEFORMATEX) + sizeof(WAVEFORMATEXTENSIBLE))
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
					else
					{
						ntStatus = STATUS_SUCCESS;
					}

					if (NT_SUCCESS(ntStatus))
					{
						// Fill in the structure the datarange structure.
						// KSDATARANGE and KSDATAFORMAT are the same.
						*(PKSDATAFORMAT)ResultantFormat = *MatchingDataRange;

						// Modify the size of the data format structure
						// to fit the WAVEFORMATEXTENSIBLE structure.
						((PKSDATAFORMAT)ResultantFormat)->FormatSize = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEXTENSIBLE);

						// Buffer description.
                        PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC((PKSDATAFORMAT)ResultantFormat+1);

                        BufferDesc->Flags = 0 ; // KSDSOUND_BUFFER_LOCHARDWARE ??
                        BufferDesc->Control = 0 ; //FIXME: KSDSOUND_BUFFER_CTRL_FREQUENCY ask the pin whether it support SRC.

						// Append the WAVEFORMATEXTENSIBLE structure.
                        PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;

						WaveFormatExt->Format.wFormatTag			= WAVE_FORMAT_EXTENSIBLE;
						WaveFormatExt->dwChannelMask				= FindPin(PinId)->ChannelConfig(); 
						ULONG PreferredSampleRate = GetPreferredSampleRate(); m_UsePreferredSampleRate = (PreferredSampleRate != 0);
						WaveFormatExt->Format.nSamplesPerSec		= (m_UsePreferredSampleRate) ? PreferredSampleRate : PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumSampleFrequency;
						WaveFormatExt->Format.nChannels				= (USHORT)_FindPreferredFormatChannels(PinId, MatchingDataRange, WaveFormatExt->Format.nSamplesPerSec);
						WaveFormatExt->Format.wBitsPerSample		= (USHORT)_FindPreferredFormatBitResolution(PinId, MatchingDataRange, WaveFormatExt->Format.nSamplesPerSec, WaveFormatExt->Format.nChannels);
						WaveFormatExt->Format.nBlockAlign			= (WaveFormatExt->Format.wBitsPerSample * WaveFormatExt->Format.nChannels) / 8;
						WaveFormatExt->Format.nAvgBytesPerSec		= (WaveFormatExt->Format.nSamplesPerSec * WaveFormatExt->Format.nBlockAlign);
						WaveFormatExt->Format.cbSize				= 22; // Take care of this value. Borrow from DDK.
						WaveFormatExt->Samples.wValidBitsPerSample	= WaveFormatExt->Format.wBitsPerSample;
						WaveFormatExt->SubFormat					= KSDATAFORMAT_SUBTYPE_PCM;

						// Overwrite the sample size in the KSDATAFORMAT structure.
						((PKSDATAFORMAT)ResultantFormat)->SampleSize = WaveFormatExt->Format.nBlockAlign;

						*ResultantFormatLength = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEXTENSIBLE);
					}
				}
				else
				{
					// For all other KSDATAFORMAT_SUBTYPE_XXX, return KSDATAFORMAT_DSOUND.

					// Validate return buffer size, if the request is only for the
					// size of the resultant structure, return it now.
					if (!OutputBufferLength)
					{
						*ResultantFormatLength = sizeof(KSDATAFORMAT_DSOUND);

						ntStatus = STATUS_BUFFER_OVERFLOW;
					}
					else if (OutputBufferLength < sizeof(KSDATAFORMAT_DSOUND))
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
					else
					{
						ntStatus = STATUS_SUCCESS;
					}

					if (NT_SUCCESS(ntStatus))
					{
						// Fill in the structure the datarange structure.
						// KSDATARANGE and KSDATAFORMAT are the same.
						*(PKSDATAFORMAT)ResultantFormat = *MatchingDataRange;

						// Modify the size of the data format structure to fit the WAVEFORMATEX structure.
						((PKSDATAFORMAT)ResultantFormat)->FormatSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);

						// Buffer description.
                        PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC((PKSDATAFORMAT)ResultantFormat+1);

                        BufferDesc->Flags = 0 ; // KSDSOUND_BUFFER_LOCHARDWARE ??
                        BufferDesc->Control = 0 ; //FIXME: KSDSOUND_BUFFER_CTRL_FREQUENCY ask the pin whether it support SRC.

						// Append the WAVEFORMATEX structure.
                        PWAVEFORMATEX WaveFormat = &BufferDesc->WaveFormatEx;

						WaveFormat->wFormatTag			= EXTRACT_WAVEFORMATEX_ID(&MatchingDataRange->SubFormat);
						WaveFormat->nSamplesPerSec		= PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumSampleFrequency;
						WaveFormat->nChannels			= (USHORT)_FindPreferredFormatChannels(PinId, MatchingDataRange, WaveFormat->nSamplesPerSec);
						WaveFormat->wBitsPerSample		= (USHORT)PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumBitsPerSample;
						WaveFormat->nBlockAlign			= (WaveFormat->wBitsPerSample * WaveFormat->nChannels) / 8;
						WaveFormat->nAvgBytesPerSec		= (WaveFormat->nSamplesPerSec * WaveFormat->nBlockAlign);
						WaveFormat->cbSize				= 0;

						// Overwrite the sample size in the KSDATAFORMAT structure.
						((PKSDATAFORMAT)ResultantFormat)->SampleSize = WaveFormat->nBlockAlign;

						*ResultantFormatLength = sizeof(KSDATAFORMAT_DSOUND);
					}
				}
			}
			else
			{
				// Unsupported specifier. It shouldn't be here. If you added the specifier to 
				// AllocInitPinDataRanges() in Descriptor.cpp, make sure it is handled here as
				// well. 

				// Validate return buffer size, if the request is only for the
				// size of the resultant structure, return it now.
				if (!OutputBufferLength)
				{
					*ResultantFormatLength = MatchingDataRange->FormatSize;

					ntStatus = STATUS_BUFFER_OVERFLOW;
				}
				else if (OutputBufferLength < MatchingDataRange->FormatSize)
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				else
				{
					ntStatus = STATUS_SUCCESS;
				}

				if (NT_SUCCESS(ntStatus))
				{
					// Copy the data format structure.
					RtlCopyMemory(ResultantFormat, MatchingDataRange, MatchingDataRange->FormatSize);

					*ResultantFormatLength = MatchingDataRange->FormatSize;
				}
			}
		}
	}

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilter::DataRangeIntersection] - ntStatus: 0x%x", ntStatus));

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::_FindPreferredFormatChannels()
 *****************************************************************************
 *//*!
 * @brief
 * Find the matching format data range's channels.
 * @details
 * The pin descriptor's data range only allow us to specify maximum channels
 * supported, not the minimum channels. This lead to the possibility that a
 * client could specify a nChannels < MaximumChannels, which doesn't work for
 * an USB-Audio device since nChannels has to be equal to MaximumChannels. So
 * this routine finds the closest data range matching the format specified,
 * and returns the data range's MaximumChannels instead.
 */
ULONG
CAudioFilter::
_FindPreferredFormatChannels
(
    IN      ULONG           PinId,
    IN      PKSDATARANGE	MatchingDataRange,
	IN		ULONG			SampleFrequency
)
{
    PAGED_CODE();

    ASSERT(MatchingDataRange);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::_FindPreferredFormatChannels]"));

	ULONG FormatChannels = PKSDATARANGE_AUDIO(MatchingDataRange)->MaximumChannels;

    PKSFILTER_DESCRIPTOR FilterDescriptor; GetDescription(&FilterDescriptor);

    if (PinId < FilterDescriptor->PinDescriptorsCount)
    {
		PKSPIN_DESCRIPTOR_EX Pin = PKSPIN_DESCRIPTOR_EX(&FilterDescriptor->PinDescriptors[PinId]);

        PKSDATARANGE * DataRanges = (PKSDATARANGE *)Pin->PinDescriptor.DataRanges;

        if (DataRanges)
        {
            for (ULONG i = 0; i < Pin->PinDescriptor.DataRangesCount; i++)
            {
                PKSDATARANGE_AUDIO DataRangeAudio = PKSDATARANGE_AUDIO(DataRanges[i]);

				// Find the actual subformat if it is extensible.
				GUID SubFormat = MatchingDataRange->SubFormat;

				if (IsEqualGUIDAligned(MatchingDataRange->SubFormat, KSDATAFORMAT_SUBTYPE_EXTENSIBLE))
				{
					if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
					{
						PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)(MatchingDataRange + 1);

						SubFormat = WaveFormatExt->SubFormat;
					}
					else if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
					{
						PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(MatchingDataRange + 1);

						PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;

						SubFormat = WaveFormatExt->SubFormat;
					}
				}

				// KSDATAFORMAT contains three GUIDs to support extensible format.  The first two GUIDs identify 
				// the type of data.  The third indicates the type of specifier used to indicate format specifics.
                if (IsEqualGUIDAligned(MatchingDataRange->MajorFormat, DataRangeAudio->DataRange.MajorFormat) &&
                    IsEqualGUIDAligned(SubFormat, DataRangeAudio->DataRange.SubFormat) &&
                    IS_VALID_WAVEFORMATEX_GUID(&MatchingDataRange->SubFormat) &&
                    IsEqualGUIDAligned(MatchingDataRange->Specifier, DataRangeAudio->DataRange.Specifier))
                {
					PKSDATARANGE_AUDIO MatchingDataRangeAudio = PKSDATARANGE_AUDIO(MatchingDataRange);

					if ((MatchingDataRangeAudio->MinimumBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                        (MatchingDataRangeAudio->MaximumBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                        (SampleFrequency >= DataRangeAudio->MinimumSampleFrequency) &&
                        (SampleFrequency <= DataRangeAudio->MaximumSampleFrequency))
                    {
						if (MatchingDataRangeAudio->MaximumChannels == DataRangeAudio->MaximumChannels)
						{
							// Found the exact match.
							FormatChannels = DataRangeAudio->MaximumChannels;

							break;
						}
						else if (MatchingDataRangeAudio->MaximumChannels < DataRangeAudio->MaximumChannels) 
						{
							// Not an exact match, see if we can do better.
							if (FormatChannels > MatchingDataRangeAudio->MaximumChannels)
							{
								// See if it closer to the number of channels that we are looking for.
								if (FormatChannels > DataRangeAudio->MaximumChannels)
								{
									// Closer to what we are looking for.
									FormatChannels = DataRangeAudio->MaximumChannels;
								}
								else
								{
									// Nope.
								}
							}
							else
							{
								// This is good for now.
								FormatChannels = DataRangeAudio->MaximumChannels;
							}
						}
                    }
                }
            }
        }
    }

    return FormatChannels;
}

/*****************************************************************************
 * CAudioFilter::_FindPreferredFormatBitResolution()
 *****************************************************************************
 *//*!
 * @brief
 * Find the matching format data range's bit resolution.
 */
ULONG
CAudioFilter::
_FindPreferredFormatBitResolution
(
    IN      ULONG           PinId,
    IN      PKSDATARANGE	MatchingDataRange,
	IN		ULONG			SampleFrequency,
	IN		ULONG			MaximumChannels
)
{
    PAGED_CODE();

    ASSERT(MatchingDataRange);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::_FindPreferredFormatBitDepth]"));

	ULONG BitResolution = PKSDATARANGE_AUDIO(MatchingDataRange)->MinimumBitsPerSample;

    PKSFILTER_DESCRIPTOR FilterDescriptor; GetDescription(&FilterDescriptor);

    if (PinId < FilterDescriptor->PinDescriptorsCount)
    {
		PKSPIN_DESCRIPTOR_EX Pin = PKSPIN_DESCRIPTOR_EX(&FilterDescriptor->PinDescriptors[PinId]);

        PKSDATARANGE * DataRanges = (PKSDATARANGE *)Pin->PinDescriptor.DataRanges;

        if (DataRanges)
        {
            for (ULONG i = 0; i < Pin->PinDescriptor.DataRangesCount; i++)
            {
                PKSDATARANGE_AUDIO DataRangeAudio = PKSDATARANGE_AUDIO(DataRanges[i]);

				// Find the actual subformat if it is extensible.
				GUID SubFormat = MatchingDataRange->SubFormat;

				if (IsEqualGUIDAligned(MatchingDataRange->SubFormat, KSDATAFORMAT_SUBTYPE_EXTENSIBLE))
				{
					if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
					{
						PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)(MatchingDataRange + 1);

						SubFormat = WaveFormatExt->SubFormat;
					}
					else if (IsEqualGUIDAligned(MatchingDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
					{
						PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(MatchingDataRange + 1);

						PWAVEFORMATEXTENSIBLE WaveFormatExt = (PWAVEFORMATEXTENSIBLE)&BufferDesc->WaveFormatEx;

						SubFormat = WaveFormatExt->SubFormat;
					}
				}

				// KSDATAFORMAT contains three GUIDs to support extensible format.  The first two GUIDs identify 
				// the type of data.  The third indicates the type of specifier used to indicate format specifics.
                if (IsEqualGUIDAligned(MatchingDataRange->MajorFormat, DataRangeAudio->DataRange.MajorFormat) &&
                    IsEqualGUIDAligned(SubFormat, DataRangeAudio->DataRange.SubFormat) &&
                    IS_VALID_WAVEFORMATEX_GUID(&MatchingDataRange->SubFormat) &&
                    IsEqualGUIDAligned(MatchingDataRange->Specifier, DataRangeAudio->DataRange.Specifier))
                {
					PKSDATARANGE_AUDIO MatchingDataRangeAudio = PKSDATARANGE_AUDIO(MatchingDataRange);

					if ((MatchingDataRangeAudio->MinimumBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                        (MatchingDataRangeAudio->MaximumBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                        (MaximumChannels == DataRangeAudio->MaximumChannels) &&
                        (SampleFrequency >= DataRangeAudio->MinimumSampleFrequency) &&
                        (SampleFrequency <= DataRangeAudio->MaximumSampleFrequency))
                    {
						BitResolution = PKSDATARANGE_AUDIO_EX(DataRangeAudio)->BitResolution;
                    }
                }
            }
        }
    }

    return BitResolution;
}

/*****************************************************************************
 * CAudioFilter::ValidateFormat()
 *****************************************************************************
 *//*!
 * @brief
 * Validates a wave format.
 * @details
 * Match the specified @em Format against the supported format.
 * @param
 * PinId Identifier of the pin.
 * @param
 * Capture TRUE for an input (capture) channel, and FALSE for an output
 * (playback) channel.
 * @param
 * Format Pointer to a KSDATAFORMAT structure indicating the format to match.
 * @return
 * Returns STATUS_SUCCESS if the specified @em Format is supported.
 * Otherwise, returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
ValidateFormat
(
    IN      ULONG           PinId,
    IN      BOOLEAN         Capture,
    IN      PKSDATAFORMAT   Format
)
{
    PAGED_CODE();

    ASSERT(Format);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::ValidateFormat] - PinId: 0x%x", PinId));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    PKSFILTER_DESCRIPTOR FilterDescriptor; GetDescription(&FilterDescriptor);

    if (PinId < FilterDescriptor->PinDescriptorsCount)
    {
		ntStatus = STATUS_NO_MATCH;

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
                                // A WAVEFORMATEX structure should appear after the generic KSDATAFORMAT
                                // if the GUIDs turn out as we expect.
                                PWAVEFORMATEX waveFormat = PWAVEFORMATEX(Format + 1);

								_DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - Tag: 0x%x, Frequency: %d, Channels: %d, bps: %d, align: %d",
															waveFormat->wFormatTag,
															waveFormat->nSamplesPerSec,
															waveFormat->nChannels,
															waveFormat->wBitsPerSample,
															waveFormat->nBlockAlign) );

                                if ((waveFormat->nChannels == DataRangeAudio->MaximumChannels) &&
                                    (waveFormat->wBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                                    (waveFormat->wBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                                    (waveFormat->nSamplesPerSec >= DataRangeAudio->MinimumSampleFrequency) &&
                                    (waveFormat->nSamplesPerSec <= DataRangeAudio->MaximumSampleFrequency))
                                {
                                    switch (waveFormat->wFormatTag)
                                    {
                                        case WAVE_FORMAT_PCM:
                                        {
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - PCM"));

											if ((waveFormat->wBitsPerSample / 8 * waveFormat->nChannels) == waveFormat->nBlockAlign/*PACKED*/)
											{
												ntStatus = STATUS_SUCCESS;

												if (m_UsePreferredSampleRate)
												{
													ULONG PreferredSampleRate = GetPreferredSampleRate();

													if (PreferredSampleRate != waveFormat->nSamplesPerSec)
													{
														ntStatus = STATUS_NO_MATCH;
													}
												}
											}
                                        }
                                        break;

                                        case WAVE_FORMAT_EXTENSIBLE:
                                        {
											//[NI Tao]if WDM stream, priority is set to LOW
											m_UsePreferredSampleRate = TRUE;
											//end
											
											if ((waveFormat->wBitsPerSample / 8 * waveFormat->nChannels) == waveFormat->nBlockAlign/*PACKED*/)
											{
												ntStatus = STATUS_SUCCESS;

												PWAVEFORMATEXTENSIBLE waveFormatExt = PWAVEFORMATEXTENSIBLE(Format + 1);

												if (IsEqualGUIDAligned(waveFormatExt->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
												{
													_DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - Extensible SubFormat: PCM: ValidBitsPerSample: %d ChannelMask: %x",
																				waveFormatExt->Samples.wValidBitsPerSample,
																				waveFormatExt->dwChannelMask));

													if (m_UsePreferredSampleRate)
													{
														ULONG PreferredSampleRate = GetPreferredSampleRate();

														if (PreferredSampleRate != waveFormat->nSamplesPerSec)
														{
															ntStatus = STATUS_NO_MATCH;

															//[NI Tao]force to trigger background svc
															this->TriggerEvent(0);
															//end
														}
													}
												}
												else
												{
													_DbgPrintF(DEBUGLVL_BLAB, ("Extensible Sub Format: {%08lX-%04lX-%04lX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
																			waveFormatExt->SubFormat.Data1,
																			waveFormatExt->SubFormat.Data2,
																			waveFormatExt->SubFormat.Data3,
																			waveFormatExt->SubFormat.Data4[0],
																			waveFormatExt->SubFormat.Data4[1],
																			waveFormatExt->SubFormat.Data4[2],
																			waveFormatExt->SubFormat.Data4[3],
																			waveFormatExt->SubFormat.Data4[4],
																			waveFormatExt->SubFormat.Data4[5],
																			waveFormatExt->SubFormat.Data4[6],
																			waveFormatExt->SubFormat.Data4[7]));
												}
											}
                                        }
                                        break;

                                        case WAVE_FORMAT_ADPCM:
                                        {
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - ADPCM"));

											ntStatus = STATUS_SUCCESS;
                                        }
                                        break;

                                        case WAVE_FORMAT_DOLBY_AC3_SPDIF:
                                        {
											// DOLBY AC3
											_DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - DOLBY AC3 SPDIF"));

											ntStatus = STATUS_SUCCESS;
										}
                                        break;

                                        case WAVE_FORMAT_WMA_OVER_SPDIF:
                                        {
                                            // WMA-over-S/PDIF
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - WMA-over-S/PDIF"));

											ntStatus = STATUS_SUCCESS;
										}
                                        break;

                                        default:
										{
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - 0x%04x", waveFormat->wFormatTag));

											ntStatus = STATUS_SUCCESS;
										}
                                        break;
                                    }
                                }
                            }
                        }
                        else if (IsEqualGUIDAligned(Format->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
                        {
                            if (Format->FormatSize >= sizeof(KSDATAFORMAT_DSOUND))
                            {
                                // A KSDSOUND_BUFFERDESC structure should appear after the generic KSDATAFORMAT
                                // if the GUIDs turn out as we expect.
                                PKSDSOUND_BUFFERDESC BufferDesc = PKSDSOUND_BUFFERDESC(Format + 1);
                                PWAVEFORMATEX waveFormat = &BufferDesc->WaveFormatEx;

								_DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - Tag: 0x%x, Frequency: %d, Channels: %d, bps: %d, align: %d",
															waveFormat->wFormatTag,
															waveFormat->nSamplesPerSec,
															waveFormat->nChannels,
															waveFormat->wBitsPerSample,
															waveFormat->nBlockAlign) );

								if ((waveFormat->nChannels == DataRangeAudio->MaximumChannels) &&
                                    (waveFormat->wBitsPerSample >= DataRangeAudio->MinimumBitsPerSample) &&
                                    (waveFormat->wBitsPerSample <= DataRangeAudio->MaximumBitsPerSample) &&
                                    (waveFormat->nSamplesPerSec >= DataRangeAudio->MinimumSampleFrequency) &&
                                    (waveFormat->nSamplesPerSec <= DataRangeAudio->MaximumSampleFrequency))
                                {
                                    switch (waveFormat->wFormatTag)
                                    {
                                        case WAVE_FORMAT_PCM:
                                        {
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - PCM"));

											if ((waveFormat->wBitsPerSample / 8 * waveFormat->nChannels) == waveFormat->nBlockAlign/*PACKED*/)
											{
												ntStatus = STATUS_SUCCESS;

												if (m_UsePreferredSampleRate)
												{
													ULONG PreferredSampleRate = GetPreferredSampleRate();

													if (PreferredSampleRate != waveFormat->nSamplesPerSec)
													{
														ntStatus = STATUS_NO_MATCH;
													}
												}
											}
                                        }
                                        break;

                                        case WAVE_FORMAT_EXTENSIBLE:
                                        {
											if ((waveFormat->wBitsPerSample / 8 * waveFormat->nChannels) == waveFormat->nBlockAlign/*PACKED*/)
											{
												ntStatus = STATUS_SUCCESS;

												PWAVEFORMATEXTENSIBLE waveFormatExt = PWAVEFORMATEXTENSIBLE(Format + 1);

												if (IsEqualGUIDAligned(waveFormatExt->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
												{
													_DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - Extensible SubFormat: PCM: ValidBitsPerSample: %d ChannelMask: %x",
																				waveFormatExt->Samples.wValidBitsPerSample,
																				waveFormatExt->dwChannelMask));

													if (m_UsePreferredSampleRate)
													{
														ULONG PreferredSampleRate = GetPreferredSampleRate();

														if (PreferredSampleRate != waveFormat->nSamplesPerSec)
														{
															ntStatus = STATUS_NO_MATCH;
														}
													}
												}
												else
												{
													_DbgPrintF(DEBUGLVL_BLAB, ("Extensible Sub Format: {%08lX-%04lX-%04lX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
																			waveFormatExt->SubFormat.Data1,
																			waveFormatExt->SubFormat.Data2,
																			waveFormatExt->SubFormat.Data3,
																			waveFormatExt->SubFormat.Data4[0],
																			waveFormatExt->SubFormat.Data4[1],
																			waveFormatExt->SubFormat.Data4[2],
																			waveFormatExt->SubFormat.Data4[3],
																			waveFormatExt->SubFormat.Data4[4],
																			waveFormatExt->SubFormat.Data4[5],
																			waveFormatExt->SubFormat.Data4[6],
																			waveFormatExt->SubFormat.Data4[7]));
												}
											}
                                        }
                                        break;

                                        case WAVE_FORMAT_ADPCM:
                                        {
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - ADPCM"));

											ntStatus = STATUS_SUCCESS;
                                        }
                                        break;

                                        case WAVE_FORMAT_DOLBY_AC3_SPDIF:
                                        {
											// DOLBY AC3
											_DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - DOLBY AC3 SPDIF"));

											ntStatus = STATUS_SUCCESS;
										}
                                        break;

                                        case WAVE_FORMAT_WMA_OVER_SPDIF:
                                        {
                                            // WMA-over-S/PDIF
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - WMA-over-S/PDIF"));

											ntStatus = STATUS_SUCCESS;
										}
                                        break;

                                        default:
										{
                                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - 0x%04x", waveFormat->wFormatTag));

											ntStatus = STATUS_SUCCESS;
										}
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            _DbgPrintF(DEBUGLVL_BLAB, ("[ValidateDataFormat] - Unsupported specifier."));
                        }
                    }

                    if (NT_SUCCESS(ntStatus)) break;
                }
            }
        }
    }

    return ntStatus;
}

#include <stdio.h>
/*****************************************************************************
 * CAudioFilter::GetLatency()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CAudioFilter::
GetLatency
(
	IN		ULONG	PinId,
	IN		ULONG	SampleRate,
    OUT     PKSTIME	OutLatency
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetLatency]"));

	if (OutLatency)
	{
		// TODO: Is there a better way to determine the latency other than hardwiring it in the 
		// configuration file ??

		//HKR,Configuration,CFGFILE,,%CFGFILE%
		WCHAR ConfigurationFile[MAX_PATH] = {0};

		if (NT_SUCCESS(m_KsAdapter->RegistryReadFromDriverSubKey(L"Configuration", L"CFGFILE", ConfigurationFile, sizeof(ConfigurationFile), NULL, NULL)))
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetLatency] - Configuration file: %ws", ConfigurationFile));

			PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor; m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

			CHAR SectionName[64]; sprintf(SectionName, "USB\\VID_%04X&PID_%04X.%03X.Audio.Latency.%d", UsbDeviceDescriptor->idVendor, UsbDeviceDescriptor->idProduct, UsbDeviceDescriptor->bcdDevice, PinId);

			CHAR SampleRateString[16]; sprintf(SampleRateString, "%d", SampleRate);

			OutLatency->Time = DrvProfileGetLong(SectionName, SampleRateString, -1, ConfigurationFile);

			if (OutLatency->Time == -1)
			{
				sprintf(SectionName, "USB\\VID_%04X&PID_%04X.Audio.Latency.%d", UsbDeviceDescriptor->idVendor, UsbDeviceDescriptor->idProduct, PinId);

				OutLatency->Time = DrvProfileGetLong(SectionName, SampleRateString, 0, ConfigurationFile);
			}

			OutLatency->Numerator = 1;
			OutLatency->Denominator = 1;
		}
		else
		{
			OutLatency->Time = 0;
			OutLatency->Numerator = 1;
			OutLatency->Denominator = 1;
		}
	}

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioFilter::GetPreferredSampleRate()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
CAudioFilter::
GetPreferredSampleRate
(	void
)
{
    PAGED_CODE();

	ULONG PreferredSampleRate = 0;

	if (m_ClockRateExtension)
	{
		UCHAR Rate = XU_CLOCK_RATE_SR_UNSPECIFIED;

		NTSTATUS ntStatus = m_ClockRateExtension->ReadParameter(REQUEST_CUR, KSPROPERTY_XU_CLOCK_RATE_SELECTOR+1, 0, &Rate, sizeof(Rate), NULL);

		if (NT_SUCCESS(ntStatus))
		{
			switch (Rate)
			{
				case XU_CLOCK_RATE_SR_44kHz:
				{
					PreferredSampleRate = 44100;
				}
				break;

				case XU_CLOCK_RATE_SR_48kHz:
				{
					PreferredSampleRate = 48000;
				}
				break;

				case XU_CLOCK_RATE_SR_88kHz:
				{
					PreferredSampleRate = 88200;
				}
				break;

				case XU_CLOCK_RATE_SR_96kHz:
				{
					PreferredSampleRate = 96000;
				}
				break;

				case XU_CLOCK_RATE_SR_176kHz:
				{
					PreferredSampleRate = 176400;
				}
				break;

				case XU_CLOCK_RATE_SR_192kHz:
				{
					PreferredSampleRate = 192000;
				}
				break;

				default:
				{
					PreferredSampleRate = 0;
				}
				break;
			}
		}
	}

	return PreferredSampleRate;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioFilter::TriggerEvent()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioFilter::
TriggerEvent
(
	IN		ULONG	EventType
)
{
	switch (EventType)
	{
		case 0: // Clock rate change event
		{
			if (m_ClockRateExtension)
			{
				SimulateStatusInterrupt(USB_AUDIO_STATUS_TYPE_ORIGINATOR_AC_INTERFACE | USB_AUDIO_STATUS_TYPE_INTERRUPT_PENDING, m_ClockRateExtension->UnitID());
			}
		}
		break;
	}
}

/*****************************************************************************
 * CAudioFilter::FindPin()
 *****************************************************************************
 *//*!
 * @brief
 */
PFILTER_PIN_DESCRIPTOR 
CAudioFilter::
FindPin
(
	IN		ULONG	PinId
)
{
	return m_FilterFactory->FindPin(PinId);
}

/*****************************************************************************
 * CAudioFilter::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
PNODE_DESCRIPTOR 
CAudioFilter::
FindNode
(
	IN		ULONG	NodeId
)
{
	return m_FilterFactory->FindNode(NodeId);
}

/*****************************************************************************
 * CAudioFilter::SimulateStatusInterrupt()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioFilter::
SimulateStatusInterrupt
(
	IN		UCHAR	StatusType,
	IN		UCHAR	Originator
)
{
	USB_AUDIO_STATUS_WORD StatusWord;

	StatusWord.bmStatusType = StatusType;
	StatusWord.bOriginator = Originator;

	m_AudioDevice->OnStatusInterrupt(1, &StatusWord, this);
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioFilter::_FindClockRateExtension()
 *****************************************************************************
 *//*!
 * @brief
 */
PNODE_DESCRIPTOR 
CAudioFilter::
_FindClockRateExtension
(	void
)
{
    PAGED_CODE();

	PNODE_DESCRIPTOR ClockRateExtension = NULL;

	PKSFILTER_DESCRIPTOR KsFilterDescriptor = NULL;

	NTSTATUS ntStatus = m_FilterFactory->GetFilterDescription(&KsFilterDescriptor);

	if (NT_SUCCESS(ntStatus))
	{
		ULONG ClockRateExtensionNodeId = ULONG(-1);

		for (ULONG i=0; i<KsFilterDescriptor->NodeDescriptorsCount; i++)
		{
			BOOL FoundClockRateExtensionNode = FALSE;

			PKSNODE_DESCRIPTOR KsNodeDescriptor = PKSNODE_DESCRIPTOR(&KsFilterDescriptor->NodeDescriptors[i]);

			if (KsNodeDescriptor)
			{
				if (IsEqualGUIDAligned(*KsNodeDescriptor->Type, KSNODETYPE_DEV_SPECIFIC))
				{
					PKSAUTOMATION_TABLE AutomationTable = PKSAUTOMATION_TABLE(KsNodeDescriptor->AutomationTable);

					if (AutomationTable)
					{
						for (ULONG j=0; j<AutomationTable->PropertySetsCount; j++)
						{
							PKSPROPERTY_SET PropertySet = PKSPROPERTY_SET(&AutomationTable->PropertySets[j]);

							if (PropertySet)
							{
								if (IS_COMPATIBLE_XU_PROPSETID(PropertySet->Set))
								{
									USHORT ExtensionCode = EXTRACT_XU_CODE(PropertySet->Set);

									if (ExtensionCode == XU_CODE_CLOCK_RATE)
									{
										FoundClockRateExtensionNode = TRUE;
										break;
									}
								}
							}
						}
					}
				}
			}

			if (FoundClockRateExtensionNode) 
			{
				ClockRateExtensionNodeId = i;
				break;
			}
		}

		ClockRateExtension = m_FilterFactory->FindNode(ClockRateExtensionNodeId);
	}

	return ClockRateExtension;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioFilter::PinPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter pin properties.
 */
DEFINE_KSPROPERTY_TABLE(CAudioFilter::PinPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_PIN_NAME,					// Id
		CAudioFilter::GetPinName,				// GetPropertyHandler or GetSupported
		sizeof(KSP_PIN),						// MinProperty
		0,										// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportPinName,			// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_PIN_GLOBALCINSTANCES,		// Id
		CAudioFilter::GetGlobalPinCInstances,	// GetPropertyHandler or GetSupported
		sizeof(KSP_PIN),						// MinProperty
		sizeof(KSPIN_CINSTANCES),				// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		NULL,									// SupportHandler
		0										// SerializedSize
	)
};	

/*****************************************************************************
 * CAudioFilter::AudioPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter pin properties.
 */
DEFINE_KSPROPERTY_TABLE(CAudioFilter::AudioPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_COPY_PROTECTION,			// Id
		CAudioFilter::GetCopyProtectControl,		// GetPropertyHandler or GetSupported
		sizeof(KSP_PIN),							// MinProperty
		sizeof(KSAUDIO_COPY_PROTECTION),			// MinData
		CAudioFilter::SetCopyProtectControl,		// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		0											// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_LATENCY,					// Id
		CAudioFilter::GetLatencyControl,			// GetPropertyHandler or GetSupported
		sizeof(KSP_PIN),							// MinProperty
		sizeof(KSTIME),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		0											// SerializedSize
	)
};	

/*****************************************************************************
 * CAudioFilter::ControlPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter properties.
 */
DEFINE_KSPROPERTY_TABLE(CAudioFilter::ControlPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR,			// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_DEVICE_DESCRIPTOR),			// MinProperty
		sizeof(USB_DEVICE_DESCRIPTOR),						// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CONFIGURATION_DESCRIPTOR,	// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CONFIGURATION_DESCRIPTOR),		// MinProperty
		sizeof(USB_CONFIGURATION_DESCRIPTOR),				// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_INTERFACE_DESCRIPTOR,		// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_INTERFACE_DESCRIPTOR),			// MinProperty
		sizeof(USB_INTERFACE_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_ENDPOINT_DESCRIPTOR,		// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_ENDPOINT_DESCRIPTOR),			// MinProperty
		sizeof(USB_ENDPOINT_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR,			// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_STRING_DESCRIPTOR),			// MinProperty
		sizeof(USB_STRING_DESCRIPTOR),						// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR,// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR),	// MinProperty
		sizeof(USB_INTERFACE_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR,	// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR),	// MinProperty
		sizeof(USB_ENDPOINT_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND,			// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CUSTOM_COMMAND),				// MinProperty
		0,													// MinData
		CAudioFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK,		// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		0,													// MinData
		CAudioFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK,	// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		0,													// MinData
		CAudioFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_PIN_OUTPUT_CFIFO_BUFFERS,	// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		sizeof(ULONG),										// MinData
		CAudioFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_PIN_INPUT_CFIFO_BUFFERS,	// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		sizeof(ULONG),										// MinData
		CAudioFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_PIN_SYNCHRONIZE_START_FRAME,// Id
		CAudioFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		sizeof(ULONG),										// MinData
		CAudioFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	)
};	

/*****************************************************************************
 * CAudioFilter::PropertySetTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(CAudioFilter::PropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Pin,								// Set
		SIZEOF_ARRAY(CAudioFilter::PinPropertyTable),	// PropertiesCount
		CAudioFilter::PinPropertyTable,					// PropertyItem
		0,												// FastIoCount
		NULL											// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,								// Set
		SIZEOF_ARRAY(CAudioFilter::AudioPropertyTable),	// PropertiesCount
		CAudioFilter::AudioPropertyTable,				// PropertyItem
		0,												// FastIoCount
		NULL											// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_DeviceControl,							// Set
		SIZEOF_ARRAY(CAudioFilter::ControlPropertyTable),	// PropertiesCount
		CAudioFilter::ControlPropertyTable,					// PropertyItem
		0,													// FastIoCount
		NULL												// FastIoTable
	)
};

/*****************************************************************************
 * CAudioFilter::AutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Filter automation table.
 */
DEFINE_KSAUTOMATION_TABLE(CAudioFilter::AutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(CAudioFilter::PropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

/*****************************************************************************
 * CAudioFilter::Categories[]
 *****************************************************************************
 *//*!
 * @brief
 * List of filter categories.
 */
GUID 
CAudioFilter::Categories[4] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_CAPTURE),
    STATICGUIDOF(KSCATEGORY_AUDIOCONTROL)
};

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioFilter::SupportPinName()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportPinName
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportPinName]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = AudioFilter->FindPin(Request->PinId);

 	if (Pin)
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
										     KSPROPERTY_TYPE_GET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_LPWSTR;
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
						   KSPROPERTY_TYPE_GET;

			// set the return value size
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_SUCCESS;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetPinName()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetPinName
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetPinName]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = AudioFilter->FindPin(Request->PinId);

	if (Pin)
	{
		struct
		{
			UCHAR	bLength;
			UCHAR	bDescriptorType;
			WCHAR	bString[32];
		} TerminalNameDescriptor;

		RtlZeroMemory(&TerminalNameDescriptor, sizeof(TerminalNameDescriptor));

		TerminalNameDescriptor.bLength = sizeof(TerminalNameDescriptor);

		USHORT LanguageId = AudioFilter->m_KsAdapter->GetLanguageId();

		// Find the terminal name, if available.
		UCHAR iTerminal = Pin->iTerminal();

		if (iTerminal)
		{
			AudioFilter->m_UsbDevice->GetStringDescriptor(iTerminal, LanguageId, PUSB_STRING_DESCRIPTOR(&TerminalNameDescriptor));

			// If there is a pin name available for use...
			if (TerminalNameDescriptor.bLength)
			{
				if (ValueSize >= TerminalNameDescriptor.bLength)
				{
					PWCHAR PinName = PWCHAR(Value);

					wcscpy(PinName, TerminalNameDescriptor.bString);
					
					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					ntStatus = STATUS_BUFFER_OVERFLOW;
				}

				ValueSize = TerminalNameDescriptor.bLength;
			}
		}
		else
		{
			ntStatus = STATUS_NOT_FOUND;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetGlobalPinCInstances()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetGlobalPinCInstances
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

//   _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetGlobalPinCInstances]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PKSFILTERFACTORY KsFilterFactory = KsFilterGetParentFilterFactory(KsGetFilterFromIrp(Irp));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (KsFilterFactory)
	{
		if (ValueSize >= sizeof(KSPIN_CINSTANCES))
		{
			PKSPIN_CINSTANCES KspCInstances = (PKSPIN_CINSTANCES)(Value);

			KspCInstances->PossibleCount = 0;
			
			KspCInstances->CurrentCount = 0;

			PKSDEVICE KsDevice = KsFilterFactoryGetDevice(KsFilterFactory);

			KsAcquireDevice(KsDevice);

			PKSFILTER KsFilter = KsFilterFactoryGetFirstChildFilter(KsFilterFactory);

			while (KsFilter)
			{
				KsFilterAcquireControl(KsFilter);

				if (Request->PinId < KsFilter->Descriptor->PinDescriptorsCount)
				{
					KspCInstances->PossibleCount = KsFilter->Descriptor->PinDescriptors[Request->PinId].InstancesPossible;

					KspCInstances->CurrentCount += KsFilterGetChildPinCount(KsFilter, Request->PinId);
				}

				KsFilterReleaseControl(KsFilter);

				KsFilter = KsFilterGetNextSiblingFilter(KsFilter);
			}

			KsReleaseDevice(KsDevice);

			ntStatus = STATUS_SUCCESS;
		}
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(KSPIN_CINSTANCES);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetCopyProtectControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetCopyProtectControl
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetCopyProtectControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = AudioFilter->FindPin(Request->PinId);

	if (Pin)
	{
		if (ValueSize >= sizeof(KSAUDIO_COPY_PROTECTION))
		{
			PKSAUDIO_COPY_PROTECTION CopyProtection = (PKSAUDIO_COPY_PROTECTION)(Value);

			if (Pin->IsSource())
			{
				ULONG CopyProtectionLevel = 0;

				ntStatus = Pin->ReadParameter(REQUEST_CUR, USB_AUDIO_TE_CONTROL_COPY_PROTECT, 0, &CopyProtectionLevel, sizeof(ULONG), NULL);

				if (NT_SUCCESS(ntStatus))
				{
					switch (CopyProtectionLevel)
					{
						case 0:
						{
							CopyProtection->fCopyrighted = FALSE;
							CopyProtection->fOriginal = TRUE;
						}
						break;

						case 1:
						{
							CopyProtection->fCopyrighted = TRUE;
							CopyProtection->fOriginal = TRUE;
						}
						break;

						case 2:
						{
							CopyProtection->fCopyrighted = TRUE;
							CopyProtection->fOriginal = FALSE;
						}
						break;
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(KSAUDIO_COPY_PROTECTION);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetCopyProtectControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetCopyProtectControl
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetCopyProtectControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = AudioFilter->FindPin(Request->PinId);

	if (Pin)
	{
		if (ValueSize >= sizeof(KSAUDIO_COPY_PROTECTION))
		{
			PKSAUDIO_COPY_PROTECTION CopyProtection = (PKSAUDIO_COPY_PROTECTION)(Value);

			if (!Pin->IsSource())
			{
				ULONG CopyProtectionLevel = 0;

				if (CopyProtection->fCopyrighted)
				{
					if (CopyProtection->fOriginal)
					{
						CopyProtectionLevel = 1;
					}
					else
					{
						CopyProtectionLevel = 2;
					}
				}

				ntStatus = Pin->WriteParameter(REQUEST_CUR, USB_AUDIO_TE_CONTROL_COPY_PROTECT, 0, &CopyProtectionLevel, sizeof(ULONG));
			}
			else
			{
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetLatencyControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetLatencyControl
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetLatencyControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = AudioFilter->FindPin(Request->PinId);

	if (Pin)
	{
		ULONG PossibleSampleRates[] = { 44100, 48000, 88200, 96000, 176400, 192000 };

		if (ValueSize >= (sizeof(KSTIME) * SIZEOF_ARRAY(PossibleSampleRates)))
		{
			ValueSize = sizeof(KSTIME) * SIZEOF_ARRAY(PossibleSampleRates);

			PKSTIME Latencies = (PKSTIME)(Value);

			for (ULONG i=0; i<SIZEOF_ARRAY(PossibleSampleRates); i++)
			{
				AudioFilter->GetLatency(Request->PinId, PossibleSampleRates[i], &Latencies[i]);
			}

			ntStatus = STATUS_SUCCESS;
		}
		else if (ValueSize >= sizeof(KSTIME))
		{
			ValueSize = sizeof(KSTIME);

			PKSTIME Latency = (PKSTIME)(Value);

			ntStatus = AudioFilter->GetLatency(Request->PinId, 0, Latency);
		}
		else
		{
			ValueSize = sizeof(KSTIME);

			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetDeviceControl()
 *****************************************************************************
 *//*!
 * @brief
 * Device control property handler.
 * @details
 * This routine gets called whenever this filter gets a property
 * request with KSPROSETPID_DeviceControl and a property set value. It is not
 * a node property but a filter property (you don't have to specify a node).
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS 
CAudioFilter::
GetDeviceControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetDeviceControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PVOID Instance = PVOID(Request+1);

	ULONG InstanceSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSPROPERTY);

	CAudioFilter * that = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    switch (Request->Id)
	{
		case KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_DEVICE_DESCRIPTOR))
			{
				PUSB_DEVICE_DESCRIPTOR Descriptor = NULL;

				ntStatus = that->m_UsbDevice->GetDeviceDescriptor(&Descriptor);

				if (NT_SUCCESS(ntStatus))
				{
					RtlCopyMemory(Value, Descriptor, sizeof(USB_DEVICE_DESCRIPTOR));
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_DEVICE_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CONFIGURATION_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_CONFIGURATION_DESCRIPTOR))
			{
				PUSB_CONFIGURATION_DESCRIPTOR Descriptor = NULL;

				ntStatus = that->m_UsbDevice->GetConfigurationDescriptor(&Descriptor);

				if (NT_SUCCESS(ntStatus))
				{
        			// validate and get the output parameter
					if (ValueSize >= Descriptor->wTotalLength)
					{
						RtlCopyMemory(Value, Descriptor, Descriptor->wTotalLength);

						ValueSize = Descriptor->wTotalLength;

					}
					else if (ValueSize >= Descriptor->bLength)
					{
						RtlCopyMemory(Value, Descriptor, Descriptor->bLength);

						ValueSize = Descriptor->bLength;
					}
					else
					{
						ValueSize = Descriptor->wTotalLength;

						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_CONFIGURATION_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_INTERFACE_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_INTERFACE_DESCRIPTOR))
			{
				if (InstanceSize >= sizeof(INTERFACE_PARAMETERS))
				{
					PINTERFACE_PARAMETERS Parameters = PINTERFACE_PARAMETERS(Instance);

					PUSB_INTERFACE_DESCRIPTOR Descriptor = NULL;

					ntStatus = that->m_UsbDevice->GetInterfaceDescriptor
								(
									Parameters->InterfaceNumber,
									Parameters->AlternateSetting,
									Parameters->InterfaceClass,
									Parameters->InterfaceSubClass,
									Parameters->InterfaceProtocol,
									&Descriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						RtlCopyMemory(Value, Descriptor, sizeof(USB_INTERFACE_DESCRIPTOR));
					}
				}
				else
				{
					ntStatus = STATUS_INVALID_PARAMETER;
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_INTERFACE_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_ENDPOINT_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_ENDPOINT_DESCRIPTOR))
			{
				if (InstanceSize >= sizeof(ENDPOINT_PARAMETERS))
				{
					PENDPOINT_PARAMETERS Parameters = PENDPOINT_PARAMETERS(Instance);

					PUSB_ENDPOINT_DESCRIPTOR Descriptor = NULL;

					ntStatus = that->m_UsbDevice->GetEndpointDescriptor
								(
									Parameters->InterfaceNumber,
									Parameters->AlternateSetting,
									Parameters->EndpointIndex,
									&Descriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						RtlCopyMemory(Value, Descriptor, sizeof(USB_ENDPOINT_DESCRIPTOR));
					}
				}
				else
				{
					ntStatus = STATUS_INVALID_PARAMETER;
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_ENDPOINT_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR:
		{
			if (InstanceSize >= sizeof(STRING_PARAMETERS))
			{
				PSTRING_PARAMETERS Parameters = PSTRING_PARAMETERS(Instance);

   				// validate and get the output parameter
				if (ValueSize >= sizeof(USB_STRING_DESCRIPTOR))
				{
					PUSB_STRING_DESCRIPTOR Descriptor = PUSB_STRING_DESCRIPTOR(Value);

					RtlZeroMemory(Descriptor, ValueSize);

					Descriptor->bLength = (ValueSize <= 254) ? UCHAR(ValueSize) : 254;
					Descriptor->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

					ntStatus = that->m_UsbDevice->GetStringDescriptor
								(
									Parameters->Index,
									Parameters->LanguageId,
									Descriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						ValueSize = Descriptor->bLength;
					}
					else if (ntStatus == STATUS_BUFFER_TOO_SMALL)
					{
						ValueSize = 254; // Maximum string descriptor size.
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR:
		{
			if (InstanceSize >= sizeof(CLASS_INTERFACE_PARAMETERS))
			{
				PCLASS_INTERFACE_PARAMETERS Parameters = PCLASS_INTERFACE_PARAMETERS(Instance);

				PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

				ntStatus = that->m_UsbDevice->GetInterfaceDescriptor
							(
								Parameters->InterfaceNumber,
								Parameters->AlternateSetting,
								-1, -1, -1,
								&InterfaceDescriptor
							);

				if (NT_SUCCESS(ntStatus))
				{
					PUSB_INTERFACE_DESCRIPTOR CsInterfaceDescriptor = NULL;

					ntStatus = that->m_UsbDevice->GetClassInterfaceDescriptor
								(
									Parameters->InterfaceNumber,
									Parameters->AlternateSetting,
									Parameters->ClassSpecificDescriptorType,
									&CsInterfaceDescriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						if (CsInterfaceDescriptor->bDescriptorType == USB_AUDIO_CS_INTERFACE)
						{
							switch (InterfaceDescriptor->bInterfaceSubClass)
							{
								case USB_AUDIO_SUBCLASS_AUDIOCONTROL:
								{
									PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR(CsInterfaceDescriptor);

        							// validate and get the output parameter
									if (ValueSize >= CsAcInterfaceDescriptor->wTotalLength)
									{
										RtlCopyMemory(Value, CsAcInterfaceDescriptor, CsAcInterfaceDescriptor->wTotalLength);

										ValueSize = CsAcInterfaceDescriptor->wTotalLength;
									}
									else if (ValueSize >= CsAcInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsAcInterfaceDescriptor, CsAcInterfaceDescriptor->bLength);

										ValueSize = CsAcInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsAcInterfaceDescriptor->wTotalLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;

								case USB_AUDIO_SUBCLASS_AUDIOSTREAMING:
								{
									PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR(CsInterfaceDescriptor);

        							// validate and get the output parameter
									if (ValueSize >= CsAsInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsAsInterfaceDescriptor, CsAsInterfaceDescriptor->bLength);

										ValueSize = CsAsInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsAsInterfaceDescriptor->bLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;

								case USB_AUDIO_SUBCLASS_MIDISTREAMING:
								{
									PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR CsMsInterfaceDescriptor = PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR(CsInterfaceDescriptor);

        							// validate and get the output parameter
									if (ValueSize >= CsMsInterfaceDescriptor->wTotalLength)
									{
										RtlCopyMemory(Value, CsMsInterfaceDescriptor, CsMsInterfaceDescriptor->wTotalLength);

										ValueSize = CsMsInterfaceDescriptor->wTotalLength;

									}
									else if (ValueSize >= CsMsInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsMsInterfaceDescriptor, CsMsInterfaceDescriptor->bLength);

										ValueSize = CsMsInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsMsInterfaceDescriptor->wTotalLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;

								default:
								{
        							// validate and get the output parameter
									if (ValueSize >= CsInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsInterfaceDescriptor, CsInterfaceDescriptor->bLength);

										ValueSize = CsInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsInterfaceDescriptor->bLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;
							}
						}
						else
						{
							ntStatus = STATUS_INVALID_PARAMETER;
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR:
		{
			if (InstanceSize >= sizeof(CLASS_ENDPOINT_PARAMETERS))
			{
				PCLASS_ENDPOINT_PARAMETERS Parameters = PCLASS_ENDPOINT_PARAMETERS(Instance);

				PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

				ntStatus = that->m_UsbDevice->GetClassEndpointDescriptor
							(
								Parameters->InterfaceNumber,
								Parameters->AlternateSetting,
								Parameters->EndpointAddress,
								Parameters->ClassSpecificDescriptorType,
								&EndpointDescriptor
							);

				if (NT_SUCCESS(ntStatus))
				{
					if (EndpointDescriptor->bDescriptorType == USB_AUDIO_CS_ENDPOINT)
					{
						PUSB_ENDPOINT_DESCRIPTOR CsEndpointDescriptor = PUSB_ENDPOINT_DESCRIPTOR(EndpointDescriptor);

        				// validate and get the output parameter
						if (ValueSize >= CsEndpointDescriptor->bLength)
						{
							RtlCopyMemory(Value, CsEndpointDescriptor, CsEndpointDescriptor->bLength);
						}
						else
						{
							ntStatus = STATUS_BUFFER_TOO_SMALL;
						}

						ValueSize = CsEndpointDescriptor->bLength;
					}
					else
					{
						ntStatus = STATUS_INVALID_PARAMETER;
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND:
		{
			if (InstanceSize >= sizeof(CUSTOM_COMMAND_PARAMETERS))
			{
				PCUSTOM_COMMAND_PARAMETERS Parameters = PCUSTOM_COMMAND_PARAMETERS(Instance);

				ntStatus = that->m_UsbDevice->CustomCommand
							(
								Parameters->RequestType,
								Parameters->Request,
								Parameters->Value,
								Parameters->Index,
								Value,
								ValueSize,
								&ValueSize,
								TRUE
							);
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_PIN_SYNCHRONIZE_START_FRAME:
		{
			if (ValueSize >= sizeof(ULONG))
			{
				that->m_SynchronizeStart = TRUE;

				that->m_UsbDevice->GetCurrentFrameNumber(&that->m_StartFrameNumber);

				that->m_StartFrameNumber += MIN_AUDIO_START_FRAME_OFFSET;

				PULONG StartFrameNumber = PULONG(Value);

				*StartFrameNumber = that->m_StartFrameNumber;

				ValueSize = sizeof(ULONG);

				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
        }
		break;
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetDeviceControl()
 *****************************************************************************
 *//*!
 * @brief
 * Device control property handler.
 * @details
 * This routine gets called whenever this filter gets a property
 * request with KSPROSETPID_DeviceControl and a property set value. It is not
 * a node property but a filter property (you don't have to specify a node).
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS 
CAudioFilter::
SetDeviceControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetDeviceControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PVOID Instance = PVOID(Request+1);

	ULONG InstanceSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSPROPERTY);

	CAudioFilter * that = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    switch (Request->Id)
	{
		case KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND:
		{
			if (InstanceSize >= sizeof(CUSTOM_COMMAND_PARAMETERS))
			{
				PCUSTOM_COMMAND_PARAMETERS Parameters = PCUSTOM_COMMAND_PARAMETERS(Instance);

				ntStatus = that->m_UsbDevice->CustomCommand
							(
								Parameters->RequestType,
								Parameters->Request,
								Parameters->Value,
								Parameters->Index,
								Parameters->Buffer,
								Parameters->BufferLength,
								NULL,
								FALSE
							);
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK:
		{
			ntStatus = that->m_KsAdapter->SetFirmwareUpgradeLock(TRUE);
        }
		break;

		case KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK:
		{
			ntStatus = that->m_KsAdapter->SetFirmwareUpgradeLock(FALSE);
        }
		break;

		case KSPROPERTY_DEVICECONTROL_PIN_OUTPUT_CFIFO_BUFFERS:
		{
			if (ValueSize >= sizeof(ULONG))
			{
				that->m_NumberOfFifoBuffers.Output = *(PULONG(Value));

				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
        }
		break;

		case KSPROPERTY_DEVICECONTROL_PIN_INPUT_CFIFO_BUFFERS:
		{
			if (ValueSize >= sizeof(ULONG))
			{
				that->m_NumberOfFifoBuffers.Input = *(PULONG(Value));

				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
        }
		break;

		case KSPROPERTY_DEVICECONTROL_PIN_SYNCHRONIZE_START_FRAME:
		{
			if (ValueSize >= sizeof(ULONG))
			{
				that->m_SynchronizeStart = TRUE;

				that->m_StartFrameNumber = *(PULONG(Value));

				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
        }
		break;
	}

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioFilter::AddControlEvent()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
AddControlEvent
(
	IN		PIRP			Irp,
	IN		PKSEVENTDATA	EventData,
	IN		PKSEVENT_ENTRY	EventEntry
)
{
    ASSERT(EventData);
	ASSERT(EventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::AddControlEvent]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

	PKSFILTER KsFilter = KsGetFilterFromIrp(Irp);
	
	ULONG NodeId = KsGetNodeIdFromIrp(Irp);

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (NodeId != KSFILTER_NODE)
	{
		PNODE_DESCRIPTOR Node = AudioFilter->FindNode(NodeId);

 		if (Node)
		{
			// ExtraEntryData: First ULONG points to the PinId, followed by
			// the NodeId. This is used in the KsGenerateEvents() callback routine 
			// to determine whether to generate an event or not.
			PULONG PinId_ = PULONG(EventEntry + 1);
			
			*PinId_ = ULONG(-1);

			PULONG NodeId_ = PULONG(PinId_ + 1);

			*NodeId_ = NodeId;

			KsFilterAddEvent(KsFilter, EventEntry);

			ntStatus = STATUS_SUCCESS;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::RemoveControlEvent()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CAudioFilter::
RemoveControlEvent
(
	IN		PFILE_OBJECT	FileObject,
	IN		PKSEVENT_ENTRY	EventEntry
)
{
    ASSERT(FileObject);
	ASSERT(EventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::RemoveControlEvent]"));

	//CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

	PKSFILTER KsFilter = KsGetFilterFromFileObject(FileObject);
	
	if (EventEntry)
	{
		RemoveEntryList(&EventEntry->ListEntry);
	}
}

/*****************************************************************************
 *//*! @struct AUDIO_EVENT_REQUEST
 *****************************************************************************
 */
typedef struct
{
    PVOID       MajorTarget;    /*!< @brief Pointer to the filter. */
    PVOID       MinorTarget;    /*!< @brief Not used. */
    GUID *      Set;            /*!< @brief Pointer to the event set. */
    ULONG       EventId;        /*!< @brief Identifier of the event. */
    BOOL        PinEvent;       /*!< @brief TRUE for a pin event, otherwise FALSE. */
    ULONG       PinId;          /*!< @brief Identifier of the pin. */
    BOOL        NodeEvent;      /*!< @brief TRUE for a node event, otherwise FALSE. */
    ULONG       NodeId;         /*!< @brief Identifier for the node. */
} AUDIO_EVENT_REQUEST, *PAUDIO_EVENT_REQUEST;

/*****************************************************************************
 * GenerateEventCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
static
BOOLEAN 
PerformCallbackVerification
(
	IN		PVOID			Context,
	IN		PKSEVENT_ENTRY	EventEntry
)
{
	PULONG PinId = PULONG(EventEntry+1);

	PULONG NodeId = PULONG(PinId + 1);

	BOOLEAN Callback = TRUE;

	PAUDIO_EVENT_REQUEST EventRequest = PAUDIO_EVENT_REQUEST(Context);

	//DbgPrint("Generate Event Callback: {%d, %d} - [%d, %d] - [%d, %d]\n", EventRequest->PinEvent, EventRequest->NodeEvent, *PinId, *NodeId, EventRequest->PinId, EventRequest->NodeId);

	if (EventRequest->PinEvent && EventRequest->NodeEvent)
	{
		Callback = (EventRequest->PinId == *PinId) && (EventRequest->NodeId == *NodeId);
	}
	else if (EventRequest->PinEvent)
	{
		Callback = (EventRequest->PinId == *PinId);
	}
	else if (EventRequest->NodeEvent)
	{
		Callback = (EventRequest->NodeId == *NodeId);
	}

	return Callback;
}

/*****************************************************************************
 * CAudioFilter::EventCallbackRoutine()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioFilter::
EventCallbackRoutine
(
	IN		PVOID	Context,
	IN		GUID *	Set,
	IN		ULONG	EventId,
	IN		BOOL	PinEvent,
	IN		ULONG	PinId,
	IN		BOOL	NodeEvent,
	IN		ULONG	NodeId
)
{
	CAudioFilter * that = (CAudioFilter *)(Context);

	AUDIO_EVENT_REQUEST EventRequest;

	EventRequest.MajorTarget = Context;
	EventRequest.MinorTarget = NULL;
	EventRequest.Set		 = Set;
	EventRequest.EventId	 = EventId;
	EventRequest.PinEvent	 = PinEvent;
	EventRequest.PinId		 = PinId;
	EventRequest.NodeEvent	 = NodeEvent;
	EventRequest.NodeId		 = NodeId;

	//DbgPrint("KsFilterGenerateEvents - Begin\n");
	
	KsFilterGenerateEvents(that->m_KsFilter, Set, EventId, 0, NULL, PerformCallbackVerification, &EventRequest);

	//DbgPrint("KsFilterGenerateEvents - End\n");
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * FEATURE_UNIT_CONTROL_SELECTOR()
 *****************************************************************************
 *//*!
 * @brief
 */
inline
UCHAR FEATURE_UNIT_CONTROL_SELECTOR
(
	IN		ULONG	PropertyId
)
{
	UCHAR ControlSelector = USB_AUDIO_FU_CONTROL_UNDEFINED;

	switch (PropertyId)
	{
		case KSPROPERTY_AUDIO_MUTE:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_MUTE;
		}
		break;

		case KSPROPERTY_AUDIO_VOLUMELEVEL:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_VOLUME;
		}
		break;

		case KSPROPERTY_AUDIO_BASS:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_BASS;
		}
		break;

		case KSPROPERTY_AUDIO_MID:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_MID;
		}
		break;

		case KSPROPERTY_AUDIO_TREBLE:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_TREBLE;
		}
		break;

		case KSPROPERTY_AUDIO_EQ_BANDS:
		case KSPROPERTY_AUDIO_EQ_LEVEL:
		case KSPROPERTY_AUDIO_NUM_EQ_BANDS:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_GRAPHIC_EQ;
		}
		break;

		case KSPROPERTY_AUDIO_AGC:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_AUTOMATIC_GAIN;
		}
		break;

		case KSPROPERTY_AUDIO_DELAY:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_DELAY;
		}
		break;

		case KSPROPERTY_AUDIO_BASS_BOOST:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_BASS_BOOST;
		}
		break;

		case KSPROPERTY_AUDIO_LOUDNESS:
		{
			ControlSelector = USB_AUDIO_FU_CONTROL_LOUDNESS;
		}
		break;
	}

	return ControlSelector;
}

/*****************************************************************************
 * PROCESSING_UNIT_CONTROL_SELECTOR()
 *****************************************************************************
 *//*!
 * @brief
 */
inline
UCHAR PROCESSING_UNIT_CONTROL_SELECTOR
(
	IN		USHORT	ProcessType,
	IN		ULONG	PropertyId
)
{
	UCHAR ControlSelector = 0; // undefined

	switch (ProcessType)
	{
		case USB_AUDIO_PROCESS_UPMIX_DOWNMIX:
		{
			switch (PropertyId)
			{
				case KSPROPERTY_AUDIO_CHANNEL_CONFIG:
				{
					ControlSelector = USB_AUDIO_UD_CONTROL_MODE_SELECT;
				}
				break;
			}
		}
		break;

		case USB_AUDIO_PROCESS_DOLBY_PROLOGIC:
		{
			switch (PropertyId)
			{
				case KSPROPERTY_AUDIO_CHANNEL_CONFIG:
				{
					ControlSelector = USB_AUDIO_DP_CONTROL_MODE_SELECT;
				}
				break;
			}
		}
		break;

		case USB_AUDIO_PROCESS_3D_STEREO_EXTENDER:
		{
			switch (PropertyId)
			{
				case KSPROPERTY_AUDIO_WIDENESS:
				{
					ControlSelector = USB_AUDIO_3D_CONTROL_SPACIOUSNESS;
				}
				break;
			}
		}
		break;

		case USB_AUDIO_PROCESS_REVERBERATION:
		{
			switch (PropertyId)
			{
				#if defined(KSPROPERTY_AUDIO_REVERB_TYPE)
				case KSPROPERTY_AUDIO_REVERB_TYPE:
				{
					ControlSelector = USB_AUDIO_RV_CONTROL_TYPE;
				}
				break;
				#endif // KSPROPERTY_AUDIO_REVERB_TYPE

				case KSPROPERTY_AUDIO_REVERB_LEVEL:
				{
					ControlSelector = USB_AUDIO_RV_CONTROL_LEVEL;
				}
				break;

				case KSPROPERTY_AUDIO_REVERB_TIME:
				{
					ControlSelector = USB_AUDIO_RV_CONTROL_TIME;
				}
				break;

				case KSPROPERTY_AUDIO_REVERB_DELAY_FEEDBACK:
				{
					ControlSelector = USB_AUDIO_RV_CONTROL_FEEDBACK;
				}
				break;
			}
		}
		break;
		
		case USB_AUDIO_PROCESS_CHORUS:
		{
			switch (PropertyId)
			{
				case KSPROPERTY_AUDIO_CHORUS_LEVEL:
				{
					ControlSelector = USB_AUDIO_CH_CONTROL_LEVEL;
				}
				break;

				case KSPROPERTY_AUDIO_CHORUS_MODULATION_RATE:
				{
					ControlSelector = USB_AUDIO_CH_CONTROL_RATE;
				}
				break;

				case KSPROPERTY_AUDIO_CHORUS_MODULATION_DEPTH:
				{
					ControlSelector = USB_AUDIO_CH_CONTROL_DEPTH;
				}
				break;
			}
		}
		break;

		case USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION:
		{
			switch (PropertyId)
			{
				#if defined(KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO)
				case KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO:
				{
					ControlSelector = USB_AUDIO_DR_CONTROL_COMPRESSION_RATE;
				}
				break;
				#endif // KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO

				#if defined(KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE)
				case KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE:
				{
					ControlSelector = USB_AUDIO_DR_CONTROL_MAXAMP;
				}
				break;
				#endif // KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE

				#if defined(KSPROPERTY_AUDIO_DRC_THRESHOLD)
				case KSPROPERTY_AUDIO_DRC_THRESHOLD:
				{
					ControlSelector = USB_AUDIO_DR_CONTROL_THRESHOLD;
				}
				break;
				#endif // KSPROPERTY_AUDIO_DRC_THRESHOLD

				#if defined(KSPROPERTY_AUDIO_DRC_ATTACK_TIME)
				case KSPROPERTY_AUDIO_DRC_ATTACK_TIME:
				{
					ControlSelector = USB_AUDIO_DR_CONTROL_ATTACK_TIME;
				}
				break;
				#endif // KSPROPERTY_AUDIO_DRC_ATTACK_TIME

				#if defined(KSPROPERTY_AUDIO_DRC_RELEASE_TIME)
				case KSPROPERTY_AUDIO_DRC_RELEASE_TIME:
				{
					ControlSelector = USB_AUDIO_DR_CONTROL_RELEASE_TIME;
				}
				break;
				#endif // KSPROPERTY_AUDIO_DRC_RELEASE_TIME
				case -1:
					break;
			}
		}
		break;
	}

	return ControlSelector;
}

/*****************************************************************************
 * ENABLE_PROCESSING_UNIT_CONTROL_SELECTOR()
 *****************************************************************************
 *//*!
 * @brief
 */
inline
UCHAR ENABLE_PROCESSING_UNIT_CONTROL_SELECTOR
(
	IN		USHORT	ProcessType
)
{
	UCHAR ControlSelector = 0; // undefined

	switch (ProcessType)
	{
		case USB_AUDIO_PROCESS_UPMIX_DOWNMIX:
		{
			ControlSelector = USB_AUDIO_UD_CONTROL_ENABLE;
		}
		break;

		case USB_AUDIO_PROCESS_DOLBY_PROLOGIC:
		{
			ControlSelector = USB_AUDIO_DP_CONTROL_ENABLE;
		}
		break;

		case USB_AUDIO_PROCESS_3D_STEREO_EXTENDER:
		{
			ControlSelector = USB_AUDIO_3D_CONTROL_ENABLE;
		}
		break;

		case USB_AUDIO_PROCESS_REVERBERATION:
		{
			ControlSelector = USB_AUDIO_RV_CONTROL_ENABLE;
		}
		break;
		
		case USB_AUDIO_PROCESS_CHORUS:
		{
			ControlSelector = USB_AUDIO_CH_CONTROL_ENABLE;
		}
		break;

		case USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION:
		{
			ControlSelector = USB_AUDIO_DR_CONTROL_ENABLE;
		}
		break;
	}

	return ControlSelector;
}

/*****************************************************************************
 * CAudioFilter::SupportMixControl()
 *****************************************************************************
 *//*!
 * @brief
 * Assists in BASICSUPPORT access on super mixer.
 * @param
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportMixControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportMixControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

	if (Node)
	{
		// get the instance channel parameter
		switch (Request->Property.Id)
		{
			case KSPROPERTY_AUDIO_MIX_LEVEL_CAPS:
			{
				if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
				{
					// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
					PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

					Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
													 KSPROPERTY_TYPE_GET;
					Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
					Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
					Description->PropTypeSet.Id    = VT_EMPTY;
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
								   KSPROPERTY_TYPE_GET;

					// set the return value size
					ValueSize = sizeof(ULONG);

					ntStatus = STATUS_SUCCESS;
				}
			}
			break;

			case KSPROPERTY_AUDIO_MIX_LEVEL_TABLE:
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
					Description->PropTypeSet.Id    = VT_EMPTY;
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
			break;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetMixControl()
 *****************************************************************************
 *//*!
 * @brief
 * Handles super mixer accesses
 * @param
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetMixControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetMixControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

	if (Node)
	{
		// get the instance channel parameter
		switch (Request->Property.Id)
		{
			case KSPROPERTY_AUDIO_MIX_LEVEL_CAPS:
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetMixControl] - Mix Level Caps"));

				ULONG InputChannelOffset = 0;

				ULONG InputChannels = Node->NumberOfChannels(1, &InputChannelOffset);

				ULONG OutputChannelOffset = 0;

				ULONG OutputChannels = Node->NumberOfChannels(0, &OutputChannelOffset);

				// validate and get the output parameter
				if (ValueSize >= (sizeof(KSAUDIO_MIXCAP_TABLE)+(InputChannels*OutputChannels-1)*(sizeof(KSAUDIO_MIX_CAPS))))
				{			
					PKSAUDIO_MIXCAP_TABLE MixCaps = PKSAUDIO_MIXCAP_TABLE(Value);

					MixCaps->InputChannels = InputChannels;
					MixCaps->OutputChannels = OutputChannels;

					for (ULONG i=InputChannelOffset, k=0; i<(InputChannelOffset+InputChannels); i++)
					{
						for (ULONG j=OutputChannelOffset; j<(OutputChannelOffset+OutputChannels); j++, k++)
						{
							LONG Level = 0;
							Node->ReadParameter(REQUEST_CUR, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), NULL);
							MixCaps->Capabilities[k].Mute = (Level == INFINITY * dB) ? TRUE: FALSE;

							Node->ReadParameter(REQUEST_MIN, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), NULL);
							MixCaps->Capabilities[k].Minimum = Level;
							
							Node->ReadParameter(REQUEST_MAX, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), NULL);
							MixCaps->Capabilities[k].Maximum = Level;

							Node->ReadParameter(REQUEST_RES, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), NULL);
							MixCaps->Capabilities[k].Reset = Level;
						
							/*_DbgPrintF(DEBUGLVL_TERSE,("MixCaps[%d][%d] - Min: 0x%x, Max: 0x%x, Res: 0x%x: Mute: 0x%x", i-InputChannelOffset, j,
														MixCaps->Capabilities[k].Minimum,
														MixCaps->Capabilities[k].Maximum,
														MixCaps->Capabilities[k].Reset,
														MixCaps->Capabilities[k].Mute
														));/**/
						}
					}

					ValueSize = sizeof(KSAUDIO_MIXCAP_TABLE)+(InputChannels*OutputChannels-1)*sizeof(KSAUDIO_MIX_CAPS);

					ntStatus = STATUS_SUCCESS;
				}
				else if (ValueSize >= 2*sizeof(ULONG))
				{
					PKSAUDIO_MIXCAP_TABLE MixCaps = PKSAUDIO_MIXCAP_TABLE(Value);

					MixCaps->InputChannels = InputChannels;
					MixCaps->OutputChannels = OutputChannels;

					ValueSize = 2*sizeof(ULONG);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
	 				ValueSize = sizeof(KSAUDIO_MIXCAP_TABLE)+(InputChannels*OutputChannels-1)*sizeof(KSAUDIO_MIX_CAPS);

					ntStatus = STATUS_BUFFER_OVERFLOW;
				}
			}
			break;

			// The support for KSPROPERTY_AUDIO_MIX_LEVEL_TABLE is commented out because Microsoft's wdmaud will
			// loop forever & exhaust system memory traversing the graph if the mix level cap/table is defined as such
			// that it is programmable in an asymmetrical fashion, ie 0-1, 1-0, 1-2, etc., as opposed to the symmetrical
			// layout, ie 0-0, 1-1, 2-2, etc.
			#if 1
			case KSPROPERTY_AUDIO_MIX_LEVEL_TABLE:
			{
			    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetMixControl] - Mix Level Table"));

				ULONG InputChannelOffset = 0;

				ULONG InputChannels = Node->NumberOfChannels(1, &InputChannelOffset);

				ULONG OutputChannelOffset = 0;

				ULONG OutputChannels = Node->NumberOfChannels(0, &OutputChannelOffset);

				// validate and get the output parameter
				if (ValueSize >= (InputChannels*OutputChannels*sizeof(KSAUDIO_MIXLEVEL)))
				{			
					PKSAUDIO_MIXLEVEL MixLevel = PKSAUDIO_MIXLEVEL(Value);

					for (ULONG i=InputChannelOffset, k=0; i<(InputChannelOffset+InputChannels); i++)
					{
						for (ULONG j=OutputChannelOffset; j<(OutputChannelOffset+OutputChannels); j++, k++)
						{
							LONG Level = 0;
							
							Node->ReadParameter(REQUEST_CUR, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG), NULL);

							MixLevel[k].Mute = (Level == INFINITY * dB) ? TRUE : FALSE;
							MixLevel[k].Level = Level;

							/*_DbgPrintF(DEBUGLVL_TERSE,("MixLevel[%d][%d] - Mute: 0x%x, Level: 0x%x", i-InputChannelOffset, j,
														MixLevel[k].Mute,
														MixLevel[k].Level
														));/**/
						}
					}

	 				ValueSize = InputChannels*OutputChannels*sizeof(KSAUDIO_MIXLEVEL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
	 				ValueSize = InputChannels*OutputChannels*sizeof(KSAUDIO_MIXLEVEL);

                    ntStatus = STATUS_BUFFER_OVERFLOW;
				}
			}
			break;
			#endif // 0
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetMixControl()
 *****************************************************************************
 *//*!
 * @brief
 * Handles super mixer accesses
 * @param
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetMixControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetMixControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

	if (Node)
	{
		// The support for KSPROPERTY_AUDIO_MIX_LEVEL_TABLE is commented out because Microsoft's wdmaud will
		// loop forever & exhaust system memory traversing the graph if the mix level cap/table is defined as such
		// that it is programmable in an asymmetrical fashion, ie 0-1, 1-0, 1-2, etc., as opposed to the symmetrical
		// layout, ie 0-0, 1-1, 2-2, etc.
		#if 1
		// get the instance channel parameter
		switch (Request->Property.Id)
		{
			case KSPROPERTY_AUDIO_MIX_LEVEL_TABLE:
			{
				ULONG InputChannelOffset = 0;

				ULONG InputChannels = Node->NumberOfChannels(1, &InputChannelOffset);

				ULONG OutputChannelOffset = 0;

				ULONG OutputChannels = Node->NumberOfChannels(0, &OutputChannelOffset);

				// validate and get the output parameter
				if (ValueSize >= (InputChannels*OutputChannels*sizeof(KSAUDIO_MIXLEVEL)))
				{			
					PKSAUDIO_MIXLEVEL MixLevel = PKSAUDIO_MIXLEVEL(Value);

					for (ULONG i=InputChannelOffset, k=0; i<(InputChannelOffset+InputChannels); i++)
					{
						for (ULONG j=OutputChannelOffset; j<(OutputChannelOffset+OutputChannels); j++, k++)
						{
							LONG Level = MixLevel[k].Mute ? (INFINITY * dB) : MixLevel[k].Level;

							/*_DbgPrintF(DEBUGLVL_TERSE,("MixLevel[%d][%d] - Mute: 0x%x, Level: 0x%x", i-InputChannelOffset, j,
														MixLevel[k].Mute,
														MixLevel[k].Level
														));/**/
							
							Node->WriteParameter(REQUEST_CUR, UCHAR(i+1), UCHAR(j+1), &Level, sizeof(LONG));
						}
					}

					ntStatus = STATUS_SUCCESS;
				}
			}
			break;
		}
		#endif // 0
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportMuxControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses a ULONG value property for MUX controls.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportMuxControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportMuxControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if to see if it is a mux source.
		if (Request->Property.Id == KSPROPERTY_AUDIO_MUX_SOURCE)
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
				Description->PropTypeSet.Id    = VT_I4;
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
 * CAudioFilter::GetMuxControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses a ULONG value property for MUX controls.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetMuxControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetMuxControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if to see if it is a mux source.
		if (Request->Property.Id == KSPROPERTY_AUDIO_MUX_SOURCE)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(ULONG))
			{		
				ntStatus = Node->ReadParameter(REQUEST_CUR, 0, 0, PULONG(Value), sizeof(ULONG), NULL);
			}
			else
			{
                ntStatus = STATUS_BUFFER_OVERFLOW;
			}

			ValueSize = sizeof(ULONG);
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetMuxControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses a ULONG value property for MUX controls.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetMuxControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetMuxControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if to see if it is a mux source.
		if (Request->Property.Id == KSPROPERTY_AUDIO_MUX_SOURCE)
		{
			// validate and set the output parameter
			if (ValueSize >= sizeof(ULONG))
			{		
				ntStatus = Node->WriteParameter(REQUEST_CUR, 0, 0, PULONG(Value), sizeof(ULONG));
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportOnOffControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an on/off value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportOnOffControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportOnOffControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		if ((Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MUTE) ||
			(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_AGC) ||
			(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_BASS_BOOST) ||
			(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_LOUDNESS))
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				ULONG NumChannels = Node->NumberOfChannels(0);

				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
												 KSPROPERTY_TYPE_GET |
												 KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
												 sizeof(KSPROPERTY_MEMBERSHEADER) + 
												 sizeof(BOOL);
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_BOOL;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = 1;
				Description->Reserved          = 0;

				// if return buffer cn also hold a range description, return it too
				if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
					  		     sizeof(KSPROPERTY_MEMBERSHEADER) + 
								 sizeof(BOOL))
				{  
					// fill in the members header
					PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

					Members->MembersFlags   = 0;
					Members->MembersSize    = 0;
					Members->MembersCount   = NumChannels;
					Members->Flags          = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL;

					// Go thru all the channels to see if they are supported on each channel.
					// If not, then this is a uniform control, ie only master channel existed.
					BOOL UniformControl = TRUE;

					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);

					for (ULONG i=0; i<NumChannels; i++)
					{
						UCHAR CN = UCHAR(i+1);
						
						BOOL Value = FALSE;

						if (NT_SUCCESS(Node->ReadParameter(REQUEST_CUR, CS, CN, &Value, sizeof(BOOL), NULL)))
						{
							UniformControl = FALSE;

							break;
						}
					}

					if (UniformControl)
					{
						Members->Flags |= KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_UNIFORM;
					}

					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
												 sizeof(KSPROPERTY_MEMBERSHEADER) +
												 sizeof(BOOL);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

					ntStatus = STATUS_SUCCESS;
				}
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
 * CAudioFilter::GetOnOffControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an on/off value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetOnOffControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetOnOffControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			if ((Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MUTE) ||
				(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_AGC) ||
				(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_BASS_BOOST) ||
				(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_LOUDNESS))
			{
				// validate and get the output parameter
				if (ValueSize >= sizeof(BOOL))
				{						
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);
					
					ntStatus = Node->ReadParameter(REQUEST_CUR, CS, CN, Value, sizeof(BOOL), NULL);

					//BUGBUG: How do I support only MASTER channel ?
					if (!NT_SUCCESS(ntStatus))
					{
						ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, sizeof(BOOL), NULL);
					}

					PBOOL OnOff = PBOOL(Value);

					_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetOnOffControl] - Channel: %d, OnOff: 0x%x", Channel, *OnOff));
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				ValueSize = sizeof(BOOL);
			}
		}
	}

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetOnOffControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an on/off value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetOnOffControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetOnOffControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			if ((Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MUTE) ||
				(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_AGC) ||
				(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_BASS_BOOST) ||
				(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_LOUDNESS))
			{
				// validate and get the output parameter
				if (ValueSize >= sizeof(BOOL))
				{
					PBOOL OnOff = PBOOL(Value);

					_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetOnOffControl] - Channel: %d, OnOff: 0x%x", Channel, *OnOff));

					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);

					ntStatus = Node->WriteParameter(REQUEST_CUR, CS, CN, Value, sizeof(BOOL));

					//BUGBUG: How do I support only MASTER channel ?
					if (!NT_SUCCESS(ntStatus))
					{
						ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, sizeof(BOOL));
					}
				}
			}
		}
	}
 
	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportLevelControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportLevelControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportLevelControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
			ULONG NumChannels = Node->NumberOfChannels(0);

			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
											 KSPROPERTY_TYPE_GET |
											 KSPROPERTY_TYPE_SET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
										 	 sizeof(KSPROPERTY_MEMBERSHEADER) +
											 sizeof(KSPROPERTY_STEPPING_LONG) * NumChannels;
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_I4;
			Description->PropTypeSet.Flags = 0;
			Description->MembersListCount  = 1;
			Description->Reserved          = 0;

			// if return buffer cn also hold a range description, return it too
			if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
							 sizeof(KSPROPERTY_MEMBERSHEADER) +
							 sizeof(KSPROPERTY_STEPPING_LONG) * NumChannels)
			{
				// fill in the members header
				PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

				Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
				Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
				Members->MembersCount   = NumChannels;
				Members->Flags          = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL; 

				// fill in the stepped range
				PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

				for (ULONG i=0; i<NumChannels; i++)
				{				
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(i+1);
					
					Node->ReadParameter(REQUEST_MIN, CS, CN, &Range[i].Bounds.SignedMinimum, sizeof(LONG), NULL);
					Node->ReadParameter(REQUEST_MAX, CS, CN, &Range[i].Bounds.SignedMaximum, sizeof(LONG), NULL);			
					Node->ReadParameter(REQUEST_RES, CS, CN, &Range[i].SteppingDelta, sizeof(LONG), NULL);

					Range[i].Reserved = 0;
				}

				// set the return value size
				ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
							sizeof(KSPROPERTY_MEMBERSHEADER) +
							sizeof(KSPROPERTY_STEPPING_LONG) * NumChannels;

				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				// set the return value size
				ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

				ntStatus = STATUS_SUCCESS;
			}
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

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetLevelControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetLevelControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetLevelControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(LONG))
			{
				// check if volume property request
				if ((Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_VOLUMELEVEL) ||
					(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_BASS) ||
					(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MID) ||
					(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_TREBLE))
				{
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);

					ntStatus = Node->ReadParameter(REQUEST_CUR, CS, CN, Value, sizeof(LONG), NULL);

					if (Request->NodeProperty.Property.Id != KSPROPERTY_AUDIO_VOLUMELEVEL)
					{
						//DbgPrint("CS: 0x%x, CN: 0x%x, ntStatus: 0x%x\n", CS, CN, ntStatus);
						//BUGBUG: How do I support only MASTER channel ?
						if (!NT_SUCCESS(ntStatus))
						{
							ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, sizeof(LONG), NULL);
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(LONG);
		}
	}

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetLevelControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetLevelControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetLevelControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(LONG))
			{
				if ((Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_VOLUMELEVEL) ||
					(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_BASS) ||
					(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MID) ||
					(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_TREBLE))
				{
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);

					ntStatus = Node->WriteParameter(REQUEST_CUR, CS, CN, Value, sizeof(LONG));

					if (Request->NodeProperty.Property.Id != KSPROPERTY_AUDIO_VOLUMELEVEL)
					{
						//DbgPrint("CS: 0x%x, CN: 0x%x, ntStatus: 0x%x\n", CS, CN, ntStatus);
						//BUGBUG: How do I support only MASTER channel ?
						if (!NT_SUCCESS(ntStatus))
						{
							ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, sizeof(LONG));
						}
					}
				}
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportEqControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses EQ value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportEqControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportEqControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// check if graphic EQ property request
		if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_NUM_EQ_BANDS)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
												 KSPROPERTY_TYPE_GET;
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
							   KSPROPERTY_TYPE_GET;

				// set the return value size
				ValueSize = sizeof(ULONG);

				ntStatus = STATUS_SUCCESS;
			}
		}
		else if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_EQ_BANDS)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
												 KSPROPERTY_TYPE_GET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_ARRAY;
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
							   KSPROPERTY_TYPE_GET;

				// set the return value size
				ValueSize = sizeof(ULONG);

				ntStatus = STATUS_SUCCESS;
			}
		}
		else if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_EQ_LEVEL)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				//Hmm... MEMBER_FLAG_MASTER_CHANNEL is not available yet, so can't really
				//put master channel support here... have to removed it, otherwise it would confuse
				//wdmaud ??
				#if defined(KSPROPERTY_MEMBER_FLAG_MASTER_CHANNEL)
				#define CHAN_OFFSET	0
				ULONG NumChannels = Node->NumberOfChannels(0) + 1 /*for master channel*/;
				#else
				#define CHAN_OFFSET	1
				ULONG NumChannels = Node->NumberOfChannels(0);
				#endif // defined(KSPROPERTY_MEMBER_FLAG_MASTER_CHANNEL)

				ULONG TotalNumberOfBands = 0;

				for (ULONG i=0; i<NumChannels; i++)
				{				
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(i+CHAN_OFFSET);
					
					ULONG NumberOfBands = 0;

					Node->ReadParameter(REQUEST_CUR, CS, CN, &NumberOfBands, sizeof(ULONG), NULL);

					TotalNumberOfBands += NumberOfBands;
				}

				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
											 	 KSPROPERTY_TYPE_GET |
												 KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
										 		 sizeof(KSPROPERTY_MEMBERSHEADER) * NumChannels +
												 sizeof(KSPROPERTY_STEPPING_LONG) * TotalNumberOfBands;
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_I4;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = NumChannels;
				Description->Reserved          = 0;

				// if return buffer cn also hold a range description, return it too
				if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
								 sizeof(KSPROPERTY_MEMBERSHEADER) * NumChannels +
								 sizeof(KSPROPERTY_STEPPING_LONG) * TotalNumberOfBands)
				{
					// fill in the members header
					PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

					for (ULONG i=0; i<NumChannels; i++)
					{				
						UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
						UCHAR CN = UCHAR(i+CHAN_OFFSET);
						
						ULONG NumberOfBands = 0;

						Node->ReadParameter(REQUEST_CUR, CS, CN, &NumberOfBands, sizeof(ULONG), NULL);

						Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
						Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
						Members->MembersCount   = NumberOfBands;
						#if defined(KSPROPERTY_MEMBER_FLAG_MASTER_CHANNEL)
						Members->Flags          = (i==0) ? KSPROPERTY_MEMBER_FLAG_MASTER_CHANNEL : 0;
						#else
						Members->Flags          = 0;
						#endif // defined(KSPROPERTY_MEMBER_FLAG_MASTER_CHANNEL)

						if (NumberOfBands)
						{
							// fill in the stepped range
							PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

							PULONG Ptr = PULONG(ExAllocatePoolWithTag(PagedPool, 2 * sizeof(ULONG)+ NumberOfBands * sizeof(LONG), 'mdW'));

							if (Ptr)
							{
								PULONG BandsPresent = PULONG(Ptr + 1);

								PLONG Levels = PLONG(BandsPresent+1);

								Node->ReadParameter(REQUEST_MIN, CS, CN, Ptr, 2 * sizeof(ULONG)+ NumberOfBands * sizeof(LONG), NULL);

								ULONG j;
								
								for (j=0; j<NumberOfBands; j++)
								{											
									Range[j].Bounds.SignedMinimum = Levels[j];
								}

								Node->ReadParameter(REQUEST_MAX, CS, CN, Ptr, 2 * sizeof(ULONG)+ NumberOfBands * sizeof(LONG), NULL);

								for (j=0; j<NumberOfBands; j++)
								{											
									Range[j].Bounds.SignedMaximum = Levels[j];
								}

								Node->ReadParameter(REQUEST_RES, CS, CN, Ptr, 2 * sizeof(ULONG)+ NumberOfBands * sizeof(LONG), NULL);

								for (j=0; j<NumberOfBands; j++)
								{											
									Range[j].SteppingDelta = Levels[j];
									Range[j].Reserved = 0;
								}

								ExFreePool(Ptr);
							}

							Members = PKSPROPERTY_MEMBERSHEADER(Range + NumberOfBands);
						}
						else
						{
							Members = PKSPROPERTY_MEMBERSHEADER(Members + 1);
						}
					}

					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
										 		sizeof(KSPROPERTY_MEMBERSHEADER) * NumChannels +
												sizeof(KSPROPERTY_STEPPING_LONG) * TotalNumberOfBands;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

					ntStatus = STATUS_SUCCESS;
				}
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
 * EqCenterFreqs[]
 *****************************************************************************
 */
const
ULONG EqCenterFreqs[] =
{
	25,			// Band# 14
	32,			// Band# 15 - 31.5
	40,			// Band# 16
	50,			// Band# 17
	63,			// Band# 18
	80,			// Band# 19
	100,		// Band# 20
	125,		// Band# 21
	160,		// Band# 22
	200,		// Band# 23
	250,		// Band# 24
	315,		// Band# 25
	400,		// Band# 26
	500,		// Band# 27
	630,		// Band# 28
	800,		// Band# 29
	1000,		// Band# 30
	1250,		// Band# 31
	1600,		// Band# 32
	2000,		// Band# 33
	2500,		// Band# 34
	3150,		// Band# 35
	4000,		// Band# 36
	5000,		// Band# 37
	6300,		// Band# 38
	8000,		// Band# 39
	10000,		// Band# 40
	12500,		// Band# 41
	16000,		// Band# 42
	20000		// Band# 43
};

/*****************************************************************************
 * CAudioFilter::GetEqControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses EQ value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetEqControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetEqControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			// check if graphic EQ property request
			if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_NUM_EQ_BANDS)
			{
				// validate and get the output parameter
				if (ValueSize >= sizeof(ULONG))
				{
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);

					ntStatus = Node->ReadParameter(REQUEST_CUR, CS, CN, Value, sizeof(ULONG), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				ValueSize = sizeof(ULONG);
			}
			else if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_EQ_BANDS)
			{
				UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
				UCHAR CN = UCHAR(Channel+1);

				struct
				{
					ULONG NumberOfBands;
					ULONG BandsPresent;
				} EQ;

				EQ.NumberOfBands = 0;

				Node->ReadParameter(REQUEST_CUR, CS, CN, &EQ, 2 * sizeof(ULONG), NULL);

				if (EQ.NumberOfBands > 0)
				{
					// validate and get the output parameter
					if (ValueSize >= EQ.NumberOfBands * sizeof(ULONG))
					{
						PULONG EqBands = PULONG(Value);

						for (ULONG i=0, N = 0; i<SIZEOF_ARRAY(EqCenterFreqs); i++)
						{
							if (EQ.BandsPresent & (1<<i))
							{
								EqBands[N] = EqCenterFreqs[i]; N++;	
							}
						}

						//ASSERT(N == EQ.NumberOfBands);

						ntStatus = STATUS_SUCCESS;
					}

					ValueSize = EQ.NumberOfBands * sizeof(ULONG);
				}
				else
				{
					ValueSize = 0;

					ntStatus = STATUS_SUCCESS;
				}
			}
			else if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_EQ_LEVEL)
			{
				UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
				UCHAR CN = UCHAR(Channel+1);

				ULONG NumberOfBands = 0;

				Node->ReadParameter(REQUEST_CUR, CS, CN, &NumberOfBands, sizeof(ULONG), NULL);

				if (NumberOfBands > 0)
				{
					// validate and get the output parameter
					if (ValueSize >= NumberOfBands * sizeof(LONG))
					{
						PULONG Ptr = PULONG(ExAllocatePoolWithTag(PagedPool, 2 * sizeof(ULONG)+ NumberOfBands * sizeof(LONG), 'mdW'));

						if (Ptr)
						{
							PULONG BandsPresent = PULONG(Ptr + 1);

							PLONG Levels = PLONG(BandsPresent+1);

							ntStatus = Node->ReadParameter(REQUEST_CUR, CS, CN, Ptr, 2 * sizeof(ULONG)+ NumberOfBands * sizeof(LONG), NULL);

							if (NT_SUCCESS(ntStatus))
							{
								RtlCopyMemory(Value, Levels, NumberOfBands * sizeof(LONG));
							}
						
							ExFreePool(Ptr);
						}
						else
						{
							ntStatus = STATUS_INSUFFICIENT_RESOURCES;
						}
					}
					else
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}

					ValueSize = NumberOfBands * sizeof(LONG);
				}
				else
				{
					ValueSize = 0;

					ntStatus = STATUS_SUCCESS;
				}
			}
		}
	}

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetEqControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses EQ value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetEqControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetEqControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			// check if graphic EQ property request
			if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_EQ_LEVEL)
			{
				UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
				UCHAR CN = UCHAR(Channel+1);

				struct
				{
					ULONG NumberOfBands;
					ULONG BandsPresent;
				} EQ;

				EQ.NumberOfBands = 0;

				Node->ReadParameter(REQUEST_CUR, CS, CN, &EQ, 2 * sizeof(ULONG), NULL);

				if (EQ.NumberOfBands > 0)
				{
					// validate and get the output parameter
					if (ValueSize >= EQ.NumberOfBands * sizeof(LONG))
					{
						PULONG BandsPresent = PULONG(ExAllocatePoolWithTag(PagedPool, sizeof(ULONG)+ EQ.NumberOfBands * sizeof(LONG), 'mdW'));

						if (BandsPresent)
						{
							*BandsPresent = EQ.BandsPresent;

							PLONG Levels = PLONG(BandsPresent+1);

							// clamp it if it is out of bounds.
							PULONG Bounds = PULONG(ExAllocatePoolWithTag(PagedPool, 2 * sizeof(ULONG)+ EQ.NumberOfBands * sizeof(LONG), 'mdW'));

							if (Bounds)
							{
								// minimum
								Node->ReadParameter(REQUEST_MIN, CS, CN, Bounds, 2 * sizeof(ULONG)+ EQ.NumberOfBands * sizeof(LONG), NULL);

								PLONG MinLevels = PLONG(Bounds+2);

								ULONG i;
								
								for (i=0; i<EQ.NumberOfBands; i++)
								{
									if (Levels[i] < MinLevels[i])
									{
										Levels[i] = MinLevels[i];
									}
								}

								// maximum
								Node->ReadParameter(REQUEST_MAX, CS, CN, Bounds, 2 * sizeof(ULONG)+ EQ.NumberOfBands * sizeof(LONG), NULL);

								PLONG MaxLevels = PLONG(Bounds+2);

								for (i=0; i<EQ.NumberOfBands; i++)
								{
									if (Levels[i] > MaxLevels[i])
									{
										Levels[i] = MaxLevels[i];
									}
								}

								ExFreePool(Bounds);
							}
							else
							{
								RtlCopyMemory(Levels, Value, EQ.NumberOfBands * sizeof(LONG));
							}

							ntStatus = Node->WriteParameter(REQUEST_CUR, CS, CN, BandsPresent, sizeof(ULONG)+ EQ.NumberOfBands * sizeof(LONG));
					
							ExFreePool(BandsPresent);
						}
						else
						{
							ntStatus = STATUS_INSUFFICIENT_RESOURCES;
						}
					}
					else
					{
						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
				}
				else
				{
					ntStatus = STATUS_SUCCESS;
				}
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetDelayControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetDelayControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetDelayControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			// check if delay property request
			if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_DELAY)
			{
				// validate and get the output parameter
				if (ValueSize >= sizeof(KSTIME))
				{
					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);

					ntStatus = Node->ReadParameter(REQUEST_CUR, CS, CN, Value, sizeof(KSTIME), NULL);
				}
				else
				{
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}

				ValueSize = sizeof(KSTIME);
			}
		}
	}

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetDelayControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetDelayControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetDelayControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeProperty.NodeId);

	if (Node)
	{
		// get the instance channel parameter
		LONG Channel = Request->Channel;

		if ((Channel == CHAN_MASTER) || (ULONG(Channel) < Node->NumberOfChannels(0)))
		{
			// check if delay property request
			if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_DELAY)
			{
				// validate and get the input parameter
				if (ValueSize == sizeof(KSTIME))
				{
					PKSTIME Delay = PKSTIME(Value);

					UCHAR CS = FEATURE_UNIT_CONTROL_SELECTOR(Request->NodeProperty.Property.Id);
					UCHAR CN = UCHAR(Channel+1);

					// clamp it if it is beyond the limits.
					KSTIME Bounds;

					// minimum
					Node->ReadParameter(REQUEST_MIN, CS, CN, &Bounds, sizeof(KSTIME), NULL);

					if ((Delay->Time * Delay->Numerator / Delay->Denominator) < (Bounds.Time * Bounds.Numerator / Bounds.Denominator))
					{
						*Delay = Bounds;
					}

					// maximum
					Node->ReadParameter(REQUEST_MAX, CS, CN, &Bounds, sizeof(KSTIME), NULL);

					if ((Delay->Time * Delay->Numerator / Delay->Denominator) > (Bounds.Time * Bounds.Numerator / Bounds.Denominator))
					{
						*Delay = Bounds;
					}

					ntStatus = Node->WriteParameter(REQUEST_CUR, CS, CN, Delay, sizeof(KSTIME));
				}
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportEnableProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an enable processing unit control.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportEnableProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportEnableProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_TOPOLOGYNODE_ENABLE)
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
 * CAudioFilter::GetEnableProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an enable processing unit control.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetEnableProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetEnableProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_TOPOLOGYNODE_ENABLE)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(BOOL))
			{
				UCHAR CS = ENABLE_PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType());

				ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, sizeof(BOOL), NULL);
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
 * CAudioFilter::SetEnableProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an enable processing unit control.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetEnableProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetEnableProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_TOPOLOGYNODE_ENABLE)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(BOOL))
			{
				UCHAR CS = ENABLE_PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType());

				ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, sizeof(BOOL));
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportModeSelectControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportModeSelectControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportModeSelectControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_AUDIO_CHANNEL_CONFIG)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				ULONG NumberOfModes = Node->NumberOfModes(NULL);

				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
												 KSPROPERTY_TYPE_GET |
												 KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) + 
												 sizeof(KSPROPERTY_MEMBERSHEADER) +
												 sizeof(LONG) * NumberOfModes;
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_I4;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = 1;
				Description->Reserved          = 0;

				// if return buffer cn also hold a range description, return it too
				if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
								 sizeof(KSPROPERTY_MEMBERSHEADER) +
								 sizeof(LONG) * NumberOfModes)
				{
					// fill in the members header
					PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

					Members->MembersFlags   = KSPROPERTY_MEMBER_VALUES;
					Members->MembersSize    = sizeof(LONG);
					Members->MembersCount   = NumberOfModes;
					Members->Flags          = 0; 

					// fill in the values
					PLONG Values = PLONG(Members + 1);

					Node->NumberOfModes(Values);

					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
								sizeof(KSPROPERTY_MEMBERSHEADER) +
								sizeof(LONG) * NumberOfModes;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

					ntStatus = STATUS_SUCCESS;
				}
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
 * CAudioFilter::GetModeSelectControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetModeSelectControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetModeSelectControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_AUDIO_CHANNEL_CONFIG)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(KSAUDIO_CHANNEL_CONFIG))
			{
				UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);

				ULONG Mode = 1;

				ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, &Mode, sizeof(ULONG), NULL);

				if (NT_SUCCESS(ntStatus))
				{
					ULONG NumberOfModes = Node->NumberOfModes(NULL);

					ASSERT(Mode <= NumberOfModes);

					PLONG ActiveSpeakerPositions = PLONG(ExAllocatePoolWithTag(PagedPool, NumberOfModes * sizeof(LONG), 'mdW'));

					if (ActiveSpeakerPositions)
					{
						Node->NumberOfModes(ActiveSpeakerPositions);

						PKSAUDIO_CHANNEL_CONFIG ChannelConfig = PKSAUDIO_CHANNEL_CONFIG(Value);
						
						ChannelConfig->ActiveSpeakerPositions = ActiveSpeakerPositions[Mode-1];

						ExFreePool(ActiveSpeakerPositions);
					}
					else
					{
						ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					}
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(KSAUDIO_CHANNEL_CONFIG);
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetModeSelectControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetModeSelectControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetModeSelectControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_AUDIO_CHANNEL_CONFIG)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(KSAUDIO_CHANNEL_CONFIG))
			{
				UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);

				ULONG NumberOfModes = Node->NumberOfModes(NULL);

				PLONG ActiveSpeakerPositions = PLONG(ExAllocatePoolWithTag(PagedPool, NumberOfModes * sizeof(LONG), 'mdW'));

				if (ActiveSpeakerPositions)
				{
					Node->NumberOfModes(ActiveSpeakerPositions);

					KSAUDIO_CHANNEL_CONFIG ChannelConfig = *(PKSAUDIO_CHANNEL_CONFIG(Value));

					// Default to mode#1 if there isn't a match.
					ULONG Mode = 1;

					for (ULONG i=0; i<NumberOfModes; i++)
					{
						if (ChannelConfig.ActiveSpeakerPositions == ActiveSpeakerPositions[i])
						{
							Mode = i+1;
							break;
						}
					}

					ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, &Mode, sizeof(ULONG));

					ExFreePool(ActiveSpeakerPositions);
				}
				else
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				}
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportSignedProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportSignedProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportSignedProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (
			#if defined(KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE) ||
			#endif // KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE			
			#if defined(KSPROPERTY_AUDIO_DRC_THRESHOLD)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_THRESHOLD) ||
			#endif // KSPROPERTY_AUDIO_DRC_THRESHOLD
			0
			)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
												 KSPROPERTY_TYPE_GET |
												 KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
										 		 sizeof(KSPROPERTY_MEMBERSHEADER) +
												 sizeof(KSPROPERTY_STEPPING_LONG);
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_I4;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = 1;
				Description->Reserved          = 0;

				// if return buffer cn also hold a range description, return it too
				if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
								 sizeof(KSPROPERTY_MEMBERSHEADER) +
								 sizeof(KSPROPERTY_STEPPING_LONG))
				{
					// fill in the members header
					PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

					Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
					Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
					Members->MembersCount   = 1;
					Members->Flags          = 0;

					// fill in the stepped range
					PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

					UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);
					
					Node->ReadParameter(REQUEST_MIN, CS, 0, &Range->Bounds.SignedMinimum, sizeof(LONG), NULL);
					Node->ReadParameter(REQUEST_MAX, CS, 0, &Range->Bounds.SignedMaximum, sizeof(LONG), NULL);			
					Node->ReadParameter(REQUEST_RES, CS, 0, &Range->SteppingDelta, sizeof(LONG), NULL);

					Range->Reserved = 0;

					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
							    sizeof(KSPROPERTY_MEMBERSHEADER) +
								sizeof(KSPROPERTY_STEPPING_LONG);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

					ntStatus = STATUS_SUCCESS;
				}
			}
			else if (ValueSize >= sizeof(LONG))
			{
				// if return buffer can hold a ULONG, return the access flags
				PULONG AccessFlags = PULONG(Value);

				*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
							   KSPROPERTY_TYPE_GET |
							   KSPROPERTY_TYPE_SET;

				// set the return value size
				ValueSize = sizeof(LONG);

				ntStatus = STATUS_SUCCESS;
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetSignedProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetSignedProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetSignedProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (
			#if defined(KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE) ||
			#endif // KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE			
			#if defined(KSPROPERTY_AUDIO_DRC_THRESHOLD)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_THRESHOLD) ||
			#endif // KSPROPERTY_AUDIO_DRC_THRESHOLD
			0
			)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(LONG))
			{
				UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);

				ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, sizeof(LONG), NULL);

			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(LONG);
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetSignedProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetSignedProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetSignedProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (
			#if defined(KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE) ||
			#endif // KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE			
			#if defined(KSPROPERTY_AUDIO_DRC_THRESHOLD)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_THRESHOLD) ||
			#endif // KSPROPERTY_AUDIO_DRC_THRESHOLD
			0
			)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(LONG))
			{
				UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);

				ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, sizeof(LONG));
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportUnsignedProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportUnsignedProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportUnsignedProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if ((Request->Property.Id == KSPROPERTY_AUDIO_WIDENESS) ||
			#if defined(KSPROPERTY_AUDIO_REVERB_TYPE)
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_TYPE) ||
			#endif // KSPROPERTY_AUDIO_REVERB_TYPE
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_TIME) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_DELAY_FEEDBACK) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_LEVEL) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_MODULATION_RATE) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_MODULATION_DEPTH) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_LEVEL) ||
			#if defined(KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO) ||
			#endif // KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO					
			#if defined(KSPROPERTY_AUDIO_DRC_ATTACK_TIME)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_ATTACK_TIME) ||
			#endif // KSPROPERTY_AUDIO_DRC_ATTACK_TIME
			#if defined(KSPROPERTY_AUDIO_DRC_RELEASE_TIME)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_RELEASE_TIME) ||
			#endif // KSPROPERTY_AUDIO_DRC_RELEASE_TIME
			0
			)
		{
			if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
			{
				// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
				PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

				Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
												 KSPROPERTY_TYPE_GET |
												 KSPROPERTY_TYPE_SET;
				Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
										 		 sizeof(KSPROPERTY_MEMBERSHEADER) +
												 sizeof(KSPROPERTY_STEPPING_LONG);
				Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
				Description->PropTypeSet.Id    = VT_UI4;
				Description->PropTypeSet.Flags = 0;
				Description->MembersListCount  = 1;
				Description->Reserved          = 0;

				// if return buffer cn also hold a range description, return it too
				if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
								 sizeof(KSPROPERTY_MEMBERSHEADER) +
								 sizeof(KSPROPERTY_STEPPING_LONG))
				{
					// fill in the members header
					PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

					Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
					Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
					Members->MembersCount   = 1;
					Members->Flags          = 0;

					// fill in the stepped range
					PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

					UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);
					
					Node->ReadParameter(REQUEST_MIN, CS, 0, &Range->Bounds.UnsignedMinimum, sizeof(ULONG), NULL);
					Node->ReadParameter(REQUEST_MAX, CS, 0, &Range->Bounds.UnsignedMaximum, sizeof(ULONG), NULL);			
					Node->ReadParameter(REQUEST_RES, CS, 0, &Range->SteppingDelta, sizeof(ULONG), NULL);

					Range->Reserved = 0;

					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
								sizeof(KSPROPERTY_MEMBERSHEADER) +
								sizeof(KSPROPERTY_STEPPING_LONG);

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					// set the return value size
					ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

					ntStatus = STATUS_SUCCESS;
				}
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
 * CAudioFilter::GetUnsignedProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetUnsignedProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetUnsignedProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if ((Request->Property.Id == KSPROPERTY_AUDIO_WIDENESS) ||
			#if defined(KSPROPERTY_AUDIO_REVERB_TYPE)
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_TYPE) ||
			#endif // KSPROPERTY_AUDIO_REVERB_TYPE
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_TIME) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_DELAY_FEEDBACK) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_LEVEL) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_MODULATION_RATE) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_MODULATION_DEPTH) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_LEVEL) ||
			#if defined(KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO) ||
			#endif // KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO					
			#if defined(KSPROPERTY_AUDIO_DRC_ATTACK_TIME)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_ATTACK_TIME) ||
			#endif // KSPROPERTY_AUDIO_DRC_ATTACK_TIME
			#if defined(KSPROPERTY_AUDIO_DRC_RELEASE_TIME)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_RELEASE_TIME) ||
			#endif // KSPROPERTY_AUDIO_DRC_RELEASE_TIME
			0
			)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(ULONG))
			{
				UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);

				ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, sizeof(ULONG), NULL);

			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(ULONG);
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetUnsignedProcessingControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetUnsignedProcessingControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetUnsignedProcessingControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if ((Request->Property.Id == KSPROPERTY_AUDIO_WIDENESS) ||
			#if defined(KSPROPERTY_AUDIO_REVERB_TYPE)
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_TYPE) ||
			#endif // KSPROPERTY_AUDIO_REVERB_TYPE
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_TIME) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_DELAY_FEEDBACK) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_REVERB_LEVEL) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_MODULATION_RATE) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_MODULATION_DEPTH) ||
			(Request->Property.Id == KSPROPERTY_AUDIO_CHORUS_LEVEL) ||
			#if defined(KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO) ||
			#endif // KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO					
			#if defined(KSPROPERTY_AUDIO_DRC_ATTACK_TIME)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_ATTACK_TIME) ||
			#endif // KSPROPERTY_AUDIO_DRC_ATTACK_TIME
			#if defined(KSPROPERTY_AUDIO_DRC_RELEASE_TIME)
			(Request->Property.Id == KSPROPERTY_AUDIO_DRC_RELEASE_TIME) ||
			#endif // KSPROPERTY_AUDIO_DRC_RELEASE_TIME
			0
			)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(ULONG))
			{
				UCHAR CS = PROCESSING_UNIT_CONTROL_SELECTOR(Node->ProcessType(), Request->Property.Id);

				ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, sizeof(ULONG));
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportEnableExtensionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportEnableExtensionControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportEnableExtensionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_TOPOLOGYNODE_ENABLE)
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
 * CAudioFilter::GetEnableExtensionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetEnableExtensionControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetEnableExtensionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_TOPOLOGYNODE_ENABLE)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(BOOL))
			{
				UCHAR CS = USB_AUDIO_XU_CONTROL_ENABLE;

				ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, sizeof(BOOL), NULL);
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
 * CAudioFilter::SetEnableExtensionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetEnableExtensionControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetEnableExtensionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		// check if processing property request
		if (Request->Property.Id == KSPROPERTY_TOPOLOGYNODE_ENABLE)
		{
			// validate and get the output parameter
			if (ValueSize >= sizeof(BOOL))
			{
				UCHAR CS = USB_AUDIO_XU_CONTROL_ENABLE;

				ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, sizeof(BOOL));
			}
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetExtensionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetExtensionControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetExtensionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (Request->Property.Id)
		{
			UCHAR CS = UCHAR(Request->Property.Id+1);

			ntStatus = Node->ReadParameter(REQUEST_CUR, CS, 0, Value, ValueSize, &ValueSize);
		}
		else
		{
			ntStatus = STATUS_SUCCESS;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetExtensionControl()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SetExtensionControl
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetExtensionControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (Request->Property.Id)
		{
			UCHAR CS = UCHAR(Request->Property.Id+1);

			ntStatus = Node->WriteParameter(REQUEST_CUR, CS, 0, Value, ValueSize);

			if (NT_SUCCESS(ntStatus))
			{
				// Simulate status interrrupt change on the audio device to invoke 
				// a control change event.
				AudioFilter->SimulateStatusInterrupt(USB_AUDIO_STATUS_TYPE_ORIGINATOR_AC_INTERFACE | USB_AUDIO_STATUS_TYPE_INTERRUPT_PENDING, Node->UnitID());
			}
		}
		else
		{
			ntStatus = STATUS_SUCCESS;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SupportCpuResources()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SupportCpuResources
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SupportCpuResources]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
										     KSPROPERTY_TYPE_GET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_I4;
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
						   KSPROPERTY_TYPE_GET;

			// set the return value size
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_SUCCESS;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetCpuResources()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
GetCpuResources
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetCpuResources]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = AudioFilter->FindNode(Request->NodeId);

 	if (Node)
	{
        if (ValueSize >= sizeof(ULONG))
        {
            *(PULONG(Value)) = KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU;

			ntStatus = STATUS_SUCCESS;
        } 
		else
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

		ValueSize = sizeof(ULONG);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SwSupportLevelControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SwSupportLevelControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SwSupportLevelControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
	{
        ULONG NumChannels = 0;
        
        AudioFilter->m_AudioDevice->GetNoOfSupportedChannel(&NumChannels);

        // if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
		PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

		Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
										 KSPROPERTY_TYPE_GET |
										 KSPROPERTY_TYPE_SET;
		Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
									 	 sizeof(KSPROPERTY_MEMBERSHEADER) +
										 sizeof(KSPROPERTY_STEPPING_LONG) * NumChannels;
		Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
		Description->PropTypeSet.Id    = VT_I4;
		Description->PropTypeSet.Flags = 0;
		Description->MembersListCount  = 1;
		Description->Reserved          = 0;

		// if return buffer cn also hold a range description, return it too
		if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
						 sizeof(KSPROPERTY_MEMBERSHEADER) +
						 sizeof(KSPROPERTY_STEPPING_LONG) * NumChannels)
		{
			// fill in the members header
			PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

			Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
			Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
			Members->MembersCount   = NumChannels;
			Members->Flags          = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL; 

			// fill in the stepped range
			PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

			for (ULONG i=0; i<NumChannels; i++)
			{				
			    ntStatus = AudioFilter->m_AudioDevice->GetMasterVolumeRange(
                                                                         i,
                                                                         (LONG*)&Range[i].Bounds.SignedMinimum,
																		 (LONG*)&Range[i].Bounds.SignedMaximum,
																		 (LONG*)&Range[i].SteppingDelta);
				Range[i].Reserved = 0;
			}

			// set the return value size
			ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
						sizeof(KSPROPERTY_MEMBERSHEADER) +
						sizeof(KSPROPERTY_STEPPING_LONG) * NumChannels;

		}
		else
		{
			// set the return value size
			ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

			ntStatus = STATUS_SUCCESS;
		}
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

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SwGetLevelControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SwGetLevelControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetLevelControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	// get the instance channel parameter
	LONG Channel = Request->Channel;

	// validate and get the output parameter
	if (ValueSize >= sizeof(LONG))
	{
		// check if volume property request
		if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_VOLUMELEVEL)
		{
            ntStatus = AudioFilter->m_AudioDevice->GetMasterVolume(Channel,(LONG*)Value);
            _DbgPrintF(DEBUGLVL_VERBOSE,("CAudioFilter::SwGetLevelControl ntStatus=%x Ch=%x Value= 0x%x\n",ntStatus, Channel, *(ULONG*)Value));
		}
	}
	else
	{
		ntStatus = STATUS_BUFFER_TOO_SMALL;
	}

	ValueSize = sizeof(LONG);

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SwSetLevelControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses level value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SwSetLevelControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SwSetLevelControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	// get the instance channel parameter
	LONG Channel = Request->Channel;

	// validate and get the output parameter
	if (ValueSize >= sizeof(LONG))
	{
		if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_VOLUMELEVEL)
		{
            ntStatus = AudioFilter->m_AudioDevice->SetMasterVolume(Channel,*(LONG*)Value);
            _DbgPrintF(DEBUGLVL_VERBOSE,("CAudioFilter::SwSetLevelControl ntStatus=%x Ch=%x Value= 0x%x\n",ntStatus, Channel, *(ULONG*)Value));
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SwSupportOnOffControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an on/off value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SwSupportOnOffControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SwSupportOnOffControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MUTE)
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
            ULONG NumChannels = 0;
            
            AudioFilter->m_AudioDevice->GetNoOfSupportedChannel(&NumChannels);

			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
											 KSPROPERTY_TYPE_GET |
											 KSPROPERTY_TYPE_SET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
											 sizeof(KSPROPERTY_MEMBERSHEADER) + 
											 sizeof(BOOL);
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_BOOL;
			Description->PropTypeSet.Flags = 0;
			Description->MembersListCount  = 1;
			Description->Reserved          = 0;

			// if return buffer cn also hold a range description, return it too
			if (ValueSize >= sizeof(KSPROPERTY_DESCRIPTION) +
				  		     sizeof(KSPROPERTY_MEMBERSHEADER) + 
							 sizeof(BOOL))
			{  
				// fill in the members header
				PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(Description + 1);

				Members->MembersFlags   = 0;
				Members->MembersSize    = 0;
				Members->MembersCount   = NumChannels;
				Members->Flags          = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL | KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_UNIFORM; // master control

                Value = (PVOID)TRUE; //always set to TRUE

				// set the return value size
				ValueSize = sizeof(KSPROPERTY_DESCRIPTION) +
											 sizeof(KSPROPERTY_MEMBERSHEADER) +
											 sizeof(BOOL);

				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				// set the return value size
				ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

				ntStatus = STATUS_SUCCESS;
			}
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

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::GetOnOffControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an on/off value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SwGetOnOffControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SwGetOnOffControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	// get the instance channel parameter
	LONG Channel = Request->Channel;

	if (Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MUTE)
	{
		// validate and get the output parameter
		if (ValueSize >= sizeof(BOOL))
		{						
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::GetOnOffControl] - Channel: %d, OnOff: 0x%x", Channel, *(ULONG*)Value));
            ntStatus = AudioFilter->m_AudioDevice->GetMasterMute((BOOL*)Value);
		}
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(BOOL);
	}

 	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CAudioFilter::SetOnOffControl()
 *****************************************************************************
 *//*!
 * @brief
 * Accesses an on/off value property.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CAudioFilter::
SwSetOnOffControl
(
	IN		PIRP							Irp,
	IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
	IN OUT	PVOID							Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SwSetOnOffControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CAudioFilter * AudioFilter = (CAudioFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	// get the instance channel parameter
	LONG Channel = Request->Channel;

	if(Request->NodeProperty.Property.Id == KSPROPERTY_AUDIO_MUTE)
	{
		// validate and get the output parameter
		if (ValueSize >= sizeof(BOOL))
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilter::SetOnOffControl] - Channel: %d, OnOff: 0x%x", Channel, *(ULONG*)Value));
            ntStatus = AudioFilter->m_AudioDevice->SetMasterMute(*(BOOL*)Value);
		}
	}
 
	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

	return ntStatus;
}
#pragma code_seg()

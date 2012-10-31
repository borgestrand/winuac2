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
 * @file       audfilter.cpp
 * @brief      This is the implementation file for C++ classes that expose 
 *             functionality of KS filters that support PCM audio formats.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "audfilter.h"

#define STR_MODULENAME "AUDFILTER: "


/*****************************************************************************
 * CKsAudioFilter::CKsAudioFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsAudioFilter::
CKsAudioFilter
(
    IN		LPCTSTR		SymbolicLink,
    IN		LPCTSTR		FriendlyName,
    OUT		HRESULT *	OutHResult
) 
:	CKsFilter(SymbolicLink, FriendlyName, OutHResult)
{
}

/*****************************************************************************
 * CKsAudioFilter::EnumeratePins()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsAudioFilter::
EnumeratePins
(	void
)
{
    BOOL OutOfMemory = FALSE;

	ULONG NumberOfPins = 0;

    // get the number of pins supported by SAD
    HRESULT hr = GetPinPropertySimple
					(  
						0,
						KSPROPSETID_Pin,
						KSPROPERTY_PIN_CTYPES,
						&NumberOfPins,
						sizeof(ULONG)
					);

    if (FAILED(hr))
    {
        _DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Failed to retrieve count of pins in audio filter"));
    }
    else // (SUCCEEDED(hr))
    {
        //
        // loop through the pins, looking for audio pins
        //
        for(ULONG PinId = 0; PinId < NumberOfPins; PinId++)
        {

            // This information is needed to create the right type of pin
            
            //
            // get COMMUNICATION ---------------
            //
            KSPIN_COMMUNICATION Communication = KSPIN_COMMUNICATION_NONE;
            
			hr = GetPinPropertySimple
					( 
						PinId,
						KSPROPSETID_Pin,
						KSPROPERTY_PIN_COMMUNICATION,
						&Communication,
						sizeof(KSPIN_COMMUNICATION)
					);
			
			if (FAILED(hr))
            {
				_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Failed to retrieve pin property KSPROPERTY_PIN_COMMUNICATION"));
            }

            if (SUCCEEDED(hr))
            {
                // ILL communication
                if ((Communication != KSPIN_COMMUNICATION_SOURCE) &&
                    (Communication != KSPIN_COMMUNICATION_SINK) &&
                    (Communication != KSPIN_COMMUNICATION_BOTH) )
                {
					_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Pin communication value doesn't make sense"));

                    hr = E_FAIL;
                }
            }

            KSPIN_DATAFLOW Dataflow = (KSPIN_DATAFLOW)0;

            if (SUCCEEDED(hr))
            {
                //
                // Get the data flow property
                //
                hr = GetPinPropertySimple
						(             
							PinId,
							KSPROPSETID_Pin,
							KSPROPERTY_PIN_DATAFLOW,
							&Dataflow,
							sizeof(KSPIN_DATAFLOW)
						);

                if (FAILED(hr))
                {
					_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Failed to retrieve pin property KSPROPERTY_PIN_DATAFLOW"));
                }
            }

            //
            // create a new pin
            //
            CKsAudioPin* NewPin = NULL;

            if (SUCCEEDED(hr))
            {
                // If the pin is an IRP sink and inputs data, it's a render pin
                if ((KSPIN_COMMUNICATION_SINK == Communication) &&
					(KSPIN_DATAFLOW_IN == Dataflow))
                {
                    NewPin = new CKsAudioRenderPin(this, PinId, &hr);
                }
                // If the pin is an IRP sink and outputs data, it's a capture pin
                else if ((KSPIN_COMMUNICATION_SINK == Communication) &&
						 (KSPIN_DATAFLOW_OUT == Dataflow))
                {
                    NewPin = new CKsAudioCapturePin(this, PinId, &hr);
                }
                // Otherwise, it's just a pin
                else
                {
                    NewPin = new CKsAudioPin(this, PinId, &hr);
                }

                if (NULL == NewPin)
                {
					_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Failed to create audio pin"));

                    hr = E_OUTOFMEMORY;
                    
					OutOfMemory = TRUE;
                    break;
                }
            }

            if (SUCCEEDED(hr))
            {
                if (NULL == m_ListPins.AddTail(NewPin))
                {
                    delete NewPin;

					_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Unable to allocate list entry to save pin in"));

                    hr = E_OUTOFMEMORY;

					OutOfMemory = TRUE;
                    break;
                }
            }
            else //if (FAILED(hr))
            {
				if (NewPin)
				{
					delete NewPin;
				}
            }
        }
    }

    // If we ran out of memory, delete all the pins, as we're in a bad state
    if (OutOfMemory)
    {
		_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - Ran out of memory enumerating pins - deleting all pins"));

        CKsPin* Pin = NULL;

        while (m_ListPins.RemoveHead(&Pin))
        {
            delete Pin;
        }

        hr = E_OUTOFMEMORY;
    }
    else
    {
        ClassifyPins();
        
        if (m_ListRenderSinkPins.IsEmpty() && 
			m_ListRenderSourcePins.IsEmpty() && 
			m_ListCaptureSinkPins.IsEmpty() && 
			m_ListCaptureSourcePins.IsEmpty())
        {
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioFilter::EnumeratePins] - No valid pins found on the filter"));

            hr = E_FAIL;
        }
        else
        {
            hr = S_OK;
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsAudioRenderFilter::CKsAudioRenderFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor
 */
CKsAudioRenderFilter::
CKsAudioRenderFilter
(
    IN		LPCTSTR		SymbolicLink,
    IN		LPCTSTR		FriendlyName,
    OUT		HRESULT *	OutHResult
) 
:	CKsAudioFilter(SymbolicLink, FriendlyName, OutHResult)
{
	m_FilterType = KS_TECHNOLOGY_TYPE_AUDIO_RENDER;
}

/*****************************************************************************
 * CKsAudioRenderFilter::NumberOfRenderPins()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
CKsAudioRenderFilter::
NumberOfRenderPins
(	void
)
{
	return m_ListRenderSinkPins.GetCount();
}

/*****************************************************************************
 * CKsAudioRenderFilter::ParseRenderPins()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CKsAudioRenderFilter::
ParseRenderPins
(
	IN		ULONG					Index,
	OUT		CKsAudioRenderPin **	OutAudioRenderPin
)
{
	ULONG i = 0;

    CKsPin * KsPin = NULL;

    LISTPOS ListPos = m_ListRenderSinkPins.GetHeadPosition();
    
	while (m_ListRenderSinkPins.GetNext(ListPos, &KsPin) && (i <= Index))
    {
		if (i == Index)
		{
			*OutAudioRenderPin = (CKsAudioRenderPin*)KsPin;

			return TRUE;
		}

		i++;
    }

    return FALSE;
}

/*****************************************************************************
 * CKsAudioRenderFilter::FindViablePin()
 *****************************************************************************
 *//*!
 * @brief
 * Look through the given list and find one that can do the specified 
 * WaveFormatEx.
 */
CKsAudioRenderPin *
CKsAudioRenderFilter::
FindViablePin
(
	IN		PWAVEFORMATEX	WaveFormatEx,
	IN		ULONG			MatchingCriteria
)
{
    ASSERT(WaveFormatEx);

    CKsPin * KsPin = NULL;

    LISTPOS ListPos = m_ListRenderSinkPins.GetHeadPosition();
    
	while (m_ListRenderSinkPins.GetNext(ListPos, &KsPin))
    {
        CKsAudioRenderPin * AudioRenderPin = (CKsAudioRenderPin*)KsPin;

        // To only look at non-digital output pins, check that AudioRenderPin->IsVolumeSupported() is TRUE,
        // as digital output pins don't have volume controls associated with them.
        if (AudioRenderPin->IsFormatSupported(WaveFormatEx, MatchingCriteria))
        {
            // This should be a valid pin
            return AudioRenderPin;
        }
    }

    return NULL;
}

/*****************************************************************************
 * CKsAudioRenderFilter::CreateRenderPin()
 *****************************************************************************
 *//*!
 * @brief
 * Look through m_ListRenderPins and find one that can do pwfx and create it.
 * @param
 * WaveFormatEx WAVEFORMATEX that the pin must support. 
 * @param
 * Looped Flag whether or not the pin plays backed looped buffers or streamed 
 * buffers.
 */
CKsAudioRenderPin *
CKsAudioRenderFilter::
CreateRenderPin
(
	IN		PWAVEFORMATEX	WaveFormatEx,
	IN		BOOL			Looped
)
{
    HRESULT hr = S_OK;

    CKsAudioRenderPin * Pin = FindViablePin(WaveFormatEx);

    if (!Pin)
    {
		_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioRenderFilter::CreateRenderPin] - Could not find a Render pin that supports the given wave format"));

        hr = E_FAIL;
    }
    else
    {
        hr = Pin->SetFormat(WaveFormatEx);

        if (FAILED(hr))
        {
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioRenderFilter::CreateRenderPin] - Failed to set Render Pin format - the pin lied about its supported formats"));
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = Pin->Instantiate(Looped);

        if (SUCCEEDED(hr))
        {
			_DbgPrintF(DEBUGLVL_BLAB,("[CKsAudioRenderFilter::CreateRenderPin] - Successfully instantiated Render Pin.  Handle = 0x%08x", Pin->GetHandle()));
        }
        else
        {
 			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioRenderFilter::CreateRenderPin] - Failed to instantiate Render Pin"));
        }
    }

    if (FAILED(hr))
    {
        // Initialize Pin to NULL again
        Pin = NULL;

        // Try to intstantiate all the pins, one at a time
        CKsPin * KsPin = NULL;

        LISTPOS ListPosPin = m_ListRenderSinkPins.GetHeadPosition();
        
		while (!Pin && m_ListRenderSinkPins.GetNext(ListPosPin, &KsPin))
        {
            CKsAudioRenderPin * AudioRenderPin = (CKsAudioRenderPin *)KsPin;

            hr = AudioRenderPin->SetFormat(WaveFormatEx);
            
			if (SUCCEEDED(hr))
            {
                hr = AudioRenderPin->Instantiate(Looped);
            }

            if (SUCCEEDED(hr))
            {
                // Save the pin in pPin
                Pin = AudioRenderPin;
                break;
            }
        }
    }

    if (FAILED(hr))
    {
        // Don't delete the pin - it's still in m_ListRenderPins
        Pin = NULL;
    }
    else
    {
        // Remove the pin from the filter's list of pins
        LISTPOS ListPosPinNode = m_ListPins.Find(Pin);

        ASSERT(ListPosPinNode);

        m_ListPins.RemoveAt(ListPosPinNode);

        ListPosPinNode = m_ListRenderSinkPins.Find(Pin);

        ASSERT(ListPosPinNode);

        m_ListRenderSinkPins.RemoveAt(ListPosPinNode);
    }

    return Pin;
}

/*****************************************************************************
 * CKsAudioRenderFilter::CanCreateRenderPin()
 *****************************************************************************
 *//*!
 * @brief
 * Look through m_ListRenderPins and check if one that supports the given 
 * wave format.
 */
BOOL
CKsAudioRenderFilter::
CanCreateRenderPin
(
	IN		PWAVEFORMATEX	WaveFormatEx
)
{
	HRESULT hr = S_OK;

    CKsAudioRenderPin* Pin = FindViablePin(WaveFormatEx);

    if (!Pin)
    {
 		_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAudioRenderFilter::CanCreateRenderPin] - Could not find a Render pin that supports the given wave format"));

        hr = E_FAIL;
    }

    return SUCCEEDED(hr);
}

/*****************************************************************************
 * CKsAudioCaptureFilter::CKsAudioCaptureFilter()
 *****************************************************************************
 *//*!
 * @brief
 */
CKsAudioCaptureFilter::
CKsAudioCaptureFilter
(
    IN		LPCTSTR		SymbolicLink,
    IN		LPCTSTR		FriendlyName,
    OUT		HRESULT *	OutHResult
) 
:	CKsAudioFilter(SymbolicLink, FriendlyName, OutHResult)
{
    m_FilterType = KS_TECHNOLOGY_TYPE_AUDIO_CAPTURE;
}

/*****************************************************************************
 * CKsAudioCaptureFilter::NumberOfCapturePins()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
CKsAudioCaptureFilter::
NumberOfCapturePins
(	void
)
{
	return m_ListCaptureSinkPins.GetCount();
}

/*****************************************************************************
 * CKsAudioCaptureFilter::ParseCapturePins()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CKsAudioCaptureFilter::
ParseCapturePins
(
	IN		ULONG					Index,
	OUT		CKsAudioCapturePin **	OutAudioCapturePin
)
{
	ULONG i = 0;

    CKsPin * KsPin = NULL;

    LISTPOS ListPos = m_ListCaptureSinkPins.GetHeadPosition();
    
	while (m_ListCaptureSinkPins.GetNext(ListPos, &KsPin))
    {
		if (i == Index)
		{
			*OutAudioCapturePin = (CKsAudioCapturePin*)KsPin;

			return TRUE;
		}

		i++;
    }

    return FALSE;
}

/*****************************************************************************
 * CKsAudioCaptureFilter::FindViablePin()
 *****************************************************************************
 *//*!
 * @brief
 */
CKsAudioCapturePin *
CKsAudioCaptureFilter::
FindViablePin
(
	IN		PWAVEFORMATEX	WaveFormatEx,
	IN		ULONG			MatchingCriteria
)
{
	ASSERT(WaveFormatEx);

    CKsPin * KsPin = NULL;

    LISTPOS ListPos = m_ListCaptureSinkPins.GetHeadPosition();

    while (m_ListCaptureSinkPins.GetNext(ListPos, &KsPin))
    {
        CKsAudioCapturePin * AudioCapturePin = (CKsAudioCapturePin*)KsPin;

        if (AudioCapturePin->IsFormatSupported(WaveFormatEx, MatchingCriteria))
        {
            // This should be a valid pin
            return AudioCapturePin;
        }
    }

    return NULL;
}

/*****************************************************************************
 * CKsAudioCaptureFilter::CreateCapturePin()
 *****************************************************************************
 *//*!
 * @brief
 */
CKsAudioCapturePin *
CKsAudioCaptureFilter::
CreateCapturePin
(
	IN		PWAVEFORMATEX	WaveFormatEx,
	IN		BOOL			Looped
)
{
    HRESULT hr = S_OK;

    CKsAudioCapturePin * Pin = FindViablePin(WaveFormatEx);

    if (!Pin)
    {
 		_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioCaptureFilter::CreateCapturePin] - Could not find a Capture pin that supports the given wave format"));

        hr = E_FAIL;
    }
    else
    {
        hr = Pin->SetFormat(WaveFormatEx);

        if (FAILED(hr))
        {
 			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioCaptureFilter::CreateCapturePin] - Failed to set Capture Pin format"));
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = Pin->Instantiate(Looped);

        if (SUCCEEDED(hr))
        {
 			_DbgPrintF(DEBUGLVL_BLAB,("[CKsAudioCaptureFilter::CreateCapturePin] - Successfully instantiated Capture Pin.  Handle = 0x%08x", Pin->GetHandle()));
        }
        else
        {
 			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioCaptureFilter::CreateCapturePin] - Failed to instantiate Capture Pin"));
        }
    }

    if (FAILED(hr))
    {
        delete Pin;

        Pin = NULL;
    }

   return Pin;
}

/*****************************************************************************
 * CKsAudioCaptureFilter::GetCapturePin()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsAudioCaptureFilter::
GetCapturePin
(
	OUT		CKsAudioCapturePin **	OutCapturePin
)
{
    HRESULT hr = S_OK;

    CKsPin * KsPin = NULL;

    if (NULL == OutCapturePin)
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        // Get the first pin on the list
        if (!m_ListPins.GetHead(&KsPin))
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        *OutCapturePin = (CKsAudioCapturePin*)(KsPin);
    }
    
    return hr;
}

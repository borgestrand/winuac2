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
 * @file       audpin.cpp
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS pins that support streaming of PCM audio.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "audpin.h"

#define STR_MODULENAME "AUDPIN: "


/*****************************************************************************
 * CKsAudioPin::CKsAudioPin()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsAudioPin::
CKsAudioPin
(
    IN		CKsAudioFilter *	KsAudioFilter, 
    IN		ULONG				PinId,
    OUT		HRESULT *			OutHResult
) 
:	CKsPin(KsAudioFilter, PinId, OutHResult)
{
    m_KsAudioFilter = KsAudioFilter;

    m_WaveFormatEx = NULL;

    m_KsDataFormatWfx = NULL;

    HRESULT hr = *OutHResult;

    if (SUCCEEDED(hr))
    {
        hr = m_ListDataRanges.Initialize(1);

        if (FAILED(hr))
		{
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::CKsAudioPin] - Failed to Initialize m_ListDataRanges"));
 		}
    }

    if (SUCCEEDED(hr))
    {
        hr = m_ListNodes.Initialize(1);

        if (FAILED(hr))
		{
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::CKsAudioPin] - Failed to Initialize m_ListNodes"));
 		}
    }

    // Create a KSPIN_CONNECT structure to describe a waveformatex pin
    if (SUCCEEDED(hr))
    {
        m_KsPinConnectSize = sizeof(KSPIN_CONNECT) + sizeof(KSDATAFORMAT_WAVEFORMATEX);

        m_KsPinConnect = (PKSPIN_CONNECT)new BYTE[m_KsPinConnectSize];
        
		if (!m_KsPinConnect)
        {
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::CKsAudioPin] - Failed to allocate m_KsPinConnect"));

            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_KsPinConnect->Interface.Set              = KSINTERFACESETID_Standard;
        m_KsPinConnect->Interface.Id               = KSINTERFACE_STANDARD_STREAMING;
        m_KsPinConnect->Interface.Flags            = 0;
        m_KsPinConnect->Medium.Set                 = KSMEDIUMSETID_Standard;
        m_KsPinConnect->Medium.Id                  = KSMEDIUM_TYPE_ANYINSTANCE;
        m_KsPinConnect->Medium.Flags               = 0;
        m_KsPinConnect->PinId                      = PinId;
        m_KsPinConnect->PinToHandle                = NULL;
        m_KsPinConnect->Priority.PriorityClass     = KSPRIORITY_NORMAL;
        m_KsPinConnect->Priority.PrioritySubClass  = 1;

        // point m_pksDataFormatWfx to just after the pConnect struct
        PKSDATAFORMAT_WAVEFORMATEX KsDataFormatWfx = (PKSDATAFORMAT_WAVEFORMATEX)(m_KsPinConnect + 1);

        // set up format for KSDATAFORMAT_WAVEFORMATEX
        KsDataFormatWfx->DataFormat.FormatSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);
        KsDataFormatWfx->DataFormat.Flags = 0;
        KsDataFormatWfx->DataFormat.Reserved = 0;
        KsDataFormatWfx->DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
        KsDataFormatWfx->DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        KsDataFormatWfx->DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

        m_KsDataFormatWfx = KsDataFormatWfx;
    }


    // Initialize the Pin;
    if (SUCCEEDED(hr))
    {
        hr = CKsAudioPin::Init();
    }

    
    *OutHResult = hr;
}

/*****************************************************************************
 * CKsAudioPin::~CKsAudioPin()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CKsAudioPin::
~CKsAudioPin
(	void
)
{
    PKSDATARANGE_AUDIO DataRangeAudio = NULL;

    // Clear datarange list
    while (m_ListDataRanges.RemoveHead(&DataRangeAudio))
    {
        delete DataRangeAudio;
    }

    CKsNode * KsNode = NULL;

    // Clear the node list
    while (m_ListNodes.RemoveHead(&KsNode))
    {
        delete KsNode;
    }

    delete[] (BYTE *)m_WaveFormatEx;
}

/*****************************************************************************
 * CKsAudioPin::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize internal data structures and does some sanity-checking.
 */
HRESULT 
CKsAudioPin::
Init
(	void
)
{
    HRESULT hr = S_OK;

    BOOL ViablePin = FALSE;

    // Make sure at least one interface is standard streaming
    for (ULONG i = 0; i < m_PinDescriptor.InterfaceCount && !ViablePin; i++)
    {
		ViablePin = ViablePin ||
					IsEqualGUID(m_PinDescriptor.Interfaces[i].Set, KSINTERFACESETID_Standard) && 
					(m_PinDescriptor.Interfaces[i].Id == KSINTERFACE_STANDARD_STREAMING);
    }

    if (!ViablePin)
    {
 		_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::Init] - No standard streaming interfaces on the pin"));

        hr = E_FAIL;
    }

    // Make sure at least one medium is standard streaming
    if (SUCCEEDED(hr))
    {
        ViablePin = FALSE;

        for (ULONG i = 0; i < m_PinDescriptor.InterfaceCount && !ViablePin; i++)
        {
            ViablePin = ViablePin ||
						IsEqualGUID(m_PinDescriptor.Mediums[i].Set, KSMEDIUMSETID_Standard) && 
						(m_PinDescriptor.Mediums[i].Id == KSMEDIUM_STANDARD_DEVIO) ;
        }

        if (!ViablePin)
        {
 			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::Init] - No standard streaming mediums on the pin"));

			hr = E_FAIL;
        }
    }

    // Make sure at least one datarange supports audio    
    if (SUCCEEDED(hr))
    {
        ViablePin = FALSE;

        PKSDATARANGE DataRange = m_PinDescriptor.DataRanges;
        
        for (ULONG i = 0; i < m_PinDescriptor.DataRangeCount; i++)
        {
            // SubType should either be compatible with WAVEFORMATEX or 
            // it should be WILDCARD
            ViablePin = ViablePin || 
						IS_VALID_WAVEFORMATEX_GUID(&DataRange->SubFormat) ||
						IsEqualGUID(DataRange->SubFormat, KSDATAFORMAT_SUBTYPE_PCM) ||
						IsEqualGUID(DataRange->SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD);

            if (ViablePin && IsEqualGUID(DataRange->MajorFormat, KSDATAFORMAT_TYPE_AUDIO))
            {
                // Copy the data range into the pin
                PKSDATARANGE_AUDIO NewDataRangeAudio = new KSDATARANGE_AUDIO;

                if (NewDataRangeAudio)
                {
                    PKSDATARANGE_AUDIO DataRangeAudio = (PKSDATARANGE_AUDIO)DataRange;

                    CopyMemory(NewDataRangeAudio, DataRangeAudio, sizeof(KSDATARANGE_AUDIO));

					if (NULL == m_ListDataRanges.AddTail(NewDataRangeAudio))
                    {
                        delete NewDataRangeAudio;

						_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::Init] - Unable to allocate list entry to save datarange in"));

                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
					_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::Init] - Unable to allocate memory to save datarange in"));

                    hr = E_OUTOFMEMORY;
                }
            }

            DataRange = (PKSDATARANGE)(((PBYTE)DataRange) + DataRange->FormatSize);
        }

        if (!ViablePin)
        {
 			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::Init] - No audio dataranges on the pin"));

            hr = E_FAIL;
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsAudioPin::SetFormat()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT    
CKsAudioPin::
SetFormat
(
	IN		PWAVEFORMATEX	WaveFormatEx
)
{
    HRESULT hr = S_OK;

    if (!(WaveFormatEx && m_KsDataFormatWfx))
    {
		_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::SetFormat] - WaveFormatEx or m_KsDataFormatWfx are NULL"));

        hr = E_FAIL;
    }

    ULONG NewFormatSize = 0;

    if (SUCCEEDED(hr))
    {
		if (WAVE_FORMAT_PCM == WaveFormatEx->wFormatTag)
		{
			NewFormatSize = sizeof(WAVEFORMATEX);
		}
		else
		{
			NewFormatSize = sizeof(WAVEFORMATEX) + WaveFormatEx->cbSize;
		}

        // If the new format differs in size from the old format, re-allocate m_KsPinConnect
        if (m_KsPinConnectSize != (NewFormatSize + sizeof(KSPIN_CONNECT) + sizeof(KSDATAFORMAT_WAVEFORMATEX) - sizeof(WAVEFORMATEX)))
        {
            // create a KSPIN_CONNECT structure to describe a waveformatex pin
            ULONG KsPinConnectSize = NewFormatSize + sizeof(KSPIN_CONNECT) + sizeof(KSDATAFORMAT_WAVEFORMATEX) - sizeof(WAVEFORMATEX);
            
			PVOID KsPinConnect = new BYTE[KsPinConnectSize];
            
			if (!KsPinConnect)
            {
				_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::SetFormat] - Unable to allocate KSPIN_CONNECT structure"));

                hr = E_OUTOFMEMORY;
            }
            else
            {
                // Copy the old pin structure to the new one
                CopyMemory(KsPinConnect, m_KsPinConnect, min(KsPinConnectSize, m_KsPinConnectSize));

                // Free the old structure
                delete[] m_KsPinConnect;

                // point m_pksPinCreate at the new structure
                m_KsPinConnect = (PKSPIN_CONNECT)KsPinConnect;

                // point m_pksDataFormatWfx to just after the pConnect struct
                m_KsDataFormatWfx = (PKSDATAFORMAT_WAVEFORMATEX)(m_KsPinConnect + 1);

                // Set the new format size parameter
                m_KsDataFormatWfx->DataFormat.FormatSize = NewFormatSize + sizeof(KSDATAFORMAT_WAVEFORMATEX) - sizeof(WAVEFORMATEX);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Copy the new format into KsDataFormatWfx
        PKSDATAFORMAT_WAVEFORMATEX KsDataFormatWfx = m_KsDataFormatWfx;

        CopyMemory(&KsDataFormatWfx->WaveFormatEx, WaveFormatEx, NewFormatSize);

        // Set the sample size in KsDataFormatWfx
        KsDataFormatWfx->DataFormat.SampleSize = (USHORT)(WaveFormatEx->nChannels * WaveFormatEx->wBitsPerSample / 8);
     
        // Save the wave format
        delete[] (BYTE*)m_WaveFormatEx;

        m_WaveFormatEx = (WAVEFORMATEX *)new BYTE[NewFormatSize];
        
		if (m_WaveFormatEx)
        {
            CopyMemory(m_WaveFormatEx, WaveFormatEx, NewFormatSize);
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsAudioPin::GetPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the current position.
 */
HRESULT
CKsAudioPin::
GetPosition
(
    OUT		KSAUDIO_POSITION *	OutPosition
)
{
    HRESULT hr = S_OK;

    if (NULL == OutPosition)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetPropertySimple(KSPROPSETID_Audio, KSPROPERTY_AUDIO_POSITION, OutPosition, sizeof(KSAUDIO_POSITION));

        if (FAILED(hr))
        {
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::GetPosition] - Failed to retrieve audio stream position - %#08x", hr));
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsAudioPin::SetPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the current position.
 */
HRESULT
CKsAudioPin::
SetPosition
(
    IN		PKSAUDIO_POSITION	Position
)
{
    HRESULT hr = S_OK;

    if (NULL == Position)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        hr = SetPropertySimple(KSPROPSETID_Audio, KSPROPERTY_AUDIO_POSITION, Position, sizeof(KSAUDIO_POSITION));

        if (FAILED(hr))
        {
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsAudioPin::GetPosition] - Failed to set audio stream position - %#08x", hr));
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsAudioPin::GetNode()
 *****************************************************************************
 *//*!
 * @brief
 * Get the CKsNode* in m_ListNodes for a given ID value.
 */
CKsNode * 
CKsAudioPin::
GetNode
(
	IN		ULONG	NodeId
)
{
    CKsNode * KsNode = NULL;

    LISTPOS ListPos = m_ListNodes.GetHeadPosition();
    
	while (m_ListNodes.GetNext(ListPos, &KsNode))
    {
        if (NodeId == KsNode->GetNodeId())
        {
            break;
        }
    }

    return KsNode;
}

/*****************************************************************************
 * CKsAudioPin::IsFormatSupported()
 *****************************************************************************
 *//*!
 * @brief
 * Check if the given format is supported.
 */
BOOL 
CKsAudioPin::
IsFormatSupported
(
	IN		PWAVEFORMATEX	WaveFormatEx,
	IN		ULONG			MatchingCriteria
)
{
    LISTPOS ListPosRange = m_ListDataRanges.GetHeadPosition();

    PKSDATARANGE_AUDIO DataRangeAudio = NULL;

    while (m_ListDataRanges.GetNext(ListPosRange, &DataRangeAudio))
    {
        if (IsEqualGUID(KSDATAFORMAT_TYPE_WILDCARD, DataRangeAudio->DataRange.MajorFormat) ||
            IsEqualGUID(KSDATAFORMAT_TYPE_AUDIO, DataRangeAudio->DataRange.MajorFormat))
        {
            // Set the format to search for
            GUID SubFormat = {DEFINE_WAVEFORMATEX_GUID(WaveFormatEx->wFormatTag)};

            // If this is a WaveFormatExtensible structure, then use its defined SubFormat
            if (WAVE_FORMAT_EXTENSIBLE == WaveFormatEx->wFormatTag)
            {
                SubFormat = ((WAVEFORMATEXTENSIBLE *)WaveFormatEx)->SubFormat;
            }

			if (IsEqualGUID(KSDATAFORMAT_TYPE_WILDCARD, DataRangeAudio->DataRange.SubFormat) ||
				IsEqualGUID(SubFormat, DataRangeAudio->DataRange.SubFormat))
            {
				if (IsEqualGUID(KSDATAFORMAT_TYPE_WILDCARD, DataRangeAudio->DataRange.Specifier) ||
					IsEqualGUID(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX, DataRangeAudio->DataRange.Specifier))
                {
					if (MatchingCriteria == KS_FORMAT_MATCHING_CRITERIA_DEFAULT)
					{	
						if ((DataRangeAudio->MaximumChannels >= WaveFormatEx->nChannels) &&
							(DataRangeAudio->MinimumBitsPerSample <= WaveFormatEx->wBitsPerSample) &&
							(DataRangeAudio->MaximumBitsPerSample >= WaveFormatEx->wBitsPerSample) &&
							(DataRangeAudio->MinimumSampleFrequency <= WaveFormatEx->nSamplesPerSec) &&
							(DataRangeAudio->MaximumSampleFrequency >= WaveFormatEx->nSamplesPerSec))
						{
							// This should be a valid pin
							return TRUE;
						}
					}
					else 
					{
						BOOL Match = TRUE;

						if (MatchingCriteria & KS_FORMAT_MATCHING_CRITERIA_CHANNEL)
						{
							if (DataRangeAudio->MaximumChannels < WaveFormatEx->nChannels)
							{
								Match = FALSE;
							}
						}

						if (MatchingCriteria & KS_FORMAT_MATCHING_CRITERIA_BIT_DEPTH)
						{
							if ((DataRangeAudio->MinimumBitsPerSample > WaveFormatEx->wBitsPerSample) ||
								(DataRangeAudio->MaximumBitsPerSample < WaveFormatEx->wBitsPerSample))
							{
								Match = FALSE;
							}
						}

						if (MatchingCriteria & KS_FORMAT_MATCHING_CRITERIA_FREQUENCY)
						{
							if ((DataRangeAudio->MinimumSampleFrequency > WaveFormatEx->nSamplesPerSec) ||
							    (DataRangeAudio->MaximumSampleFrequency < WaveFormatEx->nSamplesPerSec))
							{
								Match = FALSE;
							}
						}

						if (Match)
						{
							return Match;
						}
					}
                }
            }
        }
    }

    return FALSE;
}

/*****************************************************************************
 * CKsAudioPin::ParseDataRanges()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CKsAudioPin::
ParseDataRanges
(
	IN		ULONG					Index,
	OUT		PKSDATARANGE_AUDIO *	OutDataRangeAudio
)
{
	ULONG i = 0;

    LISTPOS ListPosRange = m_ListDataRanges.GetHeadPosition();

    PKSDATARANGE_AUDIO DataRangeAudio = NULL;

    while (m_ListDataRanges.GetNext(ListPosRange, &DataRangeAudio))
    {
		if (i == Index)
		{
			*OutDataRangeAudio = DataRangeAudio;

			return TRUE;
		}

		i++;
	}

	return FALSE;
}

/*****************************************************************************
 * CKsAudioRenderPin::CKsAudioRenderPin()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor
 */
CKsAudioRenderPin::
CKsAudioRenderPin
(
	IN		CKsAudioFilter *	KsAudioFilter, 
	IN		ULONG				PinId, 
	OUT		HRESULT *			OutHResult
) 
:	CKsAudioPin(KsAudioFilter, PinId, OutHResult)
{
    m_PinType = KS_TECHNOLOGY_TYPE_AUDIO_RENDER;
}

/*****************************************************************************
 * CKsAudioRenderPin::~CKsAudioRenderPin()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor
 */
CKsAudioRenderPin::
~CKsAudioRenderPin
(	void
)
{
}

/*****************************************************************************
 * CKsAudioCapturePin::CKsAudioCapturePin()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor
 */
CKsAudioCapturePin::
CKsAudioCapturePin
(
	IN		CKsAudioFilter *	KsAudioFilter, 
	IN		ULONG				PinId, 
	OUT		HRESULT *			OutHResult
) 
:	CKsAudioPin(KsAudioFilter, PinId, OutHResult)
{
    m_PinType = KS_TECHNOLOGY_TYPE_AUDIO_CAPTURE;
}

/*****************************************************************************
 * CKsAudioCapturePin::~CKsAudioCapturePin()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor
 */
CKsAudioCapturePin::
~CKsAudioCapturePin
(	void
)
{
}

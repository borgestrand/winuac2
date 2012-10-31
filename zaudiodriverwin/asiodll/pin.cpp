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
 * @file       pin.cpp
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS pin objects.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "ksaudio.h"
#include "pin.h"

#define STR_MODULENAME "KSPIN: "


/*****************************************************************************
 * CKsPin::CKsPin()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsPin::
CKsPin 
(
    IN		CKsFilter * KsFilter, 
    IN		ULONG		PinId,
    OUT		HRESULT *	OutHResult
) 
:	CKsIrpTarget(INVALID_HANDLE_VALUE)
{
    m_KsFilter = KsFilter;

    m_PinId = PinId;

    m_Alignment = 0;

	m_LinkPin = NULL;
    
	m_CompleteOnlyOnRunState = TRUE;
    
	m_Looped = FALSE;
    
	m_PinType = KS_TECHNOLOGY_TYPE_UNKNOWN;
    
	m_KsState = KSSTATE_STOP;
    
	m_KsPinConnect = NULL;
    
	m_KsPinConnectSize = 0;
    
	m_Category= GUID_NULL;
 
    // non-atomic initializations
    ZeroMemory(&m_PinDescriptor, sizeof(m_PinDescriptor));
    
	ZeroMemory(m_FriendlyName, sizeof(m_FriendlyName));

    //Initialize
    HRESULT hr = CKsPin::Init();

    *OutHResult = hr;
}

/*****************************************************************************
 * CKsPin::CKsPin()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CKsPin::
~CKsPin
(	void
)
{
    ClosePin();

    // Need to cast to BYTE *, because that's how CKsAudioRenderPin::CKsAudioRenderPin
    // and CKsAudioRenderPin::SetFormat allocate the memory.
    delete[] (BYTE *)m_KsPinConnect;

    // Need to cast to BYTE *, because that's how CKsFilter::GetPinPropertyMulti 
	// allocates the memory.
    delete[] (BYTE *)m_PinDescriptor.MultipleDataRanges;
    delete[] (BYTE *)m_PinDescriptor.MultipleInterfaces;
    delete[] (BYTE *)m_PinDescriptor.MultipleMediums;
}

/*****************************************************************************
 * CKsPin::ClosePin()
 *****************************************************************************
 *//*!
 * @brief
 *  Closes the pin.
 */
VOID
CKsPin::
ClosePin
(	void
)
{
    if (IsValidHandle(m_Handle))
    {
        SetState(KSSTATE_STOP);

        SafeCloseHandle(m_Handle);
    }
}

/*****************************************************************************
 * CKsPin::SetState()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
SetState
(
    IN		KSSTATE	KsState
)
{
    HRESULT hr = SetPropertySimple
					(
						KSPROPSETID_Connection,
						KSPROPERTY_CONNECTION_STATE,
						&KsState,
						sizeof(KSSTATE)
					);

    if (SUCCEEDED(hr))
    {
        m_KsState = KsState;
    }

    if (FAILED(hr))
    {
		_DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::SetState] - Failed to Set Pin[%d] State: 0x%x", m_PinId, hr));
    }

    return hr;
}

/*****************************************************************************
 * CKsPin::Instantiate()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
Instantiate
(
    IN		BOOL	Looped
)
{
    HRESULT hr = S_OK;

    if (m_KsPinConnect)
	{
		if (m_LinkPin)
		{
			ASSERT((KSPIN_COMMUNICATION_SINK == m_LinkPin->m_PinDescriptor.Communication) ||
				   (KSPIN_COMMUNICATION_BOTH == m_LinkPin->m_PinDescriptor.Communication));

			m_KsPinConnect->PinToHandle = m_LinkPin->m_Handle;
		}

		m_Looped = Looped;

		m_KsPinConnect->Interface.Id = m_Looped ? KSINTERFACE_STANDARD_LOOPED_STREAMING : KSINTERFACE_STANDARD_STREAMING;

		DWORD w32Error = KsCreatePin(m_KsFilter->m_Handle, m_KsPinConnect, GENERIC_WRITE | GENERIC_READ, &m_Handle);

		if (ERROR_SUCCESS != w32Error)
		{
			 hr = HRESULT_FROM_WIN32(w32Error);

			 if (SUCCEEDED(hr))
			 {
				// Sometimes the error codes don't map to error HRESULTs.
				hr = E_FAIL;
			 }
		}

		if (FAILED(hr))
		{
			_DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::Instantiate] - Failed to instantiate pin. hr=0x%08x", hr));
		}
	}

    return hr;
}

/*****************************************************************************
 * CKsPin::GetState()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
GetState
(
    OUT		KSSTATE *	OutKsState
)
{
    HRESULT hr = S_OK;


    if (NULL == OutKsState)
    {
        hr = E_INVALIDARG;

		_DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::GetState] - OutKsState == NULL"));
    }
    else if (!IsValidHandle(m_Handle))
    {
        hr = E_FAIL;

		_DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::GetState] - No valid pin handle"));
    }

    if (SUCCEEDED(hr))
    {
        hr = GetPropertySimple
				(
					KSPROPSETID_Connection,
					KSPROPERTY_CONNECTION_STATE,
					OutKsState,
					sizeof(KSSTATE)
				);
    }

    return hr;
}

/*****************************************************************************
 * CKsPin::Reset()
 *****************************************************************************
 *//*!
 * @brief
 * Reset the pin.
 */
HRESULT 
CKsPin::
Reset
(	void
)
{
    ULONG ResetState = KSRESET_BEGIN;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_RESET_STATE,
						&ResetState,
						sizeof(ULONG),
						NULL,
						0,
						NULL
					);

    if (FAILED(hr))
    {
		_DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::Reset] - IOCTL_KS_RESET_STATE failed"));
    }

    ResetState = KSRESET_END;
 
    hr = SynchronizedIoctl
			(
				m_Handle,
				IOCTL_KS_RESET_STATE,
				&ResetState,
				sizeof(ULONG),
				NULL,
				0,
				NULL
			);


    if (FAILED(hr))
    {
		_DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::Reset] - IOCTL_KS_RESET_STATE failed"));
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////////////
//
//  CKsPin::WriteData()
//
//  Routine Description:
//      Submit some data to the pin, using the provided KSSTREAM_HEADER and OVERLAPPED structures.
//      If the caller submits a valid event in the KSSTREAM_HEADER, the event must be unsignaled.
//
//  Arguments:
//      Pointer to the KSSTREAM_HEADER structure describing the data to send to the pin
//      Pointer to the OVERLAPPED structure to use when doing the asynchronous I/O
//
//  Return Value:
//
//     S_OK on success
//


/*****************************************************************************
 * CKsPin::WriteData()
 *****************************************************************************
 *//*!
 * @brief
 * Submit some data to the pin, using the provided KSSTREAM_HEADER and 
 * OVERLAPPED structures. If the caller submits a valid event in the 
 * KSSTREAM_HEADER, the event must be unsignaled.
 * @param
 * KsStreamHeader Pointer to the KSSTREAM_HEADER structure describing the data
 * to send to the pin.
 * @param
 * Overlapped Pointer to the OVERLAPPED structure to use when doing the 
 * asynchronous I/O.
 * @return
 * Returns S_OK on success, otherwise appropriate error code.
 */
HRESULT 
CKsPin::
WriteData
(
    IN		PKSSTREAM_HEADER	KsStreamHeader,
    IN		LPOVERLAPPED		Overlapped
)
{
    HRESULT hr = S_OK;
    
	ULONG BytesReturned = 0;

	// submit the data
    BOOL Result = DeviceIoControl
					(
						m_Handle,
						IOCTL_KS_WRITE_STREAM,
						NULL,
						0,
						KsStreamHeader,
						KsStreamHeader->Size,
						&BytesReturned,
						Overlapped
					);

    // we're paused, we should return false!
    if (Result)
    {
		//_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::WriteData] - DeviceIoControl returned TRUE even though the pin is paused"));
    }
    else
    {
        // if it did return FALSE then GetLastError should return ERROR_IO_PENDING
        DWORD w32Error = GetLastError();

        if (ERROR_IO_PENDING == w32Error)
        {
            //Life is good
            hr = S_OK;
        }
        else
        {
 			_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::WriteData] - DeviceIoControl Failed!  Error=0x%#08x", w32Error));
	    
			hr = E_FAIL;
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsPin::ReadData()
 *****************************************************************************
 *//*!
 * @brief
 * Submit some memory for the pin to read into, using the provided 
 * KSSTREAM_HEADER and OVERLAPPED structures. If the caller submits a valid 
 * event in the KSSTREAM_HEADER, the event must be unsignaled.
 * @param
 * KsStreamHeader Pointer to the KSSTREAM_HEADER structure describing the data
 * to send to the pin.
 * @param
 * Overlapped Pointer to the OVERLAPPED structure to use when doing the 
 * asynchronous I/O.
 * @return
 * Returns S_OK on success, otherwise appropriate error code.
 */
HRESULT 
CKsPin::
ReadData
(
    IN		PKSSTREAM_HEADER	KsStreamHeader,
    IN		LPOVERLAPPED		Overlapped
)
{
    HRESULT hr = S_OK;
    
	ULONG BytesReturned = 0;

    BOOL Result = DeviceIoControl
					(
						m_Handle,
						IOCTL_KS_READ_STREAM,
						NULL,
						0,
						KsStreamHeader,
						KsStreamHeader->Size,
						&BytesReturned,
						Overlapped
					);

    // we're paused, we should return false!
    if (Result)
    {
		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::ReadData] - DeviceIoControl returned TRUE even though the pin is paused"));
    }
    else
    {
        // if it did return FALSE then GetLastError should return ERROR_IO_PENDING
        DWORD w32Error = GetLastError();

        if (ERROR_IO_PENDING == w32Error)
        {
            //Life is good
            hr = S_OK;
        }
        else
        {
 			_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::ReadData] - DeviceIoControl Failed!  Error=0x%#08x", w32Error));
	    
			hr = E_FAIL;
        }
    }

    return hr;
}

/*****************************************************************************
 * CKsPin::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Does some basic data structure initialization.
 */
HRESULT 
CKsPin::
Init
(	void
)
{
    BOOL Failure = FALSE;

    // get COMMUNICATION
    HRESULT hr = m_KsFilter->GetPinPropertySimple
					(
						m_PinId,
						KSPROPSETID_Pin,
						KSPROPERTY_PIN_COMMUNICATION,
						&m_PinDescriptor.Communication,
						sizeof(KSPIN_COMMUNICATION)
					);

    if (FAILED(hr))
    {
		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_COMMUNICATION on PinID: %d", m_PinId));

        Failure = TRUE;
    }

	// get PKSPIN_INTERFACES
    hr = m_KsFilter->GetPinPropertyMulti
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_INTERFACES,
				&m_PinDescriptor.MultipleInterfaces
			);

    if (SUCCEEDED(hr))
    {
        m_PinDescriptor.InterfaceCount = m_PinDescriptor.MultipleInterfaces->Count;

        m_PinDescriptor.Interfaces = (PKSPIN_INTERFACE)(m_PinDescriptor.MultipleInterfaces+1);
    }
    else
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPETY_PIN_INTERFACES on PinID: %d", m_PinId));

		Failure = TRUE;
    }

    // get PKSPIN_MEDIUMS
    hr = m_KsFilter->GetPinPropertyMulti
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_MEDIUMS,
				&m_PinDescriptor.MultipleMediums
			);

    if (SUCCEEDED(hr))
    {
        m_PinDescriptor.MediumCount = m_PinDescriptor.MultipleMediums->Count;

        m_PinDescriptor.Mediums = (PKSPIN_MEDIUM)(m_PinDescriptor.MultipleMediums +1);
    }
    else
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_MEDIUMS on PinID: %d", m_PinId));

		Failure = TRUE;
    }

    // get PKSPIN_DATARANGES
    hr = m_KsFilter->GetPinPropertyMulti
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_DATARANGES,
				&m_PinDescriptor.MultipleDataRanges
			);

    if (SUCCEEDED(hr))
    {
        m_PinDescriptor.DataRangeCount = m_PinDescriptor.MultipleDataRanges->Count;

        m_PinDescriptor.DataRanges = (PKSDATARANGE)(m_PinDescriptor.MultipleDataRanges +1);
    }
    else
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_DATARANGES on PinID: %d", m_PinId));

		Failure = TRUE;
   }

    //get dataflow information
    hr = m_KsFilter->GetPinPropertySimple
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_DATAFLOW,
				&m_PinDescriptor.DataFlow,
				sizeof(KSPIN_DATAFLOW)
			);

    if (FAILED(hr))
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_DATAFLOW on PinID: %d", m_PinId));

		Failure = TRUE;
    }

    //get instance information
    hr = m_KsFilter->GetPinPropertySimple
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_CINSTANCES,
				&m_PinDescriptor.Instances,
				sizeof(KSPIN_CINSTANCES)
			);

    if (FAILED(hr))
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_CINSTANCES on PinID: %d", m_PinId));

		Failure = TRUE;
    }

    // Get Global instance information
    hr = m_KsFilter->GetPinPropertySimple
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_GLOBALCINSTANCES,
				&m_PinDescriptor.InstancesGlobal,
				sizeof(KSPIN_CINSTANCES)
			);

    if (FAILED(hr))
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_GLOBALCINSTANCES on PinID: %d", m_PinId));

		Failure = TRUE;
    }

    // Get Pin Category Information
    hr = m_KsFilter->GetPinPropertySimple
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_CATEGORY,
				&m_Category,
				sizeof(GUID)
			);

    if (FAILED(hr))
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_CATEGORY on PinID: %d", m_PinId));

		Failure = TRUE;
    }

    // Get Pin Name
    hr = m_KsFilter->GetPinPropertySimple
			(
				m_PinId,
				KSPROPSETID_Pin,
				KSPROPERTY_PIN_NAME,
				&m_FriendlyName,
				sizeof(m_FriendlyName) - 2
			);

    if (SUCCEEDED(hr))
    {
 		_DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Init] - KSPROPERTY_PIN_NAME: %ws", m_FriendlyName));
    }
    else
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::Init] - Failed to retrieve pin property KSPROPERTY_PIN_NAME on PinID: %d", m_PinId));

		Failure = TRUE;
    }

	// if we experienced any failures, return S_FALSE, otherwise S_OK
    hr = Failure ? S_FALSE : S_OK;

    return hr;
}

/*****************************************************************************
 * CKsPin::GetPinId()
 *****************************************************************************
 *//*!
 * @brief
 * Returns the Pin ID.
 */
ULONG 
CKsPin::
GetPinId
(	void
)
{
    return m_PinId;
}

/*****************************************************************************
 * CKsPin::GetType()
 *****************************************************************************
 *//*!
 * @brief
 */
KS_TECHNOLOGY_TYPE 
CKsPin::
GetType
(	void
)
{
    return m_PinType;
}

/*****************************************************************************
 * CKsPin::SetPinConnect()
 *****************************************************************************
 *//*!
 * @brief
 * Copies the pin connect structure.
 */
HRESULT 
CKsPin::
SetPinConnect
(
	IN		PKSPIN_CONNECT	KsPinConnect, 
	IN		ULONG			KsPinConnectSize
)
{
    HRESULT hr = S_OK;

    if ((NULL == KsPinConnect) || (KsPinConnectSize < sizeof(KSPIN_CONNECT)))
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        // Free the existing Pin Connect structure
        if (m_KsPinConnect)
        {
            delete [] m_KsPinConnect;

            m_KsPinConnect = NULL;
        }

        // Allocate memory for the new pin connect structure
        m_KsPinConnect = PKSPIN_CONNECT(new BYTE[KsPinConnectSize]);

        if (NULL == m_KsPinConnect)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Copy over the KSPIN_CONNECT structure
        CopyMemory(m_KsPinConnect, KsPinConnect, KsPinConnectSize);
    }

    return hr;
}

/*****************************************************************************
 * CKsPin::GetPinDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
PPIN_DESCRIPTOR 
CKsPin::
GetPinDescriptor
(	void
)
{
    return &m_PinDescriptor;
}

/*****************************************************************************
 * CKsPin::SetFormat()
 *****************************************************************************
 *//*!
 * @brief
 * Sets the format on the pin.
 * @param
 * DataFormat The data format to use.
 */
HRESULT 
CKsPin::
SetFormat
(
	IN		PKSDATAFORMAT	DataFormat
)
{
    HRESULT hr = SetPropertySimple
					(
						KSPROPSETID_Connection,
						KSPROPERTY_CONNECTION_DATAFORMAT,
						DataFormat,
						DataFormat->FormatSize,
						NULL,
						0
					);

    return hr;
}

/*****************************************************************************
 * CKsPin::GetDataFlow()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
GetDataFlow
(
	OUT		KSPIN_DATAFLOW *	OutDataFlow
)
{
    ASSERT(NULL != OutDataFlow);

    *OutDataFlow = m_PinDescriptor.DataFlow;

    return S_OK;
}

/*****************************************************************************
 * CKsPin::GetCommunication()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
GetCommunication
(
	OUT		KSPIN_COMMUNICATION *	OutCommunication
)
{
    ASSERT(NULL != OutCommunication);

    *OutCommunication = m_PinDescriptor.Communication;

    return S_OK;
}

/*****************************************************************************
 * CKsPin::GetFriendlyName()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
GetFriendlyName
(
	OUT		PWCHAR *	OutFriendlyName
)
{
	*OutFriendlyName = m_FriendlyName;

	return S_OK;
}

/*****************************************************************************
 * CKsPin::GetLatencies()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsPin::
GetLatencies
(
	IN		KSTIME *	Latencies,
	IN OUT	ULONG *		NumberOfLatencies
)
{
    ASSERT(NULL != Latencies);
	ASSERT(NULL != NumberOfLatencies);

    // Get Pin latency Information
    KSP_PIN KspPin;

    KspPin.Property.Set	  = KSPROPSETID_Audio;
    KspPin.Property.Id    = KSPROPERTY_AUDIO_LATENCY;
    KspPin.Property.Flags = KSPROPERTY_TYPE_GET;
    KspPin.PinId          = m_PinId;
    KspPin.Reserved       = 0;

	ULONG BytesReturned = 0;

    HRESULT hr = SynchronizedIoctl
					(
						m_KsFilter->m_Handle,
						IOCTL_KS_PROPERTY,
						&KspPin,
						sizeof(KSP_PIN),
						Latencies,
						sizeof(KSTIME) * (*NumberOfLatencies),
						&BytesReturned
					);

	*NumberOfLatencies = BytesReturned / sizeof(KSTIME);

    if (FAILED(hr))
    {
 		_DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::GetLatencies] - Failed to retrieve pin property KSPROPERTY_AUDIO_LATENCY on PinID: %d", m_PinId));
    }

    return hr;
}

/*****************************************************************************
 * CKsPin::HasDataRangeWithSpecifier()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CKsPin::
HasDataRangeWithSpecifier
(
	IN		REFGUID	FormatSpecifier
)
{
    BOOL Found = FALSE;
    
	KSDATARANGE * DataRange = m_PinDescriptor.DataRanges;;

    // Loop Through the DataRanges
    for(ULONG i = 0; i < m_PinDescriptor.DataRangeCount; i++)
    {
        // Check for a matching Specifier
        if (IsEqualGUID(DataRange->Specifier, FormatSpecifier))
        {
            // Found our matching Pin!
            Found = TRUE;
            break;
        }

        // Advance to the next datarange.
        DataRange = PKSDATARANGE(PUCHAR(DataRange) + DataRange->FormatSize);
    }

    return Found;
}

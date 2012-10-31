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
 * @file       asiodrv.cpp
 * @brief      ASIO driver implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "asiodrv.h"
#include "asiocpl.h"

//#include "dbgbuffer.h"

#define STR_MODULENAME "ASIODriver: "

// Use the follow #define to implement 24-bit support in ASIO by padding it to 32-bit.
//#define USE_24_BIT_PADDED

/*****************************************************************************
 * CAsioDriver::m_PossibleSampleRates()
 *****************************************************************************
 *//*!
 * @brief
 * Possible sample rates supported by this driver.
 */
ULONG 
CAsioDriver::
m_PossibleSampleRates[6] = { 44100, 48000, 88200, 96000, 176400, 192000 };

/*****************************************************************************
 * CAsioDriver::m_SupportedBufferSizes()
 *****************************************************************************
 *//*!
 * @brief
 * Buffer sizes supported by this driver.
 */
DOUBLE 
CAsioDriver::
m_SupportedBufferSizes[] = { 0.1, 0.17, 0.25, 0.34, 0.5, 0.75, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 30, 40, 50, 60, 80, 100, 125, 150, 200, 300, 500 };

/*****************************************************************************
 * CAsioDriver::~CAsioDriver()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CAsioDriver::
~CAsioDriver
(   void
)
{
    _DbgPrintF(DEBUGLVL_TERSE,("[CAsioDriver::~CAsioDriver]"));

	timeEndPeriod(1);

	if (m_DriverState == DRIVER_STATE_RUNNING)
	{
		Stop();

		m_DriverState = DRIVER_STATE_PREPARED;
	}

	if (m_DriverState == DRIVER_STATE_PREPARED)
	{
		DisposeBuffers();

		m_DriverState = DRIVER_STATE_INITIALIZED;
	}

	if (m_DriverState == DRIVER_STATE_INITIALIZED)
	{
		// Abort the node event thread...
		if (m_NodeThreadHandle)
		{
			SetEvent(m_AsioNodeEvent[ASIO_NODE_EVENT_ABORT]);

			// Wait for the thread to exit...
			ULONG ExitCode;

			while (GetExitCodeThread(m_NodeThreadHandle, &ExitCode))
			{
				if (ExitCode != STILL_ACTIVE) 
				{
					break;
				}
			}

			CloseHandle(m_NodeThreadHandle);
		}

		// Disable the node events.
		if (m_XuClockRateNodeId != ULONG(-1))
		{
			m_AudioRenderFilter->DisableEvent(&m_AsioNodeEventData[ASIO_NODE_EVENT_CLOCK_RATE_CHANGE], sizeof(KSEVENTDATA));
		}

		if (m_XuClockSourceNodeId != ULONG(-1))
		{
			m_AudioRenderFilter->DisableEvent(&m_AsioNodeEventData[ASIO_NODE_EVENT_CLOCK_SOURCE_CHANGE], sizeof(KSEVENTDATA));
		}

		if (m_XuDriverResyncNodeId != ULONG(-1))
		{
			m_AudioRenderFilter->DisableEvent(&m_AsioNodeEventData[ASIO_NODE_EVENT_DRIVER_RESYNC], sizeof(KSEVENTDATA));
		}

		// Save the driver settings to registry...
		_SaveDriverSettingsToRegistry();	
	}

	//edit yuanfen
	//Close handle to avoid resource leak
	for (ULONG i=0; i<ASIO_CONTROL_EVENT_COUNT; i++)
	{
		if(m_AsioControlEvent[i])
			CloseHandle(m_AsioControlEvent[i]);
	}

	if(m_WatchDogEvent[0])
		CloseHandle(m_WatchDogEvent[0]);

	if(m_WatchDogEvent[1])
		CloseHandle(m_WatchDogEvent[1]);

	for (ULONG i=0; i<ASIO_NODE_EVENT_COUNT; i++)
	{
		if(m_AsioNodeEvent[i])
			CloseHandle(m_AsioNodeEvent[i]);
	}

	if(m_StateTransitionEvent)
		CloseHandle(m_StateTransitionEvent);
	//end edit

	for (ULONG i=0; i<SIZEOF_ARRAY(m_FilterDataRanges); i++)
	{
		for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
		{
			for (ULONG k=0; k<MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS; k++)
			{
				for (ULONG l=0; l<NUMBER_OF_DATA_PACKETS; l++)
				{
					if (m_FilterDataRanges[i][j].PinDescriptors[k].Packets[l].Signal.hEvent)
					{
						CloseHandle(m_FilterDataRanges[i][j].PinDescriptors[k].Packets[l].Signal.hEvent);
					}
				}
			}
		}
	}

	if (m_AudioRenderFilter)
	{
		delete m_AudioRenderFilter;
	}

	if (m_AudioCaptureFilter)
	{
		delete m_AudioCaptureFilter;
	}

	//spewdbgbuffer();
}

/*****************************************************************************
 * CAsioDriver::NonDelegatingQueryInterface()
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
 * Returns S_OK if the interface is found. Otherwise, returns E_NOINTERFACE.
 */
STDMETHODIMP_(HRESULT)
CAsioDriver::
NonDelegatingQueryInterface
(
    IN		REFIID	Interface,
    OUT		PVOID * Object
)
{
    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::NonDelegatingQueryInterface]"));

	if (IsEqualGUID(Interface, IID_IUnknown))
    {
	    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::NonDelegatingQueryInterface] - IID_IUnknown"));

		*Object = PVOID(PUNKNOWN(this));
    }
    else
    if (IsEqualGUID(Interface, IID_IAsioDriver))
    {
	    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::NonDelegatingQueryInterface] - IID_IAsioDriver"));

        *Object = PVOID(PASIODRIVER(this));
    }
    else
    if (IsEqualGUID(Interface, m_ClassId))
    {
		_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::NonDelegatingQueryInterface] - CLSID_ASIO_DRIVER : %p", this));

		*Object = PVOID(PASIODRIVER(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

/*****************************************************************************
 * CAsioDriver::Setup()
 *****************************************************************************
 *//*!
 * @brief
 * Setup the ASIO driver.
 */
HRESULT
CAsioDriver::
Setup
(
    IN		REFCLSID	RefClsId
)
{
	//resetdbgbuffer();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Setup]"));

	#if DBG
	DWORD_PTR ProcessAffinityMask = 0;
	DWORD_PTR SystemAffinityMask = 0;

	GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Setup] - ProcessAffinityMask: 0x%x", ProcessAffinityMask));
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Setup] - SystemAffinityMask: 0x%x", SystemAffinityMask));

	DWORD PriorityClass = GetPriorityClass(GetCurrentProcess());

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Setup] - PriorityClass: 0x%x", PriorityClass));
	#endif // DBG
	
	m_DriverState = DRIVER_STATE_LOADED;

	m_ClassId = RefClsId;

	WCHAR ClsIdStr[MAX_PATH];
    StringFromGUID2(m_ClassId, ClsIdStr, MAX_PATH);

    WCHAR KeyName[MAX_PATH];
    wcscpy(KeyName, L"CLSID\\"); wcscat(KeyName, ClsIdStr); 

    HKEY ClassKey = NULL;

	DWORD w32Error = RegOpenKeyExW(HKEY_CLASSES_ROOT, KeyName, 0, KEY_READ, &ClassKey);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Setup] - RegOpenKeyExW: w32Error - %d", w32Error));

    if (ERROR_SUCCESS == w32Error)
    {
		ULONG Type = 0, Size = sizeof(m_SymbolicLink);

		w32Error = RegQueryValueEx(ClassKey, "SymbolicLink", 0, &Type, PBYTE(m_SymbolicLink), &Size);

		if (ERROR_SUCCESS == w32Error)
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Setup] - SymbolicLink: %s", m_SymbolicLink));

			Type = 0; Size = sizeof(m_FriendlyName);

			RegQueryValueEx(ClassKey, "FriendlyName", 0, &Type, PBYTE(m_FriendlyName), &Size);
		}

        RegCloseKey(ClassKey);
    }

	timeBeginPeriod(1);

	HRESULT hr = (ERROR_SUCCESS == w32Error) ? S_OK : E_FAIL;

	return hr;
}

/*****************************************************************************
 * CAsioDriver::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the ASIO driver.
 */
ASIOMETHODIMP_(ASIOBool)
CAsioDriver::
Init
(
    IN		PVOID	WndMain
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Init]"));

	m_WndMain = HWND(WndMain);
	
	HRESULT hr = E_FAIL;
 		
	// Now instantiate the render & capture using the symbolic link above.
	m_AudioRenderFilter = new CKsAudioRenderFilter(m_SymbolicLink, m_FriendlyName, &hr);
	
	if (SUCCEEDED(hr))
	{
		hr = m_AudioRenderFilter->Instantiate();
	}

	if (SUCCEEDED(hr))
	{
		hr = m_AudioRenderFilter->EnumerateNodes();
	}

	if (SUCCEEDED(hr))
	{
		hr = m_AudioRenderFilter->EnumeratePins();
	}

	if (SUCCEEDED(hr))
	{
		m_AudioCaptureFilter = new CKsAudioCaptureFilter(m_SymbolicLink, m_FriendlyName, &hr);

		if (SUCCEEDED(hr))
		{
			hr = m_AudioCaptureFilter->Instantiate();
		}

		if (SUCCEEDED(hr))
		{
			hr = m_AudioCaptureFilter->EnumerateNodes();
		}

		if (SUCCEEDED(hr))
		{
			hr = m_AudioCaptureFilter->EnumeratePins();
		}
	}

	if (SUCCEEDED(hr))
	{
		USHORT vid = 0, pid = 0;

		KSCOMPONENTID KsComponentId;

		if (SUCCEEDED(m_AudioRenderFilter->GetPropertySimple(KSPROPSETID_General, KSPROPERTY_GENERAL_COMPONENTID, &KsComponentId, sizeof(KSCOMPONENTID))))
		{
			if (IS_COMPATIBLE_USBAUDIO_MID(&KsComponentId.Manufacturer) && IS_COMPATIBLE_USBAUDIO_PID(&KsComponentId.Product))
			{
				vid = EXTRACT_USBAUDIO_MID(&KsComponentId.Manufacturer);

				pid = EXTRACT_USBAUDIO_PID(&KsComponentId.Product);

				sprintf(m_ProductIdentifier, "USB\\VID_%04X&PID_%04X", vid, pid);
			}
			else
			{
				//FIXME: Figure out other ways to extract the VID & PID.
			}
		}
		else
		{
			//FIXME: Figure out other ways to extract the VID & PID.
		}

		INIT_XU_PROPSETID(&m_XuPropSetClockRate, vid, pid, XU_CODE_CLOCK_RATE);
		INIT_XU_PROPSETID(&m_XuPropSetClockSource, vid, pid, XU_CODE_CLOCK_SOURCE);
		INIT_XU_PROPSETID(&m_XuPropSetDirectMonitor, vid, pid, XU_CODE_DIRECT_MONITOR);
		INIT_XU_PROPSETID(&m_XuPropSetDriverResync, vid, pid, XU_CODE_DRIVER_RESYNC);

		m_XuClockRateNodeId = _FindXuNode(m_XuPropSetClockRate);
		m_XuClockSourceNodeId = _FindXuNode(m_XuPropSetClockSource);
		m_XuDirectMonitorNodeId = _FindXuNode(m_XuPropSetDirectMonitor);
		m_XuDriverResyncNodeId = _FindXuNode(m_XuPropSetDriverResync);
	}

	if (SUCCEEDED(hr))
	{
		// Initialize the control events...
		for (ULONG i=0; i<ASIO_CONTROL_EVENT_COUNT; i++)
		{
			m_AsioControlEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		// Initialize the watchdog events...
		m_WatchDogEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_WatchDogEvent[1] = CreateWaitableTimer(NULL, FALSE, NULL);

		// Initialize the node events...
		for (ULONG i=0; i<ASIO_NODE_EVENT_COUNT; i++)
		{
			m_AsioNodeEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
			m_AsioNodeEventData[i].NotificationType = KSEVENTF_EVENT_HANDLE;
			m_AsioNodeEventData[i].EventHandle.Event = m_AsioNodeEvent[i];
		}

		// Initialize the pin events...
		for (ULONG i=0; i<SIZEOF_ARRAY(m_FilterDataRanges); i++)
		{
			for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
			{
				for (ULONG k=0; k<MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS; k++)
				{
					for (ULONG l=0; l<NUMBER_OF_DATA_PACKETS; l++)
					{
						m_FilterDataRanges[i][j].PinDescriptors[k].Packets[l].Signal.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
					}
				}
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Minimum supported buffer size.
		CHAR Section[64]; sprintf(Section, "%s.Audio.BufferSize", m_ProductIdentifier);

		CHAR SystemWindowsDirectory[MAX_PATH]; GetSystemWindowsDirectory(SystemWindowsDirectory, MAX_PATH);

		CHAR PathFileName[MAX_PATH]; sprintf(PathFileName, "%s\\emasio.dat", SystemWindowsDirectory);

		m_MinimumBufferSizeArrayOffset = GetPrivateProfileInt(Section, "ArrayOffset", 0, PathFileName);
	}

	if (SUCCEEDED(hr))
	{
		// Read back the settings of the ASIO driver from registry (if available).
		_RestoreDriverSettingsFromRegistry();
	}

	if (SUCCEEDED(hr))
	{
		// The bit depth is stuffed in the CLSID.
		UCHAR BitDepthMask = UCHAR(m_ClassId.Data1 & 0xF);

		// Check if the settings read from registry is valid.
		if (m_PreferredSampleSize)
		{
			BOOL FoundPreferredSampleSize = FALSE;

			USHORT BitDepth[] = { 8, 16, 24, 32 };

			for (ULONG i=1; i<SIZEOF_ARRAY(BitDepth); i++)
			{
				if (BitDepthMask & (1<<i))
				{
					if (m_PreferredSampleSize == BitDepth[i])
					{
						FoundPreferredSampleSize = TRUE;
						break;
					}
				}
			}

			if (!FoundPreferredSampleSize)
			{
				// Reset to default settings.
				m_PreferredSampleSize = m_SampleSize = 0;
			}
		}

		// Default settings should be put here.
		if ((m_PreferredSampleSize == 0) || (m_SampleSize == 0))
		{
			USHORT SampleSize[] = { 0, 8, 16, 16, 24, 24, 24, 24, 32, 32, 32, 32, 32, 32, 32, 32 };

			// Default to highest bit depth available.
			m_PreferredSampleSize = m_SampleSize = SampleSize[BitDepthMask];
		}

		// Construct the ASIO filter data ranges...
		for (ULONG i=0; i<SIZEOF_ARRAY(m_FilterDataRanges); i++)
		{
			_BuildFilterDataRanges(i, m_FilterDataRanges[i]);
		}

		// Default buffer size.
		if (m_PreferredBufferSize == 0)
		{
			m_PreferredBufferSize = 100000; // Default to 100000 * 100ns, ie 10ms
		}

		m_NumberOfOutputBuffers = _GetNumberOfOutputBuffers(m_PreferredBufferSize);

		// Check if the settings read from registry is valid.
		if (m_SamplingFrequency)
		{
			BOOL FoundPreferredSampleRate = FALSE;

			for (ULONG i=0; i<SIZEOF_ARRAY(m_PossibleSampleRates); i++)
			{
				for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
				{
					if ((m_FilterDataRanges[0][i].SampleRate == m_FilterDataRanges[1][j].SampleRate) &&
						(m_FilterDataRanges[0][i].Supported && m_FilterDataRanges[1][j].Supported))
					{
						if (m_SamplingFrequency == m_FilterDataRanges[1][j].SampleRate)
						{
							FoundPreferredSampleRate = TRUE;
							break;
						}
					}
				}

				if (FoundPreferredSampleRate) break;
			}

			if (!FoundPreferredSampleRate)
			{
				// Reset to default settings.
				m_SamplingFrequency = 0;
			}
		}

		// Default to the following if it is not available.
		if (m_SamplingFrequency == 0)
		{
			for (ULONG i=0; i<SIZEOF_ARRAY(m_PossibleSampleRates); i++)
			{
				BOOL FoundUsableSampleRate = FALSE;

				for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
				{
					if ((m_FilterDataRanges[0][i].SampleRate == m_FilterDataRanges[1][j].SampleRate) &&
						(m_FilterDataRanges[0][i].Supported && m_FilterDataRanges[1][j].Supported))
					{
						m_SamplingFrequency = m_FilterDataRanges[1][j].SampleRate;
						
						FoundUsableSampleRate = TRUE;
						break;
					}
				}

				if (FoundUsableSampleRate) break;
			}
		}

		// Initialize the default selection of data ranges.
		for (ULONG i=0; i<SIZEOF_ARRAY(m_FilterDataRanges); i++)
		{
			for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
			{			
				if (m_FilterDataRanges[i][j].SampleRate == m_SamplingFrequency)
				{
					m_SelectedDataRangeAsio[i] = &m_FilterDataRanges[i][j];
					break;
				}
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Set the clock rate (if available).
		_SetClockRate(m_SamplingFrequency);
	}

	if (SUCCEEDED(hr))
	{
		// Figure out what the driver can do...
		_DetermineDriverCapabilities();
	}

	if (SUCCEEDED(hr))
	{
		// Enable the node events.
		if (m_XuClockRateNodeId != ULONG(-1))
		{
			m_AudioRenderFilter->EnableNodeEvent(m_XuClockRateNodeId, KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, &m_AsioNodeEventData[ASIO_NODE_EVENT_CLOCK_RATE_CHANGE], sizeof(KSEVENTDATA));
		}

		if (m_XuClockSourceNodeId != ULONG(-1))
		{
			m_AudioRenderFilter->EnableNodeEvent(m_XuClockSourceNodeId, KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, &m_AsioNodeEventData[ASIO_NODE_EVENT_CLOCK_SOURCE_CHANGE], sizeof(KSEVENTDATA));
		}

		if (m_XuDriverResyncNodeId != ULONG(-1))
		{
			m_AudioRenderFilter->EnableNodeEvent(m_XuDriverResyncNodeId, KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, &m_AsioNodeEventData[ASIO_NODE_EVENT_DRIVER_RESYNC], sizeof(KSEVENTDATA));
		}

		m_NodeThreadHandle = CreateThread(NULL, 0, NodeThreadRoutine, this, 0, NULL);
	}

	if (SUCCEEDED(hr))
	{
		_GetAppHacks();

		m_StateTransitionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_DriverState = DRIVER_STATE_INITIALIZED;
	}

	ASIOBool Success = SUCCEEDED(hr) ? ASIOTrue : ASIOFalse;

	return Success;
}

/*****************************************************************************
 * CAsioDriver::GetDriverName()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOMETHODIMP_(VOID) 
CAsioDriver::
GetDriverName
(	
	OUT		CHAR *	OutDriverName
)
{
	strcpy(OutDriverName, m_FriendlyName);
}

/*****************************************************************************
 * CAsioDriver::GetDriverVersion()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOMETHODIMP_(LONG) 
CAsioDriver::
GetDriverVersion
(	void
)
{
	return 2;
}

/*****************************************************************************
 * CAsioDriver::GetErrorMessage()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOMETHODIMP_(VOID) 
CAsioDriver::
GetErrorMessage
(	
	OUT		CHAR *	OutString
)
{
	// oh... whatever.
}

/*****************************************************************************
 * CAsioDriver::Start()
 *****************************************************************************
 *//*!
 * @brief
 * Start input and output processing synchronously.
 * @details
 * This will
 * - reset the sample counter to zero
 * - start the hardware (both input and output)
 * The first call to the hosts' bufferSwitch(index == 0) then tells the host 
 * to read from input buffer A (index 0), and start processing to output 
 * buffer A while output buffer B (which has been filled by the host prior to 
 * calling ASIOStart()) is possibly sounding (see also ASIOGetLatencies()) 
 * Notes:
 * There is no restriction on the time that ASIOStart() takes
 * to perform (that is, it is not considered a realtime trigger).
 * @param
 * <None>.
 * @return
 * If neither input nor output is present, ASE_NotPresent will be returned.
 * If the hardware fails to start, ASE_HWMalfunction will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
Start
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Start]"));

	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (m_DriverState == DRIVER_STATE_PREPARED)
		{
			m_DriverState = DRIVER_STATE_RUNNING;

			SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_START]);

			WaitForSingleObject(m_StateTransitionEvent, INFINITE);
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::Stop()
 *****************************************************************************
 *//*!
 * @brief
 * Stops input and output processing altogether.
 * @details
 * Notes:
 * On return from ASIOStop(), the driver must in no case call the hosts' 
 * bufferSwitch() routine.
 * @param
 * <None>.
 * @return
 * If neither input nor output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
Stop
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::Stop]"));

	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (m_DriverState == DRIVER_STATE_RUNNING)
		{
			SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_STOP]);

			WaitForSingleObject(m_StateTransitionEvent, INFINITE);
		}
	}

	m_DriverState = DRIVER_STATE_PREPARED;

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::GetChannels()
 *****************************************************************************
 *//*!
 * @brief
 * Returns number of individual input/output channels.
 * @param
 * OutNumInputChannels Location to hold the number of available input channels.
 * @param
 * OutNumOutputChannels Location to hold the number of available output channels.
 * @return
 * If no input/output is present ASE_NotPresent will be returned.
 * If only inputs, or only outputs are available, the according other parameter 
 * will be zero, and ASE_OK is returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetChannels
(	
	OUT		LONG *	OutNumInputChannels,
	OUT		LONG *	OutNumOutputChannels
)
{
	LONG NumInputChannels = 0;

	if (m_SelectedDataRangeAsio[1])
	{
		NumInputChannels = m_SelectedDataRangeAsio[1]->MaximumChannels;
	}
	
	LONG NumOutputChannels = 0;

	if (m_SelectedDataRangeAsio[0])
	{
		NumOutputChannels = m_SelectedDataRangeAsio[0]->MaximumChannels;
	}

	if (OutNumInputChannels)
	{
		*OutNumInputChannels = NumInputChannels;
	}
	
	if (OutNumOutputChannels)
	{
		*OutNumOutputChannels = NumOutputChannels;
	}

	ASIOError asioError = ASE_OK;

	if ((NumInputChannels == 0) && (NumOutputChannels == 0))
	{
		asioError = ASE_NotPresent;
	}

	return asioError;
}

/*****************************************************************************
 * TIME_100NS_SAMPLES()
 *****************************************************************************
 */
LONG
TIME_100NS_SAMPLES
(
	IN		ULONGLONG	TimeIn100ns,
	IN		ULONG		SamplingFrequency
)
{
	LONG Samples;

	if (SamplingFrequency == 44100)
	{
		// Treat 44100 differently as Cubase LE has issue with this sample rate and the buffer sizes
		// not equal to multiples of 44 samples.
		Samples = LONG(TimeIn100ns * (SamplingFrequency / 1000) / 10000);
	}
	else
	{
		Samples = LONG(TimeIn100ns * SamplingFrequency / 10000000);
	}

	return Samples;
}

/*****************************************************************************
 * CAsioDriver::GetLatencies()
 *****************************************************************************
 *//*!
 * @brief
 * Returns the input and output latencies. This includes device specific 
 * delays, like FIFOs etc.
 * @param
 * OutInputLatency Location to hold the 'age' of the first sample frame in 
 * the input buffer when the hosts reads it in bufferSwitch() (this is 
 * theoretical, meaning it does not include the overhead and delay between 
 * the actual physical switch, and the time	when bufferSitch() enters).	This 
 * will usually be the size of one block in sample frames, plus	device 
 * specific latencies.
 * @param
 * OutOutputLatency Location to specify the time between the buffer switch,
 * and the time when the next play buffer will start to sound. The next play 
 * buffer is defined as the one the host starts	processing after (or at) 
 * bufferSwitch(), indicated by the	index parameter (0 for buffer A, 1 for 
 * buffer B). It will usually be either one block, if the host writes directly
 * to a dma buffer, or two or more blocks if the buffer is 'latched' by	the 
 * driver. As an example, on ASIOStart(), the host will have filled	the play 
 * buffer at index 1 already; when it gets the callback (with the parameter 
 * index == 0), this tells it to read from the input buffer 0, and start to 
 * fill the play buffer 0 (assuming that now play buffer 1 is already sounding). 
 * In this case, the output	latency is one block. If the driver decides to 
 * copy buffer 1 at that time, and pass it to the hardware at the next slot 
 * (which is most commonly done, but should be avoided), the output latency
 * becomes two blocks instead, resulting in a total i/o latency of at least	
 * 3 blocks. As memory access is the main bottleneck in native dsp processing,
 * and to acheive less latency, it is highly recommended to try to avoid 
 * copying (this is also why the driver is the owner of the buffers). To
 * summarize, the minimum i/o latency can be acheived if the input buffer is 
 * processed by the host into the output buffer which will physically start 
 * to sound on the next time slice. Also note that the host expects	the 
 * bufferSwitch() callback to be accessed for each time slice in order to retain 
 * sync, possibly recursively; if it fails to process a block in time, it 
 * will suspend its operation for some time in order to recover.
 * @return
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetLatencies
(
	OUT		LONG *	OutInputLatency,
	OUT		LONG *	OutOutputLatency
)
{
	LONG InputLatency = 0;

	LONG OutputLatency = 0;

	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (m_DriverState == DRIVER_STATE_PREPARED)
		{
			// Include the audio buffer size of the ASIOCreateBuffers() call.
			InputLatency = LONG(m_BufferSizeInSamples + TIME_100NS_SAMPLES(m_SelectedDataRangeAsio[1]->Latency.Time, m_SamplingFrequency));
								
			OutputLatency = LONG((m_BufferSizeInSamples * (m_NumberOfOutputBuffers - 1)) + TIME_100NS_SAMPLES(m_SelectedDataRangeAsio[0]->Latency.Time, m_SamplingFrequency));
		}
		else
		{
			// In case the call occurs before ASIOCreateBuffers(), the driver 
			// should assume preferred buffer size.
			InputLatency = LONG(TIME_100NS_SAMPLES(m_PreferredBufferSize, m_SamplingFrequency) + TIME_100NS_SAMPLES(m_SelectedDataRangeAsio[1]->Latency.Time, m_SamplingFrequency));
								
			OutputLatency = LONG((TIME_100NS_SAMPLES(m_PreferredBufferSize, m_SamplingFrequency) * (m_NumberOfOutputBuffers - 1)) + TIME_100NS_SAMPLES(m_SelectedDataRangeAsio[0]->Latency.Time, m_SamplingFrequency));
		}
	}

	if (OutInputLatency)
	{
		*OutInputLatency = InputLatency;
	}
	
	if (OutOutputLatency)
	{
		*OutOutputLatency = OutputLatency;
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::GetBufferSize()
 *****************************************************************************
 *//*!
 * @brief
 * Returns min, max, and preferred buffer sizes for input/output.
 * @details
 * Notes:
 * When minimum and maximum buffer size are equal, the preferred buffer size 
 * has to be the same value as well; granularity  should be 0 in this case.
 * @param
 * OutMinSize Will hold the minimum buffer size.
 * @param
 * OutMaxSize Will hold the maximum possible buffer size
 * @param
 * OutPreferredSize Will hold the preferred buffer size (a size which best 
 * fits performance and hardware requirements)
 * @param
 * OutGranularity Will hold the granularity at which buffer sizes may differ. 
 * Usually, the buffer size will be a power of 2; in this case, granularity 
 * will hold -1 on return, signalling possible buffer sizes starting from 
 * MinSize, increased in powers of 2 up to MaxSize.
 * @return
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetBufferSize
(
	OUT		LONG *	OutMinSize,
	OUT		LONG *	OutMaxSize,
	OUT		LONG *	OutPreferredSize,
	OUT		LONG *	OutGranularity
)
{
	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (OutMinSize)
		{
			if (m_SamplingFrequency == 44100)
			{
				// Treat 44100 differently as Cubase LE has issue with this sample rate and the buffer sizes
				// not equal to multiples of 44 samples.
				*OutMinSize = LONG(m_SupportedBufferSizes[m_MinimumBufferSizeArrayOffset] * (m_SamplingFrequency / 1000));
			}
			else
			{
				*OutMinSize = LONG(m_SupportedBufferSizes[m_MinimumBufferSizeArrayOffset] * m_SamplingFrequency / 1000);
			}
		}

		if (OutMaxSize)
		{
			if (m_SamplingFrequency == 44100)
			{
				// Treat 44100 differently as Cubase LE has issue with this sample rate and the buffer sizes
				// not equal to multiples of 44 samples.
				*OutMaxSize = LONG(m_SupportedBufferSizes[SIZEOF_ARRAY(m_SupportedBufferSizes)-1] * (m_SamplingFrequency / 1000)); 
			}
			else
			{
				*OutMaxSize = LONG(m_SupportedBufferSizes[SIZEOF_ARRAY(m_SupportedBufferSizes)-1] * m_SamplingFrequency / 1000); 
			}
		}

		if (OutPreferredSize)
		{
			if (m_SamplingFrequency == 44100)
			{
				// Treat 44100 differently as Cubase LE has issue with this sample rate and the buffer sizes
				// not equal to multiples of 44 samples.
				*OutPreferredSize = LONG(m_PreferredBufferSize * (m_SamplingFrequency / 1000) / 10000); 
			}
			else
			{
				*OutPreferredSize = LONG(m_PreferredBufferSize * m_SamplingFrequency / 10000000);
			}
		}

		if (OutGranularity) 
		{
			*OutGranularity = m_SamplingFrequency / 1000; // 1ms
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::CanSampleRate()
 *****************************************************************************
 *//*!
 * @brief
 * Inquires the hardware for the available sample rates.
 * @param
 * SampleRate The rate in question.
 * @return
 * If the inquired sample rate is not supported, ASE_NoClock will be returned.
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
CanSampleRate
(
	IN		ASIOSampleRate	SampleRate
)
{
	LONG NumInputChannels = 0, NumOutputChannels = 0;

	BOOL CaptureCapable = FALSE;

	for (ULONG i=0; i<SIZEOF_ARRAY(m_PossibleSampleRates); i++)
	{
		if (m_FilterDataRanges[1][i].SampleRate == ULONG(SampleRate))
		{
			CaptureCapable = m_FilterDataRanges[1][i].Supported;

			if (CaptureCapable)
			{
				NumInputChannels = m_FilterDataRanges[1][i].MaximumChannels;
			}
			break;
		}
	}

	BOOL RenderCapable = FALSE;

	for (ULONG i=0; i<SIZEOF_ARRAY(m_PossibleSampleRates); i++)
	{
		if (m_FilterDataRanges[0][i].SampleRate == ULONG(SampleRate))
		{
			RenderCapable = m_FilterDataRanges[0][i].Supported;

			if (RenderCapable)
			{
				NumOutputChannels = m_FilterDataRanges[0][i].MaximumChannels;
			}
			break;
		}
	}

	ASIOError asioError = ASE_NoClock;

	if (NumInputChannels && NumOutputChannels)
	{
		// Both input and output channels present, so both must be able
		// to support the sample rate.
		if (CaptureCapable && RenderCapable)
		{
			asioError = ASE_OK;
		}
	}
	else if (NumInputChannels)
	{
		// Only input channels present, so it is capable of capture at
		// the specified sample rate, it will do.
		//if (CaptureCapable)
		//{
		//	asioError = ASE_OK;
		//}
	}
	else if (NumOutputChannels)
	{
		// Only output channels present, so it is capable of render at
		// the specified sample rate, it will do.
		//if (RenderCapable)
		//{
		//	asioError = ASE_OK;
		//}
	}/**/
	else
	{
		// No input or output channels. 
		asioError = ASE_NotPresent;
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::GetSampleRate()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current sample Rate.
 * @param
 * OutSampleRate Will hold the current sample rate on return.
 * @return
 * If sample rate is unknown, OutSampleRate will be 0 and ASE_NoClock will be 
 * returned.
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetSampleRate
(	
	OUT		ASIOSampleRate *	OutSampleRate
)
{
	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (OutSampleRate)
		{
			if (m_SamplingFrequency)
			{
				*OutSampleRate = ASIOSampleRate(m_SamplingFrequency);
			}
			else
			{
				*OutSampleRate = ASIOSampleRate(0);

				asioError = ASE_NoClock;
			}
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::SetSampleRate()
 *****************************************************************************
 *//*!
 * @brief
 * Set the hardware to the requested sample Rate. If SampleRate == 0, enable 
 * external sync.
 * @param
 * SampleRate On input, the requested rate
 * @return
 * If SampleRate is unknown ASE_NoClock will be returned.  
 * If the current clock is external, and SampleRate is != 0, ASE_InvalidMode 
 * will be returned.
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
SetSampleRate
(
	IN		ASIOSampleRate	SampleRate
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::SetSampleRate] - %d", ULONG(SampleRate)));

	ASIOError asioError = CanSampleRate(SampleRate);

	if (ASE_OK == asioError)
	{
		if (m_SamplingFrequency != ULONG(SampleRate))
		{
			m_SamplingFrequency = ULONG(SampleRate);

			for (ULONG i=0; i<SIZEOF_ARRAY(m_FilterDataRanges); i++)
			{
				for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
				{
					if (m_FilterDataRanges[i][j].SampleRate == m_SamplingFrequency)
					{
						m_SelectedDataRangeAsio[i] = &m_FilterDataRanges[i][j];
						break;
					}
				}
			}

			_SetClockRate(m_SamplingFrequency);

			if (m_DriverState != DRIVER_STATE_INITIALIZED)
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::SetSampleRate] - Bad bad driver, current driver state 0x%x", m_DriverState));

				// Send a reset request because we need to shut down and restart the
				// buffers.
				SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_RESET]);
			}
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::GetClockSources()
 *****************************************************************************
 *//*!
 * @brief
 * Get the available external audio clock sources.
 * @param
 * OutClocks Points to an array of ASIOClockSource structures:
 * - index: This is used to identify the clock source when ASIOSetClockSource() 
 * is accessed, should be an index counting from zero.
 * - associatedInputChannel: The first channel of an associated input group, 
 * if any.
 * - associatedGroup: The group index of that channel. Groups of channels are 
 * defined to seperate for instance analog, S/PDIF, AES/EBU, ADAT connectors etc,
 * when present simultaniously. Note that associated channel is enumerated 
 * according to numInputs/numOutputs, means it is independant from a group 
 * (see also ASIOGetChannelInfo()) inputs are associated to a clock if the 
 * physical connection transfers both data and clock (like S/PDIF, AES/EBU, or
 * ADAT inputs). if there is no input channel associated with the clock source 
 * (like Word Clock, or internal oscillator), both associatedChannel and 
 * associatedGroup should be set to -1.
 * - isCurrentSource: on exit, ASIOTrue if this is the current clock source, 
 * ASIOFalse else
 * - name: a null-terminated string for user selection of the available sources.
 * @param
 * OutNumSources:
 * on input: the number of allocated array members
 * on output: the number of available clock sources, at least 1 (internal 
 * clock generator).
 * @return
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetClockSources
(	
	OUT		ASIOClockSource *	OutClocks,
	OUT		LONG *				OutNumSources
)
{
	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		ULONG ClockSource = 0;

		if (SUCCEEDED(_GetClockSource(&ClockSource)))
		{
			if (*OutNumSources > 0)
			{
				OutClocks[0].index = 0;
				OutClocks[0].associatedChannel = -1;
				OutClocks[0].associatedGroup = -1;
				OutClocks[0].isCurrentSource = (ClockSource == 0) ? ASIOTrue : ASIOFalse;
				strcpy(OutClocks[0].name, "Internal");
			}

			if (*OutNumSources > 1)
			{
				OutClocks[1].index = 1;
				OutClocks[1].associatedChannel = -1;
				OutClocks[1].associatedGroup = -1;
				OutClocks[1].isCurrentSource = (ClockSource == 1) ? ASIOTrue : ASIOFalse;
				strcpy(OutClocks[1].name, "SPDIF External");
			}

			*OutNumSources = 2;
		}
		else
		{
			if (*OutNumSources > 0)
			{
				OutClocks[0].index = 0;
				OutClocks[0].associatedChannel = -1;
				OutClocks[0].associatedGroup = -1;
				OutClocks[0].isCurrentSource = ASIOTrue;
				strcpy(OutClocks[0].name, "Internal");
			}

			*OutNumSources = 1;
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::SetClockSource()
 *****************************************************************************
 *//*!
 * @brief
 * Set the audio clock source.
 * @details
 * Notes:
 * Should *not* return ASE_NoClock if there is no clock signal present at the 
 * selected source; this will be inquired via ASIOGetSampleRate().
 * It should call the host callback procedure sampleRateHasChanged(), if the 
 * switch causes a sample rate change, or if no external clock is present at 
 * the selected source.
 * @param
 * Reference Index as obtained from an inquiry to ASIOGetClockSources()
 * @return
 * If no input/output is present ASE_NotPresent will be returned.
 * If the clock can not be selected because an input channel which carries the 
 * current clock source is active, ASE_InvalidMode *may* be returned (this 
 * depends on the properties of the driver and/or hardware).
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
SetClockSource
(	
	IN		LONG	Reference
)
{
	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (Reference > 1)
		{
			asioError = ASE_InvalidMode;
		}
		else
		{
			_SetClockSource(Reference);
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::GetSamplePosition()
 *****************************************************************************
 *//*!
 * @brief
 * Inquires the sample position/time stamp pair.
 * @details
 * Notes:
 * In order to be able to synchronise properly, the sample position / time 
 * stamp pair must refer to the *current block*, that is, the engine will call 
 * ASIOGetSamplePosition() in its bufferSwitch() callback and expect the time 
 * for the current block. Thus, when requested in the very first bufferSwitch 
 * after ASIO_Start(), the sample position should be zero, and the time stamp 
 * should refer to the very time where the stream was started. It also means 
 * that the sample position must be block aligned. the driver must ensure proper 
 * interpolation if the system time can not be determined for the block position. 
 * The driver is responsible for precise time stamps as it usually has most 
 * direct access to lower level resources. Proper behaviour of 
 * ASIO_GetSamplePosition() and ASIO_GetLatencies() are essential for precise 
 * media synchronization!
 * @param
 * OutSamplePosition Will hold the sample position on return. The sample 
 * position is reset to zero when ASIOStart() gets called.
 * @param
 * OutTimeStamp Will hold the system time when the sample position was 
 * latched.
 * @return
 * If no input/output is present, ASE_NotPresent will be returned.
 * If there is no clock, ASE_SPNotAdvancing will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetSamplePosition
(	
	OUT		ASIOSamples *	OutSamplePosition,
	OUT		ASIOTimeStamp *	OutTimeStamp
)
{
	ASIOError asioError = GetChannels(NULL, NULL);

	if (ASE_OK == asioError)
	{
		if (OutSamplePosition)
		{
			// Have to do this manually as the ASIOSamples hi & lo DWORD
			// is ordered in the big-endian format.
			OutSamplePosition->hi = ULONG(m_CurrentSamplePosition>>32);
			OutSamplePosition->lo = ULONG(m_CurrentSamplePosition&0xFFFFFFFF);
		}

		if (OutTimeStamp)
		{
			// Have to do this manually as the ASIOTimeStamp hi & lo DWORD
			// is ordered in the big-endian format.
			OutTimeStamp->hi = ULONG(m_CurrentTimeStamp>>32);
			OutTimeStamp->lo = ULONG(m_CurrentTimeStamp&0xFFFFFFFF);
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::GetChannelInfo()
 *****************************************************************************
 *//*!
 * @brief
 * Retreive information about the nature of a channel.
 * @details
 * If possible, the string should be organised such that the first characters 
 * are most significantly describing the nature of the port, to allow for 
 * identification even if the view showing the port name is too small to 
 * display more than 8 characters, for instance.
 * @param
 * Info Pointer to a ASIOChannelInfo structure with:
 * - channel: on input, the channel index of the channel in question.
 * - isInput: on input, ASIOTrue if info for an input channel is requested, 
 * else output
 * - channelGroup: on return, the channel group that the channel belongs to. 
 * For drivers which support different types of channels, like analog, S/PDIF, 
 * AES/EBU, ADAT etc interfaces, there should be a reasonable grouping of these 
 * types. Groups are always independant form a channel index, that is, a channel
 * index always counts from 0 to numInputs/numOutputs regardless of the group it 
 * may belong to. There will always be at least one group (group 0). Please 
 * also note that by default, the host may decide to activate channels 0 and 1; 
 * thus, these should belong to the most useful type (analog i/o, if present).
 * - type: on return, contains the sample type of the channel
 * - isActive: on return, ASIOTrue if channel is active as it was installed by 
 * ASIOCreateBuffers(), ASIOFalse else
 * - name:  describing the type of channel in question. Used to allow for user 
 * selection, and enabling of specific channels. examples: "Analog In", 
 * "SPDIF Out" etc
 * @return
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
GetChannelInfo
(	
	IN	OUT	ASIOChannelInfo *	Info
)
{
	LONG NumInputChannels = 0, NumOutputChannels = 0;

	ASIOError asioError = GetChannels(&NumInputChannels, &NumOutputChannels);

	if (ASE_OK == asioError)
	{
		PASIO_CHANNEL_DESCRIPTOR ChannelDescriptor = NULL;

		if (Info->isInput)
		{
			if (Info->channel < NumInputChannels)
			{
				if (m_SelectedDataRangeAsio[1])
				{
					ChannelDescriptor = &m_SelectedDataRangeAsio[1]->ChannelDescriptors[Info->channel];
				}
			}
			else
			{
				asioError = ASE_NotPresent;
			}
		}
		else
		{
			if (Info->channel < NumOutputChannels)
			{
				if (m_SelectedDataRangeAsio[0])
				{
					ChannelDescriptor = &m_SelectedDataRangeAsio[0]->ChannelDescriptors[Info->channel];
				}
			}
			else
			{
				asioError = ASE_NotPresent;
			}
		}

		if (ASE_OK == asioError)
		{
			if (ChannelDescriptor)
			{
				Info->isActive = (ChannelDescriptor->InUse && ChannelDescriptor->PinDescriptor->Active);
				Info->channelGroup = ChannelDescriptor->PinDescriptor->KsAudioPin->GetPinId();
				//FIXME: This should be check against the actual type, ie the wFormatTag first.
				Info->type = (m_SampleSize == 16) ? ASIOSTInt16LSB : 
							#ifdef USE_24_BIT_PADDED
							(m_SampleSize == 24) ? ASIOSTInt32LSB : 
 							#else
							(m_SampleSize == 24) ? ASIOSTInt24LSB : 
							#endif // USE_24_BIT_PADDED
							(m_SampleSize == 32) ? ASIOSTInt32LSB :
							ASIOSTInt16LSB;
				
				CHAR Section[64]; sprintf(Section, "%s.Audio.ChannelNames", m_ProductIdentifier);

				CHAR KeyName[16]; sprintf(KeyName, "%02X-%02X", Info->channelGroup, Info->channel);

				CHAR SystemWindowsDirectory[MAX_PATH]; GetSystemWindowsDirectory(SystemWindowsDirectory, MAX_PATH);

				CHAR PathFileName[MAX_PATH]; sprintf(PathFileName, "%s\\emasio.dat", SystemWindowsDirectory);

				if (!GetPrivateProfileString(Section, KeyName, NULL, Info->name, sizeof(Info->name), PathFileName))
				{
                    sprintf(Info->name, "%s [%02X-%02X]", Info->isInput ? "IN" : "OUT", Info->channelGroup, Info->channel);
				}
			}
			else
			{
				asioError = ASE_NotPresent;
			}
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::CreateBuffers()
 *****************************************************************************
 *//*!
 * @brief
 * Allocates input/output buffers for all input and output channels to be 
 * activated.
 * @details
 * Notes:
 * If individual channel selection is not possible but requested, the driver 
 * has to handle this. Namely, bufferSwitch() will only have filled buffers 
 * of enabled outputs. If possible, processing and bus activities overhead 
 * should be avoided for channels which were not enabled here.
 * @param
 * BufferInfos A pointer to an array of ASIOBufferInfo structures:
 * - isInput: on input, ASIOTrue if the buffer is to be allocated
 * for an input, output buffer else
 * - channelNum: on input, the index of the channel in question
 * (counting from 0)
 * - buffers: on exit, 2 pointers to the halves of the channels' double-buffer.
 * the size of the buffer(s) of course depend on both the ASIOSampleType as 
 * obtained from ASIOGetChannelInfo(), and BufferSize
 * @param
 * NumChannels The sum of all input and output channels to be created; thus 
 * BufferInfos is a pointer to an array of NumChannels ASIOBufferInfo 
 * structures.
 * @param
 * BufferSize Selects one of the possible buffer sizes as obtained from
 * ASIOGetBufferSizes().
 * @param
 * Callbacks A pointer to an ASIOCallbacks structure.
 * @return
 * If not enough memory is available ASE_NoMemory will be returned.
 * If no input/output is present ASE_NotPresent will be returned.
 * If BufferSize is not supported, or one or more of the BufferInfos elements
 * contain invalid settings, ASE_InvalidMode will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
CreateBuffers
(	
	IN		ASIOBufferInfo *	BufferInfos,
	IN		LONG				NumChannels,
	IN		LONG				BufferSize,
	IN		ASIOCallbacks *		Callbacks
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::CreateBuffers]"));
    
	m_PreparedDataRangeAsio[0] = m_SelectedDataRangeAsio[0];
	m_PreparedDataRangeAsio[1] = m_SelectedDataRangeAsio[1];

	LONG NumInputChannels = 0, NumOutputChannels = 0;

	ASIOError asioError = GetChannels(&NumInputChannels, &NumOutputChannels);

	if (ASE_OK == asioError)
	{
		// Inform the underlying kernel driver about the ASIO buffer size. 
		// The number of FIFO buffers needed is one less than the buffer size (in ms), but a minimum of 2 is required for proper operation.
		//ULONG NumberOfFifoBuffers = (((BufferSize * 1000 + m_SamplingFrequency - 1)/ m_SamplingFrequency) * (m_NumberOfOutputBuffers - 1)) - 1; NumberOfFifoBuffers = (NumberOfFifoBuffers >= 2) ? NumberOfFifoBuffers : 2;

		//m_AudioRenderFilter->SetPropertySimple(KSPROPSETID_DeviceControl, KSPROPERTY_DEVICECONTROL_PIN_OUTPUT_CFIFO_BUFFERS, &NumberOfFifoBuffers, sizeof(NumberOfFifoBuffers));
	}

	if (ASE_OK == asioError)
	{
		if (NumInputChannels)
		{
			BOOL AllocateInputBuffers = FALSE;

			for (ULONG i=0; i<ULONG(NumChannels); i++)
			{
				if (BufferInfos[i].isInput)
				{
					AllocateInputBuffers = TRUE;
					break;
				}
			}

			if (AllocateInputBuffers)
			{
				asioError = _AllocateBuffers(TRUE, BufferInfos, NumChannels, BufferSize);
			}
			else
			{
				ULONG SyncOption = _GetSyncOption();

				if (SyncOption & SYNC_OPTION_FORCE_RECORD_CHANNELS)
				{
					_AllocateBuffers(TRUE, NULL, 0, BufferSize);
				}
			}
		}
	}

	if (ASE_OK == asioError)
	{
		if (NumOutputChannels)
		{
			asioError = _AllocateBuffers(FALSE, BufferInfos, NumChannels, BufferSize);
		}
	}

	if (ASE_OK == asioError)
	{
		m_AsioCallbacks = *Callbacks;
	}

	if (ASE_OK == asioError)
	{
		asioError = _AllocateIoThreads();
	}

	if (ASE_OK == asioError)
	{
		asioError = _DetermineHostCapabilities();
	}

	if (ASE_OK == asioError)
	{
		m_BufferSizeInSamples = BufferSize;

		m_DriverState = DRIVER_STATE_PREPARED;
	}

	if (ASE_OK != asioError)
	{
		DisposeBuffers();
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::DisposeBuffers()
 *****************************************************************************
 *//*!
 * @brief
 * Releases all buffers for the device.
 * @details
 * This implies ASIOStop().
 * @param
 * <None>
 * @return
 * If no buffer were ever prepared, ASE_InvalidMode will be returned.
 * If no input/output is present ASE_NotPresent will be returned.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
DisposeBuffers
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::DisposeBuffers]"));

	LONG NumInputChannels = 0, NumOutputChannels = 0;

	ASIOError asioError = GetChannels(&NumInputChannels, &NumOutputChannels);

	if (ASE_OK == asioError)
	{
		asioError = _DisposeIoThreads();
	}

	if (ASE_OK == asioError)
	{
		if (NumInputChannels)
		{
			asioError = _DisposeBuffers(TRUE);
		}
	}

	if (ASE_OK == asioError)
	{
		if (NumOutputChannels)
		{
			asioError = _DisposeBuffers(FALSE);
		}
	}

	m_DriverState = DRIVER_STATE_INITIALIZED;

	m_PreparedDataRangeAsio[0] = NULL;
	m_PreparedDataRangeAsio[1] = NULL;

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::ControlPanel()
 *****************************************************************************
 *//*!
 * @brief
 * Request the driver to start a control panel component for device specific 
 * user settings. This will not be accessed on some platforms (where the 
 * component is accessed instead).
 * @details
 * Notes:
 * if the user applied settings which require a re-configuration of parts or 
 * all of the enigine and/or driver (such as a change of the block size), the 
 * asioMessage callback can be used (see ASIO_Callbacks).
 * @param
 * <None>
 * @return
 * If no panel is available ASE_NotPresent will be returned. Actually, the 
 * return code is ignored.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
ControlPanel
(	void
)
{
	CAsioControlPanel * AsioControlPanel = new CAsioControlPanel();

	if (AsioControlPanel)
	{
		ULONG BufferSizeIndex = 0;
		
		for (ULONG i=0; i<(SIZEOF_ARRAY(m_SupportedBufferSizes)-m_MinimumBufferSizeArrayOffset); i++)
		{
			if (ULONGLONG(m_SupportedBufferSizes[i+m_MinimumBufferSizeArrayOffset] * 10000) == m_PreferredBufferSize)
			{
				BufferSizeIndex = i;
				break;
			}
		}

		ULONG BitDepths[16];
		
		ULONG NumberOfBitDepths = 0;

		UCHAR BitDepthMask = UCHAR(m_ClassId.Data1 & 0xF);

		ULONG PossibleBitDepths[] = { 8, 16, 24, 32 };

		for (ULONG i=1; i<SIZEOF_ARRAY(PossibleBitDepths); i++)
		{
			if (BitDepthMask & (1<<i))
			{
				BitDepths[NumberOfBitDepths] = PossibleBitDepths[i];

				NumberOfBitDepths++;
			}
		}

		ULONG BitDepthIndex = 0;
		
		for (ULONG i=0; i<NumberOfBitDepths; i++)
		{
			if (BitDepths[i] == m_PreferredSampleSize)
			{
				BitDepthIndex = i;
				break;
			}
		}

		if (ASE_OK == AsioControlPanel->Init(m_WndMain, &m_SupportedBufferSizes[m_MinimumBufferSizeArrayOffset], SIZEOF_ARRAY(m_SupportedBufferSizes)-m_MinimumBufferSizeArrayOffset, BufferSizeIndex, BitDepths, NumberOfBitDepths, BitDepthIndex, m_PerApplicationPreferences, m_FriendlyName))
		{
			DOUBLE BufferSize = FLOAT(m_PreferredBufferSize) / 10000;

			ULONG SampleSize = m_PreferredSampleSize;

			AsioControlPanel->Run(&BufferSize, &SampleSize, &m_PerApplicationPreferences);

			BOOL ResetRequest = FALSE;

			if (m_PreferredSampleSize != SampleSize)
			{
				m_PreferredSampleSize = SampleSize;

				// Bit depth changed...
				if (m_DriverState != DRIVER_STATE_INITIALIZED)
				{
					ResetRequest = TRUE;
				}
				else
				{
					m_SampleSize = m_PreferredSampleSize;
				}
			}
			
			if (m_PreferredBufferSize != ULONGLONG(BufferSize * 10000))
			{
				m_PreferredBufferSize = ULONGLONG(BufferSize * 10000);

				// Buffer size changed...
				if (m_DriverState != DRIVER_STATE_INITIALIZED)
				{
					ResetRequest = TRUE;
				}
				else
				{
					m_NumberOfOutputBuffers = _GetNumberOfOutputBuffers(m_PreferredBufferSize);
				}
			}

			if (ResetRequest)
			{
				// Send a reset request because we need to shut down and restart the
				// buffers.
				SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_RESET]);
			}
		}

		delete AsioControlPanel;
	}

	return ASE_OK;
}

/*****************************************************************************
 * CAsioDriver::Future()
 *****************************************************************************
 *//*!
 * @brief
 * Various purposes.
 * @details
 * See selectors defined below.	  
 * @param
 * Selector Operation Code as to be defined. Zero is reserved for testing 
 * purposes.
 * @param
 * Parameters Depends on the selector; usually pointer to a structure for 
 * passing and retreiving any type and amount of parameters.
 * @return
 * The return value is also selector dependant. If the selector is unknown, 
 * ASE_InvalidParameter should be returned to prevent further calls with this 
 * selector. On success, ASE_SUCCESS must be returned
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
Future
(	
	IN		LONG	Selector,
	IN		PVOID	Parameters
)
{
	ASIOError asioError = ASE_NotPresent;

	switch (Selector)
	{
		case kAsioEnableTimeCodeRead:
		{
			if (m_DriverCapabilities.CanTimeCode)
			{
				m_TimeCodeReadEnabled = TRUE;

				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioDisableTimeCodeRead:
		{
			if (m_DriverCapabilities.CanTimeCode)
			{
				m_TimeCodeReadEnabled = FALSE;

				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioSetInputMonitor:
		{
			if (m_DriverCapabilities.CanInputMonitor)
			{
				ASIOInputMonitor* InputMonitor = (ASIOInputMonitor*)(Parameters);

				_SetInputMonitor(InputMonitor);

				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioTransport:
		{
			if (m_DriverCapabilities.CanTransport)
			{
				ASIOTransportParameters* TransportParameters = (ASIOTransportParameters*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		case kAsioSetInputGain:
		{
			if (m_DriverCapabilities.CanInputGain)
			{
				ASIOChannelControls* ChannelControls = (ASIOChannelControls*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		case kAsioGetInputMeter:
		{
			if (m_DriverCapabilities.CanInputMeter)
			{
				ASIOChannelControls* ChannelControls = (ASIOChannelControls*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		case kAsioSetOutputGain:
		{
			if (m_DriverCapabilities.CanOutputGain)
			{
				ASIOChannelControls* ChannelControls = (ASIOChannelControls*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		case kAsioGetOutputMeter:
		{
			if (m_DriverCapabilities.CanOutputMeter)
			{
				ASIOChannelControls* ChannelControls = (ASIOChannelControls*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		// The cans...
		case kAsioCanInputMonitor:
		{
			if (m_DriverCapabilities.CanInputMonitor)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioCanTimeInfo:
		{
			if (m_DriverCapabilities.CanTimeInfo)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioCanTimeCode:
		{
			if (m_DriverCapabilities.CanTimeCode)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioCanTransport:
		{
			if (m_DriverCapabilities.CanTransport)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioCanInputGain:
		{
			if (m_DriverCapabilities.CanInputGain)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;
		
		case kAsioCanInputMeter:
		{
			if (m_DriverCapabilities.CanInputMeter)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;
		
		case kAsioCanOutputGain:
		{
			if (m_DriverCapabilities.CanOutputGain)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		case kAsioCanOutputMeter:
		{
			if (m_DriverCapabilities.CanOutputMeter)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		//	DSD support
		//	The following extensions are required to allow switching
		//	and control of the DSD subsystem.
		case kAsioSetIoFormat:
		{
			if (m_DriverCapabilities.CanDoIoFormat)
			{
				ASIOIoFormat* IoFormat = (ASIOIoFormat*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		case kAsioGetIoFormat:
		{
			if (m_DriverCapabilities.CanDoIoFormat)
			{
				ASIOIoFormat* IoFormat = (ASIOIoFormat*)(Parameters);

				//TODO: Implement this...
			}
		}
		break;

		case kAsioCanDoIoFormat:
		{
			if (m_DriverCapabilities.CanDoIoFormat)
			{
				asioError = ASE_SUCCESS;
			}
		}
		break;

		// Invalid parameters...
		default:
		{
			asioError = ASE_InvalidParameter;
		}
		break;
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::OutputReady()
 *****************************************************************************
 *//*!
 * @brief
 * This tells the driver that the host has completed processing the output 
 * buffers. 
 * @details
 * if the data format required by the hardware differs from the supported asio 
 * formats, but the hardware buffers are DMA buffers, the driver will have to 
 * convert the audio stream data; as the bufferSwitch callback is usually 
 * issued at dma block switch time, the driver will have to convert the 
 * *previous* host buffer, which increases the output latency by one block.
 * When the host finds out that ASIOOutputReady() returns true, it will issue 
 * this call whenever it completed output processing. Then the driver can 
 * convert the host data directly to the dma buffer to be played next, reducing 
 * output latency by one block. Another way to look at it is, that the buffer 
 * switch is called in order to pass the *input* stream to the host, so that it 
 * can process the input into the output, and the output stream is passed to 
 * the driver when the host has completed its process.
 * Notes:
 * Please remember to adjust ASIOGetLatencies() according to whether 
 * ASIOOutputReady() was ever called or not, if your driver supports this 
 * scenario. Also note that the engine may fail to call ASIO_OutputReady() in 
 * time in overload cases. As already mentioned, bufferSwitch should be called 
 * for every block regardless of whether a block could be processed in time.
 * @param
 * <None>
 * @return
 * Only if the above mentioned scenario is given, and a reduction of output 
 * latency can be acheived by this mechanism, should ASE_OK be returned. 
 * Otherwise (and usually), ASE_NotPresent should be returned in order to 
 * prevent further calls to this function. Note that the host may want to 
 * determine if it is to use this when the system is not yet fully initialized, 
 * so ASE_OK should always be returned if the mechanism makes sense.
 */
ASIOMETHODIMP_(ASIOError) 
CAsioDriver::
OutputReady
(	void
)
{
    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::OutputReady] - 0x%x", m_DriverState));

	if (m_DriverState == DRIVER_STATE_RUNNING)
	{
		//FIXME: Make changes to support asynchronous OutputReady...
		ULONG PacketIndex = ULONG((m_CurrentSamplePosition / m_BufferSizeInSamples) + m_NumberOfOutputBuffers) % NUMBER_OF_DATA_PACKETS;

		ULONG BufferIndex = ULONG(m_CurrentSamplePosition / m_BufferSizeInSamples) % 2;

		//dbgprintf("OutputReady[%d]: %d\n", BufferIndex, timeGetTime());

		_SyncWritePinData(m_PreparedDataRangeAsio[0], PacketIndex, BufferIndex);
	}

	return ASE_OK;
}

/*****************************************************************************
 * CAsioDriver::NodeThreadRoutine()
 *****************************************************************************
 *//*!
 * @brief
 * Node events thread routine.
 */
DWORD WINAPI 
CAsioDriver::
NodeThreadRoutine
(
	IN		LPVOID	Parameter
)
{
	CAsioDriver * AsioDriver = (CAsioDriver*)(Parameter);

	if (AsioDriver)
	{
		AsioDriver->_NodeThreadHandler();
	}

	// Explicitly specify the exit code. There seems to be a bug in the
    // WOW64 code that pass the exit code STILL_ACTIVE as the return
    // value in GetExitCodeThread(), causing the program to loop forever.
	ExitThread(0);

	return 0;
}

/*****************************************************************************
 * CAsioDriver::MainThreadRoutine()
 *****************************************************************************
 *//*!
 * @brief
 * I/O processing thread routine.
 */
DWORD WINAPI 
CAsioDriver::
MainThreadRoutine
(
	IN		LPVOID	Parameter
)
{
	CAsioDriver * AsioDriver = (CAsioDriver*)(Parameter);

	if (AsioDriver)
	{
		AsioDriver->_MainThreadHandler();
	}

	// Explicitly specify the exit code. There seems to be a bug in the
    // WOW64 code that pass the exit code STILL_ACTIVE as the return
    // value in GetExitCodeThread(), causing the program to loop forever.
	ExitThread(0);

	return 0;
}

/*****************************************************************************
 * CAsioDriver::WatchDogThreadRoutine()
 *****************************************************************************
 *//*!
 * @brief
 * Watch dog thread routine.
 */
DWORD WINAPI 
CAsioDriver::
WatchDogThreadRoutine
(
	IN		LPVOID	Parameter
)
{
	CAsioDriver * AsioDriver = (CAsioDriver*)(Parameter);

	if (AsioDriver)
	{
		AsioDriver->_WatchDogThreadHandler();
	}

	// Explicitly specify the exit code. There seems to be a bug in the
    // WOW64 code that pass the exit code STILL_ACTIVE as the return
    // value in GetExitCodeThread(), causing the program to loop forever.
	ExitThread(0);

	return 0;
}

/*****************************************************************************
 * CAsioDriver::FindXuNode()
 *****************************************************************************
 *//*!
 * @brief
 * Find the node with the specified property set.
 */
ULONG 
CAsioDriver::
_FindXuNode
(
    IN		REFGUID	PropertySet
)
{
	ULONG NodeId = ULONG(-1);

	ULONG NumberOfNodes = m_AudioRenderFilter->NumberOfNodes();

    // Iterate through all the nodes
    for (ULONG i = 0; i < NumberOfNodes; i++)
    {
        CKsNode * Node = NULL;

		if (m_AudioRenderFilter->ParseNodes(i, &Node))
		{
			if (IsEqualGUID(Node->GetType(), KSNODETYPE_DEV_SPECIFIC))
			{
				if (SUCCEEDED( m_AudioRenderFilter->GetNodePropertySimple(i, PropertySet, 0, NULL, 0, NULL, 0)))
				{
					// Found the node with the property set we are looking for.
					NodeId = i;
					break;
				}
			}
		}
    }

    return NodeId;
}

/*****************************************************************************
 * CAsioDriver::_GetClockRate()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current clock rate.
 */
HRESULT
CAsioDriver::
_GetClockRate
(
	OUT		ULONG *	OutClockRate
)
{
	HRESULT hr = E_FAIL;
		
	if (m_XuClockRateNodeId != ULONG(-1))
	{
		XU_CLOCK_RATE_SELECTOR Selector; Selector.Rate = XU_CLOCK_RATE_SR_44kHz;

		hr = m_AudioRenderFilter->GetNodePropertySimple(m_XuClockRateNodeId, m_XuPropSetClockRate, KSPROPERTY_XU_CLOCK_RATE_SELECTOR, &Selector, sizeof(Selector));

		if (SUCCEEDED(hr))
		{
			switch (Selector.Rate)
			{
				case XU_CLOCK_RATE_SR_44kHz:
				{
					*OutClockRate = 44100;
				}
				break;

				case XU_CLOCK_RATE_SR_48kHz:
				{
					*OutClockRate = 48000;
				}
				break;

				case XU_CLOCK_RATE_SR_88kHz:
				{
					*OutClockRate = 88200;
				}
				break;

				case XU_CLOCK_RATE_SR_96kHz:
				{
					*OutClockRate = 96000;
				}
				break;

				case XU_CLOCK_RATE_SR_176kHz:
				{
					*OutClockRate = 176400;
				}
				break;

				case XU_CLOCK_RATE_SR_192kHz:
				{
					*OutClockRate = 192000;
				}
				break;

				default:
				{
					hr = E_FAIL;
				}
				break;
			}
		}
	}

	return hr;
}

/*****************************************************************************
 * CAsioDriver::_SetClockRate()
 *****************************************************************************
 *//*!
 * @brief
 * Set the current clock rate.
 */
HRESULT
CAsioDriver::
_SetClockRate
(
	IN		ULONG	ClockRate
)
{
	HRESULT hr = S_OK;
		
	if (m_XuClockRateNodeId != ULONG(-1))
	{
		XU_CLOCK_RATE_SELECTOR Selector;

		switch (ClockRate)
		{
			case 44100:
			{
				Selector.Rate = XU_CLOCK_RATE_SR_44kHz;
			}
			break;

			case 48000:
			{
				Selector.Rate = XU_CLOCK_RATE_SR_48kHz;
			}
			break;

			case 88200:
			{
				Selector.Rate = XU_CLOCK_RATE_SR_88kHz;
			}
			break;

			case 96000:
			{
				Selector.Rate = XU_CLOCK_RATE_SR_96kHz;
			}
			break;

			case 176400:
			{
				Selector.Rate = XU_CLOCK_RATE_SR_176kHz;
			}
			break;

			case 192000:
			{
				Selector.Rate = XU_CLOCK_RATE_SR_192kHz;
			}
			break;

			default:
			{
				hr = E_FAIL;
			}
			break;
		}

		if (SUCCEEDED(hr))
		{
			if (!_IsDeviceInUse())
			{
				hr = m_AudioRenderFilter->SetNodePropertySimple(m_XuClockRateNodeId, m_XuPropSetClockRate, KSPROPERTY_XU_CLOCK_RATE_SELECTOR, &Selector, sizeof(Selector));
			}
		}
	}

	return hr;
}

/*****************************************************************************
 * CAsioDriver::_GetClockSource()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current clock rate.
 */
HRESULT
CAsioDriver::
_GetClockSource
(
	OUT		ULONG *	OutClockSource
)
{
	HRESULT hr = E_FAIL;
		
	if (m_XuClockSourceNodeId != ULONG(-1))
	{
		XU_CLOCK_SOURCE_SELECTOR Selector; Selector.Source = XU_CLOCK_SOURCE_INTERNAL;

		hr = m_AudioRenderFilter->GetNodePropertySimple(m_XuClockSourceNodeId, m_XuPropSetClockSource, KSPROPERTY_XU_CLOCK_SOURCE_SELECTOR, &Selector, sizeof(Selector));

		if (SUCCEEDED(hr))
		{
			switch (Selector.Source)
			{
				case XU_CLOCK_SOURCE_INTERNAL:
				{
					*OutClockSource = 0;
				}
				break;

				case XU_CLOCK_SOURCE_SPDIF_EXTERNAL:
				{
					*OutClockSource = 1;
				}
				break;

				default:
				{
					hr = E_FAIL;
				}
				break;
			}
		}
	}

	return hr;
}

/*****************************************************************************
 * CAsioDriver::_SetClockSource()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current clock rate.
 */
HRESULT
CAsioDriver::
_SetClockSource
(
	IN		ULONG	ClockSource
)
{
	HRESULT hr = S_OK;
		
	if (m_XuClockSourceNodeId != ULONG(-1))
	{
		XU_CLOCK_SOURCE_SELECTOR Selector;

		switch (ClockSource)
		{
			case 0:
			{
				Selector.Source = XU_CLOCK_SOURCE_INTERNAL;
			}
			break;

			case 1:
			{
				Selector.Source = XU_CLOCK_SOURCE_SPDIF_EXTERNAL;
			}
			break;

			default:
			{
				hr = E_FAIL;
			}
			break;
		}

		if (SUCCEEDED(hr))
		{
			hr = m_AudioRenderFilter->SetNodePropertySimple(m_XuClockSourceNodeId, m_XuPropSetClockSource, KSPROPERTY_XU_CLOCK_SOURCE_SELECTOR, &Selector, sizeof(Selector));
		}
	}

	return hr;
}

/*****************************************************************************
 * CAsioDriver::_SetInputMonitor()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current clock rate.
 */
HRESULT
CAsioDriver::
_SetInputMonitor
(
	IN		ASIOInputMonitor *	InputMonitor
)
{
	HRESULT hr = S_OK;
		
	if (m_XuDirectMonitorNodeId != ULONG(-1))
	{
		XU_DIRECT_MONITOR_ROUTING Routing;

		Routing.Input = InputMonitor->input;
		Routing.Output = InputMonitor->output;
		Routing.Gain = InputMonitor->gain;
		Routing.State = InputMonitor->state ? 1 : 0;
		Routing.Pan = InputMonitor->pan;

		hr = m_AudioRenderFilter->SetNodePropertySimple(m_XuDirectMonitorNodeId, m_XuPropSetDirectMonitor, KSPROPERTY_XU_DIRECT_MONITOR_ROUTING, &Routing, sizeof(Routing));
	}

	return hr;
}

/*****************************************************************************
 * CAsioDriver::_IsDeviceInUse()
 *****************************************************************************
 *//*!
 * @brief
 * Determine if the device is in use.
 */
BOOL 
CAsioDriver::
_IsDeviceInUse
(	void
)
{
	BOOL InUse = FALSE;

	ULONG NumberOfPinFactories = 0;

    HRESULT hr = m_AudioRenderFilter->GetPinPropertySimple
										(
											0,
											KSPROPSETID_Pin,
											KSPROPERTY_PIN_CTYPES,
											&NumberOfPinFactories,
											sizeof(ULONG)
										);
        
    if (SUCCEEDED(hr))
    {
		ULONG InstantiatedPins = 0;

        // Loop through the pins
        for (ULONG PinId = 0; PinId < NumberOfPinFactories; PinId++)
        {
			KSPIN_CINSTANCES KspCInstances;

			HRESULT hr = m_AudioRenderFilter->GetPinPropertySimple
												(
													PinId,
													KSPROPSETID_Pin,
													KSPROPERTY_PIN_GLOBALCINSTANCES,
													&KspCInstances,
													sizeof(KSPIN_CINSTANCES)
												);

			if (SUCCEEDED(hr))
			{
				InstantiatedPins += KspCInstances.CurrentCount;
			}
        }

		InUse = (InstantiatedPins > 0);
    }

	return InUse;
}

/*****************************************************************************
 * CAsioDriver::_BuildFilterDataRanges()
 *****************************************************************************
 *//*!
 * @brief
 * Build the filter data ranges.
 * @details
 * For the bit depth specified for this driver, determine if the following 
 * sample rates are supported: 44100, 48000, 88200, 96000, 176400, 192000.
 * For each of the supported sample rate, determine the pins that supported the 
 * sample rate and bit depth. Find the maximum number of channels that is 
 * supported by each pin, and the total number of channels supported by all 
 * pins.
 */
HRESULT
CAsioDriver::
_BuildFilterDataRanges
(
	IN		BOOL					Capture,
	IN		PFILTER_DATARANGE_ASIO	FilterDataRanges
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges]"));

	HRESULT hr = S_OK;

	ULONG NumberOfPins = (Capture) ? m_AudioCaptureFilter->NumberOfCapturePins() : m_AudioRenderFilter->NumberOfRenderPins();

	for (ULONG iRates = 0; iRates < SIZEOF_ARRAY(m_PossibleSampleRates); iRates++)
	{
		PFILTER_DATARANGE_ASIO FilterDataRange = &FilterDataRanges[iRates];
		
		FilterDataRange->SampleRate = m_PossibleSampleRates[iRates];
		FilterDataRange->Supported = FALSE;

		for (ULONG iPin = 0; iPin < NumberOfPins; iPin++)
		{
			CKsAudioPin * AudioPin = NULL;

			if (Capture)
			{
				m_AudioCaptureFilter->ParseCapturePins(iPin, (CKsAudioCapturePin**)&AudioPin);
			}
			else
			{
				m_AudioRenderFilter->ParseRenderPins(iPin, (CKsAudioRenderPin**)&AudioPin);
			}

			if (AudioPin)
			{	
				PASIO_PIN_DESCRIPTOR PinDescriptor = &FilterDataRange->PinDescriptors[FilterDataRange->PinDescriptorCount];

				PASIO_CHANNEL_DESCRIPTOR ChannelDescriptor = &FilterDataRange->ChannelDescriptors[FilterDataRange->ChannelDescriptorCount];

				for (ULONG iDataRange = 0; ; iDataRange++)
				{
					PKSDATARANGE_AUDIO DataRangeAudio = NULL;

					if (AudioPin->ParseDataRanges(iDataRange, &DataRangeAudio))
					{
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - iDataRange: %d", iDataRange));
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - Pin: 0x%x", AudioPin->GetPinId()));
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - m_SampleSize: %d", m_SampleSize));
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - MinimumBitsPerSample: %d", DataRangeAudio->MinimumBitsPerSample));
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - MaximumBitsPerSample: %d", DataRangeAudio->MaximumBitsPerSample));
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - MinimumSampleFrequency: %d", DataRangeAudio->MinimumSampleFrequency));
						//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - MaximumSampleFrequency: %d", DataRangeAudio->MaximumSampleFrequency));

						// See if the data range matches with our requirements.
						if ((IsEqualGUID(KSDATAFORMAT_SUBTYPE_PCM, DataRangeAudio->DataRange.SubFormat)) &&
							(DataRangeAudio->MinimumBitsPerSample <= m_SampleSize) &&
							(DataRangeAudio->MaximumBitsPerSample >= m_SampleSize) &&
							(DataRangeAudio->MinimumSampleFrequency <= m_PossibleSampleRates[iRates]) &&
							(DataRangeAudio->MaximumSampleFrequency >= m_PossibleSampleRates[iRates]))
						{
							if (PinDescriptor->MaximumChannels < DataRangeAudio->MaximumChannels)
							{
								PinDescriptor->MaximumChannels = DataRangeAudio->MaximumChannels;
							}

							PinDescriptor->Usable = TRUE;
						}
					}
					else
					{
						break;
					}
				}

				if (PinDescriptor->Usable)
				{
					_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_BuildFilterDataRanges] - Found: Pin: 0x%x", AudioPin->GetPinId()));

					PinDescriptor->KsAudioPin = AudioPin;
					PinDescriptor->InUse = FALSE;
					PinDescriptor->Active = FALSE;
					PinDescriptor->NumberOfActiveChannels = 0;
					PinDescriptor->BufferSizeInSamples = 0;

					for (ULONG iPacket=0; iPacket<NUMBER_OF_DATA_PACKETS; iPacket++)
					{
						PinDescriptor->Packets[iPacket].Header.Data = NULL;
						PinDescriptor->Packets[iPacket].Header.FrameExtent = 0;
						PinDescriptor->Packets[iPacket].Header.DataUsed = 0;
						PinDescriptor->Packets[iPacket].Header.Size = sizeof(KSSTREAM_HEADER);
						PinDescriptor->Packets[iPacket].Header.PresentationTime.Numerator = 1;
						PinDescriptor->Packets[iPacket].Header.PresentationTime.Denominator = 1;
					}

					for (ULONG iChannel = 0; iChannel < PinDescriptor->MaximumChannels; iChannel++)
					{
						ChannelDescriptor[iChannel].Usable = TRUE;
						ChannelDescriptor[iChannel].InUse = FALSE;
						ChannelDescriptor[iChannel].ChannelIndex = iChannel;
						ChannelDescriptor[iChannel].PinDescriptor = PinDescriptor;
						ChannelDescriptor[iChannel].BufferSizeInSamples = 0;
						ChannelDescriptor[iChannel].Buffer[0] = NULL;
						ChannelDescriptor[iChannel].Buffer[1] = NULL;
					}

					FilterDataRange->Supported = TRUE;
					FilterDataRange->MaximumChannels += PinDescriptor->MaximumChannels;
					FilterDataRange->PinDescriptorCount += 1;
					FilterDataRange->ChannelDescriptorCount += PinDescriptor->MaximumChannels;

					// Latency information.
					KSTIME Latencies[SIZEOF_ARRAY(m_PossibleSampleRates)]; 

					ULONG NumberOfLatencies = SIZEOF_ARRAY(m_PossibleSampleRates);

					if (SUCCEEDED(AudioPin->GetLatencies(Latencies, &NumberOfLatencies)))
					{
						if (iRates < NumberOfLatencies)
						{
							if (FilterDataRange->Latency.Time < Latencies[iRates].Time)
							{
								FilterDataRange->Latency = Latencies[iRates];
							}
						}
						else
						{
							if (FilterDataRange->Latency.Time < Latencies[0].Time)
							{
								FilterDataRange->Latency = Latencies[0];
							}
						}
					}
				}
			}
		}
	}

	return hr;
}

/*****************************************************************************
 * _InstantiateAudioPin()
 *****************************************************************************
 *//*!
 * @brief
 * Instantiate the audio pin with the specified format.
 */
static 
HRESULT 
_InstantiateAudioPin
(
	IN		CKsAudioPin *	KsAudioPin,
	IN		ULONG			SampleRate,
	IN		USHORT			SampleSize,
	IN		USHORT			NumChannels
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[_InstantiateAudioPin] - PinDescriptor(0x%x) NumberOfActiveChannels: %d", KsAudioPin->GetPinId(), NumChannels));

	WAVEFORMATEX wfx;

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = NumChannels;
	wfx.nSamplesPerSec = SampleRate;
	wfx.wBitsPerSample = SampleSize;
	wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize = 0;

	HRESULT hr = KsAudioPin->SetFormat(&wfx);

	if (SUCCEEDED(hr))
	{
		hr = KsAudioPin->Instantiate(FALSE);
	}

	if (SUCCEEDED(hr))
	{
		hr = KsAudioPin->Reset();
	}

	_DbgPrintF(DEBUGLVL_VERBOSE,("[_InstantiateAudioPin] - SetFormat: 0x%x", hr));

	return hr;
}

/*****************************************************************************
 * CAsioDriver::_AllocateBuffers()
 *****************************************************************************
 *//*!
 * @brief
 * Allocates input/output buffers for all input and output channels to be 
 * activated.
 */
ASIOError
CAsioDriver::
_AllocateBuffers
(	
	IN		BOOL				Input,
	IN		ASIOBufferInfo *	BufferInfos,
	IN		LONG				NumChannels,
	IN		LONG				BufferSizeInSamples
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_AllocateBuffers] - NumChannels: %d", NumChannels));

	ASIOError asioError = ASE_OK;

	PFILTER_DATARANGE_ASIO DataRangeAsio = m_PreparedDataRangeAsio[Input?1:0];

	if (DataRangeAsio)
	{
		if (NumChannels)
		{
			for (ULONG i=0; i<ULONG(NumChannels); i++)
			{
				if ((Input && BufferInfos[i].isInput) || (!Input && !BufferInfos[i].isInput))
				{
					PASIO_CHANNEL_DESCRIPTOR ChannelDescriptor = &DataRangeAsio->ChannelDescriptors[BufferInfos[i].channelNum];

					#ifdef USE_24_BIT_PADDED
					ULONG SampleSize = (m_SampleSize == 24) ? 32 : m_SampleSize;
					#else
					ULONG SampleSize = m_SampleSize;
					#endif // USE_24_BIT_PADDED

					ChannelDescriptor->Buffer[0] = LocalAlloc(LPTR, BufferSizeInSamples * SampleSize / 8);
					ChannelDescriptor->Buffer[1] = LocalAlloc(LPTR, BufferSizeInSamples * SampleSize / 8);
					ChannelDescriptor->BufferSizeInSamples = BufferSizeInSamples;
					ChannelDescriptor->InUse = TRUE;

					BufferInfos[i].buffers[0] = ChannelDescriptor->Buffer[0];
					BufferInfos[i].buffers[1] = ChannelDescriptor->Buffer[1];

					if ((ChannelDescriptor->Buffer[0] == NULL) || (ChannelDescriptor->Buffer[1] == NULL))
					{
						asioError = ASE_NoMemory;
						break;
					}

					// Indicate to the pin where this channel is located that it is in use.
					ChannelDescriptor->PinDescriptor->InUse = TRUE;

					// Figure out how many channels is needed on this pin.
					if (ChannelDescriptor->PinDescriptor->NumberOfActiveChannels < (ChannelDescriptor->ChannelIndex+1))
					{
						ChannelDescriptor->PinDescriptor->NumberOfActiveChannels = ChannelDescriptor->ChannelIndex + 1;
					}
				}
			}
		}
		else
		{
			// A hack to get record pin instantiated for synchronization use.
			for (ULONG iPin = 0; iPin < DataRangeAsio->PinDescriptorCount; iPin++)
			{
				PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio->PinDescriptors[iPin];

				PinDescriptor->InUse = TRUE;

				PinDescriptor->NumberOfActiveChannels = 1;
			}
		}

		if (ASE_OK == asioError)
		{
			for (ULONG iPin = 0; iPin < DataRangeAsio->PinDescriptorCount; iPin++)
			{
				PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio->PinDescriptors[iPin];

				if ((PinDescriptor->InUse) && (PinDescriptor->NumberOfActiveChannels))
				{
					HRESULT hr = _InstantiateAudioPin(PinDescriptor->KsAudioPin, DataRangeAsio->SampleRate, (USHORT)m_SampleSize, (USHORT)PinDescriptor->NumberOfActiveChannels);

					if (FAILED(hr))
					{
						// Figure out the next optimal channel count to use.
						for (ULONG NumChannels = PinDescriptor->NumberOfActiveChannels+1; NumChannels <= PinDescriptor->MaximumChannels; NumChannels++)
						{
							hr = _InstantiateAudioPin(PinDescriptor->KsAudioPin, DataRangeAsio->SampleRate, (USHORT)m_SampleSize, (USHORT)NumChannels);

							if (SUCCEEDED(hr))
							{
								PinDescriptor->NumberOfActiveChannels = NumChannels;
								break;
							}
						}
					}

					if (SUCCEEDED(hr))
					{
						PinDescriptor->Active = TRUE;

						DataRangeAsio->ActivePinMask |= (1<<iPin);
					}

					for (ULONG iPacket=0; iPacket<NUMBER_OF_DATA_PACKETS; iPacket++)
					{
						PinDescriptor->Packets[iPacket].Header.FrameExtent = PinDescriptor->NumberOfActiveChannels * BufferSizeInSamples * m_SampleSize / 8;
						PinDescriptor->Packets[iPacket].Header.Data = LocalAlloc(LPTR, PinDescriptor->Packets[iPacket].Header.FrameExtent);
						PinDescriptor->Packets[iPacket].Header.DataUsed = Input ? 0 : PinDescriptor->Packets[iPacket].Header.FrameExtent;
					}

					PinDescriptor->BufferSizeInSamples = BufferSizeInSamples;

					for (ULONG iPacket=0; iPacket<NUMBER_OF_DATA_PACKETS; iPacket++)
					{
						if (PinDescriptor->Packets[iPacket].Header.Data == NULL)
						{
							asioError = ASE_NoMemory;
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		asioError = ASE_InvalidMode;
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::_DisposeBuffers()
 *****************************************************************************
 *//*!
 * @brief
 * Releases all buffers for the device.
 */
ASIOError
CAsioDriver::
_DisposeBuffers
(	
	IN		BOOL	Input
)
{
	PFILTER_DATARANGE_ASIO DataRangeAsio = m_PreparedDataRangeAsio[Input?1:0];

	if (DataRangeAsio)
	{
		for (ULONG iPin = 0; iPin < DataRangeAsio->PinDescriptorCount; iPin++)
		{
			PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio->PinDescriptors[iPin];

			if (PinDescriptor->InUse)
			{
				for (ULONG iPacket=0; iPacket<NUMBER_OF_DATA_PACKETS; iPacket++)
				{
					if (PinDescriptor->Packets[iPacket].Header.Data)
					{
						LocalFree(PinDescriptor->Packets[iPacket].Header.Data);

						PinDescriptor->Packets[iPacket].Header.Data = NULL;
						PinDescriptor->Packets[iPacket].Header.FrameExtent = 0;
						PinDescriptor->Packets[iPacket].Header.DataUsed = 0;
					}
				}

				PinDescriptor->NumberOfActiveChannels = 0;
				PinDescriptor->BufferSizeInSamples = 0;

				if (PinDescriptor->Active)
				{
					PinDescriptor->KsAudioPin->ClosePin();
				}

				PinDescriptor->Active = FALSE;
				PinDescriptor->InUse = FALSE;
			}
		}

		for (ULONG iChannel = 0; iChannel < DataRangeAsio->ChannelDescriptorCount; iChannel++)
		{
			PASIO_CHANNEL_DESCRIPTOR ChannelDescriptor = &DataRangeAsio->ChannelDescriptors[iChannel];

			if (ChannelDescriptor->Buffer[0])
			{
				LocalFree(ChannelDescriptor->Buffer[0]);
				ChannelDescriptor->Buffer[0] = NULL;
			}

			if (ChannelDescriptor->Buffer[1])
			{
				LocalFree(ChannelDescriptor->Buffer[1]);
				ChannelDescriptor->Buffer[1] = NULL;
			}

			ChannelDescriptor->BufferSizeInSamples = 0;
			ChannelDescriptor->InUse = FALSE;
		}
	}

	return ASE_OK;
}

/*****************************************************************************
 * CAsioDriver::_AllocateIoThreads()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOError
CAsioDriver::
_AllocateIoThreads
(	void
)
{
	ASIOError asioError = ASE_OK;

	m_IoThreadHandle[0] = CreateThread(NULL, 0, MainThreadRoutine, this, CREATE_SUSPENDED, NULL);

	if (!m_IoThreadHandle[0])
	{
		asioError = ASE_NoMemory;
	}

	if (ASE_OK == asioError)
	{
		m_IoThreadHandle[1] = CreateThread(NULL, 0, WatchDogThreadRoutine, this, CREATE_SUSPENDED, NULL);

		if (!m_IoThreadHandle[1])
		{
			asioError = ASE_NoMemory;
		}
	}

	if (ASE_OK == asioError)
	{
		DWORD_PTR ProcessAffinityMask = 0;
		DWORD_PTR SystemAffinityMask = 0;

		GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask);

		for (ULONG i=0; i<SIZEOF_ARRAY(m_IoThreadHandle); i++)
		{
			// Make sure that we run everywhere we can...
			SetThreadAffinityMask(m_IoThreadHandle[i], ProcessAffinityMask);

			SetThreadPriority(m_IoThreadHandle[i], THREAD_PRIORITY_TIME_CRITICAL);

			SetThreadPriorityBoost(m_IoThreadHandle[i], FALSE);

			ResumeThread(m_IoThreadHandle[i]);
		}
	}

	if (ASE_OK != asioError)
	{
		for (ULONG i=0; i<SIZEOF_ARRAY(m_IoThreadHandle); i++)
		{
			if (m_IoThreadHandle[i])
			{
				CloseHandle(m_IoThreadHandle[i]);
			}

			m_IoThreadHandle[i] = NULL;
		}
	}

	return asioError;
}

/*****************************************************************************
 * CAsioDriver::_DisposeIoThreads()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOError
CAsioDriver::
_DisposeIoThreads
(	void
)
{
	for (ULONG i=SIZEOF_ARRAY(m_IoThreadHandle); i>0; i--)
	{
		if (m_IoThreadHandle[i-1])
		{
			// Abort the thread...
			if (i == 2)
			{
				SetEvent(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_ABORT]);
			}
			else
			{
				SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_ABORT]);
			}

			// Wait for the thread to exit...
			ULONG ExitCode;

			while (GetExitCodeThread( m_IoThreadHandle[(i-1)], &ExitCode))
			{
				if (ExitCode != STILL_ACTIVE) 
				{
					break;
				}
			}

			CloseHandle(m_IoThreadHandle[(i-1)]);

			m_IoThreadHandle[(i-1)] = NULL;
		}
	}

	return ASE_OK;
}

/*****************************************************************************
 * CAsioDriver::_DetermineDriverCapabilities()
 *****************************************************************************
 *//*!
 * @brief
 * Figure out what the driver is capable of.
 */
ASIOError
CAsioDriver::
_DetermineDriverCapabilities
(	void
)
{
	ZeroMemory(&m_DriverCapabilities, sizeof(ASIO_DRIVER_CAPABILITIES));

	// Look at the Direct Monitor node id to decide whether it is supported.
	m_DriverCapabilities.CanInputMonitor = (m_XuDirectMonitorNodeId != ULONG(-1)); 
			
	m_DriverCapabilities.CanTimeInfo = TRUE; // Time info is supported.

	m_DriverCapabilities.CanTimeCode = FALSE; // Not supported unless hardware is capable.

	return ASE_OK;
}

/*****************************************************************************
 * CAsioDriver::_DetermineHostCapabilities()
 *****************************************************************************
 *//*!
 * @brief
 * Ask the host about its capabilities.
 */
ASIOError
CAsioDriver::
_DetermineHostCapabilities
(	void
)
{
	ZeroMemory(&m_HostCapabilities, sizeof(ASIO_HOST_CAPABILITIES));
	
	if (_AsioMessage(kAsioEngineVersion, 0, NULL, NULL) >= 2)
	{
		// Ask for the host capability only if the driver can support the feature.

		if (m_DriverCapabilities.CanTimeInfo)
		{
			if (_AsioMessage(kAsioSupportsTimeInfo, 0, NULL, NULL) == 1)
			{
				m_HostCapabilities.SupportTimeInfo = TRUE;

				if (m_DriverCapabilities.CanTimeCode)
				{
					if (_AsioMessage(kAsioSupportsTimeCode, 0, NULL, NULL) == 1)
					{
						m_HostCapabilities.SupportTimeCode = TRUE;
					}
				}
			}
		}

		if (m_DriverCapabilities.CanInputMonitor)
		{
			if (_AsioMessage(kAsioSupportsInputMonitor, 0, NULL, NULL) == 1)
			{
				m_HostCapabilities.SupportInputMonitor = TRUE;
			}	
		}

		if (m_DriverCapabilities.CanInputGain)
		{
			if (_AsioMessage(kAsioSupportsInputGain, 0, NULL, NULL) == 1)
			{
				m_HostCapabilities.SupportInputGain = TRUE;
			}	
		}

		if (m_DriverCapabilities.CanInputMeter)
		{
			if (_AsioMessage(kAsioSupportsInputMeter, 0, NULL, NULL) == 1)
			{
				m_HostCapabilities.SupportInputMeter = TRUE;
			}	
		}
			
		if (m_DriverCapabilities.CanOutputGain)
		{
			if (_AsioMessage(kAsioSupportsOutputGain, 0, NULL, NULL) == 1)
			{
				m_HostCapabilities.SupportOutputGain = TRUE;
			}	
		}

		if (m_DriverCapabilities.CanOutputMeter)
		{
			if (_AsioMessage(kAsioSupportsOutputMeter, 0, NULL, NULL) == 1)
			{
				m_HostCapabilities.SupportOutputMeter = TRUE;
			}	
		}
	}

	return ASE_OK;
}

/*****************************************************************************
 * CAsioDriver::_NodeThreadHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Handle the audio control change events on the nodes.
 */
VOID
CAsioDriver::
_NodeThreadHandler
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_NodeThreadHandler]"));

	BOOL Abort = FALSE;

	while (!Abort)
	{
        // Wait on node events.
        DWORD Wait = WaitForMultipleObjects(ASIO_NODE_EVENT_COUNT, m_AsioNodeEvent, FALSE, INFINITE);

		if ((Wait == WAIT_FAILED) || (Wait == WAIT_TIMEOUT))
		{
			// Something bad happened...
			_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_NodeThreadHandler] - Bad things happened: %d", Wait));
            break;
		}

        ULONG EventSignaled = Wait - WAIT_OBJECT_0;

		//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_NodeThreadHandler] - Event signaled: %d", EventSignaled));

		ULONG ControlEvent = EventSignaled;

		switch (ControlEvent)
		{
			case ASIO_NODE_EVENT_ABORT:
			{
				_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_NodeThreadHandler] - Abort"));

				// say good bye...
				Abort = TRUE;
			}
			break;

			case ASIO_NODE_EVENT_CLOCK_RATE_CHANGE:
			{
				_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_NodeThreadHandler] - Clock rate change"));

				ULONG ClockRate; 

				if (SUCCEEDED(_GetClockRate(&ClockRate)))
				{
					if (m_SamplingFrequency != ClockRate)
					{
						// Update the sample rate and the associated data range.
						m_SamplingFrequency = ClockRate;

						for (ULONG i=0; i<SIZEOF_ARRAY(m_FilterDataRanges); i++)
						{
							for (ULONG j=0; j<SIZEOF_ARRAY(m_PossibleSampleRates); j++)
							{
								if (m_FilterDataRanges[i][j].SampleRate == m_SamplingFrequency)
								{
									m_SelectedDataRangeAsio[i] = &m_FilterDataRanges[i][j];
									break;
								}
							}
						}

						if (m_DriverState != DRIVER_STATE_INITIALIZED)
						{
							// Propagate the change to the main I/O thread handler to avoid race
							// conditions.
							m_SampleRateDidChange = TRUE;

							// Send a reset request because we need to shut down and restart the
							// buffers.
							SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_RESET]);
						}
					}
				}
				else
				{
					// WTF ??
					ASSERT(0);
				}
			}
			break;

			case ASIO_NODE_EVENT_CLOCK_SOURCE_CHANGE:
			{
				_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_NodeThreadHandler] - Clock source change"));

				// Oh whatever...
			}
			break;

			case ASIO_NODE_EVENT_DRIVER_RESYNC:
			{
				_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_NodeThreadHandler] - Driver resync request"));

				if (m_DriverState == DRIVER_STATE_RUNNING)
				{
					// Send a resync request to tell the host that the driver detected underruns and
					// requires a resynchronization (start/stop).
					if ((!m_EnableResyncRequest) || (!_AsioMessage(kAsioResyncRequest, 0, NULL, NULL)))
					{
						// Send a reset request because we need to shut down and restart the
						// buffers.
						SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_RESET]);
					}
				}
			}
			break;
		}
	}
}

/*****************************************************************************
 * CAsioDriver::_MainThreadHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Handle the main I/O processing.
 */
VOID
CAsioDriver::
_MainThreadHandler
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_MainThreadHandler]"));

	ULONG ActiveSignalMask = 0;

	// Find the active data ranges.
	PFILTER_DATARANGE_ASIO DataRangeAsio[2];

	DataRangeAsio[0] = m_PreparedDataRangeAsio[0];
	DataRangeAsio[1] = m_PreparedDataRangeAsio[1];

	for (ULONG i=0; i<2; i++)
	{
		for (ULONG k=0; k<DataRangeAsio[i]->PinDescriptorCount; k++)
		{
			PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio[i]->PinDescriptors[k];

			if (PinDescriptor->Active)
			{
				ActiveSignalMask |= (1<<i);
				break;
			}
		}
	}

	ULONG SyncOption = _GetSyncOption();

	if (SyncOption & SYNC_OPTION_SYNCHRONIZE_TO_RECORD_ONLY)
	{
		if (ActiveSignalMask & 0x2)
		{
			ActiveSignalMask = 0x2;
		}
	}

	HANDLE EventPool[ASIO_CONTROL_EVENT_COUNT + (2 * MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS * NUMBER_OF_DATA_PACKETS)];

	ULONG NumberOfEvents = 0;

	// ASIO control events...
	for (ULONG i=0; i<ASIO_CONTROL_EVENT_COUNT; i++)
	{
		EventPool[NumberOfEvents] = m_AsioControlEvent[i]; NumberOfEvents++;
	}

	// Output pin events...
	for (ULONG k=0; k<MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS; k++)
	{
		PDATA_PACKET Packet = DataRangeAsio[0]->PinDescriptors[k].Packets;

		for (ULONG iPacket=0; iPacket<NUMBER_OF_DATA_PACKETS; iPacket++)
		{
			ResetEvent(Packet[iPacket].Signal.hEvent); EventPool[NumberOfEvents] = Packet[iPacket].Signal.hEvent; NumberOfEvents++;
		}
	}
	
	// Input pin events...
	for (ULONG k=0; k<MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS; k++)
	{
		PDATA_PACKET Packet = DataRangeAsio[1]->PinDescriptors[k].Packets;

		for (ULONG iPacket=0; iPacket<NUMBER_OF_DATA_PACKETS; iPacket++)
		{
			ResetEvent(Packet[iPacket].Signal.hEvent); EventPool[NumberOfEvents] = Packet[iPacket].Signal.hEvent; NumberOfEvents++;
		}
	}
	
	BOOL Abort = FALSE;

	KSSTATE State = KSSTATE_STOP;

	// WatchDog time out
	LARGE_INTEGER TimeOut; TimeOut.QuadPart=-10000*2000; // 2000ms

	// Global signal mask.
	ULONG SignalMask[2]; SignalMask[0] = SignalMask[1] = 0;

	// Signal pin masks.
	ULONG SignalPinMask[2][2]; SignalPinMask[0][0] = SignalPinMask[0][1] = SignalPinMask[1][0] = SignalPinMask[1][1] = 0;

	while (!Abort)
	{
        // Wait on control events and "packet complete" events.
        DWORD Wait = WaitForMultipleObjects(NumberOfEvents, EventPool, FALSE, INFINITE);

		if ((Wait == WAIT_FAILED) || (Wait == WAIT_TIMEOUT))
		{
			// Something bad happened...
			_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_MainThreadHandler] - Bad things happened: %d", Wait));
            break;
		}

        ULONG EventSignaled = Wait - WAIT_OBJECT_0;

		//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_MainThreadHandler] - Event signaled: %d", EventSignaled));

		if (EventSignaled < ASIO_CONTROL_EVENT_COUNT)
		{
			// Control events...
			ULONG ControlEvent = EventSignaled;

			switch (ControlEvent)
			{
				case ASIO_CONTROL_EVENT_ABORT:
				{
					_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_MainThreadHandler] - Abort"));

					CancelWaitableTimer(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED]);

					// say good bye...
					Abort = TRUE;
				}
				break;

				case ASIO_CONTROL_EVENT_START:
				{
					_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_MainThreadHandler] - Start"));

					if (State == KSSTATE_STOP)
					{			
						// Reset the signal mask.
						SignalMask[0] = SignalMask[1] = 0;

						SignalPinMask[0][0] = SignalPinMask[0][1] = SignalPinMask[1][0] = SignalPinMask[1][1] = 0;

						// Prepare the pin...
						_SyncModifyPinState(DataRangeAsio, KSSTATE_ACQUIRE);

						_SyncModifyPinState(DataRangeAsio, KSSTATE_PAUSE);
						
						m_BufferSwitchCount = 0;
						
						m_CurrentSamplePosition = 0;

						// Take a snapshot of the sample position and time stamp.
						m_RunningSamplePosition = 0;

						_SnapshotTimeStamp(&m_RunningTimeStamp);

						// Zero initialize the first N buffers...
						for (ULONG i=0; i<m_NumberOfOutputBuffers; i++)
						{
							_SyncZeroPinBuffer(DataRangeAsio, i);

							_SyncQueuePinBuffer(0x3, DataRangeAsio, i);
						}

						// Wait a bit for the buffers being queued.
						Sleep(10);

						State = KSSTATE_RUN;

						// Gentlemen, start your engine...
						_SyncModifyPinState(DataRangeAsio, KSSTATE_RUN);

						SetEvent(m_StateTransitionEvent);

						//dbgprintf("StartTime: %d\n", timeGetTime());
					}

					SetWaitableTimer(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED], &TimeOut, 0, NULL, NULL, FALSE);
 				}
				break;

				case ASIO_CONTROL_EVENT_STOP:
				{
					//dbgprintf("Stop: %d\n", timeGetTime());

					_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_MainThreadHandler] - Stop"));

					if (State == KSSTATE_RUN)
					{
						_SyncModifyPinState(DataRangeAsio, KSSTATE_PAUSE);

						State = KSSTATE_PAUSE;
					}

					if (State == KSSTATE_PAUSE)
					{
						_SyncModifyPinState(DataRangeAsio, KSSTATE_ACQUIRE);

						State = KSSTATE_ACQUIRE;
					}
					
					if (State == KSSTATE_ACQUIRE)
					{
						_SyncModifyPinState(DataRangeAsio, KSSTATE_STOP);
					}

					State = KSSTATE_STOP;

					CancelWaitableTimer(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED]);

					SetEvent(m_StateTransitionEvent);
				}
				break;

				case ASIO_CONTROL_EVENT_RESET:
				{
					_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_MainThreadHandler] - Reset"));

					SetWaitableTimer(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED], &TimeOut, 0, NULL, NULL, FALSE);

					if (m_SampleRateDidChange)
					{
						_SampleRateDidChange((ASIOSampleRate)m_SamplingFrequency);

						m_SampleRateDidChange = FALSE;
					}

					_AsioMessage(kAsioResetRequest, 0, NULL, NULL);
				}
				break;
			}
		}
		else
		{
			if (State == KSSTATE_RUN)
			{
				SetWaitableTimer(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED], &TimeOut, 0, NULL, NULL, FALSE);
			}

			// Pin events...
			ULONG PinEvent = EventSignaled - ASIO_CONTROL_EVENT_COUNT;

			if (PinEvent < (MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS * NUMBER_OF_DATA_PACKETS))
			{
				// Output pin events...
				// Data submission block.  The event signaled corresponds to one of
				// our data packets.  The device is finished with this packet, so we
				// can fill with data and submit to the pin.
				ULONG PacketNumber = PinEvent;

				ULONG BufferIndex = PacketNumber%2;

				SignalPinMask[0][BufferIndex] |= (1<<(PacketNumber/NUMBER_OF_DATA_PACKETS));

				//dbgprintf("[OUT][%d]: %d\n", BufferIndex, timeGetTime());

				if  ((SignalPinMask[0][BufferIndex] & DataRangeAsio[0]->ActivePinMask) == DataRangeAsio[0]->ActivePinMask)
				{
					if (State == KSSTATE_RUN)
					{
						//dbgprintf("Signal[0][0]: %d\n", timeGetTime());

						SignalMask[BufferIndex] |= 0x1;

						if ((SignalMask[BufferIndex] & ActiveSignalMask) == ActiveSignalMask)
						{
							if (m_BufferSwitchCount) 
							{
								m_RunningSamplePosition += m_BufferSizeInSamples;
							}

							_SnapshotTimeStamp(&m_RunningTimeStamp);

							_SyncQueuePinBuffer(0x1, DataRangeAsio, (ULONG)((m_RunningSamplePosition/m_BufferSizeInSamples)+m_NumberOfOutputBuffers)%NUMBER_OF_DATA_PACKETS);

							_SwitchBuffer(BufferIndex);

							_SyncQueuePinBuffer(0x2, DataRangeAsio, (ULONG)((m_RunningSamplePosition/m_BufferSizeInSamples)+m_NumberOfOutputBuffers)%NUMBER_OF_DATA_PACKETS);

							SignalMask[BufferIndex] = 0;
						}
					}

					// Clear the pin event mask.
					SignalPinMask[0][BufferIndex] = 0;
				}
			}
			else
			{
				// Input pin events...
				// Data submission block.  The event signaled corresponds to one of
				// our data packets.  The device is finished with this packet, so we
				// can copy to the buffer to the channel buffer.
				ULONG PacketNumber = PinEvent - (MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS * NUMBER_OF_DATA_PACKETS);

				ULONG BufferIndex = PacketNumber%2;

				//dbgprintf(" [IN][%d]: %d\n", BufferIndex, timeGetTime());

				SignalPinMask[1][BufferIndex] |= (1<<(PacketNumber/NUMBER_OF_DATA_PACKETS));

				if  ((SignalPinMask[1][BufferIndex] & DataRangeAsio[1]->ActivePinMask) == DataRangeAsio[1]->ActivePinMask)
				{
					if (State == KSSTATE_RUN)
					{
						//dbgprintf("Signal[1][0]: %d\n", timeGetTime());

						if (m_BufferSwitchCount >= m_NumberOfOutputBuffers) 
						{
							ULONG PacketIndex = (ULONG)((m_RunningSamplePosition/m_BufferSizeInSamples)+1)%NUMBER_OF_DATA_PACKETS;

							_SyncReadPinData(DataRangeAsio[1], PacketIndex, PacketIndex%2);
						}
						else
						{
							// Do not read the data for the first N buffer switches since they are suppose to be zeros.
							//_SyncReadPinData(DataRangeAsio[1], ULONG(m_BufferSwitchCount), ULONG(m_BufferSwitchCount)%2);
						}

						SignalMask[BufferIndex] |= 0x2;

						if ((SignalMask[BufferIndex] & ActiveSignalMask) == ActiveSignalMask)
						{
							if (m_BufferSwitchCount) 
							{
								m_RunningSamplePosition += m_BufferSizeInSamples;
							}

							_SnapshotTimeStamp(&m_RunningTimeStamp);

							_SyncQueuePinBuffer(0x1, DataRangeAsio, (ULONG)((m_RunningSamplePosition/m_BufferSizeInSamples)+m_NumberOfOutputBuffers)%NUMBER_OF_DATA_PACKETS);

							_SwitchBuffer(BufferIndex);

							_SyncQueuePinBuffer(0x2, DataRangeAsio, (ULONG)((m_RunningSamplePosition/m_BufferSizeInSamples)+m_NumberOfOutputBuffers)%NUMBER_OF_DATA_PACKETS);

							SignalMask[BufferIndex] = 0;
						}
					}

					// Clear the event mask.
					SignalPinMask[1][BufferIndex] = 0;
				}
			}
		}
	}

	if (State == KSSTATE_RUN)
	{
		_SyncModifyPinState(DataRangeAsio, KSSTATE_PAUSE);

		State = KSSTATE_PAUSE;
	}

	if (State == KSSTATE_PAUSE)
	{
		_SyncModifyPinState(DataRangeAsio, KSSTATE_ACQUIRE);

		State = KSSTATE_ACQUIRE;
	}
	
	if (State == KSSTATE_ACQUIRE)
	{
		_SyncModifyPinState(DataRangeAsio, KSSTATE_STOP);
	}
}

/*****************************************************************************
 * CAsioDriver::_WatchDogThreadHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Handle the watchdog function.
 */
VOID
CAsioDriver::
_WatchDogThreadHandler
(	void
)
{
    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioDriver::_WatchDogThreadHandler]"));

	HANDLE EventPool[2];

	EventPool[0] = m_WatchDogEvent[ASIO_WATCHDOG_EVENT_ABORT];
	EventPool[1] = m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED];

	BOOL Abort = FALSE;

	while (!Abort)
	{
        // Wait on control events and "packet complete" events.
        DWORD Wait = WaitForMultipleObjects(2, EventPool, FALSE, INFINITE);

		if ((Wait == WAIT_FAILED) || (Wait == WAIT_TIMEOUT))
		{
			// Something bad happened...
			_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_WatchDogThreadHandler] - Bad things happened: %d", Wait));
            break;
		}

        ULONG EventSignaled = Wait - WAIT_OBJECT_0;

		//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_WatchDogThreadHandler] - Event signaled: %d", EventSignaled));

		if (EventSignaled == ASIO_WATCHDOG_EVENT_TIMER_EXPIRED)
		{
			// The watch dog timer times out. Probably means the driver is unresponsive,
			// so reset it for good measure...
			SetEvent(m_AsioControlEvent[ASIO_CONTROL_EVENT_RESET]);

			Abort = TRUE;
		}
		else
		{
			// Abort
			_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_WatchDogThreadHandler] - Abort"));

			// say good bye...
			Abort = TRUE;
		}
	}

	CancelWaitableTimer(m_WatchDogEvent[ASIO_WATCHDOG_EVENT_TIMER_EXPIRED]);
}

/*****************************************************************************
 * CAsioDriver::_SyncModifyPinState()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SyncModifyPinState
(
	IN		PFILTER_DATARANGE_ASIO *	DataRangeAsio,
	IN		KSSTATE						NewState
)
{
	_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_SyncModifyPinState] - NewState: 0x%x", NewState));

	// Synchronize the start frame.
	if (NewState == KSSTATE_RUN)
	{
		ULONG StartFrameNumber = 0;

		m_AudioRenderFilter->GetPropertySimple(KSPROPSETID_DeviceControl, KSPROPERTY_DEVICECONTROL_PIN_SYNCHRONIZE_START_FRAME, &StartFrameNumber, sizeof(StartFrameNumber));

		m_AudioCaptureFilter->SetPropertySimple(KSPROPSETID_DeviceControl, KSPROPERTY_DEVICECONTROL_PIN_SYNCHRONIZE_START_FRAME, &StartFrameNumber, sizeof(StartFrameNumber));
	}

	// Input
	for (ULONG i=0; i<DataRangeAsio[1]->PinDescriptorCount; i++)
	{
		if (DataRangeAsio[1]->PinDescriptors[i].Active)
		{
			DataRangeAsio[1]->PinDescriptors[i].KsAudioPin->SetState(NewState);
		}
	}

	// Output
	for (ULONG i=0; i<DataRangeAsio[0]->PinDescriptorCount; i++)
	{
		if (DataRangeAsio[0]->PinDescriptors[i].Active)
		{
			DataRangeAsio[0]->PinDescriptors[i].KsAudioPin->SetState(NewState);
		}
	}
}

/*****************************************************************************
 * CAsioDriver::_SyncWritePinData()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SyncWritePinData
(
	IN		PFILTER_DATARANGE_ASIO	DataRangeAsio,
	IN		ULONG					PacketIndex,
	IN		ULONG					BufferIndex
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_SyncWritePinData] - PacketIndex: 0x%x, BufferIndex: 0x%x", PacketIndex, BufferIndex));

	for (ULONG i=0; i<DataRangeAsio->ChannelDescriptorCount; i++)
	{
		PASIO_CHANNEL_DESCRIPTOR ChannelDescriptor = &DataRangeAsio->ChannelDescriptors[i];

		PASIO_PIN_DESCRIPTOR PinDescriptor = ChannelDescriptor->PinDescriptor;

		if (PinDescriptor->Active)
		{
			PDATA_PACKET Packet = &PinDescriptor->Packets[PacketIndex];

			// Copy data from channel to pin.
			if (Packet->Header.FrameExtent)
			{
				PUCHAR Source = PUCHAR(ChannelDescriptor->Buffer[BufferIndex]);

				if (Source)
				{
					ULONG SampleSize = m_SampleSize / 8; // in bytes

					ULONG ChannelOffset = ChannelDescriptor->ChannelIndex * SampleSize;
				
					PUCHAR Destination = PUCHAR(Packet->Header.Data) + ChannelOffset;

					ULONG FrameSize = PinDescriptor->NumberOfActiveChannels * SampleSize;

					ULONG NumSamplesToCopy = Packet->Header.FrameExtent / FrameSize;

					#ifdef USE_24_BIT_PADDED
					if (m_SampleSize == 24)
					{
						for (ULONG j=0; j<NumSamplesToCopy; j++)
						{
							CopyMemory(Destination, Source+1, 3/*SampleSize*/);
							
							Destination += FrameSize;

							Source += 4;
						}
					}
					else
					#endif // USE_24_BIT_PADDED
					{
						for (ULONG j=0; j<NumSamplesToCopy; j++)
						{
							CopyMemory(Destination, Source, SampleSize);
							
							Destination += FrameSize;

							Source += SampleSize;
						}
					}
				}
			}
		}
	}
}

/*****************************************************************************
 * CAsioDriver::_SyncReadPinData()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SyncReadPinData
(
	IN		PFILTER_DATARANGE_ASIO	DataRangeAsio,
	IN		ULONG					PacketIndex,
	IN		ULONG					BufferIndex
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_SyncReadPinData] - PacketIndex: 0x%x, BufferIndex: 0x%x", PacketIndex, BufferIndex));

	for (ULONG i=0; i<DataRangeAsio->ChannelDescriptorCount; i++)
	{
		PASIO_CHANNEL_DESCRIPTOR ChannelDescriptor = &DataRangeAsio->ChannelDescriptors[i];

		PASIO_PIN_DESCRIPTOR PinDescriptor = ChannelDescriptor->PinDescriptor;

		if (PinDescriptor->Active)
		{
			PDATA_PACKET Packet = &PinDescriptor->Packets[PacketIndex];

			// Copy data from pin to channel.
			if (Packet->Header.DataUsed)
			{
				PUCHAR Destination = PUCHAR(ChannelDescriptor->Buffer[BufferIndex]);
				
				if (Destination)
				{
					ULONG SampleSize = m_SampleSize / 8; // in bytes

					ULONG ChannelOffset = ChannelDescriptor->ChannelIndex * SampleSize;

					PUCHAR Source = PUCHAR(Packet->Header.Data) + ChannelOffset;

					ULONG FrameSize = PinDescriptor->NumberOfActiveChannels * SampleSize;

					ULONG NumSamplesToCopy = Packet->Header.DataUsed / FrameSize;

					#ifdef USE_24_BIT_PADDED
					if (m_SampleSize == 24)
					{
						for (ULONG j=0; j<NumSamplesToCopy; j++)
						{
							*Destination = 0;

							CopyMemory(Destination+1, Source, 3/*SampleSize*/);
							
							Destination += 4;

							Source += FrameSize;
						}
					}
					else
					#endif // USE_24_BIT_PADDED
					{
						for (ULONG j=0; j<NumSamplesToCopy; j++)
						{
							CopyMemory(Destination, Source, SampleSize);
							
							Destination += SampleSize;

							Source += FrameSize;
						}
					}
				}
			}
		}
	}
}

/*****************************************************************************
 * CAsioDriver::_SyncZeroPinBuffer()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SyncZeroPinBuffer
(
	IN		PFILTER_DATARANGE_ASIO *	DataRangeAsio,
	IN		ULONG						PacketIndex
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_SyncZeroPinBuffer] - PacketIndex : 0x%x", PacketIndex));

	// Zero output buffers...
	for (ULONG j=0; j<DataRangeAsio[0]->PinDescriptorCount; j++)
	{
		PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio[0]->PinDescriptors[j];

		if (PinDescriptor->Active)
		{
			if (PinDescriptor->Packets[PacketIndex].Header.FrameExtent)
			{
				ZeroMemory(PinDescriptor->Packets[PacketIndex].Header.Data, PinDescriptor->Packets[PacketIndex].Header.FrameExtent);
			}
		}
	}
}

/*****************************************************************************
 * CAsioDriver::_SyncQueuePinBuffer()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SyncQueuePinBuffer
(
	IN		ULONG						Ops,
	IN		PFILTER_DATARANGE_ASIO *	DataRangeAsio,
	IN		ULONG						PacketIndex
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_SyncQueuePinBuffer] - PacketIndex : 0x%x", PacketIndex));

	if (Ops & 0x1)
	{
		// Queue input buffers...
		for (ULONG j=0; j<DataRangeAsio[1]->PinDescriptorCount; j++)
		{
			PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio[1]->PinDescriptors[j];

			if (PinDescriptor->Active)
			{
				PinDescriptor->Packets[PacketIndex].Header.DataUsed = 0;

				PinDescriptor->KsAudioPin->ReadData(&PinDescriptor->Packets[PacketIndex].Header, &PinDescriptor->Packets[PacketIndex].Signal);
			}
		}/**/
	}

	if (Ops & 0x2)
	{
		// Queue output buffers...
		for (ULONG j=0; j<DataRangeAsio[0]->PinDescriptorCount; j++)
		{
			PASIO_PIN_DESCRIPTOR PinDescriptor = &DataRangeAsio[0]->PinDescriptors[j];

			if (PinDescriptor->Active)
			{
				PinDescriptor->KsAudioPin->WriteData(&PinDescriptor->Packets[PacketIndex].Header, &PinDescriptor->Packets[PacketIndex].Signal);
			}
		}
	}
}

/*****************************************************************************
 * CAsioDriver::_SwitchBuffer()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SwitchBuffer
(
	IN		ULONG	BufferIndex
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAsioDriver::_SwitchBuffer] - BufferIndex : 0x%x", BufferIndex));

	//LARGE_INTEGER PerfFrequency; QueryPerformanceFrequency(&PerfFrequency);
	//LARGE_INTEGER PerfCount0; QueryPerformanceCounter(&PerfCount0);
	//dbgprintf("----%d\n", 1000000 * PerfCount0.QuadPart / PerfFrequency.QuadPart);

	//dbgprintf("BS[%d] BEGIN: %d\n", BufferIndex, timeGetTime());

	m_BufferSwitchCount++;

	// Latch the running sample position and time stamp.
	m_CurrentSamplePosition = m_RunningSamplePosition;
	m_CurrentTimeStamp = m_RunningTimeStamp;

	if (m_DriverCapabilities.CanTimeInfo && m_HostCapabilities.SupportTimeInfo)
	{
		ASIOTime AsioTime; 
		
		ZeroMemory(&AsioTime, sizeof(ASIOTime));

		// Time Info.
		AsioTimeInfo* TimeInfo = &AsioTime.timeInfo;

		TimeInfo->speed = 1.0;

		TimeInfo->samplePosition.hi = ULONG(m_CurrentSamplePosition>>32);
		TimeInfo->samplePosition.lo = ULONG(m_CurrentSamplePosition&0xFFFFFFFF);

		TimeInfo->systemTime.hi = ULONG(m_CurrentTimeStamp>>32);
		TimeInfo->systemTime.lo = ULONG(m_CurrentTimeStamp&0xFFFFFFFF);

		TimeInfo->flags = kSystemTimeValid | kSamplePositionValid | kSampleRateValid;;

		ULONG CurrentSampleRate = m_PreparedDataRangeAsio[0] ? m_PreparedDataRangeAsio[0]->SampleRate : 
					   			  m_PreparedDataRangeAsio[1] ? m_PreparedDataRangeAsio[1]->SampleRate : 0;

		if (CurrentSampleRate != m_SamplingFrequency)
		{
			TimeInfo->flags |= kSampleRateChanged;
		}

		TimeInfo->sampleRate = (double)m_SamplingFrequency;

		// Time code (if supported).
		if (m_DriverCapabilities.CanTimeCode && m_HostCapabilities.SupportTimeCode && m_TimeCodeReadEnabled)
		{
			ASIOTimeCode * TimeCode = &AsioTime.timeCode;

			//FIXME: The time code samples ?
			//TimeCode->timeCodeSamples = m_TimeCodeSamples;

			TimeCode->flags = kTcValid | kTcRunning | kTcOnspeed;
		}

		m_AsioCallbacks.bufferSwitchTimeInfo(&AsioTime, BufferIndex, ASIOTrue);
	}
	else
	{
		m_AsioCallbacks.bufferSwitch(BufferIndex, ASIOTrue);
	}

	//LARGE_INTEGER PerfCount1; QueryPerformanceCounter(&PerfCount1);
	//dbgprintf("-DD--%d\n", 1000000 * (PerfCount1.QuadPart - PerfCount0.QuadPart)/ PerfFrequency.QuadPart);

	//dbgprintf("BS[%d] END: %d\n", BufferIndex, timeGetTime());
}

/*****************************************************************************
 * CAsioDriver::_AsioMessage()
 *****************************************************************************
 *//*!
 * @brief
 */
LONG 
CAsioDriver::
_AsioMessage
(	
	IN		LONG		Selector, 
	IN		LONG		Value, 
	IN		PVOID		Message, 
	IN		DOUBLE *	Optional
)
{
	LONG msgRet = 0;

	if (m_AsioCallbacks.asioMessage)
	{
		msgRet = m_AsioCallbacks.asioMessage(Selector, Value, Message, Optional);
	}

	return msgRet;
}

/*****************************************************************************
 * CAsioDriver::_SampleRateDidChange()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SampleRateDidChange
(	
	IN		ASIOSampleRate	SampleRate
)
{
	if (m_AsioCallbacks.sampleRateDidChange)
	{
		m_AsioCallbacks.sampleRateDidChange(SampleRate);	
	}
}

/*****************************************************************************
 * CAsioDriver::_SnapshotTimeStamp()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SnapshotTimeStamp
(
	OUT		ULONGLONG *	OutTimeStamp
)
{
	LONGLONG TimeStamp = timeGetTime(); TimeStamp *= 1000000; // ns

	if (OutTimeStamp)
	{
		*OutTimeStamp = TimeStamp;
	}
}

#define EMU_PUBLIC_KEY "Software\\E-MU"
/*****************************************************************************
 * CAsioDriver::_SaveDriverSettingsToRegistry()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_SaveDriverSettingsToRegistry
(	void
)
{
	HKEY EmuPublicKey = NULL;

	if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, EMU_PUBLIC_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &EmuPublicKey, NULL))
	{
		HKEY AsioDriverKey = NULL;

		if (ERROR_SUCCESS == RegCreateKeyEx(EmuPublicKey, m_FriendlyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &AsioDriverKey, NULL))
		{
			HKEY PerApplicationKey = NULL;

			HKEY AppPrefsKey = NULL;

			if (ERROR_SUCCESS == RegCreateKeyEx(AsioDriverKey, "AppPrefs", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &AppPrefsKey, NULL))
			{
				CHAR FileName[MAX_PATH]; GetModuleFileName(NULL, (LPTSTR)FileName, MAX_PATH);
				CHAR * BaseName = strrchr(FileName, '\\'); if (BaseName) BaseName++; else BaseName = FileName;
				CHAR * Exe = strrchr(BaseName, '.'); if (Exe) *Exe = 0;

				RegCreateKeyEx(AppPrefsKey, BaseName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &PerApplicationKey, NULL);

				RegCloseKey(AppPrefsKey);
			}

			if (m_PerApplicationPreferences)
			{
				// Save as application specific settings.
				RegSetValueEx(PerApplicationKey, "PreferredBufferSize", 0, REG_QWORD, (BYTE*)&m_PreferredBufferSize, sizeof(ULONGLONG));
				
				RegSetValueEx(PerApplicationKey, "SampleRate", 0, REG_DWORD, (BYTE*)&m_SamplingFrequency, sizeof(ULONG));

				RegSetValueEx(PerApplicationKey, "SampleSize", 0, REG_DWORD, (BYTE*)&m_PreferredSampleSize, sizeof(ULONG));

			}
			else
			{
				// Save as global ASIO driver settings.
				RegSetValueEx(AsioDriverKey, "PreferredBufferSize", 0, REG_QWORD, (BYTE*)&m_PreferredBufferSize, sizeof(ULONGLONG));
				
				RegSetValueEx(AsioDriverKey, "SampleRate", 0, REG_DWORD, (BYTE*)&m_SamplingFrequency, sizeof(ULONG));

				RegSetValueEx(AsioDriverKey, "SampleSize", 0, REG_DWORD, (BYTE*)&m_PreferredSampleSize, sizeof(ULONG));
			}

			if (PerApplicationKey)
			{
				RegSetValueEx(PerApplicationKey, "Preferences", 0, REG_DWORD, (BYTE*)&m_PerApplicationPreferences, sizeof(BOOL));

				RegCloseKey(PerApplicationKey);
			}

			RegCloseKey(AsioDriverKey);
		}

		RegCloseKey(EmuPublicKey);
	}
}

/*****************************************************************************
 * CAsioDriver::_RestoreDriverSettingsFromRegistry()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAsioDriver::
_RestoreDriverSettingsFromRegistry
(	void
)
{
	HKEY EmuPublicKey = NULL;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, EMU_PUBLIC_KEY, 0, KEY_ALL_ACCESS, &EmuPublicKey))
	{
		HKEY AsioDriverKey = NULL;

		if (ERROR_SUCCESS == RegOpenKeyEx(EmuPublicKey, m_FriendlyName, 0, KEY_ALL_ACCESS, &AsioDriverKey))
		{
			HKEY PerApplicationKey = NULL;

			HKEY AppPrefsKey = NULL;

			if (ERROR_SUCCESS == RegOpenKeyEx(AsioDriverKey, "AppPrefs", 0, KEY_ALL_ACCESS, &AppPrefsKey))
			{
				CHAR FileName[MAX_PATH]; GetModuleFileName(NULL, (LPTSTR)FileName, MAX_PATH);
				CHAR * BaseName = strrchr(FileName, '\\'); if (BaseName) BaseName++; else BaseName = FileName;
				CHAR * Exe = strrchr(BaseName, '.'); if (Exe) *Exe = 0;

				if (ERROR_SUCCESS == RegOpenKeyEx(AppPrefsKey, BaseName, 0, KEY_ALL_ACCESS, &PerApplicationKey))
				{
					// Check to see if there is per application settings.
					ULONG Type = 0;	ULONG Size = sizeof(BOOL);
					RegQueryValueEx(PerApplicationKey, "Preferences", 0, &Type, PBYTE(&m_PerApplicationPreferences), &Size);
				}

				RegCloseKey(AppPrefsKey);
			}

			if (m_PerApplicationPreferences)
			{
				// Use application specific settings.
				ULONG Type = 0;	ULONG Size = sizeof(ULONGLONG);
				if (ERROR_SUCCESS != RegQueryValueEx(PerApplicationKey, "PreferredBufferSize", 0, &Type, PBYTE(&m_PreferredBufferSize), &Size))
				{
					// Fallback to global settings.
					RegQueryValueEx(AsioDriverKey, "PreferredBufferSize", 0, &Type, PBYTE(&m_PreferredBufferSize), &Size);
				}

				Type = 0; Size = sizeof(ULONG);
				if (ERROR_SUCCESS != RegQueryValueEx(PerApplicationKey, "SampleRate", 0, &Type, PBYTE(&m_SamplingFrequency), &Size))
				{
					// Fallback to global settings.
					RegQueryValueEx(AsioDriverKey, "SampleRate", 0, &Type, PBYTE(&m_SamplingFrequency), &Size);
				}

				Type = 0; Size = sizeof(ULONG);
				if (ERROR_SUCCESS != RegQueryValueEx(PerApplicationKey, "SampleSize", 0, &Type, PBYTE(&m_PreferredSampleSize), &Size))
				{
					// Fallback to global settings.
					RegQueryValueEx(AsioDriverKey, "SampleSize", 0, &Type, PBYTE(&m_PreferredSampleSize), &Size);
				}

				m_SampleSize = m_PreferredSampleSize;
			}
			else
			{
				// Use global ASIO driver settings.
				ULONG Type = 0;	ULONG Size = sizeof(ULONGLONG);
				RegQueryValueEx(AsioDriverKey, "PreferredBufferSize", 0, &Type, PBYTE(&m_PreferredBufferSize), &Size);

				Type = 0; Size = sizeof(ULONG);
				RegQueryValueEx(AsioDriverKey, "SampleRate", 0, &Type, PBYTE(&m_SamplingFrequency), &Size);

				Type = 0; Size = sizeof(ULONG);
				RegQueryValueEx(AsioDriverKey, "SampleSize", 0, &Type, PBYTE(&m_PreferredSampleSize), &Size);

				m_SampleSize = m_PreferredSampleSize;
			}

			if (PerApplicationKey)
			{
				RegCloseKey(PerApplicationKey);
			}

			RegCloseKey(AsioDriverKey);
		}

		RegCloseKey(EmuPublicKey);
	}
}

/*****************************************************************************
 * CAsioDriver::_GetNumberOfOutputBuffers()
 *****************************************************************************
 *//*!
 * @brief
 * Return the buffering scheme used. Default to double buffering.
 */
ULONG 
CAsioDriver::
_GetNumberOfOutputBuffers
(
	IN		ULONGLONG	BufferSize
)
{
	CHAR Section[64]; sprintf(Section, "%s.Audio.BufferSize", m_ProductIdentifier);

	CHAR SystemWindowsDirectory[MAX_PATH]; GetSystemWindowsDirectory(SystemWindowsDirectory, MAX_PATH);

	CHAR PathFileName[MAX_PATH]; sprintf(PathFileName, "%s\\emasio.dat", SystemWindowsDirectory);

	CHAR Value[16]; sprintf(Value, "%d", BufferSize / 10000);

	// Default to double buffering scheme.
	ULONG NumberOfOutputBuffers = GetPrivateProfileInt(Section, Value, 2, PathFileName);

	// Check the min/max values.
	if (NumberOfOutputBuffers > NUMBER_OF_DATA_PACKETS)
	{
		NumberOfOutputBuffers = NUMBER_OF_DATA_PACKETS;
	}
	else if (NumberOfOutputBuffers < 2)
	{
		NumberOfOutputBuffers = 2;
	}

	return NumberOfOutputBuffers;
}

/*****************************************************************************
 * CAsioDriver::_GetSyncOption()
 *****************************************************************************
 *//*!
 * @brief
 * Return the synchronization method used. Default (0) to synchronize with 
 * both record & playback. 1 to synchronize to record only if it is present.
 */
ULONG 
CAsioDriver::
_GetSyncOption
(	void
)
{
	CHAR Section[64]; sprintf(Section, "%s.Audio.Options", m_ProductIdentifier);

	CHAR SystemWindowsDirectory[MAX_PATH]; GetSystemWindowsDirectory(SystemWindowsDirectory, MAX_PATH);

	CHAR PathFileName[MAX_PATH]; sprintf(PathFileName, "%s\\emasio.dat", SystemWindowsDirectory);

	// Default to 0.
	ULONG SyncOption = GetPrivateProfileInt(Section, "Sync", 0, PathFileName);

	return SyncOption;
}

/*****************************************************************************
 * CAsioDriver::_GetAppHacks()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CAsioDriver::
_GetAppHacks
(	void
)
{
	CHAR FileName[MAX_PATH]; GetModuleFileName(NULL, (LPTSTR)FileName, MAX_PATH);
	CHAR * BaseName = strrchr(FileName, '\\'); if (BaseName) BaseName++; else BaseName = FileName;
	CHAR * Exe = strrchr(BaseName, '.'); if (Exe) *Exe = 0;

	CHAR Section[64]; sprintf(Section, "AppHacks.%s", BaseName);

	CHAR SystemWindowsDirectory[MAX_PATH]; GetSystemWindowsDirectory(SystemWindowsDirectory, MAX_PATH);

	CHAR PathFileName[MAX_PATH]; sprintf(PathFileName, "%s\\emasio.dat", SystemWindowsDirectory);

	// Default to 1.
	m_EnableResyncRequest = GetPrivateProfileInt(Section, "EnableResyncRequest", 1, PathFileName);
}


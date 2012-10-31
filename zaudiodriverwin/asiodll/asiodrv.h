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
 * @file       asiodrv.h
 * @brief      ASIO driver private definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _ASIO_DRIVER_H_
#define _ASIO_DRIVER_H_

#include "asiodll.h"
#include "xu.h"

/*****************************************************************************
 * Defines
 */
typedef enum
{
	DRIVER_STATE_LOADED = 0,
	DRIVER_STATE_INITIALIZED,
	DRIVER_STATE_PREPARED,
	DRIVER_STATE_RUNNING
} DRIVER_STATE, *PDRIVER_STATE;

typedef enum
{
	ASIO_CONTROL_EVENT_ABORT = 0,
	ASIO_CONTROL_EVENT_START,
	ASIO_CONTROL_EVENT_STOP,
	ASIO_CONTROL_EVENT_RESET,
	ASIO_CONTROL_EVENT_COUNT
} ASIO_CONTROL_EVENT, *PASIO_CONTROL_EVENT;

typedef enum
{
	ASIO_WATCHDOG_EVENT_ABORT = 0,
	ASIO_WATCHDOG_EVENT_TIMER_EXPIRED,
	ASIO_WATCHDOG_EVENT_COUNT
} ASIO_WATCHDOG_EVENT, *PASIO_WATCHDOG_EVENT;

typedef enum
{
	ASIO_NODE_EVENT_ABORT = 0,
	ASIO_NODE_EVENT_CLOCK_RATE_CHANGE,
	ASIO_NODE_EVENT_CLOCK_SOURCE_CHANGE,
	ASIO_NODE_EVENT_DRIVER_RESYNC,
	ASIO_NODE_EVENT_COUNT
} ASIO_NODE_EVENT, *PASIO_NODE_EVENT;

typedef struct
{
    KSSTREAM_HEADER	Header;
    OVERLAPPED      Signal;
} DATA_PACKET, *PDATA_PACKET;

#define NUMBER_OF_DATA_PACKETS	8

typedef struct
{
	BOOL			Usable;
	BOOL			InUse;
	BOOL			Active;
	CKsAudioPin *	KsAudioPin;
	ULONG			MaximumChannels;
	ULONG			NumberOfActiveChannels;
	ULONG			BufferSizeInSamples;
	DATA_PACKET		Packets[NUMBER_OF_DATA_PACKETS];
} ASIO_PIN_DESCRIPTOR, *PASIO_PIN_DESCRIPTOR;

typedef struct
{
	BOOL					Usable;
	BOOL					InUse;
	ULONG					ChannelIndex;
	PASIO_PIN_DESCRIPTOR	PinDescriptor;
	ULONG					BufferSizeInSamples;
	PVOID					Buffer[2];
} ASIO_CHANNEL_DESCRIPTOR, *PASIO_CHANNEL_DESCRIPTOR;

#define MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS	((MAXIMUM_WAIT_OBJECTS-ASIO_CONTROL_EVENT_COUNT)/NUMBER_OF_DATA_PACKETS/2)

typedef struct
{
	BOOL					Supported;
	ULONG					SampleRate;
	ULONG					MaximumChannels;
	ULONG					PinDescriptorCount;
	ASIO_PIN_DESCRIPTOR		PinDescriptors[MAXIMUM_NUMBER_OF_PIN_DESCRIPTORS];
	ULONG					ActivePinMask;
	KSTIME					Latency;
	ULONG					ChannelDescriptorCount;
	ASIO_CHANNEL_DESCRIPTOR	ChannelDescriptors[64];
} FILTER_DATARANGE_ASIO, *PFILTER_DATARANGE_ASIO;

typedef struct
{
	BOOL	CanInputMonitor;
	BOOL	CanTimeInfo;
	BOOL	CanTimeCode;
	BOOL	CanTransport;
	BOOL	CanInputGain;
	BOOL	CanInputMeter;
	BOOL	CanOutputGain;
	BOOL	CanOutputMeter;
	BOOL	CanDoIoFormat;
} ASIO_DRIVER_CAPABILITIES, *PASIO_DRIVER_CAPABILITIES;

typedef struct
{
	BOOL	SupportTimeInfo;
	BOOL	SupportTimeCode;
	BOOL	SupportInputMonitor;
	BOOL	SupportInputGain;
	BOOL	SupportInputMeter;
	BOOL	SupportOutputGain;
	BOOL	SupportOutputMeter;
} ASIO_HOST_CAPABILITIES, *PASIO_HOST_CAPABILITIES;

#define SYNC_OPTION_SYNCHRONIZE_TO_RECORD_ONLY	0x1
#define SYNC_OPTION_FORCE_RECORD_CHANNELS		0x2

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CAsioDriver
 *****************************************************************************
 * @brief
 * ASIO driver object.
 */
class CAsioDriver 
:	public IAsioDriver,
	public CUnknown
{
private:
	GUID					m_ClassId;
	TCHAR					m_SymbolicLink[MAX_PATH];
	TCHAR					m_FriendlyName[MAX_PATH];
	HWND					m_WndMain;
	DRIVER_STATE			m_DriverState;

	CKsAudioRenderFilter *	m_AudioRenderFilter;
	CKsAudioCaptureFilter *	m_AudioCaptureFilter;

	GUID					m_XuPropSetClockRate;
	GUID					m_XuPropSetClockSource;
	GUID					m_XuPropSetDirectMonitor;
	GUID					m_XuPropSetDriverResync;

	ULONG					m_XuClockRateNodeId;
	ULONG					m_XuClockSourceNodeId;
	ULONG					m_XuDirectMonitorNodeId;
	ULONG					m_XuDriverResyncNodeId;

	CHAR					m_ProductIdentifier[64];

	BOOL					m_PerApplicationPreferences;

	ULONG					m_SampleSize;
	ULONG					m_SamplingFrequency;

	ULONG					m_PreferredSampleSize; 

	ULONGLONG				m_PreferredBufferSize; // in 100ns unit

	ULONG					m_BufferSizeInSamples;

	BOOL					m_SampleRateDidChange;

	static ULONG			m_PossibleSampleRates[6];

	static DOUBLE			m_SupportedBufferSizes[];
	ULONG					m_MinimumBufferSizeArrayOffset;

	ULONG					m_NumberOfOutputBuffers;

	FILTER_DATARANGE_ASIO	m_FilterDataRanges[2][SIZEOF_ARRAY(m_PossibleSampleRates)];

	PFILTER_DATARANGE_ASIO	m_SelectedDataRangeAsio[2];
	PFILTER_DATARANGE_ASIO	m_PreparedDataRangeAsio[2];

	ASIOCallbacks			m_AsioCallbacks;

	HANDLE					m_AsioControlEvent[ASIO_CONTROL_EVENT_COUNT];
	HANDLE					m_WatchDogEvent[ASIO_WATCHDOG_EVENT_COUNT];

	HANDLE					m_IoThreadHandle[2];

	HANDLE					m_AsioNodeEvent[ASIO_NODE_EVENT_COUNT];
	KSEVENTDATA				m_AsioNodeEventData[ASIO_NODE_EVENT_COUNT];

	HANDLE					m_NodeThreadHandle;

	HANDLE					m_StateTransitionEvent;

	ULONGLONG				m_CurrentSamplePosition;
	ULONGLONG				m_CurrentTimeStamp;

	ULONGLONG				m_RunningSamplePosition;
	ULONGLONG				m_RunningTimeStamp;

	ULONGLONG				m_BufferSwitchCount;

	ASIO_DRIVER_CAPABILITIES	m_DriverCapabilities;	// What the driver can do...
	ASIO_HOST_CAPABILITIES		m_HostCapabilities;		// What the host supports...

	// What is communicated...
	BOOL						m_TimeCodeReadEnabled;

	// App hacks...
	BOOL					m_EnableResyncRequest;

	/*************************************************************************
     * CAsioDriver private methods
     */
	ULONG _FindXuNode
	(
		IN		REFGUID	PropertySet
	);

	HRESULT _GetClockRate
	(
		OUT		ULONG *	OutClockRate
	);

	HRESULT _SetClockRate
	(
		IN		ULONG	ClockRate
	);

	HRESULT _GetClockSource
	(
		OUT		ULONG *	OutClockSource
	);

	HRESULT _SetClockSource
	(
		IN		ULONG	ClockSource
	);

	HRESULT _SetInputMonitor
	(
		IN		ASIOInputMonitor *	InputMonitor
	);

	BOOL _IsDeviceInUse
	(	void
	);

	HRESULT _BuildFilterDataRanges
	(
		IN		BOOL					Capture,
		IN		PFILTER_DATARANGE_ASIO	FilterDataRanges
	);

	ASIOError _AllocateBuffers
	(	
		IN		BOOL				Input,
		IN		ASIOBufferInfo *	BufferInfos,
		IN		LONG				NumChannels,
		IN		LONG				BufferSizeInSamples
	);

	ASIOError _DisposeBuffers
	(	
		IN		BOOL	Input
	);

	ASIOError _AllocateIoThreads
	(	void	
	);

	ASIOError _DisposeIoThreads
	(	void	
	);

	ASIOError _DetermineDriverCapabilities
	(	void	
	);

	ASIOError _DetermineHostCapabilities
	(	void	
	);

	VOID _NodeThreadHandler
	(	void	
	);

	VOID _MainThreadHandler
	(	void	
	);

	VOID _WatchDogThreadHandler
	(	void	
	);

	VOID _SyncModifyPinState
	(
		IN		PFILTER_DATARANGE_ASIO *	DataRangeAsio,
		IN		KSSTATE						NewState
	);

	VOID _SyncWritePinData
	(
		IN		PFILTER_DATARANGE_ASIO	DataRangeAsio,
		IN		ULONG					PacketIndex,
		IN		ULONG					BufferIndex
	);

	VOID _SyncReadPinData
	(
		IN		PFILTER_DATARANGE_ASIO	DataRangeAsio,
		IN		ULONG					PacketIndex,
		IN		ULONG					BufferIndex
	);

	VOID _SyncZeroPinBuffer
	(
		IN		PFILTER_DATARANGE_ASIO *	DataRangeAsio,
		IN		ULONG						PacketIndex
	);

	VOID _SyncQueuePinBuffer
	(
		IN		ULONG						Ops,
		IN		PFILTER_DATARANGE_ASIO *	DataRangeAsio,
		IN		ULONG						PacketIndex
	);

	VOID _SwitchBuffer
	(
		IN		ULONG	BufferIndex
	);

	LONG _AsioMessage
	(	
		IN		LONG		Selector, 
		IN		LONG		Value, 
		IN		PVOID		Message, 
		IN		DOUBLE *	Optional
	);

	VOID _SampleRateDidChange
	(	
		IN		ASIOSampleRate	SampleRate
	);

	VOID _SnapshotTimeStamp
	(
		OUT		ULONGLONG *	OutTimeStamp
	);

	VOID _SaveDriverSettingsToRegistry
	(	void
	);

	VOID _RestoreDriverSettingsFromRegistry
	(	void
	);

	ULONG _GetNumberOfOutputBuffers
	(
		IN		ULONGLONG	BufferSize
	);

	ULONG _GetSyncOption
	(	void
	);

	VOID _GetAppHacks
	(	void
	);

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CAsioDriver);
    ~CAsioDriver();

    /*****************************************************************************
     * IAsioDriver implementation
     *
     * This macro is from IASIODRV.H.  It lists all the interface's functions.
     */
    IMP_IAsioDriver;

	/*************************************************************************
     * CAsioDriver public methods
     */
	HRESULT Setup
	(
		IN		REFCLSID	RefClsId
	);

	/*************************************************************************
     * Static
     */
	static
	DWORD WINAPI NodeThreadRoutine
	(
		IN		LPVOID	Parameter
	);

	static
	DWORD WINAPI MainThreadRoutine
	(
		IN		LPVOID	Parameter
	);

	static
	DWORD WINAPI WatchDogThreadRoutine
	(
		IN		LPVOID	Parameter
	);

	/*************************************************************************
     * Friends
     */
};

#endif // _ASIO_DRIVER_H_

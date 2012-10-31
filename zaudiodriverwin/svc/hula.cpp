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
 * @file       hula.cpp
 * @brief      Hula service implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  06-01-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbt.h>
#include <strsafe.h>
#include <setupapi.h>
#include <ks.h>
#include <ksmedia.h>

#include "dbg.h"
#include "listmacro.h"

#include <winioctl.h>
#include <initguid.h>
#include "PrvProp.h"
#include "ExtProp.h"
#include <usb.h>

#include <atlbase.h>
#include "audiopropstore.h"
#include "mmdeviceapi.h"
#include "AudioPropStoreWrapper.h"
#include "policyconfigclientidl.h"

#define STR_MODULENAME "emaudsv: "


/*****************************************************************************
 * Defines.
 */
#define HULA_SERVICE_NAME	"emaudsv"

typedef struct _DEVICE_INFO
{
	WCHAR				DevicePath[MAX_PATH];		// Symbolic link path.
	WCHAR				DeviceName[MAX_PATH];		// Friendly name.
	HANDLE				DeviceHandle;				// Handle returned by CreateFile().
	HANDLE				NotificationHandle;			// Handle returned by RegisterDeviceNotfication().
	WCHAR				RegisterDlls[MAX_PATH];		// List of DLLs to register.
	WCHAR				UnregisterDlls[MAX_PATH];	// List of DLLs to unregister.
	LIST_ENTRY			ListEntry;					// Linked list entry.
} DEVICE_INFO, *PDEVICE_INFO;

/************************************************************************/
/* Global Variables.
*/
	// Device Descriptor that maintains vendor id and other usb related stuff.
	USB_DEVICE_DESCRIPTOR	m_deviceDescriptor;
	
	// Handle of the device driver opened to communicate with the device.
	HANDLE	m_hDevice;
	
	// GUIDs of the property sets that are implemented in the extension units. They are
	// Clock Rate, Clock Source, Digital Input Status
	GUID		m_ClockRatePropertySet;
	
	// Node IDs of the above said property sets that are implemented in the extension units.
	ULONG		m_ClockRateNodeId;
	
	// Events and Thread handles that is used for notifications and synchronization.
	HANDLE		m_EventPool[2];
	KSEVENTDATA	m_EventData[1];
	HANDLE		m_EventThreadHandle;

/*****************************************************************************
 * Forward declarations.
 */
VOID 
ServiceMain
(
	IN		DWORD	Argc, 
	IN		LPSTR * Argv
);

DWORD 
ControlHandler
(
	IN		DWORD	Control,
	IN		DWORD	EventType,
	IN		PVOID	EventData,
	IN		PVOID	Context
);

DWORD 
InitService
(	void
);

DWORD 
StartService
(	void
);

BOOL 
RegisterDlls
(
	IN		PDEVICE_INFO	DeviceInfo,
	IN		BOOL			Register
);

VOID
EnumExistingDevices
(
	IN		LPGUID	InterfaceGuid
);

DWORD
HandleDeviceInterfaceChange
(
	IN		DWORD							EventType,
	IN		PDEV_BROADCAST_DEVICEINTERFACE	BroadcastInterface
);

DWORD
HandleDeviceChange
(
	IN		DWORD					EventType,
	IN		PDEV_BROADCAST_HANDLE	BroadcastHandle
);

BOOL
GetOSVersion
(	void
);

BOOL
Initialize
(
	IN		WCHAR* pDevicePath
);

BOOL
InitializeDescriptor
(	void
);

DWORD
GetDeviceClockRate
(	void
);

BOOL 
SyncVistaEndPointSettings
(	void
);

HRESULT
GetNodePropertySimple
(
	IN		ULONG NodeId,
	IN		REFGUID PropertySet,
	IN		ULONG PropertyId,
	OUT		PVOID Value, 
	IN		ULONG ValueSize, 
	IN		PVOID Instance=NULL,
	IN		ULONG InstanceSize=0
);

HRESULT
SynchronizedIoctl
(
	IN		HANDLE	Handle,
	IN		ULONG	CtlCode,
	IN		PVOID	InputBuffer,
	IN		ULONG InputBufferSize,
	OUT		PVOID	OutputBuffer, 
	OUT		ULONG	OutputBufferSize,
	OUT		ULONG *	OutBytesReturned
);

HRESULT
GetPropertyMulti
(
	IN		REFGUID PropertySet,
	IN		ULONG PropertyId, 
	OUT		PKSMULTIPLE_ITEM *OutKsMultipleItem
);

HRESULT
FindNode
(
	IN		REFGUID PropertySet,
	OUT		ULONG *OutNodeId
);

BOOL
IsValidHandle
(
	IN		HANDLE Handle
);

HRESULT
KsEnableNodeEvent
(
	IN		ULONG NodeId,
	IN		REFGUID	EventSet, 
	IN		ULONG EventId,
	IN		PKSEVENTDATA EventData, 
	IN		ULONG EventDataSize
);

BOOL
SafeCloseHandle
(
	IN		HANDLE& Handle
);

DWORD WINAPI
EventThreadRoutine
(
	IN		LPVOID	Parameter
);

VOID
EventThreadHandler
(	void
);

#ifdef __cplusplus
extern "C" {
#endif

// Local functions
enum eEndpointType
{
	ep_Playback,
	ep_Record
};

// Local functions
int
GetPlayBackEndptDeviceIndex
(
	IN		LPWSTR	deviceName
);

int
GetRecordEndptDeviceIndex
(
	IN		LPWSTR	deviceName
);

BOOL
SetPlayBackEndPtVistaSR
(
	IN		int		deviceID,
	IN		UINT	sampleRate,
	IN		UINT	numBits
);

BOOL
GetPlayBackEndPtVistaSR
(
	IN		int deviceID,
	OUT		UINT *sampleRate,
	OUT		UINT *numBits
);

BOOL
SetRecordEndPtVistaSR
(
	IN		int deviceID,
	IN		UINT sampleRate,
	IN		UINT numBits
);

BOOL
GetRecordEndPtVistaSR
(
	IN		int deviceID,
	OUT		UINT *sampleRate,
	OUT		UINT *numBits
);

int
GetDeviceIndex
(
	IN		eEndpointType type,
	IN		LPWSTR deviceName
);

BOOL
SetVistaSR
(
	IN		eEndpointType type,
	IN		int deviceID,
	IN		UINT sampleRate,
	IN		UINT numBits
);

BOOL
GetVistaSR
(
	IN		eEndpointType type,
	IN		int deviceID,
	OUT		UINT *sampleRate,
	OUT		UINT *numBits
);

#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * ServiceStatus
 *****************************************************************************
 */
static
SERVICE_STATUS ServiceStatus; 

/*****************************************************************************
 * ServiceStatusHandle
 *****************************************************************************
 */
static
SERVICE_STATUS_HANDLE ServiceStatusHandle = NULL; 

/*****************************************************************************
 * DeviceInterfaceNotificationHandle
 *****************************************************************************
 */
static
HDEVNOTIFY DeviceInterfaceNotificationHandle = NULL;

/*****************************************************************************
 * ListHead
 *****************************************************************************
 */
static
LIST_ENTRY ListHead;

/*****************************************************************************
 * ServiceTable[]
 *****************************************************************************
 */
static
SERVICE_TABLE_ENTRY ServiceTable[] = 
{
	{ HULA_SERVICE_NAME, ServiceMain },
	{ NULL, NULL}
};

/*****************************************************************************
 * main()
 *****************************************************************************
 *//*!
 * @brief
 * Main entry point for this program.
 */
void main()
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("Main entry point"));

    // Start the control dispatcher thread for our service.
    StartServiceCtrlDispatcher(ServiceTable);  
}

BOOL TimeBombCheck(void)
{

	return TRUE;
}

/*****************************************************************************
 * ServiceMain()
 *****************************************************************************
 *//*!
 * @brief
 * Starting point for a service.
 */
VOID 
ServiceMain
(
	IN		DWORD	Argc, 
	IN		LPSTR * Argv
) 
{ 
	_DbgPrintF(DEBUGLVL_VERBOSE,("[ServiceMain]"));

	DWORD w32Error = ERROR_SUCCESS;

	ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    ServiceStatusHandle = RegisterServiceCtrlHandlerEx(HULA_SERVICE_NAME, ControlHandler, NULL); 

    if (ServiceStatusHandle) 
    { 
		// Initialize Service 
		w32Error = InitService(); 

		if (w32Error == ERROR_SUCCESS) 
		{   
			// We report the running status to SCM. 
			ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
		}
		else
		{
			// Initialization failed
			ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
			ServiceStatus.dwWin32ExitCode = -1; 
		}

		SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
	}
	else
	{
		w32Error = ERROR_INVALID_HANDLE;
	}

	if (w32Error == ERROR_SUCCESS)
	{
		// Service is up and running. We can now call system function, so start the service
		// and go.
		StartService();
	}
}

/*****************************************************************************
 * InitService()
 *****************************************************************************
 *//*!
 * @brief
 * Service initialization.
 */
DWORD 
InitService
(	void
) 
{ 
	_DbgPrintF(DEBUGLVL_VERBOSE,("[InitService]"));

	// Nothing much can be done here because we are not suppose to call any
	// system function during service initialization.

	return ERROR_SUCCESS; 
} 

/*****************************************************************************
 * StartService()
 *****************************************************************************
 *//*!
 * @brief
 * Service initialization.
 */
DWORD 
StartService
(	void
) 
{ 
	_DbgPrintF(DEBUGLVL_VERBOSE,("[StartService]"));

	InitializeListHead(&ListHead);

	EnumExistingDevices((LPGUID)&KSCATEGORY_AUDIO);

	DEV_BROADCAST_DEVICEINTERFACE BroadcastInterface;
	BroadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	BroadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	BroadcastInterface.dbcc_classguid = KSCATEGORY_AUDIO;

	DeviceInterfaceNotificationHandle = RegisterDeviceNotification(ServiceStatusHandle, &BroadcastInterface, DEVICE_NOTIFY_SERVICE_HANDLE);

	//if OS is Vista do this
	if( GetOSVersion() )
	{
		//If Vista, once starting service, run synch up proc
		for (PLIST_ENTRY Entry = ListHead.Flink; Entry != &ListHead; Entry = Entry->Flink)
		{
			PDEVICE_INFO Info = CONTAINING_RECORD(Entry, DEVICE_INFO, ListEntry);

			if( wcsstr(Info->DevicePath, L"audio") || wcsstr(Info->DevicePath, L"Audio" ) )
			{
				BOOL bSuccess = Initialize(Info->DevicePath);

				if(bSuccess)
					SyncVistaEndPointSettings() ;
				
				break;
			}
		}
	}

	return ERROR_SUCCESS; 
} 

/*****************************************************************************
 * StopService()
 *****************************************************************************
 *//*!
 * @brief
 * Service clean up.
 */
VOID 
StopService
(	void
) 
{ 
	_DbgPrintF(DEBUGLVL_VERBOSE,("[StopService]"));

	if( GetOSVersion() )
	{
		//If service is stopped, device handle should be closed
		if (IsValidHandle(m_hDevice))
			CloseHandle(m_hDevice);
	}	
    PDEVICE_INFO DeviceInfo = NULL;

	// Clean up the device list.
	while (!IsListEmpty(&ListHead)) 
	{
        PLIST_ENTRY Entry = RemoveHeadList(&ListHead);

        PDEVICE_INFO DeviceInfo = CONTAINING_RECORD(Entry, DEVICE_INFO, ListEntry);

		RegisterDlls(DeviceInfo, FALSE);

		if (DeviceInfo->NotificationHandle) 
		{
            UnregisterDeviceNotification(DeviceInfo->NotificationHandle);

            DeviceInfo->NotificationHandle = NULL;
        }

        if ((DeviceInfo->DeviceHandle != INVALID_HANDLE_VALUE) && (DeviceInfo->DeviceHandle != NULL))
		{
            CloseHandle(DeviceInfo->DeviceHandle);

            DeviceInfo->DeviceHandle = INVALID_HANDLE_VALUE;
        }

        HeapFree(GetProcessHeap(), 0, DeviceInfo);
    }

	// Unregister the interface notification handle.
	if (DeviceInterfaceNotificationHandle)
	{
		UnregisterDeviceNotification(DeviceInterfaceNotificationHandle);

		DeviceInterfaceNotificationHandle = NULL;
	}
} 

/*****************************************************************************
 * ControlHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Control handler function for a service.
 */
DWORD 
ControlHandler
(
	IN		DWORD	Control,
	IN		DWORD	EventType,
	IN		PVOID	EventData,
	IN		PVOID	Context
)
{ 
	_DbgPrintF(DEBUGLVL_VERBOSE,("[ControlHandler]"));

	DWORD w32Error = ERROR_SUCCESS;

	switch (Control) 
    { 
        case SERVICE_CONTROL_STOP: 
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[ControlHandler] - SERVICE_CONTROL_STOP"));

			StopService();

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
		}
		break;
 
        case SERVICE_CONTROL_SHUTDOWN: 
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[ControlHandler] - SERVICE_CONTROL_SHUTDOWN"));

			StopService();

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
		}
		break;

		case SERVICE_CONTROL_DEVICEEVENT:
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[ControlHandler] - SERVICE_CONTROL_DEVICEEVENT"));

			PDEV_BROADCAST_HDR BroadcastHdr = PDEV_BROADCAST_HDR(EventData);

            if (BroadcastHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) 
			{
                HandleDeviceInterfaceChange(EventType, PDEV_BROADCAST_DEVICEINTERFACE(BroadcastHdr));

            } 
			else if (BroadcastHdr->dbch_devicetype == DBT_DEVTYP_HANDLE) 
			{
                HandleDeviceChange(EventType, PDEV_BROADCAST_HANDLE(BroadcastHdr));
            }

			//if don't do below function, mismatch occurs while switching HulaPod on/off
			if( GetOSVersion() )
				SyncVistaEndPointSettings();			
		}
		break;
        
        default:
		{	
			_DbgPrintF(DEBUGLVL_VERBOSE,("[ControlHandler] - 0x%x", Control));

			w32Error = ERROR_CALL_NOT_IMPLEMENTED;
		}
        break;
    } 
 
    // Report current status
    SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	return w32Error;
} 

/*****************************************************************************
 * ReadDllSectionParameters()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
GetPathName
(
	IN		LONG	DirId,
	IN OUT	LPWSTR	PathName,
	IN		ULONG	PathNameLength
)
{
	BOOL Success = FALSE;

	switch (DirId)
	{
		case DIRID_ABSOLUTE:
		{
			// Absolute path.
			PathName[0] = '\0';

			Success = TRUE;
		}
		break;

		case DIRID_WINDOWS:
		{
			// Windows directory path, ie C:\Windows.
			Success = GetWindowsDirectoryW(PathName, PathNameLength) != 0;
		}
		break;

		case DIRID_SYSTEM:
		{
			// System directory path, ie C:\Windows\System32.
			Success = GetSystemDirectoryW(PathName, PathNameLength) != 0;
		}
		break;

		case DIRID_DRIVERS:
		{
			// Drivers directory path, ie C:\Windows\System32\Drivers.
			Success = GetSystemDirectoryW(PathName, PathNameLength) != 0;

			StringCchCatW(PathName, PathNameLength, L"\\Drivers");
		}
		break;

		case DIRID_INF:
		{
			// INF directory path, ie C:\Windows\inf.
			Success = GetWindowsDirectoryW(PathName, PathNameLength) != 0;

			StringCchCatW(PathName, PathNameLength, L"\\inf");
		}
		break;

		case DIRID_HELP:
		{
			// Help directory path, ie C:\Windows\Help.
			Success = GetWindowsDirectoryW(PathName, PathNameLength) != 0;

			StringCchCatW(PathName, PathNameLength, L"\\Help");
		}
		break;

		case DIRID_FONTS:
		{
			// Fonts directory path, ie C:\Windows\inf.
			Success = GetWindowsDirectoryW(PathName, PathNameLength) != 0;

			StringCchCatW(PathName, PathNameLength, L"\\Fonts");
		}
		break;

		case DIRID_APPS:
		{
			// Root directory path, ie C:.
			Success = GetWindowsDirectoryW(PathName, PathNameLength) != 0;

			PWCHAR p = wcschr(PathName, '\\'); if (p) { *p = '\0'; }
		}
		break;

		default:
		{
			if (DirId >= 0x4000)
			{
				INT CsIdl = DirId - 0x4000;

				if (PathNameLength >= MAX_PATH)
				{
					Success = SUCCEEDED(SHGetFolderPathW(NULL, CsIdl, NULL, SHGFP_TYPE_DEFAULT, PathName));
				}
			}
			else
			{
				PathName[0] = '\0';
			}
		}
		break;
	}

	return Success;
}

/*****************************************************************************
 * ReadDllSectionParameters()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
ReadDllSectionParameters
(
	IN		LPWSTR	DevicePath, 
	IN		LPWSTR	DllSection,
	IN		BOOL	Register,
	IN OUT	LPWSTR	PathName,
	IN OUT	LPWSTR	FileName,
	OUT		BOOL *	OutIsExe,
	OUT		ULONG *	OutRegistrationFlags,
	OUT		ULONG *	OutTimeOut	OPTIONAL,
	IN OUT	LPWSTR	Argument	OPTIONAL
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[ReadDllSectionParameters]"));

	if (OutTimeOut)
	{
		*OutTimeOut = 60; // 60s
	}

	if (Argument)
	{
		*Argument = '\0';
	}

	BOOL Success = FALSE;

    HDEVINFO DeviceInfo = SetupDiCreateDeviceInfoList(NULL, NULL);

    if (DeviceInfo != INVALID_HANDLE_VALUE)
    {
		SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		SetupDiOpenDeviceInterfaceW(DeviceInfo, DevicePath,	0, &DeviceInterfaceData);
	                  
		SP_DEVINFO_DATA DeviceInfoData;
		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		if (SetupDiGetDeviceInterfaceDetail(DeviceInfo, &DeviceInterfaceData, NULL, 0, NULL, &DeviceInfoData) || (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			//
			// Get the friendly name for this device interface, if available. If not, then get the 
			// device friendly name, and if that fails try to get the device description.
			//
			HKEY RegistryKey = SetupDiOpenDeviceInterfaceRegKey
								(
									DeviceInfo,
									&DeviceInterfaceData,
									0,
									KEY_ALL_ACCESS
								);

			if (RegistryKey)
			{
				// DLLs
				HKEY DllKey;
				
				if (ERROR_SUCCESS == RegOpenKeyEx(RegistryKey, "Dll", 0, KEY_ALL_ACCESS, &DllKey))
				{
					// Register/Unregister
					HKEY RegisterKey;

					if (ERROR_SUCCESS == RegOpenKeyEx(DllKey, Register ? "Register" : "Unregister", 0, KEY_ALL_ACCESS, &RegisterKey))
					{
						ULONG RegType = 0; ULONG Size = MAX_PATH * sizeof(WCHAR);

						WCHAR ParametersList[MAX_PATH];

						if (ERROR_SUCCESS == RegQueryValueExW(RegisterKey, DllSection, NULL, &RegType, (BYTE*)ParametersList, &Size))
						{
							_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - %ws: %ws", DllSection, ParametersList));

							ULONG ParametersCount = 0;

							WCHAR Parameter[MAX_PATH];

							BOOL KeepBlanks = FALSE;

							ULONG N = wcslen(ParametersList), p = 0, i = 0;

							while (p <= N)
							{
								if ((ParametersList[p] == ' ') && (!KeepBlanks))
								{
									// Ignore blanks.
								}
								else if ((ParametersList[p] == ',') || (ParametersList[p] == 0))
								{
									// Got a parameter. Null terminate it.
									Parameter[i] = 0;

									_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - Parameter: %ws", Parameter));

									switch (ParametersCount)
									{
										case 0:
										{
											LONG DirId = wcstol(Parameter, NULL, 0);

											if (PathName)
											{
												GetPathName(DirId, PathName, MAX_PATH);
											}

											_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - DirId: %d: %ws", DirId, PathName));
										}
										break;

										case 1:
										{
											PWCHAR SubDir = Parameter;

											if (PathName)
											{
												if (wcslen(SubDir))
												{
													StringCchCatW(PathName, MAX_PATH, L"\\");
													StringCchCatW(PathName, MAX_PATH, SubDir);
												}
											}

											_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - SubDir: %ws", SubDir));
										}
										break;

										case 2:
										{
											BOOL IsExe = FALSE;

											if (FileName)
											{
												StringCchCopyW(FileName, MAX_PATH, Parameter);

												IsExe = wcsstr(FileName, L".exe") != NULL;
											}

											if (OutIsExe)
											{
												*OutIsExe = IsExe;
											}

											if (IsExe)
											{
												if (Argument)
												{
													StringCchCopyW(Argument, MAX_PATH, Register ? L"/RegServer" : L"/UnRegServer"); 
												}
											}

											_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - FileName: %ws", FileName));
										}
										break;

										case 3:
										{
											ULONG RegistrationFlags = wcstoul(Parameter, NULL, 0);

											if (OutRegistrationFlags)
											{
												*OutRegistrationFlags = RegistrationFlags;
											}

											_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - RegistrationFlags: 0x%x", RegistrationFlags));
										}
										break;

										case 4:
										{
											ULONG TimeOut = wcstoul(Parameter, NULL, 0);

											if (OutTimeOut)
											{
												*OutTimeOut = TimeOut;
											}

											_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - TimeOut: %d", TimeOut));
										}
										break;

										case 5:
										{
											if (Argument)
											{
												StringCchCopyW(Argument, MAX_PATH, Parameter);
											}

											_DbgPrintF(DEBUGLVL_BLAB,("[ReadDllSectionParameters] - Argument: %ws", Argument));
										}
										break;

										default:
											break;
									}

									ParametersCount++;

									// Done with current section.
									i = 0;

									KeepBlanks = FALSE;
								}
								else
								{
									if (ParametersList[p] == '"')
									{
										// For string quotes...
										KeepBlanks = !KeepBlanks;
									}
									else
									{
										Parameter[i] = ParametersList[p]; i++;
									}
								}

								p++;
							}

							Success = TRUE;
						}

						RegCloseKey(RegisterKey);
					}

					RegCloseKey(DllKey);
				}

				RegCloseKey(RegistryKey);
			}
		}

		SetupDiDestroyDeviceInfoList(DeviceInfo);
	}

    return Success;
}

/*****************************************************************************
 * ExecuteProgram()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
ExecuteProgram
(
	IN		LPWSTR	ApplicationName,
	IN		LPWSTR	CommandLine,
	IN		LPWSTR	CurrentDirectory,
	IN		ULONG	TimeOut
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[ExecuteProgram]"));

	STARTUPINFOW StartupInfo;

    ZeroMemory(&StartupInfo, sizeof(STARTUPINFOW));
    StartupInfo.cb = sizeof(STARTUPINFOW);

    PROCESS_INFORMATION ProcessInfo;

    ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));

    BOOL Process = CreateProcessW
                    (
                        ApplicationName,
                        CommandLine,
                        NULL,
                        NULL,
                        FALSE,
                        DETACHED_PROCESS,
                        NULL,
                        CurrentDirectory,
                        &StartupInfo,
                        &ProcessInfo
                    );


	_DbgPrintF(DEBUGLVL_VERBOSE,("[ExecuteProgram] - Process: %d, %d", Process));

	if (Process)
    {
        WaitForSingleObject(ProcessInfo.hProcess, TimeOut * 1000);

        CloseHandle(ProcessInfo.hProcess);

        CloseHandle(ProcessInfo.hThread);
    }

	return Process;
}

#define FLG_REGSVR_DEVICEPATH	0x40000000
#define FLG_REGSVR_WINDOWS64	0x80000000

/*****************************************************************************
 * RegisterDlls()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
RegisterDlls
(
	IN		PDEVICE_INFO	DeviceInfo,
	IN		BOOL			Register
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[RegisterDlls]"));

	PWCHAR DllSectionsList = Register ? DeviceInfo->RegisterDlls : DeviceInfo->UnregisterDlls;

	_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - %ws, Register : %d", DllSectionsList, Register));

	ULONG N = wcslen(DllSectionsList);

	if (N)
	{
		WCHAR DllSection[MAX_PATH];

		ULONG p = 0, i = 0;

		while (p <= N)
		{
			if (DllSectionsList[p] == ' ')
			{
				// Ignore blanks.
			}
			else if ((DllSectionsList[p] == ',') || (DllSectionsList[p] == 0))
			{
				// Got a section. Null terminate it.
				DllSection[i] = 0;

				_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - DllSection: %ws", DllSection));

				if (wcslen(DllSection))
				{
					WCHAR PathName[MAX_PATH], FileName[MAX_PATH], Argument[MAX_PATH];

					BOOL IsExe = FALSE;

					ULONG RegistrationFlags = 0, TimeOut = 0;

					if (ReadDllSectionParameters(DeviceInfo->DevicePath, DllSection, Register, PathName, FileName, &IsExe, &RegistrationFlags, &TimeOut, Argument))
					{
						// Do something useful here...
						_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - PathName: %ws", PathName));
						_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - FileName: %ws", FileName));
						_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - IsExe: %d", IsExe));
						_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - RegistrationFlags: 0x%x", RegistrationFlags));
						_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - TimeOut: %d", TimeOut));
						_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - Argument: %ws", Argument));

						#if !defined(_WIN64)
						if (!(RegistrationFlags & FLG_REGSVR_WINDOWS64))
						#endif // !defined(_WIN64)
						if (IsExe)
						{
							// Usage: exename cmdline
							WCHAR ApplicationName[MAX_PATH];
						
							StringCchCopyW(ApplicationName, MAX_PATH, PathName); 
							StringCchCatW(ApplicationName, MAX_PATH, L"\\");					
							StringCchCatW(ApplicationName, MAX_PATH, FileName);					

							WCHAR CommandLine[MAX_PATH * 2];

							// The argv[0] is the module name, so repeat the module name as the first token 
							// in the command line.
							StringCchCopyW(CommandLine, MAX_PATH * 2, FileName);

							StringCchCatW(CommandLine, MAX_PATH * 2, L" ");					
							StringCchCatW(CommandLine, MAX_PATH * 2, Argument);

							if (RegistrationFlags & FLG_REGSVR_DEVICEPATH)
							{
								StringCchCatW(CommandLine, MAX_PATH * 2, L" ");					
								StringCchCatW(CommandLine, MAX_PATH * 2, L"/DevPath ");
								StringCchCatW(CommandLine, MAX_PATH * 2, DeviceInfo->DevicePath);
							}

							_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - %ws %ws", ApplicationName, CommandLine));

							ExecuteProgram(ApplicationName, CommandLine, PathName, TimeOut);
						}
						else
						{
							// Usage: regsvr32 [/u][/s][/n][/i[:cmdline[]] dllname
							// /u -		Unregister server
							// /s -		Silent; display no message boxes
							// /i -		Call DllInstall passing it an optional [cmdline]; when used with /u calls dll uninstall
							// /n -		Do not call DllRegisterServer; this option must be used with /i

							WCHAR CommandLine[MAX_PATH * 2];
							
							// The argv[0] is the module name, so repeat the module name as the first token 
							// in the command line.
							StringCchCopyW(CommandLine, MAX_PATH * 2, L"regsvr32.exe /s"); 

							if (!Register)
							{
								StringCchCatW(CommandLine, MAX_PATH * 2, L" /u");
							}

							if (RegistrationFlags & FLG_REGSVR_DLLINSTALL)
							{
								if (!(RegistrationFlags & FLG_REGSVR_DLLREGISTER))
								{
									StringCchCatW(CommandLine, MAX_PATH * 2, L" /n");
								}

								StringCchCatW(CommandLine, MAX_PATH * 2, L" /i:\"");
								if (wcslen(Argument))
								{
									StringCchCatW(CommandLine, MAX_PATH * 2, Argument);
									StringCchCatW(CommandLine, MAX_PATH * 2, L" ");
								}

								if (RegistrationFlags & FLG_REGSVR_DEVICEPATH)
								{
									StringCchCatW(CommandLine, MAX_PATH * 2, L"/DevPath ");
									StringCchCatW(CommandLine, MAX_PATH * 2, DeviceInfo->DevicePath);
								}
								StringCchCatW(CommandLine, MAX_PATH * 2, L"\"");
							}

							StringCchCatW(CommandLine, MAX_PATH * 2, L" ");
							StringCchCatW(CommandLine, MAX_PATH * 2, PathName);
							StringCchCatW(CommandLine, MAX_PATH * 2, L"\\");
							StringCchCatW(CommandLine, MAX_PATH * 2, FileName);

							_DbgPrintF(DEBUGLVL_BLAB,("[RegisterDlls] - regsvr32.exe %ws", CommandLine));

							ExecuteProgram(L"regsvr32.exe", CommandLine, NULL, TimeOut);
						}
					}
				}

				// Done with current section.
				i = 0;
			}
			else
			{
				DllSection[i] = DllSectionsList[p]; i++;
			}

			p++;
		}
	}

	return TRUE;
}

/*****************************************************************************
 * GetDeviceInformation()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
GetDeviceInformation
(
	IN		LPWSTR		DevicePath, 
    IN		LPWSTR		DeviceDescription,
	IN		LPWSTR		RegisterDlls,
	IN		LPWSTR		UnregisterDlls
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[GetDeviceInformation]"));

	BOOL Success = FALSE;

    HDEVINFO DeviceInfo = SetupDiCreateDeviceInfoList(NULL, NULL);

    if (DeviceInfo != INVALID_HANDLE_VALUE)
    {
		SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		SetupDiOpenDeviceInterfaceW(DeviceInfo, DevicePath,	0, &DeviceInterfaceData);
	                  
		SP_DEVINFO_DATA DeviceInfoData;
		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		if (SetupDiGetDeviceInterfaceDetail(DeviceInfo, &DeviceInterfaceData, NULL, 0, NULL, &DeviceInfoData) || (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			//
			// Get the friendly name for this device interface, if available. If not, then get the 
			// device friendly name, and if that fails try to get the device description.
			//
			HKEY RegistryKey = SetupDiOpenDeviceInterfaceRegKey
								(
									DeviceInfo,
									&DeviceInterfaceData,
									0,
									KEY_ALL_ACCESS
								);

			if (RegistryKey)
			{
				// Friendly name
				ULONG RegType = 0; ULONG Size = MAX_PATH * sizeof(WCHAR);

				Success = (ERROR_SUCCESS == RegQueryValueExW(RegistryKey, L"FriendlyName", NULL, &RegType, (BYTE*)DeviceDescription, &Size));
				
				if (!Success)
				{
					Success =  SetupDiGetDeviceRegistryPropertyW(DeviceInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, &RegType, (BYTE*)DeviceDescription, MAX_PATH * sizeof(WCHAR), NULL);
				}

				if (!Success)
				{
					Success = SetupDiGetDeviceRegistryPropertyW(DeviceInfo, &DeviceInfoData, SPDRP_DEVICEDESC, &RegType, (BYTE*)DeviceDescription, MAX_PATH * sizeof(WCHAR), NULL);
				}

				// DLLs
				HKEY DllKey;
				
				if (ERROR_SUCCESS == RegOpenKeyEx(RegistryKey, "Dll", 0, KEY_ALL_ACCESS, &DllKey))
				{
					// RegisterDlls
					HKEY RegisterKey;

					if (ERROR_SUCCESS == RegOpenKeyEx(DllKey, "Register", 0, KEY_ALL_ACCESS, &RegisterKey))
					{
						ULONG RegType = 0; ULONG Size = MAX_PATH * sizeof(WCHAR);

						RegQueryValueExW(RegisterKey, NULL, NULL, &RegType, (BYTE*)RegisterDlls, &Size);

						RegCloseKey(RegisterKey);
					}

					// UnregisterDlls
					HKEY UnregisterKey;

					if (ERROR_SUCCESS == RegOpenKeyEx(DllKey, "Unregister", 0, KEY_ALL_ACCESS, &UnregisterKey))
					{
						ULONG RegType = 0; ULONG Size = MAX_PATH * sizeof(WCHAR);

						RegQueryValueExW(UnregisterKey, NULL, NULL, &RegType, (BYTE*)UnregisterDlls, &Size);

						RegCloseKey(UnregisterKey);
					}

					RegCloseKey(DllKey);
				}

				RegCloseKey(RegistryKey);
			}
		}

		SetupDiDestroyDeviceInfoList(DeviceInfo);
	}

    return Success;
}

/*****************************************************************************
 * EnumExistingDevices()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
EnumExistingDevices
(
	IN		LPGUID	InterfaceGuid
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[EnumExistingDevices]"));

	HDEVINFO DeviceInfo = SetupDiGetClassDevs
							(
								InterfaceGuid,              
								NULL, // Define no enumerator (global)
								NULL, // Define no
								(DIGCF_PRESENT | // Only Devices present
								DIGCF_DEVICEINTERFACE) // Function class devices.
							);

    if (DeviceInfo != INVALID_HANDLE_VALUE)
    {
		//
		// Enumerate devices...
		//
	    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		for (ULONG i = 0; SetupDiEnumDeviceInterfaces(DeviceInfo, 0, InterfaceGuid, i, &DeviceInterfaceData); i++)
		{	                                 
			_DbgPrintF(DEBUGLVL_BLAB,("[EnumExistingDevices] - i: %d", i));

			//
			// Allocate a function class device data structure to 
			// receive the information about this particular device.
			//

			PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData = NULL;

			//
			// First find out required length of the buffer
			//
			ULONG RequiredLength = 0;

			if (SetupDiGetDeviceInterfaceDetailW(DeviceInfo, &DeviceInterfaceData, NULL, 0, &RequiredLength, NULL) || (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
			{
				DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, RequiredLength);

				DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

				if (SetupDiGetDeviceInterfaceDetailW(DeviceInfo, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredLength, &RequiredLength, NULL))
				{
					PDEVICE_INFO DeviceInfo = (PDEVICE_INFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DEVICE_INFO));

					if (DeviceInfo)
					{
						DWORD w32Error = ERROR_SUCCESS;

						_DbgPrintF(DEBUGLVL_VERBOSE,("[EnumExistingDevices] - DevicePath = %ws", DeviceInterfaceDetailData->DevicePath));

						if (FAILED(StringCchCopyW(DeviceInfo->DevicePath, MAX_PATH, DeviceInterfaceDetailData->DevicePath)))
						{
							w32Error = ERROR_INVALID_PARAMETER;
						}

						if (w32Error == ERROR_SUCCESS)
						{
							DeviceInfo->DeviceHandle = CreateFileW
														(
															DeviceInfo->DevicePath,
															GENERIC_READ | GENERIC_WRITE,
															0,
															NULL,
															OPEN_EXISTING,
															0,
															NULL
														);

							_DbgPrintF(DEBUGLVL_VERBOSE,("[EnumExistingDevices] - Device Handle = %p", DeviceInfo->DeviceHandle));

							if (DeviceInfo->DeviceHandle == INVALID_HANDLE_VALUE)
							{
								w32Error = GetLastError();
							}
						}

						if (w32Error == ERROR_SUCCESS)
						{
							DEV_BROADCAST_HANDLE BroadcastHandle;

							memset(&BroadcastHandle, 0, sizeof(DEV_BROADCAST_HANDLE));
							BroadcastHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
							BroadcastHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
							BroadcastHandle.dbch_handle = DeviceInfo->DeviceHandle;

							DeviceInfo->NotificationHandle = RegisterDeviceNotification(ServiceStatusHandle, &BroadcastHandle, DEVICE_NOTIFY_SERVICE_HANDLE);

							_DbgPrintF(DEBUGLVL_VERBOSE,("[EnumExistingDevices] - Notification Handle = %p", DeviceInfo->NotificationHandle));

							if (DeviceInfo->NotificationHandle == NULL)
							{
								w32Error = GetLastError();
							}
						}

						if (w32Error == ERROR_SUCCESS)
						{
							GetDeviceInformation(DeviceInfo->DevicePath, DeviceInfo->DeviceName, DeviceInfo->RegisterDlls, DeviceInfo->UnregisterDlls);

							_DbgPrintF(DEBUGLVL_VERBOSE,("[EnumExistingDevices] - DeviceName: %ws", DeviceInfo->DeviceName));

							InitializeListHead(&DeviceInfo->ListEntry);

							InsertTailList(&ListHead, &DeviceInfo->ListEntry);     

							RegisterDlls(DeviceInfo, TRUE);
						}

						if (w32Error != ERROR_SUCCESS)
						{
							// Cleanup the mess...
							if (DeviceInfo->NotificationHandle) 
							{
								UnregisterDeviceNotification(DeviceInfo->NotificationHandle);
							}

							if ((DeviceInfo->DeviceHandle != INVALID_HANDLE_VALUE) && (DeviceInfo->DeviceHandle != NULL))
							{
								CloseHandle(DeviceInfo->DeviceHandle);
							}

							HeapFree(GetProcessHeap(), 0, DeviceInfo);
						}
					}
				}
			}

			if (DeviceInterfaceDetailData)
			{
				HeapFree(GetProcessHeap(), 0, DeviceInterfaceDetailData);
			}	                
		} 

		SetupDiDestroyDeviceInfoList(DeviceInfo);
	}
}

/*****************************************************************************
 * HandleDeviceInterfaceChange()
 *****************************************************************************
 *//*!
 * @brief
 */
DWORD
HandleDeviceInterfaceChange
(
	IN		DWORD							EventType,
	IN		PDEV_BROADCAST_DEVICEINTERFACE	BroadcastInterface
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange]"));

	DWORD w32Error = ERROR_SUCCESS;

	switch (EventType)
	{
		case DBT_DEVICEARRIVAL:
		{
			// New device arrived. Open handle to the device and register notification of type DBT_DEVTYP_HANDLE.

			_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange] - DBT_DEVICEARRIVAL: %ws",  BroadcastInterface->dbcc_name));

			PDEVICE_INFO DeviceInfo = (PDEVICE_INFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DEVICE_INFO));

			if (DeviceInfo)
			{
				if (FAILED(StringCchCopyW(DeviceInfo->DevicePath, MAX_PATH, LPWSTR(BroadcastInterface->dbcc_name))))
				{
					w32Error = ERROR_INVALID_PARAMETER;
				}

				if (w32Error == ERROR_SUCCESS)
				{
					DeviceInfo->DeviceHandle = CreateFileW
												(
													DeviceInfo->DevicePath,
													GENERIC_READ | GENERIC_WRITE,
													0,
													NULL,
													OPEN_EXISTING,
													0,
													NULL
												);

					_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange] - DBT_DEVICEARRIVAL: DeviceHandle = %p",  DeviceInfo->DeviceHandle));

					if (DeviceInfo->DeviceHandle == INVALID_HANDLE_VALUE)
					{
						w32Error = GetLastError();
					}
				}

				if (w32Error == ERROR_SUCCESS)
				{
					DEV_BROADCAST_HANDLE BroadcastHandle;

					memset(&BroadcastHandle, 0, sizeof(DEV_BROADCAST_HANDLE));
					BroadcastHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
					BroadcastHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
					BroadcastHandle.dbch_handle = DeviceInfo->DeviceHandle;

					DeviceInfo->NotificationHandle = RegisterDeviceNotification(ServiceStatusHandle, &BroadcastHandle, DEVICE_NOTIFY_SERVICE_HANDLE);

					_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange] - DBT_DEVICEARRIVAL: Notification Handle = %p", DeviceInfo->NotificationHandle));

					if (DeviceInfo->NotificationHandle == NULL)
					{
						w32Error = GetLastError();
					}
				}

				if (w32Error == ERROR_SUCCESS)
				{
					GetDeviceInformation(DeviceInfo->DevicePath, DeviceInfo->DeviceName, DeviceInfo->RegisterDlls, DeviceInfo->UnregisterDlls);

					_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange] - DeviceName: %ws", DeviceInfo->DeviceName));

					InitializeListHead(&DeviceInfo->ListEntry);

					InsertTailList(&ListHead, &DeviceInfo->ListEntry);        

					RegisterDlls(DeviceInfo, TRUE);

					//if OS is Vista do this
					if( GetOSVersion() )
					{
						//[Ni Tao 200706141813]should start thread again after being unplugged or removed
						if( wcsstr(DeviceInfo->DevicePath, L"audio") || wcsstr(DeviceInfo->DevicePath, L"Audio" ) )//Is this reasonable?
						{
							BOOL bSuccess = Initialize(DeviceInfo->DevicePath);
							
							if(bSuccess)
								SyncVistaEndPointSettings();
						}
					}
				}

				if (w32Error != ERROR_SUCCESS)
				{
					// Cleanup the mess...
					if (DeviceInfo->NotificationHandle) 
					{
						UnregisterDeviceNotification(DeviceInfo->NotificationHandle);
					}

					if ((DeviceInfo->DeviceHandle != INVALID_HANDLE_VALUE) && (DeviceInfo->DeviceHandle != NULL))
					{
						CloseHandle(DeviceInfo->DeviceHandle);
					}

					HeapFree(GetProcessHeap(), 0, DeviceInfo);
				}
			}
			else
			{
				w32Error = ERROR_OUTOFMEMORY;
			}
		}
		break;

		case DBT_DEVICEREMOVECOMPLETE:
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange] - DBT_DEVICEREMOVECOMPLETE"));
		}
		break;

        default:
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceInterfaceChange] - Unknown/unhandled event type: 0x%x", EventType));
		}
        break;
	}

	return w32Error;
}

/*****************************************************************************
 * HandleDeviceChange()
 *****************************************************************************
 *//*!
 * @brief
 */
DWORD
HandleDeviceChange
(
	IN		DWORD					EventType,
	IN		PDEV_BROADCAST_HANDLE	BroadcastHandle
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange]"));

	DWORD w32Error = ERROR_NOT_FOUND;

	// Find & validate the DEVICE_INFO structure associated with this notification.
	PDEVICE_INFO DeviceInfo = NULL;

    for (PLIST_ENTRY Entry = ListHead.Flink; Entry != &ListHead; Entry = Entry->Flink)
    {
        PDEVICE_INFO Info = CONTAINING_RECORD(Entry, DEVICE_INFO, ListEntry);

        if (BroadcastHandle->dbch_hdevnotify == Info->NotificationHandle) 
		{
			DeviceInfo = Info;

			w32Error = ERROR_SUCCESS;
            break;
        }
    }

	if (w32Error == ERROR_SUCCESS)
	{
		ASSERT(DeviceInfo);

		switch (EventType)
		{
			case DBT_DEVICEQUERYREMOVE:
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEQUERYREMOVE: %ws", DeviceInfo->DeviceName));

				RegisterDlls(DeviceInfo, FALSE);

				// User is trying to disable, uninstall, or eject our device.
				// Close the handle to the device so that the target device can
				// get removed. Do not unregister the notification
				// at this point, because we want to know whether
				// the device is successfully removed or not.
				// 
				if ((DeviceInfo->DeviceHandle != INVALID_HANDLE_VALUE) && (DeviceInfo->DeviceHandle != NULL))
				{
					CloseHandle(DeviceInfo->DeviceHandle);

					DeviceInfo->DeviceHandle = INVALID_HANDLE_VALUE;
				}
			}
			break;

			case DBT_DEVICEREMOVECOMPLETE:
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEREMOVECOMPLETE: %ws", DeviceInfo->DeviceName));

				RegisterDlls(DeviceInfo, FALSE);

				//
				// Device is getting surprise removed. So close 
				// the handle to device and unregister the PNP notification.
				//
				if (DeviceInfo->NotificationHandle) 
				{
					UnregisterDeviceNotification(DeviceInfo->NotificationHandle);

					DeviceInfo->NotificationHandle = NULL;
				}

				if ((DeviceInfo->DeviceHandle != INVALID_HANDLE_VALUE) && (DeviceInfo->DeviceHandle != NULL))
				{
					CloseHandle(DeviceInfo->DeviceHandle);

					DeviceInfo->DeviceHandle = INVALID_HANDLE_VALUE;
				}

				//
				// Unlink this deviceInfo from the list and free the memory
				//
				RemoveEntryList(&DeviceInfo->ListEntry);

				HeapFree(GetProcessHeap(), 0, DeviceInfo);
		 	}
			break;

			case DBT_DEVICEREMOVEPENDING:
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEREMOVEPENDING: %ws", DeviceInfo->DeviceName));

				//
				// Device is successfully removed so unregister the notification
				// and free the memory.
				//
				if (DeviceInfo->NotificationHandle) 
				{
					UnregisterDeviceNotification(DeviceInfo->NotificationHandle);

					DeviceInfo->NotificationHandle = NULL;
					DeviceInfo->DeviceHandle = INVALID_HANDLE_VALUE;
				}

				//
				// Unlink this deviceInfo from the list and free the memory
				//
				RemoveEntryList(&DeviceInfo->ListEntry);

				HeapFree(GetProcessHeap(), 0, DeviceInfo);
			}
			break;

			case DBT_DEVICEQUERYREMOVEFAILED:
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEQUERYREMOVEFAILED: %ws", DeviceInfo->DeviceName));

				//
				// Remove failed. So reopen the device and register for
				// notification on the new handle. But first we should unregister
				// the previous notification.
				//
				if (DeviceInfo->NotificationHandle) 
				{
					UnregisterDeviceNotification(DeviceInfo->NotificationHandle);

					DeviceInfo->NotificationHandle = NULL;
				}

				DeviceInfo->DeviceHandle = CreateFileW
											(
												DeviceInfo->DevicePath,
												GENERIC_READ | GENERIC_WRITE,
												0,
												NULL,
												OPEN_EXISTING,
												0,
												NULL
											);

				_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEQUERYREMOVEFAILED : Device Handle = %p", DeviceInfo->DeviceHandle));

				if (DeviceInfo->DeviceHandle != INVALID_HANDLE_VALUE)
				{
					//
					// Register handle based notification to receive pnp 
					// device change notification on the handle.
					//

					DEV_BROADCAST_HANDLE BroadcastHandle;

					memset(&BroadcastHandle, 0, sizeof(DEV_BROADCAST_HANDLE));
					BroadcastHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
					BroadcastHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
					BroadcastHandle.dbch_handle = DeviceInfo->DeviceHandle;

					DeviceInfo->NotificationHandle = RegisterDeviceNotification(ServiceStatusHandle, &BroadcastHandle, DEVICE_NOTIFY_SERVICE_HANDLE);

					_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEQUERYREMOVEFAILED : Notification Handle = %p", DeviceInfo->DeviceHandle));
	
					RegisterDlls(DeviceInfo, TRUE);
				}
				else
				{
					_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - DBT_DEVICEQUERYREMOVEFAILED: Failed to reopen device: %ws", DeviceInfo->DeviceName));

					//
					// Unlink this deviceInfo from the list and free the memory
					//
					RemoveEntryList(&DeviceInfo->ListEntry);

					HeapFree(GetProcessHeap(), 0, DeviceInfo);
				}
			}
			break;

			default:
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[HandleDeviceChange] - Unknown/unhandled event type: 0x%x", EventType));
			}
			break;
		}
	}

	return w32Error;
}

/*****************************************************************************
 * GetOSVersion()
 *****************************************************************************
 *//*!
 * @brief
 * Check if current OS is Windows Vista
 */
BOOL
GetOSVersion
(	void
)
{
	BOOL				bVista = FALSE;
	OSVERSIONINFO		osvi;

    osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx (&osvi);

	if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		if ( osvi.dwMajorVersion == 6 )//for Vista Only
			bVista = TRUE;		
	}

	return bVista;
}

/*****************************************************************************
 * Initialize()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
Initialize
(
	IN		WCHAR* pDevicePath
)
{
	BOOL bSuccess = TRUE;;

	_DbgPrintF(DEBUGLVL_VERBOSE, ("[Initialize]"));

	if (IsValidHandle(m_hDevice))
		CloseHandle(m_hDevice);
	// CreateFile and keep the handle here.
	m_hDevice = CreateFileW(pDevicePath, GENERIC_READ|GENERIC_WRITE, 
							0, NULL, OPEN_EXISTING, 0, NULL);

	if (!IsValidHandle(m_hDevice))
    {
       DWORD w32Error = GetLastError();

       _DbgPrintF(DEBUGLVL_ERROR,("[Initialize] - CreateFile failed for device and the ErrorCode is :0x%08x", w32Error));
	   
	   return FALSE;
    }
	else
	{
		BOOL bRet = InitializeDescriptor();

		if(!bRet)
			return FALSE;
	}

	USHORT dwProductID = m_deviceDescriptor.idProduct;
	USHORT dwVendorID = m_deviceDescriptor.idVendor;

	// Initialize the GUIDs for various property sets.
	INIT_XU_PROPSETID(&m_ClockRatePropertySet, dwVendorID, dwProductID, XU_CODE_CLOCK_RATE);
	
	// Find all the nodes that is used for property access.
	FindNode(m_ClockRatePropertySet, &m_ClockRateNodeId);
	
	// Initialize the Events 
	for (ULONG i=0; i<2; i++)
	{
		m_EventPool[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	for (ULONG i=0; i<1; i++)
	{
		ZeroMemory(&m_EventData[i], sizeof(KSEVENTDATA));

		m_EventData[i].NotificationType = KSEVENTF_EVENT_HANDLE;
		m_EventData[i].EventHandle.Event = m_EventPool[i];
	}

	// Register with the driver for the events to receive notificaitons.
	KsEnableNodeEvent(m_ClockRateNodeId, KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, &m_EventData[0], sizeof(KSEVENTDATA));

	// Create listening thread
	m_EventThreadHandle = CreateThread(NULL, 0, EventThreadRoutine, NULL, 0, NULL);

	return bSuccess;
}

/*****************************************************************************
 * InitializeDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize device descriptor
 */
BOOL
InitializeDescriptor
(	void
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[InitializeDescriptor]"));

	if(m_hDevice == NULL)
		return FALSE;

	DEVICECONTROL_DEVICE_DESCRIPTOR  ksDevDescProp;
	ksDevDescProp.Property.Set = KSPROPSETID_DeviceControl;
	ksDevDescProp.Property.Flags = KSPROPERTY_TYPE_GET;
	ksDevDescProp.Property.Id = KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR;

	ULONG BytesReturned = 0;

	BOOL Success = DeviceIoControl
					(
						m_hDevice, 
						IOCTL_KS_PROPERTY,
						&ksDevDescProp, sizeof(ksDevDescProp),
						&m_deviceDescriptor, sizeof(USB_DEVICE_DESCRIPTOR),
						&BytesReturned, 
						NULL
					);

	return (Success ? TRUE : FALSE);
}

/*****************************************************************************
 * FindNode()
 *****************************************************************************
 *//*!
 * @brief
 * Find the node with the specified property set.
 */
HRESULT
FindNode
(
	IN		REFGUID PropertySet,
	OUT		ULONG *OutNodeId
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[FindNode]"));

	*OutNodeId = ULONG(-1);

    PKSMULTIPLE_ITEM KsMultipleTopoNodes = NULL;

    HRESULT hr = GetPropertyMulti
					(
						KSPROPSETID_Topology,
						KSPROPERTY_TOPOLOGY_NODES,
						&KsMultipleTopoNodes 
					);

    if (FAILED(hr))
    {
		//_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsFilter::EnumerateNodes] - Failed to get property KSPROPSETID_Topology.KSPROPERTY_TOPOLOGY_NODES"));
    }
    else
    {
		hr = E_FAIL;

        if (KsMultipleTopoNodes)
        {
            ULONG NumberOfNodes = KsMultipleTopoNodes->Count;

            // Point to immediately following KsMultipleTopoNodes
            GUID * NodeType = (GUID*)(KsMultipleTopoNodes + 1);

            // Iterate through all the nodes
            for (ULONG NodeId = 0; NodeId < NumberOfNodes; NodeId++)
            {
                //CKsNode * Node = new CKsNode(NodeId, NodeType[NodeId], &hr);
				if (IsEqualGUID(NodeType[NodeId], KSNODETYPE_DEV_SPECIFIC))
				{
					hr = GetNodePropertySimple(NodeId, PropertySet, 0, NULL, 0, NULL, 0);

					if (SUCCEEDED(hr))
					{
						// Found the node with the property set we are looking for.
						*OutNodeId = NodeId;
						break;
					}
				}
            }
        }
    }

	if (KsMultipleTopoNodes)
	{
		delete[] (BYTE*)KsMultipleTopoNodes;
	}

    return hr;
}

/*****************************************************************************
 * GetPropertyMulti()
 *****************************************************************************
 *//*!
 * @brief
 * Multiple items request.  The function allocates memory for the caller.  
 * It is the caller's responsiblity to free this memory.
 */
HRESULT
GetPropertyMulti
(
	IN		REFGUID PropertySet,
	IN		ULONG PropertyId, 
	OUT		PKSMULTIPLE_ITEM *OutKsMultipleItem
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[GetPropertyMulti]"));

    ULONG MultipleItemSize = 0;

	KSPROPERTY KsProperty;

    ZeroMemory(&KsProperty, sizeof(KsProperty));

    KsProperty.Set   = PropertySet;
    KsProperty.Id    = PropertyId;
    KsProperty.Flags = KSPROPERTY_TYPE_GET;

    HRESULT hr = SynchronizedIoctl
					(
						m_hDevice,
						IOCTL_KS_PROPERTY,
						&KsProperty,
						sizeof(KSPROPERTY),
						NULL,
						0,
						&MultipleItemSize
					);

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        *OutKsMultipleItem = (PKSMULTIPLE_ITEM) new BYTE[MultipleItemSize];

        if (NULL == *OutKsMultipleItem)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        hr = SynchronizedIoctl
				(
					m_hDevice,
					IOCTL_KS_PROPERTY,
					&KsProperty,
					sizeof(KSPROPERTY),
					*OutKsMultipleItem,
					MultipleItemSize,
					NULL
				);
    }
    
    return hr;
}

/*****************************************************************************
 * GetNodePropertySimple()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a simple node property.
 */
HRESULT
GetNodePropertySimple
(
	IN		ULONG NodeId,
	IN		REFGUID PropertySet,
	IN		ULONG PropertyId,
	OUT		PVOID Value, 
	IN		ULONG ValueSize, 
	IN		PVOID Instance,
	IN		ULONG InstanceSize
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[GetNodePropertySimple]"));

	HRESULT hr = S_OK;
    
	ULONG PropertySize = sizeof(KSNODEPROPERTY) + InstanceSize;

    PKSNODEPROPERTY KsNodeProperty = (PKSNODEPROPERTY)new BYTE[PropertySize];

    if (NULL == KsNodeProperty)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        ZeroMemory(KsNodeProperty, sizeof(KSNODEPROPERTY));

        KsNodeProperty->Property.Set = PropertySet;
        KsNodeProperty->Property.Id = PropertyId;
        KsNodeProperty->Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
        KsNodeProperty->NodeId = NodeId;
        KsNodeProperty->Reserved = 0;

        if (Instance)
        {
            CopyMemory(PUCHAR(KsNodeProperty) + sizeof(KSNODEPROPERTY), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_hDevice,
					IOCTL_KS_PROPERTY,
					KsNodeProperty,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }    

	//cleanup memory
	if (KsNodeProperty)
	{
		delete[] (BYTE*)KsNodeProperty;
	}

	return hr;
}


/*****************************************************************************
 * SynchronizedIoctl()
 *****************************************************************************
 *//*!
 * @brief
 * Synchronized I/O control.
 */
HRESULT
SynchronizedIoctl
(
	IN		HANDLE	Handle,
	IN		ULONG	CtlCode,
	IN		PVOID	InputBuffer,
	IN		ULONG InputBufferSize,
	OUT		PVOID	OutputBuffer, 
	OUT		ULONG	OutputBufferSize,
	OUT		ULONG *	OutBytesReturned
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[SynchronizedIoctl]"));

    HRESULT hr = S_OK;

    ULONG BytesReturned;

    if (!OutBytesReturned)
    {
        OutBytesReturned = &BytesReturned;
    }

    if (!IsValidHandle(Handle))
    {
        hr = E_FAIL;

		//_DbgPrintF(DEBUGLVL_ERROR,("[SynchronizedIoctl] - Invalid Handle"));
    }
    
    OVERLAPPED Overlapped;

	if (SUCCEEDED(hr))
    {
        ZeroMemory(&Overlapped, sizeof(OVERLAPPED));

		Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (Overlapped.hEvent)
        {
            // Flag the event by setting the low-order bit so we
            // don't get completion port notifications.
            // Really! - see the description of the lpOverlapped parameter in
            // the docs for GetQueuedCompletionStatus
            Overlapped.hEvent = (HANDLE)((DWORD_PTR)Overlapped.hEvent | 0x1);
        }
        else
        {
            hr = E_OUTOFMEMORY;

			//_DbgPrintF(DEBUGLVL_ERROR,("[SynchronizedIoctl] - CreateEvent failed"));
        }
    }

    if (SUCCEEDED(hr))
    {
	    BOOL Result = DeviceIoControl(Handle, CtlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, OutBytesReturned, &Overlapped);

        if (!Result)
        {
            DWORD w32Error = GetLastError();

            if (ERROR_IO_PENDING == w32Error)
            {
                // Wait for completion
                DWORD Wait = ::WaitForSingleObject(Overlapped.hEvent, INFINITE);

                //ASSERT(WAIT_OBJECT_0 == Wait);

                if (Wait != WAIT_OBJECT_0)
                {
                    hr = E_FAIL;

					_DbgPrintF(DEBUGLVL_ERROR,("[SynchronizedIoctl] - WaitForSingleObject failed Wait: 0x%08x", Wait));
                }
            }
            else if (((ERROR_INSUFFICIENT_BUFFER == w32Error) || (ERROR_MORE_DATA == w32Error)) &&
					 (IOCTL_KS_PROPERTY == CtlCode) &&
					 (OutputBufferSize == 0))
            {
                hr = S_OK;

                Result = TRUE;
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if (!Result) 
		{
			*OutBytesReturned = 0;
		}

        SafeCloseHandle(Overlapped.hEvent);
    }
    
    return hr;
}

/*****************************************************************************
 * IsValidHandle()
 *****************************************************************************
 *//*!
 * @brief
 * Quick and Dirty check to see if the handle is good.  Only checks against
 * a small number of known bad values.
 * @return
 * Returns TRUE unless the handle is NULL or INVALID_HANDLE_VALUE.
 */
BOOL
IsValidHandle
(
	IN		HANDLE Handle
)
{
    BOOL ValidHandle = !((Handle == NULL) || (Handle == INVALID_HANDLE_VALUE));

	return ValidHandle;
}

/*****************************************************************************
 * SafeCloseHandle()
 *****************************************************************************
 *//*!
 * @brief
 * Safely closes a file handle.
 */
BOOL
SafeCloseHandle
(
	IN		HANDLE& Handle
)
{
    BOOL Success = TRUE;

    if (IsValidHandle(Handle))
    {
        Success = CloseHandle(Handle);

        Handle = INVALID_HANDLE_VALUE;
    }

	return (Success ? TRUE : FALSE);
}

/*****************************************************************************
 * KsEnableNodeEvent()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT
KsEnableNodeEvent
(
	IN		ULONG NodeId,
	IN		REFGUID	EventSet, 
	IN		ULONG EventId,
	IN		PKSEVENTDATA EventData, 
	IN		ULONG EventDataSize
)
{
	KSE_NODE KseNode;

	KseNode.Event.Set = EventSet;
	KseNode.Event.Id = EventId;
	KseNode.Event.Flags = KSEVENT_TYPE_ENABLE | KSEVENT_TYPE_TOPOLOGY;
	KseNode.NodeId = NodeId;
	KseNode.Reserved = 0;

    HRESULT hr = SynchronizedIoctl
					(
						m_hDevice,
						IOCTL_KS_ENABLE_EVENT,
						&KseNode,
						sizeof(KSE_NODE),
						EventData,
						EventDataSize,
						NULL
					);

	return hr;
}

/*****************************************************************************
 * EventThreadRoutine()
 *****************************************************************************
 *//*!
 * @brief
 * I/O processing thread routine.
 */
DWORD WINAPI 
EventThreadRoutine
(
	IN		LPVOID	Parameter
)
{	
	EventThreadHandler();

	// Explicitly specify the exit code. There seems to be a bug in the
    // WOW64 code that pass the exit code STILL_ACTIVE as the return
    // value in GetExitCodeThread(), causing the program to loop forever.
	ExitThread(0);

	return 0;
}

/*****************************************************************************
 * EventThreadHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Handle the main I/O processing.
 */
VOID
EventThreadHandler
(	void
)
{
	BOOL Abort = FALSE;

	while (!Abort)
	{
        // Wait on control events and "packet complete" events.
        DWORD Wait = WaitForMultipleObjects(2, m_EventPool, FALSE, INFINITE);

		if ((Wait == WAIT_FAILED) || (Wait == WAIT_TIMEOUT))
		{
			// Something bad happened...
            break;
		}

        ULONG EventSignaled = Wait - WAIT_OBJECT_0;

		ULONG ControlEvent = EventSignaled;

		switch (ControlEvent)
		{
			case 0:
			{
				// run synch up proc
				SyncVistaEndPointSettings();

				break;
			}
			case 1:
			{
				// say good bye...
				Abort = TRUE;
				break;
			}
		}
	}
}

/*****************************************************************************
 * GetDeviceClockRate()
 *****************************************************************************
 *//*!
 * @brief
 * Retrieve current clock rate from E-MU device.
 */
DWORD
GetDeviceClockRate
(	void
)
{
	DWORD dwClockRate = 0;
	UCHAR sr;

	//_DbgPrintF(DEBUGLVL_VERBOSE, ("m_ClockRateNodeId is: %d", m_ClockRateNodeId));

	if (m_ClockRateNodeId != ULONG(-1))
	{
		HRESULT hr = GetNodePropertySimple(m_ClockRateNodeId, m_ClockRatePropertySet, KSPROPERTY_XU_CLOCK_RATE_SELECTOR, &sr, sizeof(sr), NULL, 0);
		
		if (SUCCEEDED(hr))
		{	
			switch (sr)
			{
				case XU_CLOCK_RATE_SR_44kHz:
				{
					dwClockRate = 44100;
				}
				break;

				case XU_CLOCK_RATE_SR_48kHz:
				{
					dwClockRate = 48000;
				}
				break;

				case XU_CLOCK_RATE_SR_88kHz:
				{
					dwClockRate = 88200;
				}
				break;

				case XU_CLOCK_RATE_SR_96kHz:
				{
					dwClockRate = 96000;
				}
				break;

				case XU_CLOCK_RATE_SR_176kHz:
				{
					dwClockRate = 176400;
				}
				break;

				case XU_CLOCK_RATE_SR_192kHz:
				{
					dwClockRate = 192000;
				}
				break;

				default:
				{
					dwClockRate = 0;
				}
				break;
			}
		}
	}

	return dwClockRate;
}

/*****************************************************************************
 * SyncVistaEndPointSettings()
 *****************************************************************************
 *//*!
 * @brief
 * Synchronize up the Sample Rate value of Vista sound Control Panel.
 */
BOOL 
SyncVistaEndPointSettings
(	void
)
{	
	wchar_t* deviceName;
	PDEVICE_INFO Info;
	BOOL bSuccess = FALSE;

	_DbgPrintF(DEBUGLVL_VERBOSE, ("[SyncVistaEndPointSettings]"));

	// Set device name
	if( (m_deviceDescriptor.idVendor == 0x041E) && (m_deviceDescriptor.idProduct == 0x3F02) )/*MicroPod*/
	{
		size_t size = wcslen(L"VID_041E&PID_3F02") + 1;
		deviceName = new wchar_t[size];
		wcscpy_s(deviceName, size, L"VID_041E&PID_3F02");
	}
	else if( (m_deviceDescriptor.idVendor == 0x041E) && (m_deviceDescriptor.idProduct == 0x3F04) )/*HulaPod*/
	{
		size_t size = wcslen(L"VID_041E&PID_3F04") + 1;
		deviceName = new wchar_t[size];
		wcscpy_s(deviceName, size, L"VID_041E&PID_3F04");
	}
	else if( (m_deviceDescriptor.idVendor == 0x041E) && (m_deviceDescriptor.idProduct == 0x3F0A) )/*MicroPre*/
	{
		size_t size = wcslen(L"VID_041E&PID_3F0A") + 1;
		deviceName = new wchar_t[size];
		wcscpy_s(deviceName, size, L"VID_041E&PID_3F0A");
	}
	else/*Not Supported*/
		return FALSE;

	//Retrieve the Clock Rate of E-MU Device
	ULONG sampleRate = GetDeviceClockRate();
	_DbgPrintF(DEBUGLVL_VERBOSE, ("[SyncVistaEndPointSettings] - clock rate is:%d", sampleRate));

	int deviceIndex = GetPlayBackEndptDeviceIndex(deviceName);

	//_DbgPrintF(DEBUGLVL_VERBOSE, ("[SyncVistaEndPointSettings] - deviceIndex is:%d", deviceIndex));
	bSuccess  = SetPlayBackEndPtVistaSR(deviceIndex, sampleRate, 24);
		
	//_DbgPrintF(DEBUGLVL_VERBOSE, ("[SyncVistaEndPointSettings] - set playback endpt SR return :%d", bSuccess));

	//Set for record endpoint sample rate
	deviceIndex = GetRecordEndptDeviceIndex(deviceName);
	bSuccess  = SetRecordEndPtVistaSR(deviceIndex, sampleRate, 24);
	
	delete [] deviceName;

	return bSuccess;
}

// Local functions
/*****************************************************************************
 * GetDeviceIndex()
 *****************************************************************************
 *//*!
 * @brief
 */
int
GetDeviceIndex
(
	IN		eEndpointType type,
	IN		LPWSTR deviceName
)
{
	HRESULT hRes = CoInitialize(NULL);
	
	CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;
    CComPtr<IMMDevice>           spDevice;
	
	EDataFlow flow;
	if (type == ep_Playback)
		flow = eRender;
	else if (type == ep_Record)
		flow = eCapture;
	else 
		return -1;

	HRESULT hr = spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	UINT Count = 0;

    if (SUCCEEDED(hr))
    {
		hr = spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &spEndpoints);
		
		if (SUCCEEDED(hr))
        {
            hr = spEndpoints->GetCount(&Count);

            if (SUCCEEDED(hr))
            {
                for (UINT32 i=0; i<Count; i++)
                {
                    hr = spEndpoints->Item(i, &spDevice);

                    if (SUCCEEDED(hr))
                    {
                        LPWSTR DeviceID = NULL;
                        IAudioPropStore* pAudioPropStore = NULL;

                        hr = spDevice->GetId(&DeviceID);
						
						// Get Endpoint Property Store
						GetEndpointPropertyStore(DeviceID, &pAudioPropStore);
						
						PROPVARIANT var;
                        PropVariantInit(&var);
						// Define System Name Property Key
						PROPERTYKEY SystemNameKey; 
						SystemNameKey.fmtid.Data1 = 0xb3f8fa53;
						SystemNameKey.fmtid.Data2 = 0x0004;
						SystemNameKey.fmtid.Data3 = 0x438e;
						SystemNameKey.fmtid.Data4[0] = 0x90;
						SystemNameKey.fmtid.Data4[1] = 0x03;
						SystemNameKey.fmtid.Data4[2] = 0x51;
						SystemNameKey.fmtid.Data4[3] = 0xa4;
						SystemNameKey.fmtid.Data4[4] = 0x6e; 
						SystemNameKey.fmtid.Data4[5] = 0x13;
						SystemNameKey.fmtid.Data4[6] = 0x9b;
						SystemNameKey.fmtid.Data4[7] = 0xfc;
						SystemNameKey.pid = 2;
						
						pAudioPropStore->GetValue(SystemNameKey, &var); 

						// Check for device name
						wchar_t *check = NULL;
						check = wcsstr(var.bstrVal, deviceName);
						if (check)
							return i; // Return the index						
						
						pAudioPropStore->Release();
					}
               
                    if (spDevice)
                    {
						spDevice.Release();
                        spDevice = NULL;
                    }
                }
            }
        }

        if (spEnumerator)
        {
            spEnumerator = NULL;
        }

        if (spEndpoints)
        {
            spEndpoints = NULL;
        }
   }

	if (SUCCEEDED(hRes))
		CoUninitialize();

	return -1;
}

/*****************************************************************************
 * SetVistaSR()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
SetVistaSR
(
	IN		eEndpointType type,
	IN		int deviceID,
	IN		UINT sampleRate,
	IN		UINT numBits
)
{
	HRESULT hRes = CoInitialize(NULL);
	if (deviceID < 0)
		return false;
	
	BOOL success = false;

	CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;
    CComPtr<IMMDevice>           spDevice;

	EDataFlow flow;
	if (type == ep_Playback)
		flow = eRender;
	else if (type == ep_Record)
		flow = eCapture;
	else 
		return false;

	HRESULT hr = spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

	UINT Count = 0;
    if (SUCCEEDED(hr))
    {
		hr = spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &spEndpoints);

        if (SUCCEEDED(hr))
        {
            hr = spEndpoints->GetCount(&Count);

            if (SUCCEEDED(hr) && (deviceID < (int)Count))
            {
				hr = spEndpoints->Item(deviceID, &spDevice);
                
				if (SUCCEEDED(hr))
				{
					LPWSTR DeviceID = NULL;
					hr = spDevice->GetId(&DeviceID);
					if (SUCCEEDED(hr))
					{
						// Set Format
						hr = SetEndpointFormat(DeviceID, sampleRate, numBits);
						if (SUCCEEDED(hr))
							success = true;

						if (spDevice)
						{
							spDevice.Release();
							spDevice = NULL;
						}
                	}
				}
			}
        }

        if (spEnumerator)
        {
            spEnumerator = NULL;
        }

        if (spEndpoints)
        {
            spEndpoints = NULL;
        }
    }

	if (SUCCEEDED(hRes))
		CoUninitialize();

	return success;
}

/*****************************************************************************
 * GetVistaSR()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
GetVistaSR
(
	IN		eEndpointType type,
	IN		int deviceID,
	OUT		UINT *sampleRate,
	OUT		UINT *numBits
)
{
	HRESULT hRes = CoInitialize(NULL);

	if (deviceID < 0)
		return false;
	
	BOOL success = false;

	CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;
    CComPtr<IMMDevice>           spDevice;

	EDataFlow flow;
	if (type == ep_Playback)
		flow = eRender;
	else if (type == ep_Record)
		flow = eCapture;
	else 
		return false;

	HRESULT hr = spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

	UINT Count = 0;
    if (SUCCEEDED(hr))
    {
        hr = spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &spEndpoints);
		
        if (SUCCEEDED(hr))
        {
            hr = spEndpoints->GetCount(&Count);

            if (SUCCEEDED(hr) && (deviceID < (int)Count))
            {
				hr = spEndpoints->Item(deviceID, &spDevice);
                
				if (SUCCEEDED(hr))
				{
					LPWSTR DeviceID = NULL;
					hr = spDevice->GetId(&DeviceID);
					if (SUCCEEDED(hr))
					{
						// Get Format
						hr = GetEndpointFormat(DeviceID, sampleRate, numBits);
						if (SUCCEEDED(hr))
							success = true;

						if (spDevice)
						{
							spDevice.Release();
							spDevice = NULL;
						}
                	}
				}
			}
        }

        if (spEnumerator)
        {
            spEnumerator = NULL;
        }

        if (spEndpoints)
        {
            spEndpoints = NULL;
        }
    }

	if (SUCCEEDED(hRes))
		CoUninitialize();
	return success;
}

/*****************************************************************************
 * GetPlayBackEndptDeviceIndex()
 *****************************************************************************
 *//*!
 * @brief
 */
int
GetPlayBackEndptDeviceIndex
(
	IN		LPWSTR	deviceName
)
{
	return GetDeviceIndex(ep_Playback, deviceName);
}

/*****************************************************************************
 * GetRecordEndptDeviceIndex()
 *****************************************************************************
 *//*!
 * @brief
 */
int
GetRecordEndptDeviceIndex
(
	IN		LPWSTR	deviceName
)
{
	return GetDeviceIndex(ep_Record, deviceName);
}

/*****************************************************************************
 * SetPlayBackEndPtVistaSR()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
SetPlayBackEndPtVistaSR
(
	IN		int		deviceID,
	IN		UINT	sampleRate,
	IN		UINT	numBits
)
{
	return SetVistaSR(ep_Playback, deviceID, sampleRate, numBits); 
}

/*****************************************************************************
 * GetPlayBackEndPtVistaSR()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
GetPlayBackEndPtVistaSR
(
	IN		int deviceID,
	OUT		UINT *sampleRate,
	OUT		UINT *numBits
)
{
	return GetVistaSR(ep_Playback, deviceID, sampleRate, numBits); 
}

/*****************************************************************************
 * SetRecordEndPtVistaSR()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
SetRecordEndPtVistaSR
(
	IN		int deviceID,
	IN		UINT sampleRate,
	IN		UINT numBits
)
{
	return SetVistaSR(ep_Record, deviceID, sampleRate, numBits); 
}
 
/*****************************************************************************
 * GetRecordEndPtVistaSR()
 *****************************************************************************
 *//*!
 * @brief
 */ 
BOOL
GetRecordEndPtVistaSR
(
	IN		int deviceID,
	OUT		UINT *sampleRate,
	OUT		UINT *numBits
)
{
	return GetVistaSR(ep_Record, deviceID, sampleRate, numBits); 
}

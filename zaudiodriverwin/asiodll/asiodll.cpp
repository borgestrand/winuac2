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
 * @file	   asiodll.cpp
 * @brief      ASIO DLL entry points implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "asiodll.h"
#include "factory.h"

#define STR_MODULENAME "asiodll: "

HINSTANCE hDllInstance = NULL;

HANDLE hDllHeap = NULL;

/*****************************************************************************
 * DllMain()
 *****************************************************************************
 * DLL entry point.
 */
BOOL WINAPI
DllMain
(
    IN      HINSTANCE   Instance,
    IN      ULONG       Reason,
    IN      PVOID       Reserved
)
{
	BOOL result = TRUE;

    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            hDllInstance = Instance;

            hDllHeap = HeapCreate(0, 0x10000, 0);

			DisableThreadLibraryCalls(hDllInstance);
        }
        break;

        case DLL_PROCESS_DETACH:
        {
            if (hDllHeap) HeapDestroy(hDllHeap);
        }
        break;
    }

    return result;
}

/*****************************************************************************
 * DllCanUnloadNow()
 *****************************************************************************
 *//*!
 * @brief
 * Determines whether the DLL that implements this function is in use. If
 * not, the caller can unload the DLL from memory.
 *
 * Note: OLE does not provide this function. DLLs that support the OLE
 * Component Object Model (COM) should implement and export DllCanUnloadNow.
 *
 * @details
 * A call to DllCanUnloadNow determines whether the DLL from which it is
 * exported is still in use. A DLL is no longer in use when it is not
 * managing any existing objects (the reference count on all of its objects
 * is 0).
 *
 * Notes to Callers:
 * You should not have to call DllCanUnloadNow directly. OLE calls it only
 * through a call to the CoFreeUnusedLibraries function. When it returns
 * S_OK, CoFreeUnusedLibraries frees the DLL.
 *
 * Notes to Implementers:
 * You need to implement DllCanUnloadNow in, and export it from, DLLs that
 * are to be dynamically loaded through a call to the CoGetClassObject
 * function. (You also need to implement and export the DllGetClassObject
 * function in the same DLL).
 *
 * If a DLL loaded through a call to CoGetClassObject fails to export
 * DllCanUnloadNow, the DLL will not be unloaded until the application calls
 * the CoUninitialize function to release the OLE libraries.
 *
 * If the DLL links to another DLL, returning S_OK from DllCanUnloadNow will
 * also cause the second, dependent DLL to be unloaded. To eliminate the
 * possibility of a crash, the primary DLL should call the CoLoadLibrary
 * function, specifying the path to the second DLL as the first parameter,
 * and setting the auto free parameter to TRUE. This forces the COM library
 * to reload the second DLL and set it up for a call to CoFreeUnusedLibraries
 * to free it separately when appropriate.
 *
 * DllCanUnloadNow should return S_FALSE if there are any existing references
 * to objects that the DLL manages.
 *
 * @return
 * - S_OK
 *   The DLL can be unloaded.
 * - S_FALSE
 *   The DLL cannot be unloaded now.
 */
STDAPI 
DllCanUnloadNow
(   void
)
{
	if (CClassFactory::IsLocked() || CUnknown::ObjectsActive())
	{
        return S_FALSE;
    } 
	else 
	{
		return S_OK;
    }
}

/*****************************************************************************
 * DllGetClassObject()
 *****************************************************************************
 *//*!
 * @brief
 * Retrieves the class object from a DLL object handler or object application.
 * DllGetClassObject is called from within the CoGetClassObject function when
 * the class context is a DLL.
 * Note: OLE does not provide this function. DLLs that support the OLE
 * Component Object Model (COM) must implement DllGetClassObject in OLE
 * object handlers or DLL applications.
 *
 * @details
 * If a call to the CoGetClassObject function finds the class object that is
 * to be loaded in a DLL, CoGetClassObject uses the DLL's exported
 * DllGetClassObject function.
 *
 * Notes to Callers:
 * You should not call DllGetClassObject directly. When an object is defined
 * in a DLL, CoGetClassObject calls the CoLoadLibrary function to load the
 * DLL, which, in turn, calls DllGetClassObject.
 *
 * Notes to Implementers:
 * You need to implement DllGetClassObject in (and export it from) DLLs that
 * support the OLE Component Object Model.
 *
 * @param
 * rclsid
 * CLSID that will associate the correct data and code.
 *
 * @param
 * riid
 * Reference to the identifier of the interface that the caller is to use to
 * communicate with the class object. Usually, this is IID_IClassFactory
 * (defined in the OLE headers as the interface identifier for IClassFactory).
 *
 * @param
 * ppv
 * Address of pointer variable that receives the interface pointer requested
 * in riid. Upon successful return, *ppv contains the requested interface
 * pointer. If an error occurs, the interface pointer is NULL.
 *
 * @return
 * This function supports the standard return values E_INVALIDARG,
 * E_OUTOFMEMORY and E_UNEXPECTED, as well as the following:
 * - S_OK
 *   The object was retrieved successfully.
 * - CLASS_E_CLASSNOTAVAILABLE
 *   The DLL does not support the class (object definition).
 */
STDAPI 
DllGetClassObject
(
    IN      REFCLSID    rclsid,
    IN      REFIID      riid,
    OUT     PVOID *     ppv
)
{
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	CClassFactory * Factory = new CClassFactory(NULL);

	if (Factory)
	{
		Factory->AddRef();

		hr = Factory->Init(rclsid);

		if (SUCCEEDED(hr))
		{
			hr = Factory->QueryInterface(riid, ppv);
		}

		Factory->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

    return hr;
}

/*****************************************************************************
 * RegisterInprocServer()
 *****************************************************************************
 */
HRESULT 
RegisterInprocServer
(
    IN      REFGUID ClassId,
    IN      LPTSTR  DriverName,
	IN		LPTSTR	FriendlyName,
	IN		LPTSTR	SymbolicLink
)
{
    WCHAR ClsIdStr[MAX_PATH];
    StringFromGUID2(ClassId, ClsIdStr, MAX_PATH);

	WCHAR KeyName[MAX_PATH];
	wcscpy(KeyName, L"CLSID\\"); wcscat(KeyName, ClsIdStr); 

	HKEY ClassKey = NULL;

    DWORD w32Error = RegCreateKeyExW(HKEY_CLASSES_ROOT, KeyName, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &ClassKey, NULL);

    if (ERROR_SUCCESS == w32Error)
    {
		HKEY ServerKey = NULL;

		w32Error = RegCreateKeyExW(ClassKey, L"InprocServer32", 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &ServerKey, NULL);

		if (ERROR_SUCCESS == w32Error)
		{
			RegSetValueEx(ServerKey, NULL, 0, REG_SZ, (BYTE*)DriverName, (_tcslen(DriverName)+1)*sizeof(TCHAR));

			RegSetValueEx(ServerKey, TEXT("ThreadingModel"), 0, REG_SZ, (BYTE*)TEXT("Apartment"), (_tcslen(TEXT("Apartment"))+1)*sizeof(TCHAR));

			RegCloseKey(ServerKey);
		}

		RegSetValueEx(ClassKey, TEXT("FriendlyName"), 0, REG_SZ, (BYTE*)FriendlyName, (_tcslen(FriendlyName)+1)*sizeof(TCHAR));

		RegSetValueEx(ClassKey, TEXT("SymbolicLink"), 0, REG_SZ, (BYTE*)SymbolicLink, (_tcslen(SymbolicLink)+1)*sizeof(TCHAR));

        RegCloseKey(ClassKey);
	}

	if (ERROR_SUCCESS == w32Error)
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

/*****************************************************************************
 * UnregisterInprocServer()
 *****************************************************************************
 */
HRESULT 
UnregisterInprocServer
(
    IN      REFGUID ClassId
)
{
    WCHAR ClsIdStr[MAX_PATH];
    StringFromGUID2(ClassId, ClsIdStr, MAX_PATH);

    WCHAR KeyName[MAX_PATH];

	wcscpy(KeyName, L"CLSID\\"); wcscat(KeyName, ClsIdStr); wcscat(KeyName, L"\\InprocServer32");  
	
	RegDeleteKeyW(HKEY_CLASSES_ROOT, KeyName);

    wcscpy(KeyName, L"CLSID\\"); wcscat(KeyName, ClsIdStr);

    RegDeleteKeyW(HKEY_CLASSES_ROOT, KeyName);

    return S_OK;
}

#define ASIO_MARKER "{BE8FE0D5-AC9A-472b-8676-007B3AEB829B}"
#define ASIO_PUBLIC_KEY "Software\\ASIO"

/*****************************************************************************
 * RegisterAsioDriver()
 *****************************************************************************
 */
HRESULT 
RegisterAsioDriver
(
	IN		LPTSTR	FriendlyName,
    IN      REFGUID ClassId,
    IN      LPTSTR  DriverName,
	IN		LPTSTR	SymbolicLink
)
{
	HKEY AsioPublicKey = NULL;

	DWORD w32Error = RegCreateKeyEx(HKEY_LOCAL_MACHINE, ASIO_PUBLIC_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &AsioPublicKey, NULL);

	if (ERROR_SUCCESS == w32Error)
	{
		HKEY AsioDriverKey = NULL;

		w32Error = RegCreateKeyEx(AsioPublicKey, FriendlyName, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &AsioDriverKey, NULL);

		if (ERROR_SUCCESS == w32Error)
		{
			WCHAR ClsIdStr[MAX_PATH];
			StringFromGUID2(ClassId, ClsIdStr, MAX_PATH);

			RegSetValueEx(AsioDriverKey, NULL, 0, REG_SZ, (BYTE*)ASIO_MARKER, (_tcslen(ASIO_MARKER)+1)*sizeof(TCHAR));

			RegSetValueExW(AsioDriverKey, L"CLSID", 0, REG_SZ, (BYTE*)ClsIdStr, (wcslen(ClsIdStr)+1)*sizeof(WCHAR));

			RegSetValueEx(AsioDriverKey, "Driver", 0, REG_SZ, (BYTE*)DriverName, (_tcslen(DriverName)+1)*sizeof(TCHAR));

			RegSetValueEx(AsioDriverKey, "SymbolicLink", 0, REG_SZ, (BYTE*)SymbolicLink, (_tcslen(SymbolicLink)+1)*sizeof(TCHAR));

			RegCloseKey(AsioDriverKey);
		}

		RegCloseKey(AsioPublicKey);
	}

    if (ERROR_SUCCESS == w32Error)
    {
        return RegisterInprocServer(ClassId, DriverName, FriendlyName, SymbolicLink);
    }
    else
    {
        return E_FAIL;
    }
}

/*****************************************************************************
 * UnregisterAsioDriver()
 *****************************************************************************
 */
HRESULT 
UnregisterAsioDriver
(
	IN		LPTSTR	FriendlyName,
    IN      REFGUID ClassId
)
{
	HKEY AsioPublicKey = NULL;

	DWORD w32Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ASIO_PUBLIC_KEY, 0, KEY_ALL_ACCESS, &AsioPublicKey);

	if (ERROR_SUCCESS == w32Error)
	{
		RegDeleteKey(AsioPublicKey, FriendlyName);

		RegCloseKey(AsioPublicKey);
	}

    if (ERROR_SUCCESS == w32Error)
    {
        return UnregisterInprocServer(ClassId);
    }
    else
    {
        return E_FAIL;
    }
}

typedef struct
{
	TCHAR	FriendlyName[MAX_PATH];
	TCHAR	SymbolicLink[MAX_PATH];
	GUID	ClassId;
} ASIO_DRIVER_INFORMATION, *PASIO_DRIVER_INFORMATION;

/*****************************************************************************
 * LookupFriendlyNames()
 *****************************************************************************
 */
ULONG 
LookupFriendlyNames
(
	IN		LPTSTR	FriendlyName
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[LookupFriendlyNames]"));

	ULONG FreeIndex = 1;

	HKEY AsioPublicKey = NULL;

	DWORD w32Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ASIO_PUBLIC_KEY, 0, KEY_ALL_ACCESS, &AsioPublicKey);

	if (ERROR_SUCCESS == w32Error)
	{
		ULONG NumberOfSubKeys = 0;

		w32Error = RegQueryInfoKey(AsioPublicKey, NULL, NULL, NULL, &NumberOfSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		if ((ERROR_SUCCESS == w32Error) && NumberOfSubKeys)
		{
			PASIO_DRIVER_INFORMATION DriverInfo = PASIO_DRIVER_INFORMATION(LocalAlloc(LPTR, NumberOfSubKeys * sizeof(ASIO_DRIVER_INFORMATION)));

			if (DriverInfo)
			{
				ZeroMemory(DriverInfo, NumberOfSubKeys * sizeof(ASIO_DRIVER_INFORMATION));

				for (ULONG i=0; i<NumberOfSubKeys; i++)
				{
					ULONG FriendlyNameLength = MAX_PATH;

					FILETIME LastWriteTime;	

					w32Error = RegEnumKeyEx(AsioPublicKey, i, DriverInfo[i].FriendlyName, &FriendlyNameLength, NULL, NULL, NULL, &LastWriteTime);
				}

				BOOLEAN AvailableIndices[32];

				for (ULONG Index=1; Index<=32; Index++)
				{
					TCHAR AsioFriendlyName[MAX_PATH];

					if (Index > 1)
					{											
						_stprintf(AsioFriendlyName, "%s (%d)", FriendlyName, Index);
					}
					else
					{
						_stprintf(AsioFriendlyName, "%s", FriendlyName);
					}

					BOOLEAN FoundFriendlyName = FALSE;

					for (ULONG i=0; i<NumberOfSubKeys; i++)
					{
						if (!_tcsicmp(DriverInfo[i].FriendlyName, AsioFriendlyName))
						{
							FoundFriendlyName = TRUE;
							break;
						}
					}

					AvailableIndices[Index-1] = !FoundFriendlyName;
				}

				for (ULONG Index=1; Index<=32; Index++)
				{
					if (AvailableIndices[Index-1])
					{
						FreeIndex = Index;
						break;
					}
				}				

				LocalFree(DriverInfo);
			}
		}

		RegCloseKey(AsioPublicKey);
	}

	return FreeIndex;
}

#define EMU_PUBLIC_KEY "Software\\E-MU"

/*****************************************************************************
 * LookupClassId()
 *****************************************************************************
 */
GUID
LookupClassId
(
	IN		LPTSTR	SymbolicLink
)
{
	GUID ClassId;

	HKEY EmuPublicKey = NULL;

	DWORD w32Error = RegCreateKeyEx(HKEY_LOCAL_MACHINE, EMU_PUBLIC_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &EmuPublicKey, NULL);

	if (ERROR_SUCCESS == w32Error)
	{
		HKEY ClassKey = NULL;

		w32Error = RegCreateKeyEx(EmuPublicKey, "{EB5A82E1-B4E7-47e8-97B0-F0751F97C2D1}", 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &ClassKey, NULL);

		if (ERROR_SUCCESS == w32Error)
		{
			TCHAR ValueName[MAX_PATH]; _tcscpy(ValueName, SymbolicLink);

			TCHAR * token = _tcschr(ValueName, '\\');

			while (token)
			{
				*token = '#';

				token = _tcschr(token+1, '\\');
			}
 
			ULONG Type = 0;
			ULONG Size = sizeof(ClassId);
						
			w32Error = RegQueryValueEx(ClassKey, ValueName, 0, &Type, PBYTE(&ClassId), &Size);

			if (ERROR_SUCCESS != w32Error)
			{
				CoCreateGuid(&ClassId);
				
				RegSetValueEx(ClassKey, ValueName, 0, REG_BINARY, (BYTE*)&ClassId, sizeof(ClassId));

				w32Error = ERROR_SUCCESS;
			}

			RegCloseKey(ClassKey);
		}

		RegCloseKey(EmuPublicKey);
	}
	
	if (ERROR_SUCCESS != w32Error)
	{
		CoCreateGuid(&ClassId);
	}

	return ClassId;
}

/*****************************************************************************
 * RegisterServer()
 *****************************************************************************
 */
HRESULT
RegisterServer
(
	IN		LPTSTR	TargetSymbolicLink
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[RegisterServer]"));

	HRESULT hr = E_FAIL;

    TCHAR ModuleFileName[MAX_PATH];

    if (GetModuleFileName(hDllInstance, ModuleFileName, MAX_PATH))
    {
		LPTSTR DriverName = ModuleFileName;

		CKsEnumerator * KsEnumerator = new CKsEnumerator(&hr);

		if (KsEnumerator)
		{
			if (SUCCEEDED(hr))
			{
				GUID Categories[] = { STATIC_KSCATEGORY_AUDIO, STATIC_KSCATEGORY_RENDER, STATIC_KSCATEGORY_CAPTURE };

				hr = KsEnumerator->EnumerateFilters
						(
							KS_TECHNOLOGY_TYPE_AUDIO_RENDER, 
							Categories,      
							SIZEOF_ARRAY(Categories),
							TRUE,
							FALSE,
							TRUE
						);

				if (SUCCEEDED(hr))
				{
					CKsFilter * KsFilter; 

					LISTPOS ListPos = KsEnumerator->m_ListFilters.GetHeadPosition();
				    
					_DbgPrintF(DEBUGLVL_VERBOSE,("[RegisterServer] - ksenumerator found : hr= 0x%x", hr));

					while (KsEnumerator->m_ListFilters.GetNext(ListPos, &KsFilter))
					{
 						LPTSTR SymbolicLink; KsFilter->GetSymbolicLink(&SymbolicLink);

						if ((TargetSymbolicLink == NULL) || (!_tcsicmp(TargetSymbolicLink, SymbolicLink)))
						{
 							LPTSTR FriendlyName; KsFilter->GetFriendlyName(&FriendlyName);

							_DbgPrintF(DEBUGLVL_VERBOSE,("[RegisterServer] - %s", FriendlyName));

							// Instantiate the render & capture filter for this symbolic link.
							HRESULT hr;
							
							CKsAudioRenderFilter * KsAudioRenderFilter = new CKsAudioRenderFilter(SymbolicLink, FriendlyName, &hr);

							if (SUCCEEDED(hr))
							{
								hr = KsAudioRenderFilter->Instantiate();

								if (SUCCEEDED(hr))
								{
									hr = KsAudioRenderFilter->EnumeratePins();
								}

								if (SUCCEEDED(hr))
								{
									CKsAudioCaptureFilter * KsAudioCaptureFilter = new CKsAudioCaptureFilter(SymbolicLink, FriendlyName, &hr);

									if (SUCCEEDED(hr))
									{
										hr = KsAudioCaptureFilter->Instantiate();

										if (SUCCEEDED(hr))
										{
											hr = KsAudioCaptureFilter->EnumeratePins();
										}

										if (SUCCEEDED(hr))
										{					
											GUID ClassId = LookupClassId(SymbolicLink); 
											
											// Check the pin caps to see if the filter is capable of the following
											// bit depths and register an ASIO driver.											
											WAVEFORMATEX WaveFormatEx; WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;

											UCHAR BitDepthMask = 0;

											USHORT BitDepth[] = { 8, 16, 24, 32 };

											for (ULONG i=1; i<SIZEOF_ARRAY(BitDepth); i++)
											{
												WaveFormatEx.wBitsPerSample = BitDepth[i];

												if ((KsAudioRenderFilter->FindViablePin(&WaveFormatEx, KS_FORMAT_MATCHING_CRITERIA_BIT_DEPTH)) &&
													(KsAudioCaptureFilter->FindViablePin(&WaveFormatEx, KS_FORMAT_MATCHING_CRITERIA_BIT_DEPTH)))
												{
													BitDepthMask |= 1<<i;
												}
											}

											UCHAR RateMask = 0;

											ULONG Rates[] = { 44100, 48000, 88200, 96000, 176400, 192000 };

											for (ULONG i=0; i<SIZEOF_ARRAY(Rates); i++)
											{
												WaveFormatEx.nSamplesPerSec = Rates[i];

												if ((KsAudioRenderFilter->FindViablePin(&WaveFormatEx, KS_FORMAT_MATCHING_CRITERIA_FREQUENCY)) &&
													(KsAudioCaptureFilter->FindViablePin(&WaveFormatEx, KS_FORMAT_MATCHING_CRITERIA_FREQUENCY)))
												{
													RateMask |= 1<<i;
												}
											}

											if (BitDepthMask)
											{
												// Stuff the bit depth & rate in the CLSID.
												ClassId.Data1 &= 0xFFFFFF00; ClassId.Data1 |= BitDepthMask;
												ClassId.Data1 &= 0xFFFF00FF; ClassId.Data1 |= USHORT(RateMask)<<8;

												// Add the ASIO prefix to the friendly name.
												TCHAR AsioFriendlyName[MAX_PATH];

												_stprintf(AsioFriendlyName, "ASIO %s", FriendlyName);

												ULONG Index = LookupFriendlyNames(AsioFriendlyName);

												if (Index > 1)
												{											
													_stprintf(AsioFriendlyName, "ASIO %s (%d)", FriendlyName, Index);
												}

												RegisterAsioDriver(AsioFriendlyName, ClassId, DriverName, SymbolicLink);
											}
										}

										delete KsAudioCaptureFilter;
									}
								}

								delete KsAudioRenderFilter;
							}
						}
					}
				}
			}

			delete KsEnumerator;
		}
	}

	return hr;
}

/*****************************************************************************
 * UnregisterServer()
 *****************************************************************************
 */
HRESULT 
UnregisterServer
(
	IN		LPTSTR	TargetSymbolicLink
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[UnregisterServer]"));

	HRESULT hr = S_OK;

	HKEY AsioPublicKey = NULL;

	DWORD w32Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ASIO_PUBLIC_KEY, 0, KEY_ALL_ACCESS, &AsioPublicKey);

	if (ERROR_SUCCESS == w32Error)
	{
		ULONG NumberOfSubKeys = 0;

		w32Error = RegQueryInfoKey(AsioPublicKey, NULL, NULL, NULL, &NumberOfSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		if ((ERROR_SUCCESS == w32Error) && NumberOfSubKeys)
		{
			PASIO_DRIVER_INFORMATION DriverInfo = PASIO_DRIVER_INFORMATION(LocalAlloc(LPTR, NumberOfSubKeys * sizeof(ASIO_DRIVER_INFORMATION)));

			if (DriverInfo)
			{
				ZeroMemory(DriverInfo, NumberOfSubKeys * sizeof(ASIO_DRIVER_INFORMATION));

				for (ULONG i=0; i<NumberOfSubKeys; i++)
				{
					ULONG FriendlyNameLength = MAX_PATH;

					FILETIME LastWriteTime;	

					w32Error = RegEnumKeyEx(AsioPublicKey, i, DriverInfo[i].FriendlyName, &FriendlyNameLength, NULL, NULL, NULL, &LastWriteTime);
				}

				for (ULONG i=0; i<NumberOfSubKeys; i++)
				{
					BOOL FoundAsioDriverKey = FALSE;

					// Open the key up to check if our driver is the owner to this key.
					HKEY AsioDriverKey = NULL;

					w32Error = RegOpenKeyEx(AsioPublicKey, DriverInfo[i].FriendlyName, 0, KEY_ALL_ACCESS, &AsioDriverKey);
					
					if (ERROR_SUCCESS == w32Error)
					{
						ULONG Type = 0;					
						TCHAR AsioMarker[MAX_PATH];									
						ULONG Size = sizeof(AsioMarker);
									
						w32Error = RegQueryValueEx(AsioDriverKey, NULL, 0, &Type, PBYTE(AsioMarker), &Size);

						if (ERROR_SUCCESS == w32Error)
						{
							if (!_tcsicmp(AsioMarker, ASIO_MARKER))
							{
								// Key belong us...
								ULONG Type = 0;					
								WCHAR ClsIdStr[MAX_PATH];									
								ULONG Size = sizeof(ClsIdStr);

								w32Error = RegQueryValueExW(AsioDriverKey, L"CLSID", 0, &Type, PBYTE(ClsIdStr), &Size);
								
								if (ERROR_SUCCESS == w32Error)
								{
									CLSIDFromString(ClsIdStr, &DriverInfo[i].ClassId);
								}

								if (TargetSymbolicLink)
								{
									ULONG Type = 0;					
									ULONG Size = sizeof(DriverInfo[i].SymbolicLink);

									w32Error = RegQueryValueEx(AsioDriverKey, "SymbolicLink", 0, &Type, PBYTE(DriverInfo[i].SymbolicLink), &Size);

									if (ERROR_SUCCESS == w32Error)
									{
										if (!_tcsicmp(TargetSymbolicLink, DriverInfo[i].SymbolicLink))
										{
											FoundAsioDriverKey = TRUE;
										}
									}
								}
								else
								{
									FoundAsioDriverKey = TRUE;
								}
							}
						}
						
						RegCloseKey(AsioDriverKey);
					}

					if (FoundAsioDriverKey)
					{
						UnregisterAsioDriver(DriverInfo[i].FriendlyName, DriverInfo[i].ClassId);
					}
				}

				LocalFree(DriverInfo);
			}
		}

		RegCloseKey(AsioPublicKey);
	}

	return hr;
}

/*****************************************************************************
 * DllRegisterServer()
 *****************************************************************************
 */
STDAPI 
DllRegisterServer
(   void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[DllRegisterServer]"));

	HRESULT hr = RegisterServer(NULL);

	return hr;
}

/*****************************************************************************
 * DllUnregisterServer()
 *****************************************************************************
 */
STDAPI 
DllUnregisterServer
(   void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[DllUnregisterServer]"));

	HRESULT hr = UnregisterServer(NULL);

	return hr;
}

/*****************************************************************************
 * DllInstall()
 *****************************************************************************
 */
STDAPI 
DllInstall
(
	IN		BOOL	Install,
	IN		LPCWSTR	CmdLine
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[DllInstall]"));

	HRESULT hr = S_OK;

	_DbgPrintF(DEBUGLVL_VERBOSE,("[DllInstall] - Install : %d, CmdLine : %ws", Install, CmdLine));

	if (CmdLine)
	{
		PWCHAR DevicePathW = wcsstr(CmdLine, L"/DevPath");

		if (DevicePathW)
		{
			DevicePathW += wcslen(L"/DevPath")+1;

			TCHAR SymbolicLink[MAX_PATH];

			_stprintf(SymbolicLink, "%ws", DevicePathW);

			_DbgPrintF(DEBUGLVL_BLAB,("[DllInstall] - SymbolicLink : %s", SymbolicLink));

			if (Install)
			{
				hr = RegisterServer(SymbolicLink);
			}
			else
			{
				hr = UnregisterServer(SymbolicLink);
			}
		}
	}

	return hr;
}

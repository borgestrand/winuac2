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
 * @file       enum.cpp
 * @brief      This is the implementation file for the C++ class which encapsulates
 *			   the SetupDi functions needed to enumerate KS filters.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "enum.h"
#include <setupapi.h>
#include <mmsystem.h>

#define STR_MODULENAME "KSENUM: "


/*****************************************************************************
 * CKsEnumerator::CKsEnumerator()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsEnumerator::
CKsEnumerator
(
    OUT		HRESULT *	OutHResult
)
{
    // Initialize the list
    HRESULT hr = m_ListFilters.Initialize(1);
    
    *OutHResult = hr;
}

/*****************************************************************************
 * CKsEnumerator::~CKsEnumerator()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CKsEnumerator::
~CKsEnumerator
(	void
)
{
	DestroyLists();
}

/*****************************************************************************
 * CKsEnumerator::DestroyLists()
 *****************************************************************************
 *//*!
 * @brief
 * Dumps the contents of the lists.
 */
VOID 
CKsEnumerator::
DestroyLists
(	void
)
{
	CKsFilter * KsFilter = NULL;

    // clean list of filters
    while (m_ListFilters.RemoveHead(&KsFilter))
    {
        delete KsFilter;
    }
}

/*****************************************************************************
 * CKsEnumerator::EnumerateFilters()
 *****************************************************************************
 *//*!
 * @brief
 * Enumerate filters that match the given parameters.
 */
HRESULT 
CKsEnumerator::
EnumerateFilters
(
    IN		KS_TECHNOLOGY_TYPE	FilterType,
    IN		GUID *				Categories,
    IN		ULONG				CategoriesCount,
    IN		BOOL				NeedPins,
    IN		BOOL				NeedNodes,
    IN		BOOL				Instantiate
)
{
	HRESULT hr = S_OK;

    DestroyLists();

    if (Categories &&  !IsEqualGUID(GUID_NULL, Categories[0]))
    {
	    HDEVINFO hDevInfo = NULL;

        // Get a handle to the device set specified by the guid
        hDevInfo = SetupDiGetClassDevs
					(
						&Categories[0], 
						NULL, 
						NULL, 
						DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
					);

        if (CKsIrpTarget::IsValidHandle(hDevInfo))
        {
            // Loop through members of the set and get details for each
            for (INT iClassMember = 0;; iClassMember++)
            {             
                SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
                
				DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
                DeviceInterfaceData.Reserved = 0;

                BOOL Result = SetupDiEnumDeviceInterfaces
								(
									hDevInfo, 
									NULL, 
									&Categories[0], 
									iClassMember,
									&DeviceInterfaceData
								);

                if (!Result)
                {
                    // This just means that we've enumerate all the devices - it's not a real error
                    break;
                }

                // Get details for the device registered in this class
                ULONG InterfaceDetailsSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + MAX_PATH * sizeof(WCHAR);

                PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetails = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new BYTE[InterfaceDetailsSize];

				if (!DeviceInterfaceDetails)
                {
                    _DbgPrintF(DEBUGLVL_VERBOSE, ("[CKsEnumerator::EnumFilters] - Failed to allocate SP_DEVICE_INTERFACE_DETAIL_DATA structure"));

					hr = E_OUTOFMEMORY;
                    break;
                }

                DeviceInterfaceDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                SP_DEVINFO_DATA DevInfoData;

                DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                DevInfoData.Reserved = 0;

				Result = SetupDiGetDeviceInterfaceDetail
							(
								hDevInfo, 
								&DeviceInterfaceData, 
								DeviceInterfaceDetails, 
								InterfaceDetailsSize,
								NULL, 
								&DevInfoData
							);

                if (Result)
                {
                    // 
                    // check additional category guids which may (or may not) have been supplied
                    //
                    for (ULONG iCategory = 1; iCategory < CategoriesCount && Result; iCategory++)
                    {
						SP_DEVICE_INTERFACE_DATA DeviceInterfaceDataAlias;
		                
						DeviceInterfaceDataAlias.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
						DeviceInterfaceDataAlias.Reserved = 0;

                        Result = SetupDiGetDeviceInterfaceAlias
									(
										hDevInfo,
										&DeviceInterfaceData,
										&Categories[iCategory],
										&DeviceInterfaceDataAlias
									);

                        if (!Result)
                        {
                            _DbgPrintF(DEBUGLVL_BLAB, ("[CKsEnumerator::EnumFilters] - Failed to get requested DeviceInterfaceAlias"));
                        }

                        //
                        // Check if the this interface alias is enabled.
                        //
                        if (Result)
                        {
                            if (!DeviceInterfaceDataAlias.Flags || (DeviceInterfaceDataAlias.Flags & SPINT_REMOVED))
                            {
                                _DbgPrintF(DEBUGLVL_BLAB, ("[CKsEnumerator::EnumFilters] - DeviceInterfaceAlias disabled."));

								Result = FALSE;
                            }
                        }
                    }

					TCHAR FriendlyName[MAX_PATH] = {0};

					if (Result)
					{
						// Get the device interface friendly name, if available.
						HKEY RegistryKey = SetupDiOpenDeviceInterfaceRegKey
											(
												hDevInfo,
												&DeviceInterfaceData,
												0,
												KEY_ALL_ACCESS
											);

						if (RegistryKey)
						{
							ULONG Type = 0;
							ULONG Size = sizeof(FriendlyName);

							if (ERROR_SUCCESS != RegQueryValueEx(RegistryKey, "FriendlyName", NULL, &Type, (PBYTE)FriendlyName, &Size))
							{
								// Get the device friendly name, if available.
								if (!SetupDiGetDeviceRegistryProperty
										(
											hDevInfo, 
											&DevInfoData, 
											SPDRP_FRIENDLYNAME,
											NULL,
											(PBYTE)FriendlyName, 
											sizeof(FriendlyName),
											NULL 
										))
								{
									// Last resort, fallback to the device description.
									SetupDiGetDeviceRegistryProperty
										(
											hDevInfo, 
											&DevInfoData, 
											SPDRP_DEVICEDESC,
											NULL,
											(PBYTE)FriendlyName, 
											sizeof(FriendlyName),
											NULL 
										);
								}
							}

							RegCloseKey(RegistryKey);
						}
					}

					if (Result)
                    {
						HRESULT hrFilter = S_OK;

                        CKsFilter * NewKsFilter = NULL;
                        
                        switch (FilterType)
                        {
                            case KS_TECHNOLOGY_TYPE_UNKNOWN:
							{
                                NewKsFilter = new CKsFilter(DeviceInterfaceDetails->DevicePath, FriendlyName, &hrFilter);
							}
							break;

                            case KS_TECHNOLOGY_TYPE_AUDIO_RENDER:
							{
                                NewKsFilter = new CKsAudioRenderFilter(DeviceInterfaceDetails->DevicePath, FriendlyName, &hrFilter);
							}
							break;

                            case KS_TECHNOLOGY_TYPE_AUDIO_CAPTURE:
							{
                                NewKsFilter = new CKsAudioCaptureFilter(DeviceInterfaceDetails->DevicePath, FriendlyName, &hrFilter);
							}
							break;

                            default:
							{
                                NewKsFilter = new CKsFilter(DeviceInterfaceDetails->DevicePath, FriendlyName, &hrFilter);
							}
							break;
                        }

                        if (NULL == NewKsFilter)
                        {
                            hrFilter = E_OUTOFMEMORY;
                        }
                        else
                        {
                            if (SUCCEEDED(hrFilter) && (NeedNodes || NeedPins || Instantiate))
                            {
                                hrFilter = NewKsFilter->Instantiate();

                                if (NeedNodes && SUCCEEDED(hrFilter))
								{
                                    hrFilter = NewKsFilter->EnumerateNodes();
								}

                                // This enumerates Devices for SysAudio devices
                                // For normal devices, it just enumerates the pins
                                if (NeedPins && SUCCEEDED(hrFilter))
								{
                                    hrFilter = NewKsFilter->EnumeratePins();
								}
                            }

                            if (SUCCEEDED(hrFilter))
                            {
                                if (NULL == m_ListFilters.AddTail(NewKsFilter))
                                {
                                    hrFilter = E_OUTOFMEMORY;
                                }
                            }
                        }

                        if (FAILED(hrFilter))
                        {
                            delete NewKsFilter;

                            _DbgPrintF(DEBUGLVL_ERROR, ("Failed to create filter for %s", DeviceInterfaceDetails->DevicePath));
                        }
                    }
                }

                delete[] (BYTE*)DeviceInterfaceDetails;
            } // for

	        SetupDiDestroyDeviceInfoList(hDevInfo);
        }
        else
        {
	        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsEnumerator::EnumFilters] - No devices found"));
            
			hr = E_FAIL;
        }
    }

    // If we didn't find anything, be sure to return an error code
    if (SUCCEEDED(hr) &&  m_ListFilters.IsEmpty())
    {
        hr = E_FAIL;
    }

    return hr;
}

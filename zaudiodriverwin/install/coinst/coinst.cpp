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
 * @file       coinst.cpp
 * @brief      Hula co-installer implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  06-01-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>

#include "dbg.h"

#define STR_MODULENAME "EMCOINST: "

/*****************************************************************************
 * Defines.
 */
#define HULA_SERVICE_NAME	"emaudsv"

/*****************************************************************************
 * Forward declarations.
 */

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
			DisableThreadLibraryCalls(Instance);
        }
        break;

        case DLL_PROCESS_DETACH:
        {
        }
        break;
    }

    return result;
}

/*****************************************************************************
 * CoInstallerProc()
 *****************************************************************************
 *//*!
 * @brief
 * Entry point for the co-installer.
 */
DWORD CALLBACK
CoInstallerProc 
(
   IN		DI_FUNCTION					InstallFunction,
   IN		HDEVINFO					DeviceInfoSet,
   IN		PSP_DEVINFO_DATA			DeviceInfoData,  OPTIONAL
   IN OUT	PCOINSTALLER_CONTEXT_DATA	Context
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CoInstallerProc]"));

	DWORD w32Error = NO_ERROR;

    switch (InstallFunction)
    {
		case DIF_INSTALLDEVICE:
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CoInstallerProc] - DIF_INSTALLDEVICE"));

			if (Context->PostProcessing) 
			{           
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CoInstallerProc] - DIF_INSTALLDEVICE: PostProcessing"));

				// Connect to the Service Control Manager and open the Services database.
				SC_HANDLE ScManager = OpenSCManager
										(
											NULL,                   // local machine
											NULL,                   // local database
											SC_MANAGER_ALL_ACCESS   // access required
										);

				if (ScManager)
				{
					// Open the handle to the existing service.
					SC_HANDLE Service = OpenService
										(
											ScManager,
											HULA_SERVICE_NAME,
											SERVICE_ALL_ACCESS
										);

					if (Service)
					{
						// Start the execution of the service.
						if (!StartService(Service, 0, NULL)) 
						{
							DWORD w32Error = GetLastError();

							if (w32Error == ERROR_SERVICE_ALREADY_RUNNING) 
							{
								_DbgPrintF(DEBUGLVL_BLAB,("[CoInstallerProc] - StartService: ERROR_SERVICE_ALREADY_RUNNING"));
							} 
							else 
							{
								_DbgPrintF(DEBUGLVL_BLAB,("[CoInstallerProc] - StartService: %d", w32Error));
							}

						}

						// Close the service object.
						CloseServiceHandle(Service);
					}

					// Close handle to service control manager.
					CloseServiceHandle(ScManager);
				}
 			}
			else
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CoInstallerProc] - DIF_INSTALLDEVICE: PreProcessing"));

				w32Error = ERROR_DI_POSTPROCESSING_REQUIRED; // Set for PostProcessing
			}   
		}
		break;

		default:
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CoInstallerProc] - DIF_XXX: 0x%x", InstallFunction));
		}
		break;
    }
    
    return w32Error;    
}


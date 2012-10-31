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
 * @file       Main.cpp
 * @brief      Driver entry and exit points implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */

/*! @brief Debug module name. */
#define STR_MODULENAME "Main: "

#include "KsAdapter.h"


/*****************************************************************************
 * Defines
 */
/*****************************************************************************
 * Externals
 */

/*****************************************************************************
 * DESCRIPTOR AND DISPATCH LAYOUT
 *****************************************************************************
 */

/*****************************************************************************
 * KsDeviceDispatch
 *****************************************************************************
 * This is the dispatch table for the adapter.  Plug and play notifications 
 * as well as power management notifications are dispatched through this table.
 */
const
KSDEVICE_DISPATCH KsDeviceDispatch = 
{
    CKsAdapter::DispatchCreate,					// Pnp Add Device
    CKsAdapter::DispatchPnpStart,				// Pnp Start
    NULL,										// Post-Start
    CKsAdapter::DispatchPnpQueryStop,			// Pnp Query Stop
    CKsAdapter::DispatchPnpCancelStop,			// Pnp Cancel Stop
    CKsAdapter::DispatchPnpStop,				// Pnp Stop
    CKsAdapter::DispatchPnpQueryRemove,			// Pnp Query Remove
    CKsAdapter::DispatchPnpCancelRemove,		// Pnp Cancel Remove
    CKsAdapter::DispatchPnpRemove,				// Pnp Remove
    CKsAdapter::DispatchPnpQueryCapabilities,	// Pnp Query Capabilities
    CKsAdapter::DispatchPnpSupriseRemoval,		// Pnp Surprise Removal
    NULL,										// Power Query Power
    NULL,										// Power Set Power
    NULL										// Pnp Query Interface
};

/*****************************************************************************
 * KsDeviceDescriptor
 *****************************************************************************
 * This is the device descriptor for the adapter.  It points to the dispatch 
 * table and contains a list of filter descriptors that describe filter-types 
 * that this device supports.  Note that the filter-descriptors can be created 
 * dynamically and the factories created via KsCreateFilterFactory as well.  
 */
const
KSDEVICE_DESCRIPTOR KsDeviceDescriptor = 
{
    &KsDeviceDispatch,
	0,
    NULL,
    KSDEVICE_DESCRIPTOR_VERSION
};

#pragma code_seg("PAGE")

/*****************************************************************************
 * DriverEntry()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when the driver is loaded.
 * @details
 * Pass off control to the AVStream initialization function (KsInitializeDriver) 
 * and return the status code from it.
 * @param
 * DriverObject Pointer to the DRIVER_OBJECT structure.
 * @param
 * RegistryPathName Pointer to a unicode string storing the registry path
 * name.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
extern "C"
NTSTATUS
DriverEntry
(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPathName
)
{
    PAGED_CODE();

    _DbgPrintF( DEBUGLVL_VERBOSE, ("Starting breakpoint for debugging"));


	//
    // Tell the class driver to initialize the driver.
    //
	NTSTATUS ntStatus = KsInitializeDriver(DriverObject, RegistryPathName, &KsDeviceDescriptor);

	if (NT_SUCCESS(ntStatus))
	{
        // Install the shutdown dispatch handler.
        DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = CKsAdapter::DispatchShutdown;
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * _purecall()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * The C++ compiler loves me.
 * @details
 * TODO: Figure out how to put this into portcls.sys
 */
int __cdecl
_purecall( void )
{
    ASSERT( !"Pure virtual function called" );
    return 0;
}

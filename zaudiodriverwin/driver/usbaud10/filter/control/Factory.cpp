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
 * @file       Factory.cpp
 * @brief      Device control filter factory implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "Factory.h"

#include "Filter.h"
#include "PrvProp.h"
#include "Tables.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "CTRL_FILTER_FACTORY: "

/*****************************************************************************
 * Referenced forward
 */

#pragma code_seg("PAGE")

/*****************************************************************************
 * CreateControlFilterFactory()
 *****************************************************************************
 *//*!
 * @brief
 * Filter factory create function.
 * @details
 * Creates a device control filter factory.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CreateControlFilterFactory
(
    IN		PKSDEVICE	KsDevice,
	IN		PWCHAR		RefString,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CControlFilterFactory::Instantiate]"));

	NTSTATUS ntStatus;

	CControlFilterFactory * FilterFactory = new(NonPagedPool,'tSvA') CControlFilterFactory(NULL);

	if (FilterFactory)
	{
		FilterFactory->AddRef();

		ntStatus = FilterFactory->Init(KsDevice, Parameter1, Parameter2);

		PKSFILTER_DESCRIPTOR KsFilterDescriptor = NULL;

		if (NT_SUCCESS(ntStatus))
		{
			ntStatus = FilterFactory->GetFilterDescription(&KsFilterDescriptor);
		}

		if (NT_SUCCESS(ntStatus))
		{
			#define PROXY_CLSID	L"{17CCA71B-ECD7-11D0-B908-00A0C9223196}"

			PKSADAPTER KsAdapter = PKSADAPTER(KsDevice->Context);

			for (ULONG i = 0; i < KsFilterDescriptor->CategoriesCount; i++)
			{
				// Do what normally done by INF AddInterface directive.
				KsAdapter->AddSubDeviceInterface(RefString, KsFilterDescriptor->Categories[i]);

				KsAdapter->SetSubDeviceParameter(RefString, KsFilterDescriptor->Categories[i], L"CLSID", REG_SZ, PROXY_CLSID, sizeof(PROXY_CLSID));
			}

			CControlFilterFactory::SetupFriendlyName(KsAdapter, KsFilterDescriptor, RefString);

			KsAcquireDevice(KsDevice);

			PKSFILTERFACTORY KsFilterFactory = NULL;

			ntStatus = KsCreateFilterFactory
							(
								KsDevice->FunctionalDeviceObject, 
								KsFilterDescriptor, 
								RefString,
								NULL,
								0,
								NULL,//CControlFilter::SleepCallback,
								NULL,//CControlFilter::WakeCallback,
								&KsFilterFactory
							);

			if (NT_SUCCESS(ntStatus))
			{
				KsFilterFactory->Context = PVOID(FilterFactory);
			}

			KsReleaseDevice(KsDevice);
		}

		if (NT_SUCCESS(ntStatus))
		{
			//
			// Add the item to the object bag if we were successful. Whenever the device goes 
			// away, the bag is cleaned up and we will be freed.
			//
			// For backwards compatibility with DirectX 8.0, we must grab the device mutex 
			// before doing this.  For Windows XP, this is not required, but it is still safe.
			//
			KsAcquireDevice(KsDevice);

			ntStatus = KsAddItemToObjectBag(KsDevice->Bag, FilterFactory, (PFNKSFREE)CControlFilterFactory::Destruct);

			KsReleaseDevice(KsDevice);
		}

		if (NT_SUCCESS(ntStatus))
		{
			// Keeping this object...
			FilterFactory->AddRef();
		}

		// Release the private reference.
		FilterFactory->Release();
	}
	else
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
 
    return ntStatus;
}

/*****************************************************************************
 * CControlFilterFactory::SetupFriendlyName()
 *****************************************************************************
 *//*!
 * @brief
 * Change the generic device name to match the USB device ones.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CControlFilterFactory::
SetupFriendlyName
(
	IN		PKSADAPTER				KsAdapter,
	IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
	IN		PWCHAR					RefString
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CControlFilterFactory::SetupFriendlyName]"));

	PUSB_DEVICE UsbDevice = KsAdapter->GetUsbDevice();

	UCHAR iIndex = 0;

	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor = NULL;

	NTSTATUS ntStatus = UsbDevice->GetDeviceDescriptor(&DeviceDescriptor);

	if (NT_SUCCESS(ntStatus))
	{
		iIndex = DeviceDescriptor->iProduct;
	}

	if (iIndex)
	{
		struct
		{
			UCHAR	bLength;
			UCHAR	bDescriptorType;
			WCHAR	bString[126];
		} FriendlyNameDescriptor;

		RtlZeroMemory(&FriendlyNameDescriptor, sizeof(FriendlyNameDescriptor));

		FriendlyNameDescriptor.bLength = sizeof(FriendlyNameDescriptor);

		USHORT LanguageId = KsAdapter->GetLanguageId();

		ntStatus = UsbDevice->GetStringDescriptor(iIndex, LanguageId, PUSB_STRING_DESCRIPTOR(&FriendlyNameDescriptor));

		if (NT_SUCCESS(ntStatus))
		{
			for (ULONG i = 0; i < KsFilterDescriptor->CategoriesCount; i++)
			{
				KsAdapter->SetSubDeviceParameter(RefString, KsFilterDescriptor->Categories[i], L"FriendlyName", REG_SZ, FriendlyNameDescriptor.bString, sizeof(FriendlyNameDescriptor.bString));
			}
		}
	}
}

/*****************************************************************************
 * CControlFilterFactory::Destruct()
 *****************************************************************************
 *//*!
 * @brief
 * This is the free callback for the bagged filter factory.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CControlFilter object.
 * @return
 * None.
 */
VOID
CControlFilterFactory::
Destruct 
(
	IN		PVOID	Self
)
{
	CControlFilterFactory * FilterFactory = (CControlFilterFactory *)(Self);

	FilterFactory->Release();
}

/*****************************************************************************
 * CControlFilterFactory::NonDelegatingQueryInterface()
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
 * Returns STATUS_SUCCESS if the interface is found. Otherwise, returns
 * STATUS_INVALID_PARAMETER.
 */
STDMETHODIMP
CControlFilterFactory::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CControlFilterFactory::NonDelegatingQueryInterface]"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        //
        // We reference the interface for the caller.
        //
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

/*****************************************************************************
 * CControlFilterFactory::~CControlFilterFactory()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CControlFilterFactory::
~CControlFilterFactory
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[~CControlFilterFactory::~CControlFilterFactory]"));

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}

	if (m_KsAdapter)
    {
        m_KsAdapter->Release();
    }
}

/*****************************************************************************
 * CControlFilterFactory::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initializes the filter factory.
 * @details
 * The caller of @b Init should run at IRQL_PASSIVE_LEVEL.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CControlFilterFactory::
Init
(
	IN		PKSDEVICE	KsDevice,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
)
{
    PAGED_CODE();

    ASSERT(KsDevice);

    _DbgPrintF(DEBUGLVL_BLAB,("[CControlFilterFactory::Init]"));

	m_KsDevice = KsDevice;

	m_KsAdapter = PKSADAPTER(KsDevice->Context);
	m_KsAdapter->AddRef();

	m_UsbDevice = m_KsAdapter->GetUsbDevice();
	m_UsbDevice->AddRef();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	// Normally you would figure out what the filter descriptor here...

	if (!NT_SUCCESS(ntStatus))
	{
		// Cleanup the mess...
		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}

		if (m_KsAdapter)
		{
			m_KsAdapter->Release();
			m_KsAdapter = NULL;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CControlFilterFactory::GetDescription()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the filter description.
 * @details
 * Gets a pointer to a filter description. It provides a location
 * to deposit a pointer in miniport's description structure. This is the
 * placeholder for the FromNode or ToNode fields in connections which
 * describe connections to the filter's pins.
 * @param
 * OutKsFilterDescriptor Pointer to the filter description.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS 
CControlFilterFactory::
GetFilterDescription
(
	OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutKsFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CControlFilterFactory::GetDescription]"));

    *OutKsFilterDescriptor = PKSFILTER_DESCRIPTOR(&KsFilterDescriptor);

    return STATUS_SUCCESS;
}

#pragma code_seg()


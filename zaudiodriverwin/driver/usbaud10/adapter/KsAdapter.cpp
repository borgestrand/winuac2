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
 * @file       KsAdapter.cpp
 * @brief      Implementation of the KS adapter.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */

/*! @brief Debug module name. */
#define STR_MODULENAME "KsAdapter: "

#include "KsAdapter.h"

/*****************************************************************************
 * Defines
 */

/*****************************************************************************
 * Externals
 */
NTSTATUS
CreateControlFilterFactory
(
	IN		PKSDEVICE	KsDevice,
	IN		PWCHAR		RefString,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
);

NTSTATUS
CreateAudioFilterFactory
(
	IN		PKSDEVICE	KsDevice,
	IN		PWCHAR		RefString,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
);

NTSTATUS
CreateMidiFilterFactory
(
	IN		PKSDEVICE	KsDevice,
	IN		PWCHAR		RefString,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
);

/*****************************************************************************
 * Referenced forward
 */

/*****************************************************************************
 * CKsAdapter::DispatchCreate()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Add Device dispatch for the adapter.  It creates
 * the CKsAdapter and associates it with the KS device via the bag.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
DispatchCreate
(
    IN		PKSDEVICE	KsDevice
)
{
    PAGED_CODE();

    NTSTATUS ntStatus;

	CKsAdapter * KsAdapter = new (NonPagedPool) CKsAdapter(NULL);

	if (KsAdapter)
	{
		KsAdapter->AddRef();

		ntStatus = KsAdapter->Init(KsDevice);

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

			ntStatus = KsAddItemToObjectBag(KsDevice->Bag, KsAdapter, (PFNKSFREE)CKsAdapter::Destruct);

			KsReleaseDevice(KsDevice);
		}

		if (NT_SUCCESS(ntStatus))
		{
			// Keeping this object...
			KsAdapter->AddRef();

			KsDevice->Context = PVOID(KsAdapter);
		}

		// Release the private reference.
		KsAdapter->Release();
	}
	else
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpStart()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Start dispatch for the adapter.  It simply bridges to
 * StartDevice() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
DispatchPnpStart
(
    IN		PKSDEVICE			KsDevice,
    IN		PIRP				Irp,
    IN		PCM_RESOURCE_LIST	TranslatedResourceList,
    IN		PCM_RESOURCE_LIST	UntranslatedResourceList
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	NTSTATUS ntStatus = KsAdapter->StartDevice(TranslatedResourceList, UntranslatedResourceList);

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpQueryStop()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Query Stop dispatch for the adapter.  It simply bridges to
 * QueryStopDevice() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
DispatchPnpQueryStop
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	NTSTATUS ntStatus = KsAdapter->QueryStopDevice();

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpCancelStop()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Cancel Stop dispatch for the adapter.  It simply bridges to
 * CancelStopDevice() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * None.
 */
VOID
CKsAdapter::
DispatchPnpCancelStop
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	KsAdapter->CancelStopDevice();
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpStop()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Stop dispatch for the adapter.  It simply bridges to
 * StopDevice() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
DispatchPnpStop
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	KsAdapter->StopDevice();
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpQueryRemove()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Query Remove dispatch for the adapter.  It simply bridges to
 * QueryStopRemove() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
DispatchPnpQueryRemove
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	NTSTATUS ntStatus = KsAdapter->QueryRemoveDevice();

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpCancelRemove()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Remove Stop dispatch for the adapter.  It simply bridges to
 * CancelRemoveDevice() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
DispatchPnpCancelRemove
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	KsAdapter->CancelRemoveDevice();
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpRemove()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Remove dispatch for the adapter.  It simply bridges to
 * RemoveDevice() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
DispatchPnpRemove
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	KsAdapter->RemoveDevice();
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpSupriseRemoval()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Suprise Removal dispatch for the adapter.  It simply
 * bridges to SupriseRemoval() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
DispatchPnpSupriseRemoval
(
    IN		PKSDEVICE	KsDevice,
    IN		PIRP		Irp
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	KsAdapter->SupriseRemoval();
}

/*****************************************************************************
 * CKsAdapter::DispatchPnpQueryCapabilities()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the Pnp Query Capabilities dispatch for the adapter.  It simply
 * bridges to QueryCapabilities() in the context of the CKsAdapter.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
DispatchPnpQueryCapabilities
(
    IN		PKSDEVICE				KsDevice,
    IN		PIRP					Irp,
	IN	OUT	PDEVICE_CAPABILITIES	Capabilities
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

	NTSTATUS ntStatus = KsAdapter->QueryCapabilities(Capabilities);

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::DispatchShutdown()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * IRP_MJ_SHUTDOWN handler. It simply bridges to Shutdown() in the context
 * of the CKsAdapter.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
DispatchShutdown
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
)
{
	PKSDEVICE KsDevice = KsGetDeviceForDeviceObject(DeviceObject);

	if (KsDevice)
	{
		CKsAdapter * KsAdapter = (CKsAdapter *)(KsDevice->Context);

		KsAdapter->Shutdown();
	}

    NTSTATUS ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;

    // Complete the IRP.
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::Destruct()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This is the free callback for the bagged adapter.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CKsAdapter object.
 * @return
 * None.
 */
VOID
CKsAdapter::
Destruct
(
	IN		PVOID	Self
)
{
	CKsAdapter * KsAdapter = (CKsAdapter *)(Self);

	KsAdapter->Release();
}

/*****************************************************************************
 * CKsAdapter::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize a KS adapter object.
 * @param
 * KsDevice Pointer to the KSDEVICE structure representing the AVStream
 * device.
  * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
Init
(
    IN      PKSDEVICE	KsDevice
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::Init]"));

	m_KsDevice = KsDevice;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CKsAdapter::~CKsAdapter()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CKsAdapter::
~CKsAdapter
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_TERSE,("[CKsAdapter::~CKsAdapter]"));

    // Unregister the shutdown notification.
	if (m_ShutdownNotification)
	{
		IoUnregisterShutdownNotification(m_KsDevice->FunctionalDeviceObject);
		m_ShutdownNotification = FALSE;
	}

	SaveSettingsToRegistry();

	while (m_DeviceUsageCount)
    {
        LARGE_INTEGER DueTime;
        DueTime.QuadPart = -500000; // 50ms
        KeDelayExecutionThread(KernelMode, FALSE, &DueTime);
    }

	if (m_MidiDevice)
    {
        m_MidiDevice->Release();
        m_MidiDevice = NULL;
    }

    if (m_AudioDevice)
    {
        m_AudioDevice->Release();
        m_AudioDevice = NULL;
    }

    if (m_UsbDevice)
    {
		m_UsbDevice->StopDevice();
        m_UsbDevice->Release();
        m_UsbDevice = NULL;
    }
}

/*****************************************************************************
 * CKsAdapter::NonDelegatingQueryInterface()
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
CKsAdapter::
NonDelegatingQueryInterface
(
    IN		REFIID	Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IKsAdapter))
    {
        *Object = PVOID(PKSADAPTER(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

/*****************************************************************************
 * CKsAdapter::StartDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when the device is started.
 * @details
 * It is responsible for starting the filters.  This code is specific to
 * the adapter because it calls out filters for functions that are specific
 * to the adapter.
 * @param
 * TranslatedResourceList The translated resource list.
 * @param
 * UntranslatedResourceList The translated untranslated list.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
StartDevice
(
    IN		PCM_RESOURCE_LIST	TranslatedResourceList,
    IN		PCM_RESOURCE_LIST	UntranslatedResourceList
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::StartDevice]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

    // By PnP, it's possible to receive multiple starts without an intervening
    // stop (to reevaluate resources, for example).  Thus, we only perform
    // initialization on the initial start and ignore any subsequent start.
    if ((!m_KsDevice->Started) && DEVICE_READY_TO_START(m_PnpState))
	{
		// See if the device is connected or not...
		if (!(m_DeviceStatus & DEVICE_STATUS_NOT_CONNECTED))
		{
			// Assign the resources, and start the device.
			ntStatus = ConfigureDevice();

			if (NT_SUCCESS(ntStatus))
			{
				ntStatus = InitializeAudio();
			}

			if (NT_SUCCESS(ntStatus))
			{
				ntStatus = InitializeMidi();
			}

			if (NT_SUCCESS(ntStatus))
			{
				// Setup the language support.
				InitializeLanguageSupport();

				// Get the name of the device from the hardware, and updated the FriendlyName as it
				// appeared in Device Manager. The device name defaults to whatever the INF says if it is not
				// updated.
				PUSB_DEVICE_DESCRIPTOR DeviceDescriptor = NULL;

				m_UsbDevice->GetDeviceDescriptor(&DeviceDescriptor);

				if (DeviceDescriptor)
				{
					if (DeviceDescriptor->iProduct)
					{
						struct
						{
							UCHAR	bLength;
							UCHAR	bDescriptorType;
							WCHAR	bString[126];
						} FriendlyNameDescriptor;

						RtlZeroMemory(&FriendlyNameDescriptor, sizeof(FriendlyNameDescriptor));

						FriendlyNameDescriptor.bLength = sizeof(FriendlyNameDescriptor);

						m_UsbDevice->GetStringDescriptor(DeviceDescriptor->iProduct, m_LanguageId, PUSB_STRING_DESCRIPTOR(&FriendlyNameDescriptor));

						// Update the friendly name instead of device decription. Fix WHQL issue!
						SetDeviceProperty(DevicePropertyFriendlyName, (wcslen(FriendlyNameDescriptor.bString)+1) * sizeof(WCHAR), FriendlyNameDescriptor.bString);
					}
				}
			}

			if (NT_SUCCESS(ntStatus))
			{
				ntStatus = InstallFilterFactories();
			}

			// The device status can change if the device is disconnect while it is starting.
			// Check if the device is still connected.
			if (NT_SUCCESS(ntStatus))
			{
				if (m_DeviceStatus & DEVICE_STATUS_NOT_CONNECTED)
				{
					ntStatus = STATUS_DEVICE_NOT_CONNECTED;
				}
			}

			if (NT_SUCCESS(ntStatus))
			{
				// Restore the driver settings.
				RestoreSettingsFromRegistry();

				// Register for shutdown notification.
				m_ShutdownNotification = NT_SUCCESS(IoRegisterShutdownNotification(m_KsDevice->FunctionalDeviceObject));

				m_PnpState = PNP_STATE_STARTED;
			}
		}
	}

	if (!NT_SUCCESS(ntStatus))
	{
		// Clean up our mess...
		if (m_MidiDevice)
		{
			m_MidiDevice->Release();
			m_MidiDevice = NULL;
		}

		if (m_AudioDevice)
		{
			m_AudioDevice->Release();
			m_AudioDevice = NULL;
		}

		if (m_UsbDevice)
		{
			m_UsbDevice->StopDevice();
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::QueryStopDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system to determine whether the
 * device can be stopped.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
QueryStopDevice
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::QueryStopDevice]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_PnpState |= PNP_STATE_PENDING;

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::CancelStopDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system to cancel the pending stop
 * of the device.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
CancelStopDevice
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::CancelStopDevice]"));

	m_PnpState &= ~PNP_STATE_PENDING;
}

/*****************************************************************************
 * CKsAdapter::StopDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when the device is stopped.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
StopDevice
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::StopDevice]"));

    // Unregister the shutdown notification.
	if (m_ShutdownNotification)
	{
		IoUnregisterShutdownNotification(m_KsDevice->FunctionalDeviceObject);
		m_ShutdownNotification = FALSE;
	}

	SaveSettingsToRegistry();

	while (m_DeviceUsageCount)
    {
        LARGE_INTEGER DueTime;
        DueTime.QuadPart = -500000; // 50ms
        KeDelayExecutionThread(KernelMode, FALSE, &DueTime);
    }

	if (m_MidiDevice)
    {
        m_MidiDevice->Release();
        m_MidiDevice = NULL;
    }

    if (m_AudioDevice)
    {
        m_AudioDevice->Release();
        m_AudioDevice = NULL;
    }

    if (m_UsbDevice)
    {
		m_UsbDevice->StopDevice();
        m_UsbDevice->Release();
        m_UsbDevice = NULL;
    }

	m_PnpState = PNP_STATE_STOPPED;
}

/*****************************************************************************
 * CKsAdapter::QueryRemoveDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system to determine whether the
 * device can be removed.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
QueryRemoveDevice
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::QueryRemoveDevice]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_PnpState |= PNP_STATE_PENDING;

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::CancelRemoveDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system to cancel the pending removal
 * of the device.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
CancelRemoveDevice
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::CancelRemoveDevice]"));

	m_PnpState &= ~PNP_STATE_PENDING;
}

/*****************************************************************************
 * CKsAdapter::RemoveDevice()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when the device is removed.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
RemoveDevice
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::RemoveDevice]"));

	if (DEVICE_READY_FOR_REMOVAL(m_PnpState))
	{
		m_PnpState = PNP_STATE_REMOVED;

		m_DeviceStatus |= DEVICE_STATUS_NOT_CONNECTED;
	}
}

/*****************************************************************************
 * CKsAdapter::SupriseRemoval()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when the device is removed
 * without stopping, ie suprise removal.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
SupriseRemoval
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::SupriseRemoval]"));

    if (DEVICE_READY_FOR_SUPRISE_REMOVAL(m_PnpState))
    {
		m_PnpState = PNP_STATE_SURPRISE_REMOVAL;

		m_DeviceStatus |= DEVICE_STATUS_NOT_CONNECTED;
    }
}

/*****************************************************************************
 * CKsAdapter::QueryCapabilities()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system to query the device
 * capabilities.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
QueryCapabilities
(
	IN	OUT	PDEVICE_CAPABILITIES	Capabilities
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::QueryCapabilities]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

    // Set the SurpiseremovalOK capability to true because this device
    // can safely tolerate suprise remove, and we want to avoid
    // the unsafe device removal UI.
	Capabilities->SurpriseRemovalOK = TRUE;

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::Shutdown()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when it is shutting down.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CKsAdapter::
Shutdown
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::Shutdown]"));

	SaveSettingsToRegistry();
}

/*****************************************************************************
 * CKsAdapter::ConfigureDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Configures the hardware to use the indicated resources.
 * @param
 * ResourceList The resource list from StartDevice().
 * @return
 * Returns STATUS_SUCCESS iff the configuration is valid, otherwise returns
 * an apprioriate error code.
 */
NTSTATUS
CKsAdapter::
ConfigureDevice
(	void
)
{
    PAGED_CODE();

    //ASSERT(ResourceList);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::ConfigureDevice]"));

	NTSTATUS ntStatus;

	// Setup your hardware here.
    m_UsbDevice = new (NonPagedPool) CUsbDevice(NULL);

	if (m_UsbDevice)
	{
		m_UsbDevice->AddRef();

        ntStatus = m_UsbDevice->Init(m_KsDevice->NextDeviceObject, UsbDeviceCallbackRoutine, this);
	}
	else
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = m_UsbDevice->StartDevice();
	}

	if (!NT_SUCCESS(ntStatus))
	{
        // Clean up our mess...
		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::InitializeAudio()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the audio hardware.
 * @param
 * ResourceList The resource list from StartDevice()
 * @return
 * Returns STATUS_SUCCESS if the resource requirements is met. Otherwise
 * returns an appropriate error code.
 */
NTSTATUS
CKsAdapter::
InitializeAudio
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeAudio]"));

	NTSTATUS ntStatus;

	// Setup your MIDI hardware here if it is necessary.
    m_AudioDevice = new(NonPagedPool) CAudioDevice(NULL);

    if (m_AudioDevice)
    {
        m_AudioDevice->AddRef();

        ntStatus = m_AudioDevice->Init(m_UsbDevice);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	if (!NT_SUCCESS(ntStatus))
	{
        // Clean up our mess...
		if (m_AudioDevice)
		{
			m_AudioDevice->Release();
			m_AudioDevice = NULL;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::InitializeMidi()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the MIDI hardware.
 * @param
 * ResourceList The resource list from StartDevice()
 * @return
 * Returns STATUS_SUCCESS if the resource requirements is met. Otherwise
 * returns an appropriate error code.
 */
NTSTATUS
CKsAdapter::
InitializeMidi
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeMidi]"));

	NTSTATUS ntStatus;

	// Setup your MIDI hardware here if it is necessary.
    m_MidiDevice = new(NonPagedPool) CMidiDevice(NULL);

    if (m_MidiDevice)
    {
        m_MidiDevice->AddRef();

        ntStatus = m_MidiDevice->Init(m_UsbDevice);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	if (!NT_SUCCESS(ntStatus))
	{
        // Clean up our mess...
		if (m_MidiDevice)
		{
			m_MidiDevice->Release();
			m_MidiDevice = NULL;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CHAR2HEX()
 *****************************************************************************
 *//*!
 * @brief
 * Converts character to hex value.
 * @param
 * c Character
 * @return
 * Returns the hexadecimal value.
 */
inline
BYTE
CHAR2HEX
(
    IN      WCHAR   c
)
{
    PAGED_CODE();

    switch (c)
    {
        case '0': return(0x00);
        case '1': return(0x01);
        case '2': return(0x02);
        case '3': return(0x03);
        case '4': return(0x04);
        case '5': return(0x05);
        case '6': return(0x06);
        case '7': return(0x07);
        case '8': return(0x08);
        case '9': return(0x09);
        case 'a':
        case 'A': return(0x0A);
        case 'b':
        case 'B': return(0x0B);
        case 'c':
        case 'C': return(0x0C);
        case 'd':
        case 'D': return(0x0D);
        case 'e':
        case 'E': return(0x0E);
        case 'f':
        case 'F': return(0x0F);
        default : return(0xFF);
    }
}

/*****************************************************************************
 * CKsAdapter::InitializeLanguageSupport()
 *****************************************************************************
 *//*!
 * @brief
 * Initialize the language support.
 * @param
 * <None>
 * @return
 * Returns STATUS_SUCCESS if the resource requirements is met. Otherwise
 * returns an appropriate error code.
 */
NTSTATUS
CKsAdapter::
InitializeLanguageSupport
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport]"));

	//HKR,Language,LANGFILE,,%LANGFILE%
	WCHAR LangFile[MAX_PATH] = {0};

	if (NT_SUCCESS(RegistryReadFromDriverSubKey(L"Language", L"LANGFILE", LangFile, sizeof(LangFile), NULL, NULL)))
	{
		_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Language file: %ws", LangFile));

		m_UsbDevice->SetupLanguageSupport(LangFile);
	}
	else
	{
		m_UsbDevice->SetupLanguageSupport(NULL);
	}

	// Get the preferred language id. Default to US English.
	ULONG PreferredLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

	//HKR,Language,LANGID,,%LANGID%
	WCHAR LangIdString[8] = {0};

	if (NT_SUCCESS(RegistryReadFromDriverSubKey(L"Language", L"LANGID", LangIdString, sizeof(LangIdString), NULL, NULL)))
	{
		if (wcslen(LangIdString) >= 4)
		{
			PreferredLanguageId = (USHORT(CHAR2HEX(LangIdString[0]))<<12) | (USHORT(CHAR2HEX(LangIdString[1]))<<8) |
								  (USHORT(CHAR2HEX(LangIdString[2]))<<4)  | (USHORT(CHAR2HEX(LangIdString[3])));
		}
	}

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Preferred language id: 0x%04X", PreferredLanguageId));

	struct
	{
		UCHAR	bLength;
		UCHAR	bDescriptorType;
		USHORT	wLANGID[126];
	} LanguageDescriptor;

	LanguageDescriptor.bLength = sizeof(LanguageDescriptor);

	BOOL LanguageSupportFound = FALSE;

	// Get the list of supported languages.
	NTSTATUS ntStatus = m_UsbDevice->GetStringDescriptor(0, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), PUSB_STRING_DESCRIPTOR(&LanguageDescriptor));

	if (NT_SUCCESS(ntStatus))
	{
		ULONG NumberOfSupportedLanguages = (LanguageDescriptor.bLength - 2)/2;

		// First, look for the LANGID values in the LanguageDescriptor that match the preferred LANGID.
		// If an exact match is found, use that LANGID.
		for (ULONG i=0; i<NumberOfSupportedLanguages; i++)
		{
			if (LanguageDescriptor.wLANGID[i] == USHORT(PreferredLanguageId))
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Using preferred language: 0x%04X", LanguageDescriptor.wLANGID[i]));

				m_LanguageId = LanguageDescriptor.wLANGID[i];

				LanguageSupportFound = TRUE;

				break;
			}
		}

		if (!LanguageSupportFound)
		{
			// Otherwise, look next for a match to the LANG_XXX value with the SUBLANG_NEUTRAL as the SUBLANG_XXX.
			// If such a match is found, use that LANGID.
			for (ULONG i=0; i<NumberOfSupportedLanguages; i++)
			{
				if (LanguageDescriptor.wLANGID[i] == MAKELANGID(PRIMARYLANGID(USHORT(PreferredLanguageId)), SUBLANG_NEUTRAL))
				{
					_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Matched language neutral LANGID: 0x%04X", LanguageDescriptor.wLANGID[i]));

					m_LanguageId = LanguageDescriptor.wLANGID[i];

					LanguageSupportFound = TRUE;

					break;
				}
			}
		}

		if (!LanguageSupportFound)
		{
			// Otherwise, look next for a match to the LANG_XXX value and any valid SUBLANG_XXX for the same
			// LANG_XXX family. If such a partial match is found, use that LANGID.
			for (ULONG i=0; i<NumberOfSupportedLanguages; i++)
			{
				if (PRIMARYLANGID(LanguageDescriptor.wLANGID[i]) == PRIMARYLANGID(USHORT(PreferredLanguageId)))
				{
					_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Match primary language LANGID: 0x%04X", PRIMARYLANGID(LanguageDescriptor.wLANGID[i])));

					m_LanguageId = LanguageDescriptor.wLANGID[i];

					LanguageSupportFound = TRUE;

					break;
				}
			}
		}

		if (!LanguageSupportFound)
		{
			// Otherwise, use the default US English.
			for (ULONG i=0; i<NumberOfSupportedLanguages; i++)
			{
				if (LanguageDescriptor.wLANGID[i] == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
				{
					_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Using US English: 0x%04X", LanguageDescriptor.wLANGID[i]));

					m_LanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

					LanguageSupportFound = TRUE;

					break;
				}
			}
		}

		if (!LanguageSupportFound)
		{
			// Oh crap... no US English, use the first available language.
			if (NumberOfSupportedLanguages > 0)
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - Using first available language: 0x%04X", LanguageDescriptor.wLANGID[0]));

				m_LanguageId = LanguageDescriptor.wLANGID[0];

				LanguageSupportFound = TRUE;
			}
		}
	}

	if (!LanguageSupportFound)
	{
		_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::InitializeLanguageSupport] - No language support found."));

		// There is no strings available for this device in the firmware or in the string database.
		// Whatever... let it be US English.
		m_LanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	}

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::SaveSettingsToRegistry()
 *****************************************************************************
 *//*!
 */
NTSTATUS
CKsAdapter::
SaveSettingsToRegistry
(	void
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_AudioDevice)
	{
		ULONG SizeOfParameterBlocks = m_AudioDevice->GetSizeOfParameterBlocks();

		if (SizeOfParameterBlocks)
		{
			PUCHAR ParameterBlocks = PUCHAR(ExAllocatePoolWithTag(PagedPool, SizeOfParameterBlocks, 'mdW'));

			if (ParameterBlocks)
			{
				ntStatus = m_AudioDevice->SaveParameterBlocks(ParameterBlocks, SizeOfParameterBlocks, &SizeOfParameterBlocks);

				if (NT_SUCCESS(ntStatus))
				{
					ntStatus = RegistryWriteToDriverSubKey(L"Settings", L"0", ParameterBlocks, SizeOfParameterBlocks, REG_BINARY);
				}

				ExFreePool(ParameterBlocks);
			}
			else
			{
				ntStatus = STATUS_NO_MEMORY;
			}
		}
		else
		{
			ntStatus = STATUS_SUCCESS;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::RestoreSettingsFromRegistry()
 *****************************************************************************
 *//*!
 */
NTSTATUS
CKsAdapter::
RestoreSettingsFromRegistry
(	void
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	if (m_AudioDevice)
	{
		ULONG SizeOfParameterBlocks = m_AudioDevice->GetSizeOfParameterBlocks();

		if (SizeOfParameterBlocks)
		{
			PUCHAR ParameterBlocks = PUCHAR(ExAllocatePoolWithTag(PagedPool, SizeOfParameterBlocks, 'mdW'));

			if (ParameterBlocks)
			{
				ntStatus = RegistryReadFromDriverSubKey(L"Settings", L"0", ParameterBlocks, SizeOfParameterBlocks, &SizeOfParameterBlocks, NULL);

				if (NT_SUCCESS(ntStatus))
				{
					ntStatus = m_AudioDevice->RestoreParameterBlocks(ParameterBlocks, SizeOfParameterBlocks);
				}

				ExFreePool(ParameterBlocks);
			}
			else
			{
				ntStatus = STATUS_NO_MEMORY;
			}
		}
		else
		{
			ntStatus = STATUS_SUCCESS;
		}
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CKsAdapter::GetKsDevice()
 *****************************************************************************
 *//*!
 * Gets a pointer to the KS device.
 * @param
 * None
 * @return
 * Returns the pointer to the KS device.
 */
STDMETHODIMP_(PKSDEVICE)
CKsAdapter::
GetKsDevice
(   void
)
{
    return m_KsDevice;
}

/*****************************************************************************
 * CKsAdapter::GetUsbDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a pointer to the USB device.
 * @param
 * None
 * @return
 * Returns the pointer to the USB device.
 */
STDMETHODIMP_(PUSB_DEVICE)
CKsAdapter::
GetUsbDevice
(   void
)
{
    return m_UsbDevice;
}

/*****************************************************************************
 * CKsAdapter::GetAudioDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a pointer to the audio device.
 * @param
 * None
 * @return
 * Returns the pointer to the audio device.
 */
STDMETHODIMP_(PAUDIO_DEVICE)
CKsAdapter::
GetAudioDevice
(   void
)
{
    return m_AudioDevice;
}

/*****************************************************************************
 * CKsAdapter::GetMidiDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a pointer to the MIDI device.
 * @param
 * None
 * @return
 * Returns the pointer to the MIDI device.
 */
STDMETHODIMP_(PMIDI_DEVICE)
CKsAdapter::
GetMidiDevice
(   void
)
{
    return m_MidiDevice;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CKsAdapter::GetLanguageId()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the current language ID.
 * @param
 * None
 * @return
 * Returns the current language ID.
 */
STDMETHODIMP_(USHORT)
CKsAdapter::
GetLanguageId
(   void
)
{
	PAGED_CODE();

    return m_LanguageId;
}

/*****************************************************************************
 * CKsAdapter::IsReadyForIO()
 *****************************************************************************
 *//*!
 * @brief
 * Determine if the adapter is ready for I/O operations based on the PnP
 * state.
 * @param
 * None
 * @return
 * Returns TRUE if the adapter is ready for I/O, otherwise FALSE.
 */
STDMETHODIMP_(BOOL)
CKsAdapter::
IsReadyForIO
(   void
)
{
	PAGED_CODE();

	return (m_KsDevice->Started && DEVICE_READY_FOR_IO(m_PnpState));
}

/*****************************************************************************
 * CKsAdapter::ReferenceDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Add a reference to the device.
 * @param
 * None
 * @return
 * Returns the previous reference count.
 */
STDMETHODIMP_(LONG)
CKsAdapter::
ReferenceDevice
(   void
)
{
	PAGED_CODE();

	return InterlockedIncrement(&m_DeviceUsageCount);
}

/*****************************************************************************
 * CKsAdapter::DereferenceDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Remove a reference from the device.
 * @param
 * None
 * @return
 * Returns the previous reference count.
 */
STDMETHODIMP_(LONG)
CKsAdapter::
DereferenceDevice
(   void
)
{
	PAGED_CODE();

	return InterlockedDecrement(&m_DeviceUsageCount);
}

/*****************************************************************************
 * CKsAdapter::SetFirmwareUpgradeLock()
 *****************************************************************************
 *//*!
 * @brief
 * Setup the device for firmware upgrade process.
 * @param
 * LockOperation TRUE to obtain exclusive use of the device while upgrading
 * the device firmware. FALSE to relinquish the exclusive use of the device.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
SetFirmwareUpgradeLock
(
	IN		BOOL	LockOperation
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_ACCESS_DENIED;

    KsAcquireDevice(m_KsDevice);

	if (LockOperation)
	{
        // If there is no active streams, then proceed with the firmware upgrade.
        if (m_DeviceUsageCount == 0)
		{
            m_DeviceStatus |= DEVICE_STATUS_FIRMWARE_UPGRADE;

            ntStatus = STATUS_SUCCESS;
		}
	}
	else
	{
        m_DeviceStatus &= ~DEVICE_STATUS_FIRMWARE_UPGRADE;

		ntStatus = STATUS_SUCCESS;
	}

    KsReleaseDevice(m_KsDevice);

	return ntStatus;
}

#include <stdio.h>
/*****************************************************************************
 * CKsAdapter::SetDeviceProperty()
 *****************************************************************************
 *//*!
 * @brief
 * Store information about a device such as device description.
 * @param
 * DeviceProperty Type of device property.
 * @param
 * BufferLength Length in bytes of the @em PrpertyBuffer.
 * @param
 * PropertyBuffer Pointer to the property buffer.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
SetDeviceProperty
(
    IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
    IN      ULONG                       BufferLength,
    IN      PVOID                       PropertyBuffer
)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_SUCCESS;

    ULONG Type = REG_BINARY;

	UNICODE_STRING ValueName;

    switch (DeviceProperty)
    {
        case DevicePropertyDeviceDescription:
            RtlInitUnicodeString(&ValueName, L"DeviceDesc");
            Type = REG_SZ;
            break;
        case DevicePropertyManufacturer:
            RtlInitUnicodeString(&ValueName, L"Mfg");
            Type = REG_SZ;
            break;
		case DevicePropertyFriendlyName:
			RtlInitUnicodeString(&ValueName, L"FriendlyName");
            Type = REG_SZ;
            break;
        default:
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
    }

	PWCHAR HardwareIDs = NULL;

    if (NT_SUCCESS(ntStatus))
    {
        ULONG ResultLength;

        ntStatus = IoGetDeviceProperty(m_KsDevice->PhysicalDeviceObject,
                                       DevicePropertyHardwareID,
                                       0,
                                       NULL,
                                       &ResultLength);

        if ((ntStatus == STATUS_BUFFER_TOO_SMALL) ||
            (ntStatus == STATUS_BUFFER_OVERFLOW))
        {
            HardwareIDs = PWCHAR(ExAllocatePoolWithTag(PagedPool,ResultLength, 'mdW'));

            if (HardwareIDs)
            {
                ntStatus = IoGetDeviceProperty(m_KsDevice->PhysicalDeviceObject,
                                              DevicePropertyHardwareID,
                                              ResultLength,
                                              HardwareIDs,
                                              &ResultLength);

                if (NT_SUCCESS(ntStatus))
                {
                    for (ULONG i = 0; i < wcslen(HardwareIDs); i++)
                    {
                        if (HardwareIDs[i] == ',') // The separator for multiple IDs in Win9x.
                        {
                            HardwareIDs[i] = UNICODE_NULL;
                            break;
                        }
                    }
                }
            }
            else
            {
                ntStatus = STATUS_NO_MEMORY;
            }
        }
    }

    PWCHAR Prefix = NULL;

    if (NT_SUCCESS(ntStatus))
    {
        HANDLE RegistryKey = NULL;

        ntStatus = IoOpenDeviceRegistryKey(m_KsDevice->PhysicalDeviceObject,
                                           PLUGPLAY_REGKEY_CURRENT_HWPROFILE |
                                           PLUGPLAY_REGKEY_DEVICE,
                                           KEY_ALL_ACCESS,
                                           &RegistryKey);

        if (NT_SUCCESS(ntStatus))
        {
            ULONG ResultLength = 0;

            ntStatus = ZwQueryKey(RegistryKey,
                                  KeyBasicInformation,
                                  NULL,
                                  0,
                                  &ResultLength);

            if ((ntStatus == STATUS_BUFFER_TOO_SMALL) ||
                (ntStatus == STATUS_BUFFER_OVERFLOW))
            {
                PKEY_BASIC_INFORMATION KeyInformation = PKEY_BASIC_INFORMATION(ExAllocatePoolWithTag(PagedPool, ResultLength, 'mdW'));

                if (KeyInformation)
                {
                    ntStatus = ZwQueryKey(RegistryKey,
                                          KeyBasicInformation,
                                          KeyInformation,
                                          ResultLength,
                                          &ResultLength);

                    if (NT_SUCCESS(ntStatus))
                    {
                        Prefix = PWCHAR(ExAllocatePoolWithTag(PagedPool, KeyInformation->NameLength + sizeof(UNICODE_NULL), 'mdW'));

                        if (Prefix)
                        {
                            RtlZeroMemory(Prefix, KeyInformation->NameLength + sizeof(UNICODE_NULL));
                            RtlCopyMemory(Prefix, KeyInformation->Name, KeyInformation->NameLength);
                        }
                        else
                        {
                            ntStatus = STATUS_NO_MEMORY;
                        }
                    }

                    ExFreePool(KeyInformation);
                }
                else
                {
                    ntStatus = STATUS_NO_MEMORY;
                }
            }

            ZwClose(RegistryKey);
        }
    }


    if (NT_SUCCESS(ntStatus))
    {
        ASSERT(HardwareIDs);
        ASSERT(Prefix);

		PWCHAR HardwareID = HardwareIDs;

		// Loop thru all the hardware ids to set the property.
		while (wcslen(HardwareID))
		{
			ULONG KeyNameLength = (wcslen(Prefix) + wcslen(HardwareID) + wcslen(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Enum") + 1 + 16/*Guard*/) * sizeof(WCHAR);

			PWCHAR KeyName = PWCHAR(ExAllocatePoolWithTag(PagedPool, KeyNameLength, 'mdW'));

			if (KeyName)
			{
				RtlZeroMemory(KeyName, KeyNameLength);

				// This for Win2K/Whistler based OSes.
				swprintf(KeyName, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Enum\\%s\\%s", HardwareID, Prefix);

				OBJECT_ATTRIBUTES ObjectAttributes;

				UNICODE_STRING ObjectName;

				RtlInitUnicodeString(&ObjectName, KeyName);

				InitializeObjectAttributes
									(
										&ObjectAttributes,
										&ObjectName,
										OBJ_CASE_INSENSITIVE,
										NULL,
										NULL
									);

				HANDLE RegistryKey = NULL;

				ntStatus = ZwOpenKey(&RegistryKey, KEY_ALL_ACCESS, &ObjectAttributes);

				if (!NT_SUCCESS(ntStatus))
				{
					RtlZeroMemory(KeyName, KeyNameLength);

					// This for Win9x based OSes.
					swprintf(KeyName, L"\\Registry\\Machine\\Enum\\%s\\%s", HardwareID, Prefix);

					OBJECT_ATTRIBUTES ObjectAttributes;

					UNICODE_STRING ObjectName;

					RtlInitUnicodeString(&ObjectName, KeyName);

					InitializeObjectAttributes
										(
											&ObjectAttributes,
											&ObjectName,
											OBJ_CASE_INSENSITIVE,
											NULL,
											NULL
										);

					ntStatus = ZwOpenKey(&RegistryKey, KEY_ALL_ACCESS, &ObjectAttributes);
				}

				if (NT_SUCCESS(ntStatus))
				{
					ntStatus = ZwSetValueKey
									(
										RegistryKey,
										&ValueName,
										0,
										Type,
										PropertyBuffer,
										BufferLength
									);

					ZwClose(RegistryKey);
				}

				ExFreePool(KeyName);
			}
			else
			{
				ntStatus = STATUS_NO_MEMORY;
			}

			HardwareID += wcslen(HardwareID)+1;
		}
	}

    if (HardwareIDs)
    {
        ExFreePool(HardwareIDs);
    }

    if (Prefix)
    {
        ExFreePool(Prefix);
    }

    return ntStatus;
}

#include "profile.h"
/*****************************************************************************
 * CKsAdapter::InstallFilterFactories()
 *****************************************************************************
 *//*!
 * @brief
 * Install the filter factories.
 * @param
 * DeviceObject Pointer to the DEVICE_OBJECT structure.
 * @param
 * Irp Pointer to IRP structure.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
InstallFilterFactories
(	void
)
{
    PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	#if 0
	// This chunk of code dump the configuration descriptor into a file.
	{
	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor;
	m_UsbDevice->GetDeviceDescriptor(&DeviceDescriptor);

	WCHAR wszFileName[256];
	swprintf(wszFileName, L"\\SystemRoot\\system32\\drivers\\%04X%04X.bin", DeviceDescriptor->idVendor, DeviceDescriptor->idProduct);

	DbgPrint("%ws\n", wszFileName);

	UNICODE_STRING FileName;

    RtlInitUnicodeString(&FileName, PCWSTR(wszFileName));

    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    HANDLE FileHandle;

	IO_STATUS_BLOCK IoStatusBlock;

	NTSTATUS ntStatus = ZwCreateFile
						(
							&FileHandle,
							GENERIC_WRITE,
							&ObjectAttributes,
							&IoStatusBlock,
							NULL,
							FILE_ATTRIBUTE_NORMAL,
							FILE_SHARE_WRITE | FILE_SHARE_READ,
							FILE_OVERWRITE_IF,
							FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT,
							NULL,
							0
						);

	DbgPrint("ZwCreateFile - ntStatus = 0x%x\n", ntStatus);

	if (NT_SUCCESS(ntStatus))
	{
		PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;

		m_UsbDevice->GetConfigurationDescriptor(&ConfigurationDescriptor);

		DbgPrint("Configuration descriptor: total length = %d\n", ConfigurationDescriptor->wTotalLength);

		ZwWriteFile(FileHandle, NULL, NULL, NULL, &IoStatusBlock, ConfigurationDescriptor, ConfigurationDescriptor->wTotalLength, NULL, NULL);

		ZwClose(FileHandle);
	}
	}
	#endif // 0

	#if DBG
	{
		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		m_UsbDevice->GetInterfaceDescriptor(-1, -1, USB_CLASS_CODE_AUDIO, USB_AUDIO_SUBCLASS_AUDIOCONTROL, -1, &InterfaceDescriptor);

		if (InterfaceDescriptor)
		{
			PRINT_USB_DESCRIPTOR(InterfaceDescriptor);

			PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = NULL;

			m_UsbDevice->GetClassInterfaceDescriptor(InterfaceDescriptor->bInterfaceNumber, 0, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAcInterfaceDescriptor);

			if (CsAcInterfaceDescriptor)
			{
				_DbgPrintF(DEBUGLVL_TERSE, ("Class-specific AC Interface Header Descriptor:"));
				_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
				_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", CsAcInterfaceDescriptor->bLength));
				_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", CsAcInterfaceDescriptor->bDescriptorType));
				_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", CsAcInterfaceDescriptor->bDescriptorSubtype));
				_DbgPrintF(DEBUGLVL_TERSE, ("bcdADC 0x%x", CsAcInterfaceDescriptor->bcdADC));
				_DbgPrintF(DEBUGLVL_TERSE, ("wTotalLength 0x%x", CsAcInterfaceDescriptor->wTotalLength));
				_DbgPrintF(DEBUGLVL_TERSE, ("bInCollection %d", CsAcInterfaceDescriptor->bInCollection));
				for (UCHAR i=0; i<CsAcInterfaceDescriptor->bInCollection; i++)
				{
					_DbgPrintF(DEBUGLVL_TERSE, ("baInterfaceNr(%d) = 0x%x", i+1, CsAcInterfaceDescriptor->baInterfaceNr[i]));
				}
				_DbgPrintF(DEBUGLVL_TERSE, (""));
			}
		}
	}
	#endif // DBG

	// Install audio filters, if any.
	if (m_AudioDevice)
	{
		PAUDIO_INTERFACE AudioInterface = NULL;

		if (m_AudioDevice->ParseInterfaces(0, &AudioInterface))
		{
			CreateAudioFilterFactory
			(
				m_KsDevice,
				L"Audio",
				NULL,
				NULL
			);
		}
	}

	// Install MIDI filters, if any.
	if (m_MidiDevice)
	{
		// Determine if the MIDI topology need to be splitted up.
		LONG Split = 0;

		WCHAR ConfigurationFile[MAX_PATH] = {0};

		if (NT_SUCCESS(RegistryReadFromDriverSubKey(L"Configuration", L"CFGFILE", ConfigurationFile, sizeof(ConfigurationFile), NULL, NULL)))
		{
			PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor; m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

			CHAR SectionName[64]; sprintf(SectionName, "USB\\VID_%04X&PID_%04X.%03X.Midi.Topology", UsbDeviceDescriptor->idVendor, UsbDeviceDescriptor->idProduct, UsbDeviceDescriptor->bcdDevice);

			Split = DrvProfileGetLong(SectionName, "Split", -1, ConfigurationFile);

			if (Split == -1)
			{
				sprintf(SectionName, "USB\\VID_%04X&PID_%04X.Midi.Topology", UsbDeviceDescriptor->idVendor, UsbDeviceDescriptor->idProduct);

				Split = DrvProfileGetLong(SectionName, "Split", 0, ConfigurationFile);
			}
		}

		// Install the factories.
		ULONG i = 0, k = 0;

		PMIDI_TOPOLOGY MidiTopology = NULL;

		while (m_MidiDevice->ParseTopology(i, &MidiTopology))
		{
			if (Split != 0)
			{
				// Split the topology into individual cables.
				ULONG j = 0;

				PMIDI_CABLE MidiCable = NULL;

				while (m_MidiDevice->ParseCables(j, &MidiCable))
				{
					if (MidiCable->InterfaceNumber() == MidiTopology->InterfaceNumber())
					{
						swprintf(m_RefString[k], L"MIDI%02X.%02X.%02X", MidiCable->InterfaceNumber(), MidiCable->EndpointAddress(), MidiCable->CableNumber());

						CreateMidiFilterFactory
						(
							m_KsDevice,
							m_RefString[k],
							MidiTopology,
							MidiCable
						);

						k++;
					}

					j++;
				}
			}
			else
			{
				swprintf(m_RefString[k], L"MIDI%02X", MidiTopology->InterfaceNumber());

				CreateMidiFilterFactory
				(
					m_KsDevice,
					m_RefString[k],
					MidiTopology,
					NULL
				);

				k++;
			}

			i++;
		}
	}

	// Install a private filter for supporting private IKsPropertySet.
	CreateControlFilterFactory
	(
		m_KsDevice,
		L"Control",
		NULL,
		NULL
	);

	return ntStatus;
}


/*****************************************************************************
 * _BuildDeviceInterfaceRegistryPathName()
 *****************************************************************************
 *//*!
 * @brief
 * Parse the symbolic link and fix it up for IoOpenDeviceInterfaceRegistryKey().
 * @param
 * SymbolicLink Symbolic link.
 * @param
 * InterfaceClassGuid Interface class guid.
 * @param
 * DeviceName Name of the device.
 * @param
 * RegistryPathName Pointer to the location to store registry path name.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
static
BOOLEAN
_BuildDeviceInterfaceRegistryPathName
(
    IN      PWCHAR  SymbolicLink,
    IN      REFGUID InterfaceClassGuid,
    IN      PWCHAR  DeviceName,
    OUT     PWCHAR  RegistryPathName
)
{
    PAGED_CODE();

	WCHAR ParsedSymbolicLink[128] = {0};

    // \\?\PCI#VEN_125D&DEV_1968&SUBSYS_00851028&REV_00#2&ebb567f&0&40#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\Topology

    wcscpy(ParsedSymbolicLink, SymbolicLink);

    BOOLEAN Found = FALSE;

	PWCHAR subString = wcsstr(ParsedSymbolicLink, DeviceName);

	if (subString && *(subString - 1) == '\\')
	{
		Found = TRUE;
		*subString = '#';
		*(subString + 1) = '\0';
		wcscat(ParsedSymbolicLink, DeviceName);
	}

    if (Found)
    {
        ULONG i;
        ULONG Length = wcslen(ParsedSymbolicLink);
        // NT4/Win2K : Fix '\\?\', Win98 : Fix '\DosDevices\'.
        for (i=0; i<Length; i++)
        {
            if ((Length - i) >= 2)
            {
                if ((SymbolicLink[i] == '?') && (SymbolicLink[i+1] == '?'))
                {
                    // Change the '?' to '\\'.
                    ParsedSymbolicLink[i] = '\\';
                    break;
                }
            }

            if ((Length - i) >= 10)
            {
                if ((SymbolicLink[i  ] == 'D') && (SymbolicLink[i+1] == 'o') &&
                    (SymbolicLink[i+2] == 's') && (SymbolicLink[i+3] == 'D') &&
                    (SymbolicLink[i+4] == 'e') && (SymbolicLink[i+5] == 'v') &&
                    (SymbolicLink[i+6] == 'i') && (SymbolicLink[i+7] == 'c') &&
                    (SymbolicLink[i+8] == 'e') && (SymbolicLink[i+9] == 's'))
                {
                    // Change the '\DosDevices\' to '\\.\'.
                    ParsedSymbolicLink[i]   = '\\';
                    ParsedSymbolicLink[i+1] = '.';

                    i+=2;
                    for (ULONG j=i+wcslen(L"DosDevices")-2; j<Length+1; j++, i++)
                    {
                        ParsedSymbolicLink[i] = ParsedSymbolicLink[j];
                    }

                    break;
                }
            }
        }

        // Change '\\?\' to '##?#'
        ParsedSymbolicLink[0] = '#';
        ParsedSymbolicLink[1] = '#';
        ParsedSymbolicLink[3] = '#';

        // Build the registry path name.
        wcscpy(RegistryPathName, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\");
        UNICODE_STRING GuidString;
        RtlStringFromGUID(InterfaceClassGuid, &GuidString);
        memcpy(RegistryPathName+wcslen(RegistryPathName),GuidString.Buffer, GuidString.Length);
        RtlFreeUnicodeString(&GuidString);
        wcscat(RegistryPathName, L"\\");
        wcscat(RegistryPathName, ParsedSymbolicLink);
    }

    return Found;
}

/*****************************************************************************
 * CKsAdapter::AddSubDeviceInterface()
 *****************************************************************************
 *//*!
 * @brief
 * Add the subdevice inerface.
 * @param
 * SubDeviceName Name of the sub device.
 * @param
 * InterfaceClassGuid Interface class guid.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
AddSubDeviceInterface
(
	IN		PWCHAR	SubDeviceName,
    IN      REFGUID InterfaceClassGuid
)
{
    PAGED_CODE();

    PWCHAR SymbolicLinkList = NULL;

    NTSTATUS ntStatus = IoGetDeviceInterfaces
                    (
						&InterfaceClassGuid,
                        m_KsDevice->PhysicalDeviceObject,
                        DEVICE_INTERFACE_INCLUDE_NONACTIVE,
                        &SymbolicLinkList
                    );

    if (NT_SUCCESS(ntStatus))
    {
        PWCHAR SymbolicLink = SymbolicLinkList;

        while (*SymbolicLink)
        {
			WCHAR ParsedSymbolicLink[128];

			wcscpy(ParsedSymbolicLink, SymbolicLink);

			PWCHAR subString = wcsrchr(ParsedSymbolicLink, L'\\');

			if (subString)
			{
				*(subString + 1) = '\0';
				wcscat(ParsedSymbolicLink, SubDeviceName);
			}

            // \\?\PCI#VEN_125D&DEV_1968&SUBSYS_00851028&REV_00#2&ebb567f&0&40#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\Topology
            WCHAR RegistryPathName[256] = {0};

            if (_BuildDeviceInterfaceRegistryPathName(ParsedSymbolicLink, InterfaceClassGuid, SubDeviceName, RegistryPathName))
            {
				OBJECT_ATTRIBUTES ObjectAttributes;
				UNICODE_STRING ObjectName;

				RtlInitUnicodeString(&ObjectName, RegistryPathName);

				InitializeObjectAttributes
									(
										&ObjectAttributes,
										&ObjectName,
										OBJ_CASE_INSENSITIVE,
										NULL,
										NULL
									);

				HANDLE DeviceInterfaceKey = NULL;

				ntStatus = ZwCreateKey(&DeviceInterfaceKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);

                if (NT_SUCCESS(ntStatus))
                {
                    UNICODE_STRING ValueKeyName;
                    RtlInitUnicodeString(&ValueKeyName, L"SymbolicLink");

					// set the value.
                    ntStatus = ZwSetValueKey(DeviceInterfaceKey, &ValueKeyName, 0, REG_SZ, ParsedSymbolicLink, (wcslen(ParsedSymbolicLink)+1)*sizeof(WCHAR));

					ZwClose(DeviceInterfaceKey);
                }

                // found the key...
                break;
            }

            SymbolicLink += wcslen(SymbolicLink)+1;
        }

        ExFreePool(SymbolicLinkList);
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::AddSubDeviceInterface] - IoGetDeviceInterfaces() failed : %lx", ntStatus));
    }

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::SetSubDeviceParameter()
 *****************************************************************************
 *//*!
 * @brief
 * Set the subdevice parameters.
 * @param
 * SubDeviceName Name of the sub device.
 * @param
 * InterfaceClassGuid Interface class guid.
 * @param
 * ValueName Name of the value to set.
 * @param
 * Type Type fo the value to set.
 * @param
 * Data Pointer to the value.
 * @param
 * DataSize Size of the value.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
SetSubDeviceParameter
(
	IN		PWCHAR	SubDeviceName,
    IN      REFGUID InterfaceClassGuid,
    IN      PWCHAR  ValueName,
    IN      ULONG   Type,
    IN      PVOID   Data,
    IN      ULONG   DataSize
)
{
    PAGED_CODE();

    PWCHAR SymbolicLinkList = NULL;

    NTSTATUS ntStatus = IoGetDeviceInterfaces
                    (
                        &InterfaceClassGuid,
                        m_KsDevice->PhysicalDeviceObject,
                        DEVICE_INTERFACE_INCLUDE_NONACTIVE,
                        &SymbolicLinkList
                    );

    if (NT_SUCCESS(ntStatus))
    {
        PWCHAR SymbolicLink = SymbolicLinkList;

        while (*SymbolicLink)
        {
            // \\?\PCI#VEN_125D&DEV_1968&SUBSYS_00851028&REV_00#2&ebb567f&0&40#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\Topology
            WCHAR RegistryPathName[256] = {0};

            if (_BuildDeviceInterfaceRegistryPathName(SymbolicLink, InterfaceClassGuid, SubDeviceName, RegistryPathName))
            {
				OBJECT_ATTRIBUTES ObjectAttributes;
				UNICODE_STRING ObjectName;

				RtlInitUnicodeString(&ObjectName, RegistryPathName);

				InitializeObjectAttributes
									(
										&ObjectAttributes,
										&ObjectName,
										OBJ_CASE_INSENSITIVE,
										NULL,
										NULL
									);

				HANDLE DeviceInterfaceKey = NULL;

				ntStatus = ZwCreateKey(&DeviceInterfaceKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);

                if (NT_SUCCESS(ntStatus))
                {
					OBJECT_ATTRIBUTES ObjectAttributes;
					UNICODE_STRING ObjectName;

					RtlInitUnicodeString(&ObjectName, L"Device Parameters");

					InitializeObjectAttributes
										(
											&ObjectAttributes,
											&ObjectName,
											OBJ_CASE_INSENSITIVE,
											DeviceInterfaceKey,
											NULL
										);

					HANDLE DeviceParametersKey = NULL;

					ntStatus = ZwCreateKey(&DeviceParametersKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);

                    if (NT_SUCCESS(ntStatus))
                    {
                        UNICODE_STRING ValueKeyName;
                        RtlInitUnicodeString(&ValueKeyName, ValueName);

						// set the value.
						ntStatus = ZwSetValueKey(DeviceParametersKey, &ValueKeyName, 0, Type, Data, DataSize);

                        ZwClose(DeviceParametersKey);
                    }

                    ZwClose(DeviceInterfaceKey);
                }

                // found the key...
                break;
            }

            SymbolicLink += wcslen(SymbolicLink)+1;
        }

        ExFreePool(SymbolicLinkList);
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::SetSubDeviceParameter] - IoGetDeviceInterfaces() failed : %lx", ntStatus));
    }

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::GetSubDeviceParameter()
 *****************************************************************************
 *//*!
 * @brief
 * Get the subdevice parameter.
 * @param
 * SubDeviceName Name of the sub device.
 * @param
 * InterfaceClassGuid Interface class guid.
 * @param
 * ValueName Name of the value to get.
 * @param
 * Type Type fo the value to get.
 * @param
 * Data Pointer to the value.
 * @param
 * DataSize Pointer to the location that this method put the size of the
 * value.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
GetSubDeviceParameter
(
	IN		PWCHAR	SubDeviceName,
    IN      REFGUID InterfaceClassGuid,
    IN      PWCHAR  ValueName,
    IN      PULONG  Type,
    IN      PVOID   Data,
    IN      PULONG  DataSize
)
{
    PAGED_CODE();

    PWCHAR SymbolicLinkList = NULL;

    NTSTATUS ntStatus = IoGetDeviceInterfaces
                    (
                        &InterfaceClassGuid,
                        m_KsDevice->PhysicalDeviceObject,
                        DEVICE_INTERFACE_INCLUDE_NONACTIVE,
                        &SymbolicLinkList
                    );

    if (NT_SUCCESS(ntStatus))
    {
        PWCHAR SymbolicLink = SymbolicLinkList;

        while (*SymbolicLink)
        {
            // \\?\PCI#VEN_125D&DEV_1968&SUBSYS_00851028&REV_00#2&ebb567f&0&40#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\Topology
            WCHAR RegistryPathName[256] = {0};

            if (_BuildDeviceInterfaceRegistryPathName(SymbolicLink, InterfaceClassGuid, SubDeviceName, RegistryPathName))
            {
				OBJECT_ATTRIBUTES ObjectAttributes;
				UNICODE_STRING ObjectName;

				RtlInitUnicodeString(&ObjectName, RegistryPathName);

				InitializeObjectAttributes
									(
										&ObjectAttributes,
										&ObjectName,
										OBJ_CASE_INSENSITIVE,
										NULL,
										NULL
									);

				HANDLE DeviceInterfaceKey = NULL;

				ntStatus = ZwCreateKey(&DeviceInterfaceKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);

                if (NT_SUCCESS(ntStatus))
                {
					OBJECT_ATTRIBUTES ObjectAttributes;
					UNICODE_STRING ObjectName;

					RtlInitUnicodeString(&ObjectName, L"Device Parameters");

					InitializeObjectAttributes
										(
											&ObjectAttributes,
											&ObjectName,
											OBJ_CASE_INSENSITIVE,
											DeviceInterfaceKey,
											NULL
										);

					HANDLE DeviceParametersKey = NULL;

					ntStatus = ZwOpenKey(&DeviceParametersKey, KEY_ALL_ACCESS, &ObjectAttributes);

                    if (NT_SUCCESS(ntStatus))
                    {
                        ULONG ResultLength;

                        PVOID KeyValueInfo = ExAllocatePoolWithTag(PagedPool, sizeof(KEY_VALUE_PARTIAL_INFORMATION), 'mdW');

                        if (KeyValueInfo)
                        {
                            UNICODE_STRING ValueKeyName;
                            RtlInitUnicodeString(&ValueKeyName, ValueName);

                            // get the value.
                            ntStatus = ZwQueryValueKey
										(	
											DeviceParametersKey, 
											&ValueKeyName,
											KeyValuePartialInformation,
											KeyValueInfo,
											sizeof(KEY_VALUE_PARTIAL_INFORMATION),
											&ResultLength
										);

                            if (STATUS_BUFFER_OVERFLOW == ntStatus)
                            {
                                ExFreePool(KeyValueInfo);

                                KeyValueInfo = ExAllocatePoolWithTag(PagedPool, ResultLength, 'mdW');

                                if (KeyValueInfo)
                                {
                                    // get the value.
									ntStatus = ZwQueryValueKey
												(	
													DeviceParametersKey, 
													&ValueKeyName,
													KeyValuePartialInformation,
													KeyValueInfo,
													ResultLength,
													&ResultLength
												);
                                }
                            }

                            if (NT_SUCCESS(ntStatus))
                            {
                                PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = PKEY_VALUE_PARTIAL_INFORMATION(KeyValueInfo);

                                if (PartialInfo->DataLength <= *DataSize)
                                {
                                    RtlCopyMemory(Data, PartialInfo->Data, PartialInfo->DataLength);
                                }
                                else
                                {
                                    ntStatus = STATUS_BUFFER_TOO_SMALL;
                                }

                                *DataSize = PartialInfo->DataLength;

                                if (Type)
                                {
                                    *Type = PartialInfo->Type;
                                }
                            }

                            if (KeyValueInfo) ExFreePool(KeyValueInfo);
                        }
                        else
                        {
                            ntStatus = STATUS_NO_MEMORY;
                        }

                        ZwClose(DeviceParametersKey);
                    }

                    ZwClose(DeviceInterfaceKey);
                }

                // found the key...
                break;
            }

            SymbolicLink += wcslen(SymbolicLink)+1;
        }

        ExFreePool(SymbolicLinkList);
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsAdapter::GetSubDeviceParameter] - IoGetDeviceInterfaces() failed : %lx", ntStatus));
    }

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::RegistryReadFromDriverSubKey()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * Read specified data from Driver/<SubKeyName> key.
 * @param
 * SubKeyName_ Sub key name.
 * @param
 * ValueName_ Value name.
 * @param
 * Data Pointer to the buffer that store the information read.
 * @param
 * DataSize Size of the data buffer.
 * @param
 * OutDataSize Size of returned data.
 * @param
 * OutDataType Type of data.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
RegistryReadFromDriverSubKey
(
    IN      PWCHAR	SubKeyName_,
    IN      PWCHAR  ValueName_,
    OUT     PVOID   Data,
    IN      ULONG   DataSize,
	OUT     ULONG * OutDataSize		OPTIONAL,
	OUT     ULONG * OutDataType		OPTIONAL
)
{
    HANDLE DriverKey = NULL;

   	NTSTATUS ntStatus = IoOpenDeviceRegistryKey
                        (
                            m_KsDevice->PhysicalDeviceObject,
                            PLUGPLAY_REGKEY_DRIVER,
                            KEY_ALL_ACCESS,
                            &DriverKey
                        );

    if (NT_SUCCESS(ntStatus))
    {
	    // Open the Settings subkey.
		OBJECT_ATTRIBUTES ObjectAttributes;
		UNICODE_STRING ObjectName;

		RtlInitUnicodeString(&ObjectName, SubKeyName_);

		InitializeObjectAttributes
							(
								&ObjectAttributes,
								&ObjectName,
								OBJ_CASE_INSENSITIVE,
								DriverKey,
								NULL
							);

		HANDLE SubKey = NULL;

		ntStatus = ZwCreateKey(&SubKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);

        if (NT_SUCCESS(ntStatus))
        {
			UNICODE_STRING ValueName;
			// init value name.
			RtlInitUnicodeString(&ValueName, ValueName_);

            ULONG ResultLength;

            // Do you Driver/<SubKeyName> key accesses here using the IRegistryKey interface
            // pointed to by SubKey.
            ntStatus = ZwQueryValueKey
						(
							SubKey,
							&ValueName,
                            KeyValuePartialInformation,
                            NULL,
                            0,
                            &ResultLength
						);

			if ((ntStatus == STATUS_BUFFER_TOO_SMALL) || (ntStatus == STATUS_BUFFER_OVERFLOW))
			{
				// allocate memory to hold key info.
				PVOID KeyInfo = ExAllocatePoolWithTag(PagedPool, ResultLength, 'mdW');

				if (KeyInfo)
				{
					// query the value key.
					ntStatus = ZwQueryValueKey
								(
									SubKey,
									&ValueName,
									KeyValuePartialInformation,
									KeyInfo,
									ResultLength,
									&ResultLength
								);

					// Compare to the expected values.
					if (NT_SUCCESS(ntStatus))
					{
						PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfo;

						if (PartialInfo->DataLength <= DataSize)
						{
							RtlCopyMemory(Data, PartialInfo->Data, PartialInfo->DataLength);
						}
						else
						{
							ntStatus = STATUS_BUFFER_OVERFLOW;
						}

						if (OutDataSize) *OutDataSize = PartialInfo->DataLength;
						if (OutDataType) *OutDataType = PartialInfo->Type;
					}

					// Free the memory pool.
					ExFreePool(KeyInfo);
				}
				else
				{
					ntStatus = STATUS_NO_MEMORY;
				}
			}

            // Release the Driver/Settings key.
            ZwClose(SubKey);
        }

        // Release the driver key
        ZwClose(DriverKey);
    }

    return ntStatus;
}

/*****************************************************************************
 * CKsAdapter::RegistryWriteToDriverSubKey()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * Read specified data from Driver/<SubKeyName> key.
 * @param
 * SubKeyName_ Sub key name.
 * @param
 * ValueName_ Value name.
 * @param
 * Data Pointer to the buffer that store the information read.
 * @param
 * DataSize Size of the data buffer.
 * @param
 * DataType Type of data.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CKsAdapter::
RegistryWriteToDriverSubKey
(
    IN      PWCHAR	SubKeyName_,
    IN      PWCHAR  ValueName_,
    IN		PVOID   Data,
    IN      ULONG   DataSize,
	IN		ULONG	DataType
)
{
    HANDLE DriverKey = NULL;

   	NTSTATUS ntStatus = IoOpenDeviceRegistryKey
                        (
                            m_KsDevice->PhysicalDeviceObject,
                            PLUGPLAY_REGKEY_DRIVER,
                            KEY_ALL_ACCESS,
                            &DriverKey
                        );

    if (NT_SUCCESS(ntStatus))
    {
	    // Open the Settings subkey.
		OBJECT_ATTRIBUTES ObjectAttributes;
		UNICODE_STRING ObjectName;

		RtlInitUnicodeString(&ObjectName, SubKeyName_);

		InitializeObjectAttributes
							(
								&ObjectAttributes,
								&ObjectName,
								OBJ_CASE_INSENSITIVE,
								DriverKey,
								NULL
							);

		HANDLE SubKey = NULL;

		ntStatus = ZwCreateKey(&SubKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);

        if (NT_SUCCESS(ntStatus))
        {
			UNICODE_STRING ValueName;
			// init value name.
			RtlInitUnicodeString(&ValueName, ValueName_);

            ULONG ResultLength;

            // Do you Driver/<SubKeyName> key accesses here using the IRegistryKey interface
            // pointed to by SubKey.
            ntStatus = ZwSetValueKey(SubKey, &ValueName, 0, DataType, Data,	DataSize);

            // Release the Driver/Settings key.
            ZwClose(SubKey);
        }

        // Release the driver key
        ZwClose(DriverKey);
    }

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CKsAdapter::UsbDeviceCallbackRoutine()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CKsAdapter::
UsbDeviceCallbackRoutine
(
	IN		ULONG	Reason,
	IN		PVOID	Parameter,
	IN		PVOID	Context
)
{
	CKsAdapter * that = (CKsAdapter *)(Context);

	if (Reason == USB_DEVICE_CALLBACK_REASON_DEVICE_DISCONNECTED)
	{
		that->m_DeviceStatus |= DEVICE_STATUS_NOT_CONNECTED;
	}
}

#pragma code_seg()

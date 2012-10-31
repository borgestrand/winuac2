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
 * @file       UsbDev.cpp
 * @brief      USB device class implementation.
 * @details	   This is a derivative work from the KS1000's SBUSB.* codebase.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-26-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "UsbDev.h"
#include "Profile.h"

#define STR_MODULENAME "CUsbDevice: "

#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbDevice::NonDelegatingQueryInterface()
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
CUsbDevice::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CUsbDevice::NonDelegatingQueryInterface]"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PUSB_DEVICE(this)));
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
 * CUsbDevice::~CUsbDevice()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CUsbDevice::
~CUsbDevice
(	void
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::~CUsbDevice]"));

	UnconfigureDevice();
}

/*****************************************************************************
 * CUsbDevice::Init()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
Init
(
    IN      PDEVICE_OBJECT      NextLowerDeviceObject,
	IN		USB_DEVICE_CALLBACK	CallbackRoutine,
	IN		PVOID				CallbackData
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::Init]"));

    m_NextLowerDeviceObject = NextLowerDeviceObject;
	m_CallbackRoutine = CallbackRoutine;
	m_CallbackData = CallbackData;

    m_Halted = FALSE;

    m_UsbConfigurationDescriptor = NULL;
    m_UsbConfigurationHandle = NULL;
    m_NumInterfaces	= 0;

	m_NumberOfLanguageSupported = 0;

	QueryBusInterface(USB_BUSIF_USBDI_VERSION_0, PINTERFACE(&m_BusInterfaceV0), sizeof(USB_BUS_INTERFACE_USBDI_V0));

	NTSTATUS ntStatus = QueryBusInterface(USB_BUSIF_USBDI_VERSION_1, PINTERFACE(&m_BusInterfaceV1), sizeof(USB_BUS_INTERFACE_USBDI_V1));

	if (NT_SUCCESS(ntStatus))
	{
		_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::Init] - IsDeviceHighSpeed = 0x%x", m_BusInterfaceV1.IsDeviceHighSpeed(m_BusInterfaceV1.BusContext)));
	}

	return STATUS_SUCCESS;
}

#include <stdio.h>
/*****************************************************************************
 * CUsbDevice::SetupLanguageSupport()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
SetupLanguageSupport
(
	IN		PWSTR	LanguageFile
)
{
    PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::SetupLanguageSupport]"));

	m_NumberOfLanguageSupported = 0;

	if (LanguageFile)
	{
		ULONG i;

		wcscpy(m_LanguageFile, LanguageFile);

		// First find the user-supplied localization file.
		CHAR SectionName[64]; sprintf(SectionName, "USB\\VID_%04X&PID_%04X.LANGID", m_UsbDeviceDescriptor.idVendor, m_UsbDeviceDescriptor.idProduct);

		for (i=0;; i++)
		{
			CHAR KeyName[8]; sprintf(KeyName, "%d", i+1);

			m_LanguageSupport[i].Location = LANGUAGE_SUPPORT_LOCATION_FILE;

			m_LanguageSupport[i].wLANGID = DrvProfileGetUshort(SectionName, KeyName, 0, LanguageFile);

			if (m_LanguageSupport[i].wLANGID)
			{
				m_NumberOfLanguageSupported++;
			}
			else
			{
				break;
			}
		}

		// Fixup the USB device descriptors, if it is asked to do so.
		sprintf(SectionName, "USB\\VID_%04X&PID_%04X.FIXUP.%03X", m_UsbDeviceDescriptor.idVendor, m_UsbDeviceDescriptor.idProduct, m_UsbDeviceDescriptor.bcdDevice);

		UCHAR iIndex = DrvProfileGetUchar(SectionName, "iManufacturer", 0, LanguageFile);

		if (iIndex)
		{
			m_UsbDeviceDescriptor.iManufacturer = iIndex;
		}

		iIndex = DrvProfileGetUchar(SectionName, "iProduct", 0, LanguageFile);

		if (iIndex)
		{
			m_UsbDeviceDescriptor.iProduct = iIndex;
		}

		iIndex = DrvProfileGetUchar(SectionName, "iSerialNumber", 0, LanguageFile);

		if (iIndex)
		{
			m_UsbDeviceDescriptor.iSerialNumber = iIndex;
		}

		// Fixup the USB configuration descriptors, if it is asked to do so.
		for (i=0;; i++)
		{
			CHAR KeyName[8]; sprintf(KeyName, "%d", i+1);

			ULONG Entry = DrvProfileGetUlong(SectionName, KeyName, 0xFFFFFFFF, LanguageFile);

			if (Entry != 0xFFFFFFFF)
			{
				USHORT Offset = USHORT(Entry >> 16);

				if (Offset < m_UsbConfigurationDescriptor->wTotalLength)
				{
					PUCHAR Descriptor = PUCHAR(m_UsbConfigurationDescriptor);

					if (Descriptor[Offset] == UCHAR((Entry & 0xFF00)>>8))
					{
						Descriptor[Offset] = UCHAR(Entry & 0xFF);
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	// Now get the USB device provided language support.
	struct
	{
		UCHAR	bLength;
		UCHAR	bDescriptorType;
		USHORT	wLANGID[126];
	} LanguageDescriptor;

	RtlZeroMemory(&LanguageDescriptor, sizeof(LanguageDescriptor));

    struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

    UsbBuildGetDescriptorRequest
	(
		(PURB)&Urb,
        sizeof Urb,
        USB_STRING_DESCRIPTOR_TYPE,
        0,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        &LanguageDescriptor,
        NULL,
        sizeof(LanguageDescriptor),
        NULL
	);

    NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

    if (NT_SUCCESS(ntStatus))
    {
		if (LanguageDescriptor.bLength >= 2)
		{
			// Put the USB provided language ID code at the end of the list so that
			// it can be overriden by the one that is provided in the file.
			for (UCHAR i=0; i<((LanguageDescriptor.bLength-2)/2); i++)
			{
				BOOL Duplicate = FALSE;

				for (ULONG j=0; j<m_NumberOfLanguageSupported; j++)
				{
					if (m_LanguageSupport[j].wLANGID == LanguageDescriptor.wLANGID[i])
					{
						Duplicate = TRUE;
						break;
					}
				}

				if (!Duplicate)
				{
					m_LanguageSupport[m_NumberOfLanguageSupported].Location = LANGUAGE_SUPPORT_LOCATION_DEVICE;
					m_LanguageSupport[m_NumberOfLanguageSupported].wLANGID = LanguageDescriptor.wLANGID[i];

					m_NumberOfLanguageSupported++;
				}
			}
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::QueryBusInterface()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
QueryBusInterface
(
	IN		USHORT		BusInterfaceVersion,
    IN		PINTERFACE	BusInterface,
	IN		USHORT		BusInterfaceSize
)
{
    PAGED_CODE();

    PIRP Irp = NULL;

	NTSTATUS ntStatus = CreateIrp(&Irp);

	if (NT_SUCCESS(ntStatus))
	{
		// All PNP Irp's need the status field intialized to
		// STATUS_NOT_SUPPORTED.
		Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

		PIO_STACK_LOCATION NextIrpStack = IoGetNextIrpStackLocation(Irp);

		ASSERT(NextIrpStack);

		NextIrpStack->MajorFunction = IRP_MJ_PNP;
		NextIrpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

		// Bus interface query.
		NextIrpStack->Parameters.QueryInterface.Interface = BusInterface;
		NextIrpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;
		NextIrpStack->Parameters.QueryInterface.InterfaceType = &USB_BUS_INTERFACE_USBDI_GUID;
		NextIrpStack->Parameters.QueryInterface.Size = BusInterfaceSize;
		NextIrpStack->Parameters.QueryInterface.Version = BusInterfaceVersion;

		USBD_IO_COMPLETION_CONTEXT CompletionContext;

		CompletionContext.Context = this;
		CompletionContext.ProcessEvent = TRUE;
		KeInitializeEvent(&CompletionContext.Event, NotificationEvent, FALSE);

		IoSetCompletionRoutine(Irp, CompletionRoutine, &CompletionContext, TRUE, TRUE, TRUE);

        ntStatus = IoCallDriver(m_NextLowerDeviceObject, Irp);

		if (ntStatus == STATUS_PENDING)
		{
			// Wait forever until this IRP is completed.
			KeWaitForSingleObject(&CompletionContext.Event, Executive, KernelMode, FALSE, NULL);

			ntStatus = Irp->IoStatus.Status;
		}

		IoFreeIrp(Irp);
	}

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CUsbDevice::StopDevice()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
StopDevice
(	void
)
{
	UnconfigureDevice();

	m_Halted = TRUE;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::StartDevice()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
StartDevice
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::StartDevice]"));

    struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

    UsbBuildGetDescriptorRequest
	(
		(PURB)&Urb,
        sizeof Urb,
        USB_DEVICE_DESCRIPTOR_TYPE,
        0,
        0,
        &m_UsbDeviceDescriptor,
        NULL,
        sizeof(m_UsbDeviceDescriptor),
        NULL
	);

    NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

    if (NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("Device Descriptor:"));
        _DbgPrintF(DEBUGLVL_TERSE,("-------------------------"));
        _DbgPrintF(DEBUGLVL_TERSE,("bLength %d", m_UsbDeviceDescriptor.bLength));
        _DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", m_UsbDeviceDescriptor.bDescriptorType));
        _DbgPrintF(DEBUGLVL_TERSE,("bcdUSB 0x%x", m_UsbDeviceDescriptor.bcdUSB));
        _DbgPrintF(DEBUGLVL_TERSE,("bDeviceClass 0x%x", m_UsbDeviceDescriptor.bDeviceClass));
        _DbgPrintF(DEBUGLVL_TERSE,("bDeviceSubClass 0x%x", m_UsbDeviceDescriptor.bDeviceSubClass));
        _DbgPrintF(DEBUGLVL_TERSE,("bDeviceProtocol 0x%x", m_UsbDeviceDescriptor.bDeviceProtocol));
        _DbgPrintF(DEBUGLVL_TERSE,("bMaxPacketSize0 0x%x", m_UsbDeviceDescriptor.bMaxPacketSize0));
        _DbgPrintF(DEBUGLVL_TERSE,("idVendor 0x%x", m_UsbDeviceDescriptor.idVendor));
        _DbgPrintF(DEBUGLVL_TERSE,("idProduct 0x%x", m_UsbDeviceDescriptor.idProduct));
        _DbgPrintF(DEBUGLVL_TERSE,("bcdDevice 0x%x", m_UsbDeviceDescriptor.bcdDevice));
        _DbgPrintF(DEBUGLVL_TERSE,("iManufacturer 0x%x", m_UsbDeviceDescriptor.iManufacturer));
        _DbgPrintF(DEBUGLVL_TERSE,("iProduct 0x%x", m_UsbDeviceDescriptor.iProduct));
        _DbgPrintF(DEBUGLVL_TERSE,("iSerialNumber 0x%x", m_UsbDeviceDescriptor.iSerialNumber));
        _DbgPrintF(DEBUGLVL_TERSE,("bNumConfigurations 0x%x\n", m_UsbDeviceDescriptor.bNumConfigurations));

		// Check vendor/product ids if you want to here. If it doesn't match your expectation, then
		// return STATUS_DEVICE_CONFIGURATION_ERROR.
		// TODO: Perform basic vendor/product ids check.
    }

    if (NT_SUCCESS(ntStatus))
	{
        ntStatus = ConfigureDevice();
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::ConfigureDevice()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
ConfigureDevice
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::ConfigureDevice]"));

	struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

    USB_CONFIGURATION_DESCRIPTOR Ucd;

    UsbBuildGetDescriptorRequest
	(
        (PURB)&Urb, // points to the URB to be filled in
        sizeof(Urb),
        USB_CONFIGURATION_DESCRIPTOR_TYPE,
        0,      // number of configuration descriptor
        0,      // this parameter not used for configuration descriptors
        &Ucd,   // points to a USB_CONFIGURATION_DESCRIPTOR
        NULL,
        sizeof(Ucd),
        NULL
    );

    NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

    if (NT_SUCCESS(ntStatus))
	{
		m_UsbConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR)ExAllocatePoolWithTag(NonPagedPool, Ucd.wTotalLength, 'mdW');

		if (m_UsbConfigurationDescriptor)
		{
			UsbBuildGetDescriptorRequest
			(
				(PURB)&Urb, // points to the URB to be filled in
				sizeof(Urb),
				USB_CONFIGURATION_DESCRIPTOR_TYPE,
				0,      // number of configuration descriptor
				0,      // this parameter not used for configuration descriptors
				m_UsbConfigurationDescriptor,  // points to a USB_CONFIGURATION_DESCRIPTOR
				NULL,
				Ucd.wTotalLength,
				NULL
			);

			ntStatus = CallUSBD((PURB)&Urb);
		}
		else
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	if (NT_SUCCESS(ntStatus))
	{
		_DbgPrintF(DEBUGLVL_TERSE,("Configuration Descriptor: "));
		_DbgPrintF(DEBUGLVL_TERSE,("bLength 0x%x", m_UsbConfigurationDescriptor->bLength));
		_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", m_UsbConfigurationDescriptor->bDescriptorType));
		_DbgPrintF(DEBUGLVL_TERSE,("wTotalLength 0x%x", m_UsbConfigurationDescriptor->wTotalLength));
		_DbgPrintF(DEBUGLVL_TERSE,("bNumInterfaces 0x%x", m_UsbConfigurationDescriptor->bNumInterfaces));
		_DbgPrintF(DEBUGLVL_TERSE,("bConfigurationValue 0x%x", m_UsbConfigurationDescriptor->bConfigurationValue));
		_DbgPrintF(DEBUGLVL_TERSE,("iConfiguration 0x%x", m_UsbConfigurationDescriptor->iConfiguration));
		_DbgPrintF(DEBUGLVL_TERSE,("bmAttributes 0x%x", m_UsbConfigurationDescriptor->bmAttributes));
		_DbgPrintF(DEBUGLVL_TERSE,("MaxPower 0x%x", m_UsbConfigurationDescriptor->MaxPower));

		ntStatus = SelectDefaultInterface();
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::UnconfigureDevice()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
UnconfigureDevice
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::UnconfigureDevice]"));

    //
    // Send the select configuration urb with a NULL pointer for the configuration
    // handle. This closes the configuration and puts the device in the 'unconfigured'
    // state.
    //
    struct _URB_SELECT_CONFIGURATION Urb;
    UsbBuildSelectConfigurationRequest((PURB)&Urb, sizeof(Urb), NULL);

	CallUSBD((PURB)&Urb);

	if (m_InterfaceList)
    {
        for (ULONG i=0; i<m_NumInterfaces; i++)
        {
            if (m_InterfaceList[i].Interface)
			{
                ExFreePool(m_InterfaceList[i].Interface);

	            m_InterfaceList[i].Interface = NULL;
			}
        }

        ExFreePool(m_InterfaceList);

		m_InterfaceList = NULL;
    }

    if (m_UsbConfigurationDescriptor)
	{
        ExFreePool(m_UsbConfigurationDescriptor);

		m_UsbConfigurationDescriptor = NULL;
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::SuspendDevice()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
SuspendDevice
(	void
)
{
    m_Halted = TRUE;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::ResumeDevice()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
ResumeDevice
(	void
)
{
    m_Halted = FALSE;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::BuildInterfaceList()
 *****************************************************************************
 *//*!
 * @brief
 * This routine builds an array of USBD_INTERFACE_LIST_ENTRY structures
 * from a Configuration Descriptor.  This array will then be suitable
 * for use in a call to USBD_CreateConfigurationRequestEx().
 * @details
 * It is the responsibility of the caller to free the returned array
 * of USBD_INTERFACE_LIST_ENTRY structures.
 */
PUSBD_INTERFACE_LIST_ENTRY
CUsbDevice::
BuildInterfaceList
(
	IN		PUSB_CONFIGURATION_DESCRIPTOR	ConfigurationDescriptor
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::BuildInterfaceList]"));

    PUSBD_INTERFACE_LIST_ENTRY InterfaceList = NULL;

    ULONG NumInterfaces = ConfigurationDescriptor->bNumInterfaces;

	if (NumInterfaces > 0)
	{
        // Allocate a USBD_INTERFACE_LIST_ENTRY structure for each
        // Interface in the Configuration Descriptor, plus one more to
        // null terminate the array.
		InterfaceList = (PUSBD_INTERFACE_LIST_ENTRY)ExAllocatePoolWithTag(NonPagedPool, (NumInterfaces + 1) * sizeof(USBD_INTERFACE_LIST_ENTRY), 'mdW');

		if (InterfaceList)
        {
			// Note that this also null terminates the list.
            RtlZeroMemory(InterfaceList, (NumInterfaces + 1) * sizeof(USBD_INTERFACE_LIST_ENTRY));

            // Parse out the Interface Descriptor for Alternate Interface setting zero for
			// each Interface from the Configuration Descriptor.
            // Note that some devices have been implemented which do not consecutively number
			// their Interface Descriptors.
            // InterfaceNumber may increment and skip an interface number without incrementing
			// NumInterfacesFound.
            for (ULONG InterfaceNumber = 0, NumInterfacesFound = 0; NumInterfacesFound < NumInterfaces; InterfaceNumber++)
            {
                PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = USBD_ParseConfigurationDescriptorEx
																(
																	ConfigurationDescriptor,
																	ConfigurationDescriptor,
																	InterfaceNumber,    // InterfaceNumber
																	0,                  // AlternateSetting Zero
																	-1,                 // InterfaceClass, don't care
																	-1,                 // InterfaceSubClass, don't care
																	-1                  // InterfaceProtocol, don't care
																);

                if (InterfaceDescriptor)
                {
					#if 1
					_DbgPrintF(DEBUGLVL_TERSE,("\nInterface Descriptor: "));
					_DbgPrintF(DEBUGLVL_TERSE,("bLength 0x%x", InterfaceDescriptor->bLength));
					_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", InterfaceDescriptor->bDescriptorType));
					_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceNumber 0x%x", InterfaceDescriptor->bInterfaceNumber));
					_DbgPrintF(DEBUGLVL_TERSE,("bAlternateSetting 0x%x", InterfaceDescriptor->bAlternateSetting));
					_DbgPrintF(DEBUGLVL_TERSE,("bNumEndPoints 0x%x", InterfaceDescriptor->bNumEndpoints));
					_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceClass 0x%x", InterfaceDescriptor->bInterfaceClass));
					_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceSubClass 0x%x", InterfaceDescriptor->bInterfaceSubClass));
					_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceProtocol 0x%x", InterfaceDescriptor->bInterfaceProtocol));
					_DbgPrintF(DEBUGLVL_TERSE,("iInterface 0x%x\n", InterfaceDescriptor->iInterface));
					#endif

                    InterfaceList[NumInterfacesFound].InterfaceDescriptor = InterfaceDescriptor;

                    NumInterfacesFound++;
                }

                // Prevent an endless loop due to an incorrectly formed configuration Descriptor.
				// The maximum interface number is 255.
                if (InterfaceNumber > 256)
                {
                    ExFreePool(InterfaceList);

					InterfaceList = NULL;
					break;
                }
            }
        }
	}

	return InterfaceList;
}

/*****************************************************************************
 * CUsbDevice::SelectDefaultInterface()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
SelectDefaultInterface
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::SelectDefaultInterface]"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

    m_NumInterfaces = m_UsbConfigurationDescriptor->bNumInterfaces;

    m_InterfaceList = BuildInterfaceList(m_UsbConfigurationDescriptor);

    if (m_InterfaceList)
	{
		PURB Urb = USBD_CreateConfigurationRequestEx(m_UsbConfigurationDescriptor, m_InterfaceList);

		if (Urb)
		{
			PUSBD_INTERFACE_INFORMATION InterfaceInfo = &Urb->UrbSelectConfiguration.Interface;

			ULONG i;

			for (i=0; i<InterfaceInfo->NumberOfPipes; i++)
			{
				//
				// perform pipe initialization here
				// set the transfer size and any pipe flags we use
				// USBD sets the rest of the Interface struct members
				//
				InterfaceInfo->Pipes[i].MaximumTransferSize = USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
			}

			ntStatus = CallUSBD(Urb);

			if (NT_SUCCESS(ntStatus))
			{
				m_UsbConfigurationHandle = Urb->UrbSelectConfiguration.ConfigurationHandle;
			}

			// Make copy of interfaces.
			for (i=0; i<m_NumInterfaces; i++)
			{
				PUSBD_INTERFACE_INFORMATION InterfaceInfo = (PUSBD_INTERFACE_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, m_InterfaceList[i].Interface->Length, 'mdW');

				if (InterfaceInfo)
				{
					RtlCopyMemory(InterfaceInfo, m_InterfaceList[i].Interface, m_InterfaceList[i].Interface->Length);

					m_InterfaceList[i].Interface = InterfaceInfo;
				}
				else
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				}
			}

			ExFreePool(Urb);
		}
		else
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}
	else
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}

	if (!NT_SUCCESS(ntStatus))
	{
        if (m_InterfaceList)
		{
			for (ULONG i=0; i<m_NumInterfaces; i++)
			{
				if (m_InterfaceList[i].Interface)
				{
					ExFreePool(m_InterfaceList[i].Interface);

					m_InterfaceList[i].Interface = NULL;
				}
			}

			ExFreePool(m_InterfaceList);

			m_InterfaceList = NULL;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::SelectAlternateInterface()
 *****************************************************************************
 *//*!
 * @brief
 * This routine attempts to select the specified alternate interface
 * setting for the specified interface.
 * @details
 * The InterfaceNumber and AlternateSetting parameters will be
 * validated by this routine and are not assumed to be valid.
 * This routines assumes that the device is currently configured, i.e.
 * Start() has been successfully called and Stop() has not been called.
 * This routine assumes that no pipes will be accessed while this
 * routine is executing as the m_InterfaceList data is updated by this routine.
 * This exclusion synchronization must be handled external to this routine.
 */
NTSTATUS
CUsbDevice::
SelectAlternateInterface
(
    IN		UCHAR	InterfaceNumber,
    IN		UCHAR	AlternateSetting
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::SelectAlternateInterface]"));

	// Find the Interface Descriptor which matches the InterfaceNumber
    // and AlternateSetting parameters.
    PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

	NTSTATUS ntStatus = GetInterfaceDescriptor(InterfaceNumber, AlternateSetting, -1, -1, -1, &InterfaceDescriptor);

    if (InterfaceDescriptor == NULL)
    {
        // Interface Descriptor not found, bad InterfaceNumber or
        // AlternateSetting.
        ntStatus = STATUS_INVALID_PARAMETER;
    }

	if (NT_SUCCESS(ntStatus))
	{
		// Allocate a URB_FUNCTION_SELECT_INTERFACE request structure
		//
		UCHAR NumEndpoints = InterfaceDescriptor->bNumEndpoints;

		USHORT UrbSize = GET_SELECT_INTERFACE_REQUEST_SIZE(NumEndpoints);

		PURB Urb = (PURB)ExAllocatePoolWithTag(NonPagedPool, UrbSize, 'mdW');

		if (Urb)
		{
			// Initialize the URB
			RtlZeroMemory(Urb, UrbSize);

			// Build the URB
			UsbBuildSelectInterfaceRequest
			(
				Urb,
				(USHORT)UrbSize,
				m_UsbConfigurationHandle,
				InterfaceNumber,
				AlternateSetting
			);

			PUSBD_INTERFACE_INFORMATION InterfaceInfoUrb = &Urb->UrbSelectInterface.Interface;

			InterfaceInfoUrb->Length = GET_USBD_INTERFACE_SIZE(NumEndpoints);

			for (UCHAR i = 0; i < NumEndpoints; i++)
			{
				InterfaceInfoUrb->Pipes[i].MaximumTransferSize = USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
				//FIXME: This shouldn't be here...
				//InterfaceInfoUrb->Pipes[i].PipeFlags |= USBD_PF_ENABLE_RT_THREAD_ACCESS;
			}

			// Allocate a USBD_INTERFACE_INFORMATION structure to hold the
			// result of the URB_FUNCTION_SELECT_INTERFACE request.
			PUSBD_INTERFACE_INFORMATION InterfaceInfoCopy = (PUSBD_INTERFACE_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, GET_USBD_INTERFACE_SIZE(NumEndpoints), 'mdW');

			if (InterfaceInfoCopy)
			{
			    // Now issue the USB request to select the alternate interface
				ntStatus = CallUSBD(Urb);

				_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::SelectAlternateInterface] - ntStatus: 0x%x", ntStatus));

				if (NT_SUCCESS(ntStatus))
				{
					// Save a copy of the interface information returned by the
					// SELECT_INTERFACE request.  This gives us a list of
					// PIPE_INFORMATION structures for each pipe opened in this
					// selected alternate interface setting.
					//
					ASSERT(InterfaceInfoUrb->Length == GET_USBD_INTERFACE_SIZE(NumEndpoints));

					RtlCopyMemory(InterfaceInfoCopy, InterfaceInfoUrb, GET_USBD_INTERFACE_SIZE(NumEndpoints));
				}
				else
				{
					// How to recover from a select alternate interface failure?
					//
					// The other currently configured interfaces (if any) should
					// not be disturbed by this select alternate interface failure.
					//
					// Just note that this interface now has no currently
					// configured pipes (i.e. InterfaceInfoCopy->NumberOfPipes == 0)
					//
					RtlZeroMemory(InterfaceInfoCopy, GET_USBD_INTERFACE_SIZE(NumEndpoints));

					InterfaceInfoCopy->Length = GET_USBD_INTERFACE_SIZE(NumEndpoints);
					InterfaceInfoCopy->InterfaceNumber = InterfaceNumber;
					InterfaceInfoCopy->AlternateSetting = AlternateSetting;
				}

				for (UCHAR i = 0; i < m_NumInterfaces; i++)
				{
					// Save pointers back into the USBD_PIPE_INFORMATION structures
					// contained within the above saved USBD_INTERFACE_INFORMATION
					// structure.
					//
					// This is makes it easier to iterate over all currently
					// configured pipes without having to interate over all
					// currently configured interfaces as we are in the middle of
					// doing right now.
					if ((m_InterfaceList[i].Interface->InterfaceNumber == InterfaceNumber) &&
						(InterfaceInfoCopy != NULL))
					{
						// Free the USBD_INTERFACE_INFORMATION for the previously
						// selected alternate interface setting and swap in the new
						// USBD_INTERFACE_INFORMATION copy.
						ExFreePool(m_InterfaceList[i].Interface);

						m_InterfaceList[i].InterfaceDescriptor = InterfaceDescriptor;
						m_InterfaceList[i].Interface = InterfaceInfoCopy;

						InterfaceInfoCopy = NULL;
					}
				}
			}
			else
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			}

			// Done with the URB
			ExFreePool(Urb);
		}
		else
		{
			// Could not allocate the URB.
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetInterfaceInformation()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetInterfaceInformation
(
	IN		UCHAR							InterfaceNumber,
	IN		UCHAR							AlternateSetting,
	OUT		PUSBD_INTERFACE_INFORMATION *	OutInterfaceInformation
)
{
	NTSTATUS ntStatus = STATUS_NOT_FOUND;

	for (ULONG i=0; i < m_NumInterfaces; i++)
	{
		if ((m_InterfaceList[i].InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
			(m_InterfaceList[i].InterfaceDescriptor->bAlternateSetting == AlternateSetting))
		{
			*OutInterfaceInformation = m_InterfaceList[i].Interface;

			ntStatus = STATUS_SUCCESS;
			break;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::CreateIrp()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
CreateIrp
(
    OUT		PIRP *	OutIrp
)
{
	ASSERT(OutIrp);

	*OutIrp = NULL;

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if (!m_Halted)
	{
        CCHAR StackSize = (CCHAR)(m_NextLowerDeviceObject->StackSize + 1);

		PIRP Irp = IoAllocateIrp(StackSize, FALSE);

		if (Irp)
		{
			*OutIrp = Irp;

			ntStatus = STATUS_SUCCESS;
		}
		else
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::RecycleIrp()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
RecycleIrp
(
	IN		PURB					Urb,
	IN		PIRP					Irp,
	IN		PIO_COMPLETION_ROUTINE	CompletionRoutine,
	IN		PVOID					Context
)
{
	IoReuseIrp(Irp, STATUS_SUCCESS);

	if (!Irp || !Urb || !CompletionRoutine)
    {
        return STATUS_INVALID_PARAMETER;
    }

    PIO_STACK_LOCATION NextIrpStack = IoGetNextIrpStackLocation(Irp);

	ASSERT(NextIrpStack);

    NextIrpStack->Parameters.Others.Argument1 = Urb;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    IoSetCompletionRoutine(Irp, CompletionRoutine, Context, TRUE, TRUE, TRUE);

    return IoCallDriver(m_NextLowerDeviceObject, Irp);
}

/*****************************************************************************
 * CUsbDevice::ResetIrp()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
ResetIrp
(
	IN		PIRP	Irp
)
{
    CCHAR StackSize = (CCHAR)(m_NextLowerDeviceObject->StackSize + 1);

    IoInitializeIrp(Irp, IoSizeOfIrp(StackSize), StackSize);

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::CompletionRoutine()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
CompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
	IN		PIRP            Irp,
    IN		PVOID           Context
)
{
	PUSBD_IO_COMPLETION_CONTEXT CompletionContext = PUSBD_IO_COMPLETION_CONTEXT(Context);

	if (CompletionContext)
	{
		CUsbDevice * that = (CUsbDevice *)CompletionContext->Context;

		if (Irp->IoStatus.Status == STATUS_DEVICE_NOT_CONNECTED)
		{
			if (that->m_CallbackRoutine)
			{
				that->m_CallbackRoutine(USB_DEVICE_CALLBACK_REASON_DEVICE_DISCONNECTED, NULL, that->m_CallbackData);
			}
		}
	}

    if (CompletionContext)
	{
		if (CompletionContext->ProcessEvent)
		{
			KeSetEvent((PRKEVENT)&CompletionContext->Event, IO_SOUND_INCREMENT, FALSE);
		}
	}

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/*****************************************************************************
 * CUsbDevice::CallUSBD()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
CallUSBD
(
    IN		PURB			Urb,
	IN		PLARGE_INTEGER	TimeOut
)
{
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    PIRP Irp = NULL;

	NTSTATUS ntStatus = CreateIrp(&Irp);

	if (NT_SUCCESS(ntStatus))
	{
		PIO_STACK_LOCATION NextIrpStack = IoGetNextIrpStackLocation(Irp);

		ASSERT(NextIrpStack);

		NextIrpStack->Parameters.Others.Argument1 = Urb;
		NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
		NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

		USBD_IO_COMPLETION_CONTEXT CompletionContext;

		CompletionContext.Context = this;

		if (KeGetCurrentIrql() == DISPATCH_LEVEL)
		{
			CompletionContext.ProcessEvent = FALSE;

			IoSetCompletionRoutine(Irp, CompletionRoutine, &CompletionContext, TRUE, TRUE, TRUE);

            ntStatus = IoCallDriver(m_NextLowerDeviceObject, Irp);
		}
		else
		{
			CompletionContext.ProcessEvent = TRUE;

			KeInitializeEvent(&CompletionContext.Event, NotificationEvent, FALSE);

			IoSetCompletionRoutine(Irp, CompletionRoutine, &CompletionContext, TRUE, TRUE, TRUE);

            ntStatus = IoCallDriver(m_NextLowerDeviceObject, Irp);

			if (ntStatus == STATUS_PENDING)
			{
				if (TimeOut)
				{
					ntStatus = KeWaitForSingleObject(&CompletionContext.Event, Executive, KernelMode, FALSE, TimeOut);

					if (ntStatus == STATUS_TIMEOUT)
					{
						ntStatus = STATUS_IO_TIMEOUT;

						// Cancel the Irp we just sent.
						IoCancelIrp(Irp);

						// And wait until the cancel completes.
						ntStatus = KeWaitForSingleObject(&CompletionContext.Event, Executive, KernelMode, FALSE, NULL);
					}
					else
					{
						ntStatus = Irp->IoStatus.Status;
					}
				}
				else
				{
					// Wait forever until this IRP is completed.
					ntStatus = KeWaitForSingleObject(&CompletionContext.Event, Executive, KernelMode, FALSE, NULL);
				}
			}
		}

		IoFreeIrp(Irp);
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetCurrentFrameNumber()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetCurrentFrameNumber
(
    OUT		ULONG *	OutFrameNumber
)
{
	NTSTATUS ntStatus;

	if (m_BusInterfaceV0.QueryBusTime)
	{
		ntStatus = m_BusInterfaceV0.QueryBusTime(m_BusInterfaceV0.BusContext, OutFrameNumber);
	}
	else
	{
		struct _URB_GET_CURRENT_FRAME_NUMBER Urb;

		Urb.Hdr.Function = URB_FUNCTION_GET_CURRENT_FRAME_NUMBER;
		Urb.Hdr.Length   = sizeof(Urb);
		Urb.FrameNumber = (ULONG)-1;

		ntStatus = CallUSBD((PURB)&Urb);

		if(NT_SUCCESS(ntStatus))
		{
			*OutFrameNumber = Urb.FrameNumber;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::SetSyncFrameNumber()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CUsbDevice::
SetSyncFrameNumber
(
	IN		ULONG	SyncFrameNumber
)
{
	m_SyncFrameNumber = SyncFrameNumber;
}

/*****************************************************************************
 * CUsbDevice::GetSyncFrameNumber()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
CUsbDevice::
GetSyncFrameNumber
(	void
)
{
	return m_SyncFrameNumber;
}

/*****************************************************************************
 * CUsbDevice::GetDeviceDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetDeviceDescriptor
(
	OUT		PUSB_DEVICE_DESCRIPTOR *	OutDeviceDescriptor
)
{
	ASSERT(OutDeviceDescriptor);

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if (m_UsbDeviceDescriptor.bLength)
	{
		*OutDeviceDescriptor = &m_UsbDeviceDescriptor;

		ntStatus = STATUS_SUCCESS;
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetConfigurationDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetConfigurationDescriptor
(
	OUT		PUSB_CONFIGURATION_DESCRIPTOR *	OutConfigurationDescriptor
)
{
	ASSERT(OutConfigurationDescriptor);

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if (m_UsbConfigurationDescriptor)
	{
		*OutConfigurationDescriptor = m_UsbConfigurationDescriptor;

		ntStatus = STATUS_SUCCESS;
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetInterfaceDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetInterfaceDescriptor
(
    IN		LONG						InterfaceNumber,
    IN		LONG						AlternateSetting,
	IN		LONG						InterfaceClass,
	IN		LONG						InterfaceSubClass,
	IN		LONG						InterfaceProtocol,
    OUT		PUSB_INTERFACE_DESCRIPTOR *	OutInterfaceDescriptor
)
{
	ASSERT(OutInterfaceDescriptor);

	NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_UsbConfigurationDescriptor)
	{
		// The USB common-class generic parent driver (usbccgp.sys) will split a USB device configuration
		// along its functional grouping. It updates the bNumInterfaces to reflect the number of 
		// interfaces in the sub-device, but it will not modify the bInterfaceNumber of the interfaces
		// to zero-offset it. Hence this check will fail if the first interface number in the configuration is
		// not zero. Comment out to avoid this failure.
		//if ((InterfaceNumber != -1) && (m_UsbConfigurationDescriptor->bNumInterfaces < InterfaceNumber))
		//{
		//	ntStatus = STATUS_INVALID_PARAMETER;
		//}

		//if (NT_SUCCESS(ntStatus))
		{
			// parse the config descriptor for the interface and
			// alternate setting we want
			PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = USBD_ParseConfigurationDescriptorEx
															(
																m_UsbConfigurationDescriptor,
																m_UsbConfigurationDescriptor,
																InterfaceNumber,
																AlternateSetting,
																InterfaceClass,
																InterfaceSubClass,
																InterfaceProtocol
															);

			if (InterfaceDescriptor)
			{
				*OutInterfaceDescriptor = InterfaceDescriptor;
			}
			else
			{
				*OutInterfaceDescriptor = NULL;

				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
	}
	else
	{
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetEndpointDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetEndpointDescriptor
(
    IN		UCHAR						InterfaceNumber,
    IN		UCHAR						AlternateSetting,
    IN		UCHAR						EndpointAddress,
    OUT		PUSB_ENDPOINT_DESCRIPTOR *	OutEndpointDescriptor
)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (m_UsbConfigurationDescriptor)
	{
		UCHAR CurrentPipeIndex;
		BOOL CorrectInterface = FALSE;
		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		PUCHAR DescriptorEnd = PUCHAR(m_UsbConfigurationDescriptor) + m_UsbConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)m_UsbConfigurationDescriptor;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
			((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (CommonDescriptor->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						return STATUS_UNSUCCESSFUL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if ((InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
							(InterfaceDescriptor->bAlternateSetting == AlternateSetting))
						{
							CorrectInterface = TRUE;
							CurrentPipeIndex = 0;
						}
						else
						{
							CorrectInterface = FALSE;
						}
					}
					break;

				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if (((CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
						 (CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)+2)) ||
						(InterfaceDescriptor == NULL))
					{
						return STATUS_UNSUCCESSFUL;
					}

					if (CorrectInterface)
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)CommonDescriptor;

						if (EndpointDescriptor->bEndpointAddress == EndpointAddress)
						{
							*OutEndpointDescriptor = EndpointDescriptor;

							return STATUS_SUCCESS;
						}
						else
						{
							CurrentPipeIndex++;
						}
					}
					break;

				default:

					if (InterfaceDescriptor == NULL)
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;
			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return STATUS_UNSUCCESSFUL;

}

/*****************************************************************************
 * CUsbDevice::GetEndpointDescriptorByIndex()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetEndpointDescriptorByIndex
(
    IN		UCHAR						InterfaceNumber,
    IN		UCHAR						AlternateSetting,
    IN		UCHAR						PipeIndex,
    OUT		PUSB_ENDPOINT_DESCRIPTOR *	OutEndpointDescriptor
)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (m_UsbConfigurationDescriptor)
	{
		UCHAR CurrentPipeIndex;
		BOOL CorrectInterface = FALSE;
		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		PUCHAR DescriptorEnd = PUCHAR(m_UsbConfigurationDescriptor) + m_UsbConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)m_UsbConfigurationDescriptor;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
			((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (CommonDescriptor->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						return STATUS_UNSUCCESSFUL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if ((InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
							(InterfaceDescriptor->bAlternateSetting == AlternateSetting))
						{
							if (InterfaceDescriptor->bNumEndpoints <= PipeIndex)
							{
								return STATUS_UNSUCCESSFUL;
							}

							CorrectInterface = TRUE;
							CurrentPipeIndex = 0;
						}
						else
						{
							CorrectInterface = FALSE;
						}
					}
					break;

				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if (((CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
						 (CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)+2)) ||
						(InterfaceDescriptor == NULL))
					{
						return STATUS_UNSUCCESSFUL;
					}

					if (CorrectInterface)
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)CommonDescriptor;

						if (CurrentPipeIndex == PipeIndex)
						{
							*OutEndpointDescriptor = EndpointDescriptor;

							return STATUS_SUCCESS;
						}
						else
						{
							CurrentPipeIndex++;
						}
					}
					break;

				default:

					if (InterfaceDescriptor == NULL)
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;
			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return STATUS_UNSUCCESSFUL;

}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbDevice::GetStringDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetStringDescriptor
(
	IN		UCHAR					Index,
	IN		USHORT					LanguageId,
	IN		PUSB_STRING_DESCRIPTOR	StringDescriptor
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

	UCHAR SizeOfStringDescriptor = StringDescriptor->bLength;

	// Reset it to zero for good measure.
	StringDescriptor->bLength = 0;

	RtlZeroMemory(StringDescriptor->bString, SizeOfStringDescriptor-2);

	if (Index == 0)
	{
		// Get LANGID codes.
		UCHAR DescriptorLength = UCHAR(2 + m_NumberOfLanguageSupported * sizeof(USHORT));

		if (DescriptorLength > SizeOfStringDescriptor)
		{
			DescriptorLength = SizeOfStringDescriptor;
		}

		StringDescriptor->bLength = DescriptorLength;
		StringDescriptor->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

		ULONG NumberOfLanguages = (DescriptorLength - 2) / sizeof(USHORT);

		PUSHORT wLANGID = PUSHORT(StringDescriptor->bString);

		for (ULONG i=0; i<NumberOfLanguages; i++)
		{
			wLANGID[i] = m_LanguageSupport[i].wLANGID;
		}

		ntStatus = STATUS_SUCCESS;
	}
	else
	{
		// Regular get string descriptor.
		PLANGUAGE_SUPPORT LanguageSupport = NULL;

		for (ULONG i=0; i<m_NumberOfLanguageSupported; i++)
		{
			if (LanguageId == m_LanguageSupport[i].wLANGID)
			{
				LanguageSupport = &m_LanguageSupport[i];
				break;
			}
		}

		if (LanguageSupport)
		{
			if (LanguageSupport->Location == LANGUAGE_SUPPORT_LOCATION_FILE)
			{
				CHAR SectionName[64]; sprintf(SectionName, "USB\\VID_%04X&PID_%04X.%04X", m_UsbDeviceDescriptor.idVendor, m_UsbDeviceDescriptor.idProduct, LanguageId);
				CHAR KeyName[8]; sprintf(KeyName, "%d", Index);

				StringDescriptor->bLength = (UCHAR)(DrvProfileGetUnicodeString(SectionName, KeyName, L"", StringDescriptor->bString, (SizeOfStringDescriptor-2)/sizeof(WCHAR), m_LanguageFile) * sizeof(WCHAR));

				if (StringDescriptor->bLength)
				{
					StringDescriptor->bLength += 2;

					StringDescriptor->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					// Fallback to the device.
					struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

					UsbBuildGetDescriptorRequest
					(
						(PURB)&Urb,
						sizeof Urb,
						USB_STRING_DESCRIPTOR_TYPE,
						Index,
						LanguageId,
						StringDescriptor,
						NULL,
						SizeOfStringDescriptor,
						NULL
					);

					ntStatus = CallUSBD((PURB)&Urb);

					if (NT_SUCCESS(ntStatus))
					{
						// Properly terminate the UNICODE string with NULL.
						ULONG Length = (StringDescriptor->bLength-2) / sizeof(WCHAR);

						if (Length < ((SizeOfStringDescriptor-2)/sizeof(WCHAR)))
						{
							PUSHORT String = PUSHORT(StringDescriptor->bString);

							String[Length] = 0;
						}
					}
				}
			}
			else if (LanguageSupport->Location == LANGUAGE_SUPPORT_LOCATION_DEVICE)
			{
				struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

				UsbBuildGetDescriptorRequest
				(
					(PURB)&Urb,
					sizeof Urb,
					USB_STRING_DESCRIPTOR_TYPE,
					Index,
					LanguageId,
					StringDescriptor,
					NULL,
					SizeOfStringDescriptor,
					NULL
				);

				ntStatus = CallUSBD((PURB)&Urb);

				if (NT_SUCCESS(ntStatus))
				{
					// Properly terminate the UNICODE string with NULL.
					ULONG Length = (StringDescriptor->bLength-2) / sizeof(WCHAR);

					if (Length < ((SizeOfStringDescriptor-2)/sizeof(WCHAR)))
					{
						PUSHORT String = PUSHORT(StringDescriptor->bString);

						String[Length] = 0;
					}
				}
			}
			else
			{
				// Huh ?
				ASSERT(0);

				StringDescriptor->bLength = 2;
				StringDescriptor->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		else
		{
			StringDescriptor->bLength = 2;
			StringDescriptor->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

			ntStatus = STATUS_NOT_SUPPORTED;
		}
	}

	if (NT_SUCCESS(ntStatus))
	{
		_DbgPrintF(DEBUGLVL_TERSE,("String Descriptor:"));
		_DbgPrintF(DEBUGLVL_TERSE,("-------------------------"));
		_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", StringDescriptor->bLength));
		_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", StringDescriptor->bDescriptorType));
		_DbgPrintF(DEBUGLVL_TERSE,("bString %ws", StringDescriptor->bString));
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CUsbDevice::GetClassInterfaceDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetClassInterfaceDescriptor
(
    IN		UCHAR						InterfaceNumber,
    IN		UCHAR						AlternateSetting,
	IN		UCHAR						ClassSpecificDescriptorType,
    OUT		PUSB_INTERFACE_DESCRIPTOR *	OutInterfaceDescriptor
)
{
	ASSERT(OutInterfaceDescriptor);

	NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_UsbConfigurationDescriptor)
	{
		// The USB common-class generic parent driver (usbccgp.sys) will split a USB device configuration
		// along its functional grouping. It updates the bNumInterfaces to reflect the number of 
		// interfaces in the sub-device, but it will not modify the bInterfaceNumber of the interfaces
		// to zero-offset it. Hence this check will fail if the first interface number in the configuration is
		// not zero. Comment out to avoid this failure.
		//if (m_UsbConfigurationDescriptor->bNumInterfaces < InterfaceNumber)
		//{
		//	ntStatus = STATUS_INVALID_PARAMETER;
		//}

		//if (NT_SUCCESS(ntStatus))
		{
			// parse the config descriptor for the interface and
			// alternate setting we want
			PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = USBD_ParseConfigurationDescriptorEx
															(
																m_UsbConfigurationDescriptor,
																m_UsbConfigurationDescriptor,
																InterfaceNumber,
																AlternateSetting,
																-1,
																-1,
																-1
															);

			if (InterfaceDescriptor)
			{
				PUCHAR DescriptorEnd = PUCHAR(m_UsbConfigurationDescriptor) + m_UsbConfigurationDescriptor->wTotalLength;

				PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)(PUCHAR(InterfaceDescriptor) + InterfaceDescriptor->bLength);

				if (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				    ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
				{
					if (CommonDescriptor->bDescriptorType == ClassSpecificDescriptorType)
					{
						*OutInterfaceDescriptor = PUSB_INTERFACE_DESCRIPTOR(CommonDescriptor);
					}
					else
					{
						ntStatus = STATUS_UNSUCCESSFUL;
					}
				}
				else
				{
					ntStatus = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
	}
	else
	{
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	if (!NT_SUCCESS(ntStatus))
	{
		*OutInterfaceDescriptor = NULL;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetClassEndpointDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetClassEndpointDescriptor
(
    IN		UCHAR						InterfaceNumber,
    IN		UCHAR						AlternateSetting,
    IN		UCHAR						EndpointAddress,
	IN		UCHAR						ClassSpecificDescriptorType,
    OUT		PUSB_ENDPOINT_DESCRIPTOR *	OutEndpointDescriptor
)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (m_UsbConfigurationDescriptor)
	{
		BOOL CorrectInterface = FALSE;

		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		PUCHAR DescriptorEnd = PUCHAR(m_UsbConfigurationDescriptor) + m_UsbConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)m_UsbConfigurationDescriptor;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  			   ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (CommonDescriptor->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						return STATUS_UNSUCCESSFUL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if ((InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
							(InterfaceDescriptor->bAlternateSetting == AlternateSetting))
						{
							CorrectInterface = TRUE;
						}
						else
						{
							CorrectInterface = FALSE;
						}
					}
					break;

				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if (((CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
						 (CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)+2)) ||
						(InterfaceDescriptor == NULL))
					{
						return STATUS_UNSUCCESSFUL;
					}

					if (CorrectInterface)
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)CommonDescriptor;

						if (EndpointDescriptor->bEndpointAddress == EndpointAddress)
						{
							CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)(PUCHAR(EndpointDescriptor) + EndpointDescriptor->bLength);

							if (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  								((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
							{
								if (CommonDescriptor->bDescriptorType == ClassSpecificDescriptorType)
								{
									*OutEndpointDescriptor = PUSB_ENDPOINT_DESCRIPTOR(CommonDescriptor);

									ntStatus = STATUS_SUCCESS;
								}
								else
								{
									ntStatus = STATUS_UNSUCCESSFUL;
								}
							}
							else
							{
								ntStatus = STATUS_UNSUCCESSFUL;
							}

							return ntStatus;
						}
					}
					break;

				default:

					if (InterfaceDescriptor == NULL)
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;
			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return STATUS_UNSUCCESSFUL;

}

// Control Class functions

/*****************************************************************************
 * CUsbDevice::CustomCommand()
 *****************************************************************************
 *//*!
 * @brief
 * Send a vendor command to the device.
 * @param
 * Request Request code for setup packet.
 * @param
 * Value Value for setup packet.
 * @param
 * Index Index for setup packet.
 * @param
 * Buffer_ Pointer to input/output buffer (optional).
 * @param
 * BufferLength Size of input/output buffer.
 * @param
 * OutBufferLength Pointer to the size of output buffer (optional).
 * @param
 * Input TRUE for input, FALSE for output.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CUsbDevice::
CustomCommand
(
	IN		UCHAR			RequestType,
	IN		UCHAR			Request,
	IN		USHORT			Value,
	IN		USHORT			Index,
	IN		PVOID			Buffer_,
	IN		ULONG			BufferLength,
	OUT		PULONG			OutBufferLength,
	IN		BOOLEAN			Input,
	IN		PLARGE_INTEGER	TimeOut
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	if (KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
		PVOID Buffer = NULL;

		if (BufferLength)
		{
			Buffer = ExAllocatePoolWithTag(NonPagedPool, BufferLength, 'mdW');

			if (Buffer)
			{
				if (Input)
				{
					RtlZeroMemory(Buffer, BufferLength);
				}
				else
				{
					RtlCopyMemory(Buffer, Buffer_, BufferLength);
				}
			}
			else
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			}
		}

		if (NT_SUCCESS(ntStatus))
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST Urb;

			UsbBuildVendorRequest
			(
				(PURB)&Urb,
				RequestType,
				sizeof(Urb),
				Input ? USBD_TRANSFER_DIRECTION_IN : USBD_TRANSFER_DIRECTION_OUT,
				0,
				Request,
				Value,
				Index,
				Buffer,
				NULL,
				BufferLength,
				NULL
			);

			ntStatus = CallUSBD((PURB)&Urb, TimeOut);

			if (NT_SUCCESS(ntStatus))
			{
				if (Input && Buffer && Buffer_ && Urb.TransferBufferLength)
				{
					RtlCopyMemory(Buffer_, Buffer, Urb.TransferBufferLength);
				}

				if (OutBufferLength)
				{
					*OutBufferLength = Urb.TransferBufferLength;
				}
			}
			
			//if (!NT_SUCCESS(ntStatus))
			//{
			//	DbgPrint("[CustomCommand] - Urb.Hdr.Status : 0x%x\n", Urb.Hdr.Status);
			//}
		}

		if (Buffer)
		{
			ExFreePool(Buffer);
		}
    }
	else
	{
		ntStatus = STATUS_ACCESS_DENIED;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::ControlClassEndpointCommand()
 *****************************************************************************
 *//*!
 * @brief
 * Send a vendor command to the device endpoint.
 * @param
 * Request Request code for setup packet.
 * @param
 * Value Value for setup packet.
 * @param
 * Index Index for setup packet.
 * @param
 * Buffer Pointer to input buffer (optional).
 * @param
 * BufferLength Size of input buffer.
 * @param
 * OutBufferLength Pointer to the size of output buffer (optional).
 * @param
 * Input TRUE for input, FALSE for output.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CUsbDevice::
ControlClassEndpointCommand
(
    IN		UCHAR	Request,
    IN		USHORT	Value,
    IN		USHORT	Index,
    IN		PVOID	Buffer,
    IN		ULONG	BufferLength,
	OUT		PULONG	OutBufferLength,
    IN		BOOLEAN Input
)
{
	LARGE_INTEGER TimeOut; TimeOut.QuadPart = -50000000; // 5s

    return CustomCommand
				(
					URB_FUNCTION_CLASS_ENDPOINT,
					Request,
					Value,
					Index,
					Buffer,
					BufferLength,
					OutBufferLength,
					Input,
					&TimeOut
				);
}

/*****************************************************************************
 * CUsbDevice::ControlClassInterfaceCommand()
 *****************************************************************************
 *//*!
 * @brief
 * Send a vendor command to the device interface.
 * @param
 * Request Request code for setup packet.
 * @param
 * Value Value for setup packet.
 * @param
 * Index Index for setup packet.
 * @param
 * Buffer Pointer to input buffer (optional).
 * @param
 * BufferLength Size of input buffer.
 * @param
 * OutBufferLength Pointer to the size of output buffer (optional).
 * @param
 * Input TRUE for input, FALSE for output.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CUsbDevice::
ControlClassInterfaceCommand
(
    IN		UCHAR	Request,
    IN		USHORT	Value,
    IN		USHORT	Index,
    IN		PVOID	Buffer,
    IN		ULONG	BufferLength,
	OUT		PULONG	OutBufferLength,
    IN		BOOLEAN Input
)
{
	LARGE_INTEGER TimeOut; TimeOut.QuadPart = -50000000; // 5s

	return CustomCommand
            (
                URB_FUNCTION_CLASS_INTERFACE,
                Request,
                Value,
                Index,
                Buffer,
                BufferLength,
				OutBufferLength,
                Input,
				&TimeOut
            );
}

// pipe related functions

/*****************************************************************************
 * CUsbDevice::ResetPipe()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
ResetPipe
(
	IN		USBD_PIPE_HANDLE	PipeHandle
)
{
	NTSTATUS ntStatus = STATUS_ACCESS_DENIED;

	if (KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
		PURB Urb = (PURB)ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST), 'mdW');

		if (Urb)
		{
			Urb->UrbHeader.Length = (USHORT)sizeof(struct _URB_PIPE_REQUEST);
			Urb->UrbHeader.Function = (USHORT)URB_FUNCTION_RESET_PIPE;
			Urb->UrbPipeRequest.PipeHandle = PipeHandle;

			ntStatus = CallUSBD(Urb);

			if (!NT_SUCCESS(ntStatus))
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::ResetPipe] Failed: %08x urb status %08x\n", ntStatus, Urb->UrbHeader.Status));
			}

			ExFreePool(Urb);
		}
		else
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::AbortPipe()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
AbortPipe
(
	IN		USBD_PIPE_HANDLE	PipeHandle
)
{
	NTSTATUS ntStatus = STATUS_ACCESS_DENIED;

	if (KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
		PURB Urb = (PURB)ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST), 'mdW');

		if (Urb)
		{
			Urb->UrbHeader.Length = (USHORT)sizeof(struct _URB_PIPE_REQUEST);
			Urb->UrbHeader.Function = (USHORT)URB_FUNCTION_ABORT_PIPE;
			Urb->UrbPipeRequest.PipeHandle = PipeHandle;

			ntStatus = CallUSBD(Urb);

			if (!NT_SUCCESS(ntStatus))
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CUsbDevice::AbortPipe] Failed: %08x urb status %08x\n", ntStatus, Urb->UrbHeader.Status));
			}

			ExFreePool(Urb);
		}
		else
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::ResetPort()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
ResetPort
(	void
)
{
    KEVENT Event;
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

	IO_STATUS_BLOCK IoStatus;

    PIRP Irp = IoBuildDeviceIoControlRequest
				(
					IOCTL_INTERNAL_USB_RESET_PORT,
                    m_NextLowerDeviceObject,
					NULL,
					0,
					NULL,
					0,
					TRUE,
					&Event,
					&IoStatus
				);

    NTSTATUS ntStatus = IoCallDriver(m_NextLowerDeviceObject, Irp);

    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

        ntStatus = IoStatus.Status;
    }

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetPortStatus()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetPortStatus
(
	OUT		PULONG	OutPortStatus
)
{
    KEVENT Event;
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

	IO_STATUS_BLOCK IoStatus;

    PIRP Irp = IoBuildDeviceIoControlRequest
				(
					IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                    m_NextLowerDeviceObject,
					NULL,
					0,
					NULL,
					0,
					TRUE,
					&Event,
	                &IoStatus
				);

    PIO_STACK_LOCATION NextIrpStack = IoGetNextIrpStackLocation(Irp);

	NextIrpStack->Parameters.Others.Argument1 = OutPortStatus;

    NTSTATUS ntStatus = IoCallDriver(m_NextLowerDeviceObject, Irp);

    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

		ntStatus = IoStatus.Status;
    }

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::IsDeviceHighSpeed()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL
CUsbDevice::
IsDeviceHighSpeed
(	void
)
{
	if (m_BusInterfaceV1.IsDeviceHighSpeed)
	{
		return m_BusInterfaceV1.IsDeviceHighSpeed(m_BusInterfaceV1.BusContext);
	}
	else
	{
		return FALSE;
	}
}

/*****************************************************************************
 * CUsbDevice::ClearFeature()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
ClearFeature
(
	IN		USHORT	Operation,
	IN		USHORT	FeatureSelector,
	IN		USHORT	Index
)
{
	struct _URB_CONTROL_FEATURE_REQUEST Urb;

	UsbBuildFeatureRequest
	(
		(PURB)&Urb,
		Operation,
		FeatureSelector,
		Index,
		NULL
	);

	NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

	return ntStatus;
}

#include "usbaudio.h"

// Debugging routines
/*****************************************************************************
 * PrintUsbDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
PrintUsbDescriptor
(
	IN		PVOID	Descriptor
)
{
	PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)Descriptor;

	switch (CommonDescriptor->bDescriptorType)
	{
		case USB_CONFIGURATION_DESCRIPTOR_TYPE:
		{
			PUSB_CONFIGURATION_DESCRIPTOR Descriptor = PUSB_CONFIGURATION_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE,("Configuration Descriptor: "));
			_DbgPrintF(DEBUGLVL_TERSE, ("------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE,("wTotalLength 0x%x", Descriptor->wTotalLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bNumInterfaces 0x%x", Descriptor->bNumInterfaces));
			_DbgPrintF(DEBUGLVL_TERSE,("bConfigurationValue 0x%x", Descriptor->bConfigurationValue));
			_DbgPrintF(DEBUGLVL_TERSE,("iConfiguration 0x%x", Descriptor->iConfiguration));
			_DbgPrintF(DEBUGLVL_TERSE,("bmAttributes 0x%x", Descriptor->bmAttributes));
			_DbgPrintF(DEBUGLVL_TERSE,("MaxPower 0x%x", Descriptor->MaxPower));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		case USB_INTERFACE_DESCRIPTOR_TYPE:
		{
			PUSB_INTERFACE_DESCRIPTOR Descriptor = PUSB_INTERFACE_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Interface Descriptor: "));
			_DbgPrintF(DEBUGLVL_TERSE, ("---------------------"));
			_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceNumber 0x%x", Descriptor->bInterfaceNumber));
			_DbgPrintF(DEBUGLVL_TERSE,("bAlternateSetting 0x%x", Descriptor->bAlternateSetting));
			_DbgPrintF(DEBUGLVL_TERSE,("bNumEndPoints 0x%x", Descriptor->bNumEndpoints));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceClass 0x%x", Descriptor->bInterfaceClass));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceSubClass 0x%x", Descriptor->bInterfaceSubClass));
			_DbgPrintF(DEBUGLVL_TERSE,("bInterfaceProtocol 0x%x", Descriptor->bInterfaceProtocol));
			_DbgPrintF(DEBUGLVL_TERSE,("iInterface 0x%x", Descriptor->iInterface));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		case USB_ENDPOINT_DESCRIPTOR_TYPE:
		{
			PUSB_ENDPOINT_DESCRIPTOR Descriptor = PUSB_ENDPOINT_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Endpoint Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("--------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, ("bEndpointAddress 0x%x", Descriptor->bEndpointAddress));
			_DbgPrintF(DEBUGLVL_TERSE, ("bmAttributes 0x%x", Descriptor->bmAttributes));
			_DbgPrintF(DEBUGLVL_TERSE, ("wMaxPacketSize 0x%x", Descriptor->wMaxPacketSize));
			_DbgPrintF(DEBUGLVL_TERSE, ("bInterval 0x%x", Descriptor->bInterval));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		case USB_STRING_DESCRIPTOR_TYPE:
		{
			PUSB_STRING_DESCRIPTOR Descriptor = PUSB_STRING_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE,("String Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE,("-------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE,("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE,("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE,("bString %ws", Descriptor->bString));
		}
		break;

		case USB_AUDIO_CS_INTERFACE:
		{
			PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR Descriptor = PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Class-specific MS Interface Header Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
			_DbgPrintF(DEBUGLVL_TERSE, ("bcdMSC 0x%x", Descriptor->bcdMSC));
			_DbgPrintF(DEBUGLVL_TERSE, ("wTotalLength 0x%x", Descriptor->wTotalLength));
			_DbgPrintF(DEBUGLVL_TERSE, (""));

			if (Descriptor->wTotalLength > Descriptor->bLength)
			{
				PUCHAR DescriptorEnd = PUCHAR(Descriptor) + Descriptor->wTotalLength;

				PUSB_AUDIO_COMMON_DESCRIPTOR CommonDescriptor = PUSB_AUDIO_COMMON_DESCRIPTOR(PUCHAR(Descriptor) + Descriptor->bLength);

				while (((PUCHAR(CommonDescriptor) + sizeof(USB_AUDIO_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				  	   ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
				{
					// ESI-ROMIO firmware has a bug in its CS MS interface descriptor. The total length it specify
					// for the CS MS interface is 0x4D00, but its configuration length is only 0x71. Go figure!!
					// This will break the infinite loop, but driver verifier will catch the access beyond allocated
					// memory, so don't run this routine if you have the verifier on.
					if (CommonDescriptor->bLength == 0) break;

					switch (CommonDescriptor->bDescriptorSubtype)
					{
						case USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK:
						{
							PUSB_AUDIO_MIDI_IN_JACK_DESCRIPTOR Descriptor = PUSB_AUDIO_MIDI_IN_JACK_DESCRIPTOR(CommonDescriptor);

							_DbgPrintF(DEBUGLVL_TERSE, ("MIDI IN Jack Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackType 0x%x", Descriptor->bJackType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackID 0x%x", Descriptor->bJackID));
							_DbgPrintF(DEBUGLVL_TERSE, ("iJack 0x%x", Descriptor->iJack));
							_DbgPrintF(DEBUGLVL_TERSE, (""));
						}
						break;

						case USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK:
						{
							PUSB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR Descriptor = PUSB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR(CommonDescriptor);

							_DbgPrintF(DEBUGLVL_TERSE, ("MIDI OUT Jack Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackType 0x%x", Descriptor->bJackType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bJackID 0x%x", Descriptor->bJackID));
							_DbgPrintF(DEBUGLVL_TERSE, ("bNrInputPins 0x%x", Descriptor->bNrInputPins));

							PUSB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR Pair = PUSB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR(PUCHAR(Descriptor) + USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_SOURCE_OFFSET);

							for (UCHAR i=0; i<Descriptor->bNrInputPins; i++)
							{
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourceID[%d] 0x%x", i, Pair[i].bSourceID));
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourcePin[%d] 0x%x", i, Pair[i].bSourcePin));
							}

							UCHAR iJack = *(PUCHAR(Descriptor) + USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_IJACK_OFFSET(Descriptor->bNrInputPins));
							_DbgPrintF(DEBUGLVL_TERSE, ("iJack 0x%x", iJack));

							_DbgPrintF(DEBUGLVL_TERSE, (""));
						}
						break;

						case USB_AUDIO_MS_DESCRIPTOR_ELEMENT:
						{
							PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR Descriptor = PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR(CommonDescriptor);

							_DbgPrintF(DEBUGLVL_TERSE, ("MIDI Element Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
							_DbgPrintF(DEBUGLVL_TERSE, ("bElementID 0x%x", Descriptor->bElementID));
							_DbgPrintF(DEBUGLVL_TERSE, ("bNrInputPins 0x%x", Descriptor->bNrInputPins));

							PUSB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR Pair = PUSB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR(PUCHAR(Descriptor) + USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_SOURCE_OFFSET);

							for (UCHAR i=0; i<Descriptor->bNrInputPins; i++)
							{
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourceID[%d] 0x%x", i, Pair[i].bSourceID));
								_DbgPrintF(DEBUGLVL_TERSE, ("baSourcePin[%d] 0x%x", i, Pair[i].bSourcePin));
							}

							PUSB_AUDIO_MIDI_ELEMENT_INFORMATION Information = PUSB_AUDIO_MIDI_ELEMENT_INFORMATION(PUCHAR(Descriptor) + USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_INFORMATION_OFFSET(Descriptor->bNrInputPins));

							_DbgPrintF(DEBUGLVL_TERSE, ("bNrOutputPins 0x%x", Information->bNrOutputPins));
							_DbgPrintF(DEBUGLVL_TERSE, ("bInTerminalLink 0x%x", Information->bInTerminalLink));
							_DbgPrintF(DEBUGLVL_TERSE, ("bOutTerminalLink 0x%x", Information->bOutTerminalLink));
							_DbgPrintF(DEBUGLVL_TERSE, ("bElCapsSize 0x%x", Information->bElCapsSize));

							for (UCHAR i=0; i<Information->bElCapsSize; i++)
							{
								_DbgPrintF(DEBUGLVL_TERSE, ("bmElementCaps[%d] 0x%x", i, Information->bmElementCaps[i]));
							}

							UCHAR iElement = *(PUCHAR(Descriptor) + USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_IELEMENT_OFFSET(Descriptor->bNrInputPins, Information->bElCapsSize));
							_DbgPrintF(DEBUGLVL_TERSE, ("iElement 0x%x", iElement));

							_DbgPrintF(DEBUGLVL_TERSE, (""));
						}
						break;

						default:
							_DbgPrintF(DEBUGLVL_TERSE, ("Unknown MIDI Descriptor:"));
							_DbgPrintF(DEBUGLVL_TERSE, ("----------------------------------------------"));
							_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", CommonDescriptor->bLength));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", CommonDescriptor->bDescriptorType));
							_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", CommonDescriptor->bDescriptorSubtype));
							break;
					}

					CommonDescriptor = (PUSB_AUDIO_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
				}
			}
		}
		break;

		case USB_AUDIO_CS_ENDPOINT:
		{
			PUSB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR Descriptor = PUSB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR(CommonDescriptor);

			_DbgPrintF(DEBUGLVL_TERSE, ("Class-specific MS Bulk Data Endpoint Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("-----------------------------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d", Descriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x", Descriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorSubtype 0x%x", Descriptor->bDescriptorSubtype));
			_DbgPrintF(DEBUGLVL_TERSE, ("bNumEmbMIDIJacks 0x%x", Descriptor->bNumEmbMIDIJacks));
			for (ULONG i=0; i<Descriptor->bNumEmbMIDIJacks;i++)
			{
				_DbgPrintF(DEBUGLVL_TERSE, ("baAssocJackID[%d] 0x%x", i, Descriptor->baAssocJackID[i]));
			}
			_DbgPrintF(DEBUGLVL_TERSE, (""));
		}
		break;

		default:
			_DbgPrintF(DEBUGLVL_TERSE, ("Unknown Descriptor:"));
			_DbgPrintF(DEBUGLVL_TERSE, ("-----------------------------------------------"));
			_DbgPrintF(DEBUGLVL_TERSE, ("bLength %d\n", CommonDescriptor->bLength));
			_DbgPrintF(DEBUGLVL_TERSE, ("bDescriptorType 0x%x\n", CommonDescriptor->bDescriptorType));
			_DbgPrintF(DEBUGLVL_TERSE, (""));
			break;
	}
}

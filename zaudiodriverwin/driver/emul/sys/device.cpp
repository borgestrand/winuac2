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
 * @file	   device.cpp
 * @brief	   USB device implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "device.h"

#define STR_MODULENAME "device: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbDevice::~CUsbDevice()
 *****************************************************************************
 */
CUsbDevice::
~CUsbDevice
(	void
)
{
	PAGED_CODE();

	m_ConfigurationList.DeleteAllItems();
}

/*****************************************************************************
 * CUsbDevice::Init()
 *****************************************************************************
 */
NTSTATUS
CUsbDevice::
Init
(
    IN      PDEVICE_OBJECT      NextLowerDeviceObject
)
{
	PAGED_CODE();

	m_NextLowerDeviceObject = NextLowerDeviceObject;

	m_CurrentConfigurationIndex = UCHAR(-1);

	NTSTATUS ntStatus = GetDeviceDescriptor(&m_UsbDeviceDescriptor);

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
    }

	// See how many configurations are there and create a configuration
	// object for each of them.
	for (UCHAR i=0; i<m_UsbDeviceDescriptor.bNumConfigurations; i++)
	{
		NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

		CUsbConfiguration * UsbConfiguration = new(NonPagedPool) CUsbConfiguration();

		if (UsbConfiguration)
		{
			ntStatus = UsbConfiguration->Init(this, i);
		}

		if (NT_SUCCESS(ntStatus))
		{
			ULONG OtherUsbAudioDescriptorSize = UsbConfiguration->GetOtherUsbAudioDescriptorSize();

			if (OtherUsbAudioDescriptorSize)
			{
				PUCHAR OtherUsbAudioDescriptor = PUCHAR(ExAllocatePool(NonPagedPool, OtherUsbAudioDescriptorSize));

				if (OtherUsbAudioDescriptor)
				{
					UsbConfiguration->GetOtherUsbAudioDescriptor(OtherUsbAudioDescriptor, OtherUsbAudioDescriptorSize);

					ExFreePool(OtherUsbAudioDescriptor);
				}
			}

			m_ConfigurationList.Put(UsbConfiguration);
		}

		if (!NT_SUCCESS(ntStatus))
		{
			delete UsbConfiguration;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetUsbConfiguration()
 *****************************************************************************
 */
NTSTATUS  
CUsbDevice::
GetUsbConfiguration
(
	IN		UCHAR					ConfigurationIndex,
	OUT		CUsbConfiguration **	OutUsbConfiguration
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	ULONG idx = 0;

	for (CUsbConfiguration * UsbConfiguration = m_ConfigurationList.First(); UsbConfiguration; UsbConfiguration = m_ConfigurationList.Next(UsbConfiguration))
	{
		if (ConfigurationIndex == idx)
		{
			*OutUsbConfiguration = UsbConfiguration;

			ntStatus = STATUS_SUCCESS;
			break;
		}

		idx++;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::SelectUsbConfiguration()
 *****************************************************************************
 */
NTSTATUS 
CUsbDevice::
SelectUsbConfiguration
(
	IN		UCHAR	ConfigurationIndex
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (ConfigurationIndex < m_ConfigurationList.Count())
	{
		m_CurrentConfigurationIndex = ConfigurationIndex;

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::DeselectUsbConfiguration()
 *****************************************************************************
 */
NTSTATUS 
CUsbDevice::
DeselectUsbConfiguration
(	void
)
{
	PAGED_CODE();

	m_CurrentConfigurationIndex = UCHAR(-1);

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbDevice::GetCurrentUsbConfiguration()
 *****************************************************************************
 */
NTSTATUS  
CUsbDevice::
GetCurrentUsbConfiguration
(
	OUT		CUsbConfiguration **	OutUsbConfiguration
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = GetUsbConfiguration(m_CurrentConfigurationIndex, OutUsbConfiguration);

	return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CUsbDevice::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	// device descriptor
	ULONG TotalLength = sizeof(USB_DEVICE_DESCRIPTOR);

	return TotalLength;
}

/*****************************************************************************
 * CUsbDevice::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CUsbDevice::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer
)
{
	PAGED_CODE();

	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor = PUSB_DEVICE_DESCRIPTOR(Buffer);

	DeviceDescriptor->bLength = sizeof(USB_DEVICE_DESCRIPTOR);
	DeviceDescriptor->bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE;
	DeviceDescriptor->bcdUSB = m_UsbDeviceDescriptor.bcdUSB;
	DeviceDescriptor->bDeviceClass = 0xEF;
	DeviceDescriptor->bDeviceSubClass = 0x02;
	DeviceDescriptor->bDeviceProtocol = 0x01;
	DeviceDescriptor->bMaxPacketSize0 = m_UsbDeviceDescriptor.bMaxPacketSize0;
	DeviceDescriptor->idVendor = m_UsbDeviceDescriptor.idVendor;
	DeviceDescriptor->idProduct = m_UsbDeviceDescriptor.idProduct;
	DeviceDescriptor->bcdDevice = m_UsbDeviceDescriptor.bcdDevice;
	DeviceDescriptor->iManufacturer = m_UsbDeviceDescriptor.iManufacturer;
	DeviceDescriptor->iProduct = m_UsbDeviceDescriptor.iProduct;
	DeviceDescriptor->iSerialNumber = m_UsbDeviceDescriptor.iSerialNumber;
	DeviceDescriptor->bNumConfigurations = m_UsbDeviceDescriptor.bNumConfigurations;

	return DeviceDescriptor->bLength;
}

#pragma code_seg()

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
 * CUsbDevice::GetDeviceDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbDevice::
GetDeviceDescriptor
(
	OUT		PUSB_DEVICE_DESCRIPTOR	DeviceDescriptor
)
{
	ASSERT(DeviceDescriptor);

    struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

    UsbBuildGetDescriptorRequest
	(
		(PURB)&Urb,
        sizeof Urb,
        USB_DEVICE_DESCRIPTOR_TYPE,
        0,
        0,
        DeviceDescriptor,
        NULL,
        sizeof(USB_DEVICE_DESCRIPTOR),
        NULL
	);

    NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

    return ntStatus;
}

/*****************************************************************************
 * CUsbDevice::GetConfigurationDescriptor()
 *****************************************************************************
 */
PUSB_CONFIGURATION_DESCRIPTOR 
CUsbDevice::
GetConfigurationDescriptor
(
	IN		UCHAR	ConfigurationIndex
)
{
	PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor = NULL;

	struct _URB_CONTROL_DESCRIPTOR_REQUEST Urb;

    USB_CONFIGURATION_DESCRIPTOR Ucd;

    UsbBuildGetDescriptorRequest
	(
        (PURB)&Urb, // points to the URB to be filled in
        sizeof(Urb),
        USB_CONFIGURATION_DESCRIPTOR_TYPE,
        ConfigurationIndex,      // number of configuration descriptor
        0,      // this parameter not used for configuration descriptors
        &Ucd,   // points to a USB_CONFIGURATION_DESCRIPTOR
        NULL,
        sizeof(Ucd),
        NULL
    );

    NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

    if (NT_SUCCESS(ntStatus))
	{
		ConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR)ExAllocatePool(NonPagedPool, Ucd.wTotalLength);

		if (ConfigurationDescriptor)
		{
			UsbBuildGetDescriptorRequest
			(
				(PURB)&Urb, // points to the URB to be filled in
				sizeof(Urb),
				USB_CONFIGURATION_DESCRIPTOR_TYPE,
				ConfigurationIndex,      // number of configuration descriptor
				0,      // this parameter not used for configuration descriptors
				ConfigurationDescriptor,  // points to a USB_CONFIGURATION_DESCRIPTOR
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


	return ConfigurationDescriptor;
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
		StringDescriptor->bLength,
		NULL
	);

	NTSTATUS ntStatus = CallUSBD((PURB)&Urb);

	return ntStatus;
}

#pragma code_seg()

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
			Buffer = ExAllocatePool(NonPagedPool, BufferLength);

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

#pragma code_seg()

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
 * @file	   filter.cpp
 * @brief	   USB device lower filter driver implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "filter.h"

#define STR_MODULENAME "filter: "

/*****************************************************************************
 * Forward declarations.
 *****************************************************************************
 */
PCHAR
PnPMinorFunctionString 
(
    IN		UCHAR	MinorFunction
);

PCHAR
UrbFunctionString 
(
    IN		USHORT	UrbFunction
);

NTSTATUS
FilterAddDevice
(
    IN		PDRIVER_OBJECT	DriverObject,
    IN		PDEVICE_OBJECT	PhysicalDeviceObject
);

NTSTATUS
FilterDispatchPnp 
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
);

NTSTATUS
FilterDispatchPower
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
);

VOID
FilterUnload
(
    IN		PDRIVER_OBJECT	DriverObject
);

NTSTATUS
FilterPass 
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
);

NTSTATUS
FilterStartCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID           Context
);

NTSTATUS
FilterDeviceUsageNotificationCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID           Context
);

NTSTATUS
FilterDispatchIo
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
);

NTSTATUS
FilterCreateUsbDevice
(
    IN		PDEVICE_OBJECT	DeviceObject
);

NTSTATUS
FilterDeleteUsbDevice
(
    IN		PDEVICE_OBJECT	DeviceObject
);

#pragma code_seg("PAGE")

/*****************************************************************************
 * DriverEntry()
 *****************************************************************************
 *//*!
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * This function is called by the operating system when the driver is loaded.
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
    IN		PDRIVER_OBJECT	DriverObject,
    IN		PUNICODE_STRING	RegistryPathName
)
{
	PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE, ("Starting breakpoint for debugging"));

	NTSTATUS ntStatus = STATUS_SUCCESS;

	//
    // Create dispatch points
	//
	PDRIVER_DISPATCH * DispatchFunction = DriverObject->MajorFunction;

    for (ULONG i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
        DispatchFunction[i] = FilterPass;
    }

    DriverObject->MajorFunction[IRP_MJ_PNP]            = FilterDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = FilterDispatchPower;
    DriverObject->DriverExtension->AddDevice           = FilterAddDevice;
    DriverObject->DriverUnload                         = FilterUnload;

    //
    // Set the following dispatch points as we will be doing
    // something useful to these requests instead of just
    // passing them down. 
    // 
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = FilterDispatchIo;

    return ntStatus;
}

/*****************************************************************************
 * FilterUnload()
 *****************************************************************************
 * Free all the allocated resources in DriverEntry, etc.
 */
VOID
FilterUnload
(
    IN		PDRIVER_OBJECT	DriverObject
)
{
    PAGED_CODE ();

    //
    // The device object(s) should be NULL now
    // (since we unload, all the devices objects associated with this
    // driver must be deleted.
    //
    ASSERT(DriverObject->DeviceObject == NULL);

    //
    // We should not be unloaded until all the devices we control
    // have been removed from our queue.
    //
	_DbgPrintF(DEBUGLVL_VERBOSE,("[FilterUnload] - Unloaded"));
}

/*****************************************************************************
 * FilterAddDevice()
 *****************************************************************************
 */
NTSTATUS
FilterAddDevice
(
    IN		PDRIVER_OBJECT	DriverObject,
    IN		PDEVICE_OBJECT	PhysicalDeviceObject
)
{
    PAGED_CODE ();

    PDEVICE_OBJECT DeviceObject = NULL;

    //
    // Create a filter device object.
    //
    NTSTATUS ntStatus = IoCreateDevice 
						(
							DriverObject,
                            sizeof(DEVICE_EXTENSION),
                            NULL,  // No Name
                            FILE_DEVICE_UNKNOWN,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &DeviceObject
						);


    if (NT_SUCCESS(ntStatus)) 
	{
		_DbgPrintF(DEBUGLVL_VERBOSE, ("[FilterAddDevice] - PDO (0x%x) FDO (0x%x)\n", PhysicalDeviceObject, DeviceObject));

		PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

		DeviceExtension->NextLowerDriver = IoAttachDeviceToDeviceStack(DeviceObject, PhysicalDeviceObject);

		//
		// Failure for attachment is an indication of a broken plug & play system.
		//
		if (NULL == DeviceExtension->NextLowerDriver) 
		{
			IoDeleteDevice(DeviceObject);

			ntStatus = STATUS_UNSUCCESSFUL;
		}

		if (NT_SUCCESS(ntStatus))
		{
			DeviceObject->Flags |= DeviceExtension->NextLowerDriver->Flags &
								   (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);

			DeviceObject->DeviceType = DeviceExtension->NextLowerDriver->DeviceType;

			DeviceObject->Characteristics = DeviceExtension->NextLowerDriver->Characteristics;

			DeviceExtension->Self = DeviceObject;

			//
			// Let us use remove lock to keep count of IRPs so that we don't 
			// deteach and delete our deviceobject until all pending I/Os in our
			// devstack are completed. Remlock is required to protect us from
			// various race conditions where our driver can get unloaded while we
			// are still running dispatch or completion code.
			//	    
			IoInitializeRemoveLock(&DeviceExtension->RemoveLock, POOL_TAG, 1, 100);	                                

			//
			// Set the initial state of the Filter DO
			//
			INITIALIZE_PNP_STATE(DeviceExtension);

			_DbgPrintF(DEBUGLVL_VERBOSE,("[FilterAddDevice] - %x to %x->%x \n", DeviceObject, DeviceExtension->NextLowerDriver, PhysicalDeviceObject));

			DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
		}
    }

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * FilterPass()
 *****************************************************************************
 * The default dispatch routine.  If this driver does not recognize the
 * IRP, then it should send it down, unmodified.
 * If the device holds iris, this IRP must be queued in the device extension
 * No completion routine is required.
 *
 * For demonstrative purposes only, we will pass all the (non-PnP) Irps down
 * on the stack (as we are a filter driver). A real driver might choose to
 * service some of these Irps.
 *
 * As we have NO idea which function we are happily passing on, we can make
 * NO assumptions about whether or not it will be called at raised IRQL.
 * For this reason, this function must be in put into non-paged pool
 * (aka the default location).
 */
NTSTATUS
FilterPass 
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
)
{
    NTSTATUS    status;
    
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    NTSTATUS ntStatus = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Irp);

	if (NT_SUCCESS(ntStatus))
	{
		IoSkipCurrentIrpStackLocation (Irp);

		ntStatus = IoCallDriver(DeviceExtension->NextLowerDriver, Irp);
		
		IoReleaseRemoveLock(&DeviceExtension->RemoveLock, Irp); 
	}
	else
	{
        Irp->IoStatus.Status = ntStatus;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

	return ntStatus;
}


#pragma code_seg("PAGE")

/*****************************************************************************
 * FilterDispatchPnp()
 *****************************************************************************
 * The plug and play dispatch routines.
 */
NTSTATUS
FilterDispatchPnp 
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp
)
{
    PAGED_CODE();

    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

    _DbgPrintF(DEBUGLVL_VERBOSE,("FilterDO %s IRP:0x%x", PnPMinorFunctionString(IrpStack->MinorFunction), Irp));

    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    NTSTATUS ntStatus = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Irp);

    if (NT_SUCCESS(ntStatus))
	{
		switch (IrpStack->MinorFunction) 
		{
			case IRP_MN_START_DEVICE:
			{
				//
				// The device is starting.
				// We cannot touch the device (send it any non pnp irps) until a
				// start device has been passed down to the lower drivers.
				//
			    KEVENT Event; KeInitializeEvent(&Event, NotificationEvent, FALSE);

				IoCopyCurrentIrpStackLocationToNext(Irp);
				
				IoSetCompletionRoutine
							(
								Irp,
								(PIO_COMPLETION_ROUTINE) FilterStartCompletionRoutine,
								&Event,
								TRUE,
								TRUE,
								TRUE
							);

				ntStatus = IoCallDriver(DeviceExtension->NextLowerDriver, Irp);
		        
				//
				// Wait for lower drivers to be done with the Irp. Important thing to
				// note here is when you allocate memory for an event in the stack  
				// you must do a KernelMode wait instead of UserMode to prevent 
				// the stack from getting paged out.
				//
				if (ntStatus == STATUS_PENDING) 
				{
					KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);          

					ntStatus = Irp->IoStatus.Status;
				}

				if (NT_SUCCESS(ntStatus)) 
				{
					//
					// As we are successfully now back, we will
					// first set our state to Started.
					//
					SET_NEW_PNP_STATE(DeviceExtension, Started);

					//
					// On the way up inherit FILE_REMOVABLE_MEDIA during Start.
					// This characteristic is available only after the driver stack is started!.
					//
					if (DeviceExtension->NextLowerDriver->Characteristics & FILE_REMOVABLE_MEDIA) 
					{
						DeviceObject->Characteristics |= FILE_REMOVABLE_MEDIA;
					}

					//
					// If the PreviousPnPState is stopped then we are being stopped temporarily
					// and restarted for resource rebalance. 
					//
					if (Stopped != DeviceExtension->PreviousPnPState) 
					{
						//
						// Device is started for the first time.
						//
						FilterCreateUsbDevice(DeviceObject);
					}
				}
		        
				Irp->IoStatus.Status = ntStatus;

				IoCompleteRequest (Irp, IO_NO_INCREMENT);
				
				IoReleaseRemoveLock(&DeviceExtension->RemoveLock, Irp); 

				return ntStatus;
			}

			case IRP_MN_REMOVE_DEVICE:
			{
				//
				// Wait for all outstanding requests to complete
				//
				_DbgPrintF(DEBUGLVL_VERBOSE,("Waiting for outstanding requests"));

				IoReleaseRemoveLockAndWait(&DeviceExtension->RemoveLock, Irp);

				IoSkipCurrentIrpStackLocation(Irp);

				ntStatus = IoCallDriver(DeviceExtension->NextLowerDriver, Irp);

				SET_NEW_PNP_STATE(DeviceExtension, Deleted);
		        
				FilterDeleteUsbDevice(DeviceObject);

				IoDetachDevice(DeviceExtension->NextLowerDriver);
				IoDeleteDevice(DeviceObject);

				return ntStatus;
			}

			case IRP_MN_QUERY_STOP_DEVICE:
			{
				SET_NEW_PNP_STATE(DeviceExtension, StopPending);

				ntStatus = STATUS_SUCCESS;
			}
			break;

			case IRP_MN_CANCEL_STOP_DEVICE:
			{
				//
				// Check to see whether you have received cancel-stop
				// without first receiving a query-stop. This could happen if someone
				// above us fails a query-stop and passes down the subsequent
				// cancel-stop.
				//
				if (StopPending == DeviceExtension->DevicePnPState)
				{
					//
					// We did receive a query-stop, so restore.
					//
					RESTORE_PREVIOUS_PNP_STATE(DeviceExtension);
				}

				ntStatus = STATUS_SUCCESS; // We must not fail this IRP.
			}
			break;

			case IRP_MN_STOP_DEVICE:
			{
				SET_NEW_PNP_STATE(DeviceExtension, Stopped);

				ntStatus = STATUS_SUCCESS;
			}
			break;

			case IRP_MN_QUERY_REMOVE_DEVICE:
			{
				SET_NEW_PNP_STATE(DeviceExtension, RemovePending);

				ntStatus = STATUS_SUCCESS;
			}
			break;

			case IRP_MN_SURPRISE_REMOVAL:
			{
				SET_NEW_PNP_STATE(DeviceExtension, SurpriseRemovePending);

				ntStatus = STATUS_SUCCESS;
			}
			break;

			case IRP_MN_CANCEL_REMOVE_DEVICE:
			{
				//
				// Check to see whether you have received cancel-remove
				// without first receiving a query-remove. This could happen if
				// someone above us fails a query-remove and passes down the
				// subsequent cancel-remove.
				//
				if (RemovePending == DeviceExtension->DevicePnPState)
				{
					//
					// We did receive a query-remove, so restore.
					//
					RESTORE_PREVIOUS_PNP_STATE(DeviceExtension);
				}

				ntStatus = STATUS_SUCCESS; // We must not fail this IRP.
			}
			break;

			case IRP_MN_DEVICE_USAGE_NOTIFICATION:
			{
				//
				// On the way down, pagable might become set. Mimic the driver
				// above us. If no one is above us, just set pagable.
				//
				if ((DeviceObject->AttachedDevice == NULL) ||
					(DeviceObject->AttachedDevice->Flags & DO_POWER_PAGABLE)) 
				{
					DeviceObject->Flags |= DO_POWER_PAGABLE;
				}

				IoCopyCurrentIrpStackLocationToNext(Irp);

				IoSetCompletionRoutine
					(
						Irp,
						FilterDeviceUsageNotificationCompletionRoutine,
						NULL,
						TRUE,
						TRUE,
						TRUE
					);

				return IoCallDriver(DeviceExtension->NextLowerDriver, Irp);
			}

			default:
			{
				//
				// If you don't handle any IRP you must leave the
				// status as is.
				//
				ntStatus = Irp->IoStatus.Status;
			}
			break;
		}

		//
		// Pass the IRP down and forget it.
		//
		Irp->IoStatus.Status = ntStatus;

		IoSkipCurrentIrpStackLocation(Irp);
		
		ntStatus = IoCallDriver(DeviceExtension->NextLowerDriver, Irp);

		IoReleaseRemoveLock(&DeviceExtension->RemoveLock, Irp); 
	}
	else
	{
        Irp->IoStatus.Status = ntStatus;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * FilterStartCompletionRoutine()
 *****************************************************************************
 * A completion routine for use when calling the lower device objects to
 * which our filter deviceobject is attached.
 */
NTSTATUS
FilterStartCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP            Irp,
    IN		PVOID           Context
)
{
    PKEVENT Event = (PKEVENT)Context;

    //
    // If the lower driver didn't return STATUS_PENDING, we don't need to 
    // set the event because we won't be waiting on it. 
    // This optimization avoids grabbing the dispatcher lock, and improves perf.
    //
    if (Irp->PendingReturned == TRUE) 
	{
        KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    }

    //
    // The dispatch routine will have to call IoCompleteRequest
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}

/*****************************************************************************
 * FilterDeviceUsageNotificationCompletionRoutine()
 *****************************************************************************
 * A completion routine for use when calling the lower device objects to
 * which our filter deviceobject is attached.
 */
NTSTATUS
FilterDeviceUsageNotificationCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP            Irp,
    IN		PVOID           Context
)
{
    if (Irp->PendingReturned) 
	{
        IoMarkIrpPending(Irp);
    }

    //
    // On the way up, pagable might become clear. Mimic the driver below us.
    //
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	if (!(DeviceExtension->NextLowerDriver->Flags & DO_POWER_PAGABLE)) 
	{
        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
    }

    IoReleaseRemoveLock(&DeviceExtension->RemoveLock, Irp); 

    return STATUS_CONTINUE_COMPLETION;
}

/*****************************************************************************
 * FilterDispatchPower()
 *****************************************************************************
 * This routine is the dispatch routine for power irps.
 */
NTSTATUS
FilterDispatchPower
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP            Irp
)
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    NTSTATUS ntStatus = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Irp);

    if (NT_SUCCESS(ntStatus))
	{
		PoStartNextPowerIrp(Irp);

		IoSkipCurrentIrpStackLocation(Irp);

		ntStatus = PoCallDriver(DeviceExtension->NextLowerDriver, Irp);

		IoReleaseRemoveLock(&DeviceExtension->RemoveLock, Irp); 
	}
	else
	{ 
		// may be device is being removed.
        Irp->IoStatus.Status = ntStatus;

        PoStartNextPowerIrp(Irp);
        
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
        
		return ntStatus;
    }

	return ntStatus;
}

/*****************************************************************************
 * FilterDispatchIo()
 *****************************************************************************
 * This routine is the dispatch routine for non passthru irps. We will check 
 * the input device object to see if the request is meant for the control 
 * device object. If it is, we will handle and complete the IRP, if not, we 
 * will pass it down to the lower driver.
 */
NTSTATUS
FilterDispatchIo
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP            Irp
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    
    Irp->IoStatus.Information = 0;
    
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) 
	{
		case IOCTL_INTERNAL_USB_SUBMIT_URB:
		{
			PURB Urb = PURB(IrpStack->Parameters.Others.Argument1);
			
			if (Urb)
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("URB: %s", UrbFunctionString(Urb->UrbHeader.Function)));
				
				switch (Urb->UrbHeader.Function)
				{
					case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
					{
						switch (Urb->UrbControlDescriptorRequest.DescriptorType)
						{
							case USB_DEVICE_DESCRIPTOR_TYPE:
							{
								_DbgPrintF(DEBUGLVL_VERBOSE,("DescriptorType: %s", "USB_DEVICE_DESCRIPTOR_TYPE"));
								_DbgPrintF(DEBUGLVL_VERBOSE,("Index: %d", Urb->UrbControlDescriptorRequest.Index));

								if (DeviceExtension->UsbDevice)
								{
									ntStatus = DeviceExtension->UsbDevice->GetOtherUsbAudioDescriptor(PUCHAR(Urb->UrbControlDescriptorRequest.TransferBuffer));

									Irp->IoStatus.Status = ntStatus;

									IoCompleteRequest(Irp, IO_NO_INCREMENT);
								}
								else
								{
									ntStatus = FilterPass(DeviceObject, Irp);
								}
							}
							break;

							case USB_CONFIGURATION_DESCRIPTOR_TYPE:
							{
								_DbgPrintF(DEBUGLVL_VERBOSE,("DescriptorType: %s", "USB_CONFIGURATION_DESCRIPTOR_TYPE"));
								_DbgPrintF(DEBUGLVL_VERBOSE,("Index: %d", Urb->UrbControlDescriptorRequest.Index));

								if (DeviceExtension->UsbDevice)
								{
									CUsbConfiguration * UsbConfiguration = NULL;

									ntStatus = DeviceExtension->UsbDevice->GetUsbConfiguration(Urb->UrbControlDescriptorRequest.Index, &UsbConfiguration);

									if (NT_SUCCESS(ntStatus))
									{
										ntStatus = UsbConfiguration->GetOtherUsbAudioDescriptor(PUCHAR(Urb->UrbControlDescriptorRequest.TransferBuffer), Urb->UrbControlDescriptorRequest.TransferBufferLength);
									}

									Irp->IoStatus.Status = ntStatus;

									IoCompleteRequest(Irp, IO_NO_INCREMENT);
								}
								else
								{
									ntStatus = FilterPass(DeviceObject, Irp);
								}
							}
							break;

							case USB_STRING_DESCRIPTOR_TYPE:
							{
								_DbgPrintF(DEBUGLVL_VERBOSE,("DescriptorType: %s", "USB_STRING_DESCRIPTOR_TYPE"));
								_DbgPrintF(DEBUGLVL_VERBOSE,("Index: %d", Urb->UrbControlDescriptorRequest.Index));
								_DbgPrintF(DEBUGLVL_VERBOSE,("LanguageId: %d", Urb->UrbControlDescriptorRequest.LanguageId));

								ntStatus = FilterPass(DeviceObject, Irp);
							}
							break;
						}
					}
					break;

					case URB_FUNCTION_SELECT_CONFIGURATION:
					{
						_DbgPrintF(DEBUGLVL_VERBOSE,("ConfigurationDescriptor: %p", Urb->UrbSelectConfiguration.ConfigurationDescriptor));

						if (DeviceExtension->UsbDevice)
						{
							if (Urb->UrbSelectConfiguration.ConfigurationDescriptor)
							{
								DeviceExtension->UsbDevice->SelectUsbConfiguration(Urb->UrbSelectConfiguration.ConfigurationDescriptor->bConfigurationValue-1);
							}
							else
							{
								DeviceExtension->UsbDevice->DeselectUsbConfiguration();
							}

							ntStatus = FilterPass(DeviceObject, Irp);
						}
						else
						{
							ntStatus = FilterPass(DeviceObject, Irp);
						}

						_DbgPrintF(DEBUGLVL_VERBOSE,("ConfigurationHandle: %p", Urb->UrbSelectConfiguration.ConfigurationHandle));
					}
					break;

					case URB_FUNCTION_SELECT_INTERFACE:
					{
						_DbgPrintF(DEBUGLVL_VERBOSE,("ConfigurationHandle: %p", Urb->UrbSelectInterface.ConfigurationHandle));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Interface.Length: %d", Urb->UrbSelectInterface.Interface.Length));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Interface.InterfaceNumber: %d", Urb->UrbSelectInterface.Interface.InterfaceNumber));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Interface.AlternateSetting: %d", Urb->UrbSelectInterface.Interface.AlternateSetting));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Interface.Class: %02x", Urb->UrbSelectInterface.Interface.Class));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Interface.SubClass: %02x", Urb->UrbSelectInterface.Interface.SubClass));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Interface.Protocol: %02x", Urb->UrbSelectInterface.Interface.Protocol));

						if (DeviceExtension->UsbDevice)
						{
							CUsbConfiguration * UsbConfiguration = NULL;

							ntStatus = DeviceExtension->UsbDevice->GetCurrentUsbConfiguration(&UsbConfiguration);

							if (NT_SUCCESS(ntStatus))
							{
								UCHAR InterfaceNumber = Urb->UrbSelectInterface.Interface.InterfaceNumber;

								CUsbInterface * UsbInterface = NULL;
								
								ntStatus = UsbConfiguration->GetInterface(InterfaceNumber, &UsbInterface);

								if (NT_SUCCESS(ntStatus))
								{
									UsbInterface->SelectAlternateSetting(Urb->UrbSelectInterface.Interface.AlternateSetting, &Urb->UrbSelectInterface.Interface.AlternateSetting);
								}
							}
						}

						ntStatus = FilterPass(DeviceObject, Irp);
					}
					break;

					case URB_FUNCTION_CLASS_INTERFACE:
					case URB_FUNCTION_CLASS_ENDPOINT:
					{
						_DbgPrintF(DEBUGLVL_VERBOSE,("RequestTypeReservedBits: %02x", Urb->UrbControlVendorClassRequest.RequestTypeReservedBits));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Request: %02x", Urb->UrbControlVendorClassRequest.Request));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Value: %04x", Urb->UrbControlVendorClassRequest.Value));
						_DbgPrintF(DEBUGLVL_VERBOSE,("Index: %04x", Urb->UrbControlVendorClassRequest.Index));
					
						if (DeviceExtension->UsbDevice)
						{
							CUsbConfiguration * UsbConfiguration = NULL;

							ntStatus = DeviceExtension->UsbDevice->GetCurrentUsbConfiguration(&UsbConfiguration);

							if (NT_SUCCESS(ntStatus))
							{
								UCHAR InterfaceNumber = UCHAR(Urb->UrbControlVendorClassRequest.Index & 0xFF);

								CUsbInterface * UsbInterface = NULL;
								
								ntStatus = UsbConfiguration->GetInterface(InterfaceNumber, &UsbInterface);

								if (NT_SUCCESS(ntStatus))
								{
									UCHAR EntityID = UCHAR(Urb->UrbControlVendorClassRequest.Index >> 8);

									PENTITY Entity = NULL;

									ntStatus = UsbInterface->GetEntity(EntityID, &Entity);

									if (NT_SUCCESS(ntStatus))
									{
										if (Urb->UrbControlVendorClassRequest.Request & 0x80)
										{
											// Get request
											ntStatus = Entity->ReadParameterBlock
														(
															Urb->UrbControlVendorClassRequest.Request & (~0x80),
															UCHAR((Urb->UrbControlVendorClassRequest.Value & 0xFF00) >> 8),
															UCHAR((Urb->UrbControlVendorClassRequest.Value & 0x00FF)),
															Urb->UrbControlVendorClassRequest.TransferBuffer,
															Urb->UrbControlVendorClassRequest.TransferBufferLength,
															&Urb->UrbControlVendorClassRequest.TransferBufferLength
														);

										}
										else
										{
											// Set request
											ntStatus = Entity->WriteParameterBlock
														(
															Urb->UrbControlVendorClassRequest.Request & (~0x80),
															UCHAR((Urb->UrbControlVendorClassRequest.Value & 0xFF00) >> 8),
															UCHAR((Urb->UrbControlVendorClassRequest.Value & 0x00FF)),
															Urb->UrbControlVendorClassRequest.TransferBuffer,
															Urb->UrbControlVendorClassRequest.TransferBufferLength
														);
										}
									}
								}
							}

							Irp->IoStatus.Status = ntStatus;

							IoCompleteRequest(Irp, IO_NO_INCREMENT);
						}
						else
						{
							ntStatus = FilterPass(DeviceObject, Irp);
						}
					}
					break;

					default:
					{
						ntStatus = FilterPass(DeviceObject, Irp);
					}
					break;
				}
			}
			else
			{
				ntStatus = FilterPass(DeviceObject, Irp);
			}
		}
		break;

        default:
		{
            ntStatus = FilterPass(DeviceObject, Irp);
		}
        break;
    }
    
	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * FilterCreateUsbDevice()
 *****************************************************************************
 */
NTSTATUS
FilterCreateUsbDevice
(
    IN		PDEVICE_OBJECT	DeviceObject
)
{
	PAGED_CODE();

	NTSTATUS ntStatus;

	CUsbDevice * UsbDevice = new(NonPagedPool) CUsbDevice();

	if (UsbDevice)
	{
	    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

		ntStatus = UsbDevice->Init(DeviceExtension->NextLowerDriver);

		if (NT_SUCCESS(ntStatus))
		{
			DeviceExtension->UsbDevice = UsbDevice;
		}
	}
	else
	{
		ntStatus = STATUS_NO_MEMORY;
	}

	if (!NT_SUCCESS(ntStatus))
	{
		if (UsbDevice)
		{
			UsbDevice->Destruct();
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * FilterDeleteUsbDevice()
 *****************************************************************************
 */
NTSTATUS
FilterDeleteUsbDevice
(
    IN		PDEVICE_OBJECT	DeviceObject
)
{
	PAGED_CODE();

	PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	if (DeviceExtension->UsbDevice)
	{
		DeviceExtension->UsbDevice->Destruct();
		DeviceExtension->UsbDevice = NULL;
	}

	return STATUS_SUCCESS;
}

#pragma code_seg()

#if DBG
/*****************************************************************************
 * PnPMinorFunctionString()
 *****************************************************************************
 */
PCHAR
PnPMinorFunctionString 
(
    IN		UCHAR	MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";

        default:
            return "UNKNOWN PNP IRP";
    }
}

/*****************************************************************************
 * UrbFunctionString()
 *****************************************************************************
 */
PCHAR
UrbFunctionString 
(
    IN		USHORT	UrbFunction
)
{
    switch (UrbFunction)
    {
        case URB_FUNCTION_SELECT_CONFIGURATION:
            return "URB_FUNCTION_SELECT_CONFIGURATION";
        case URB_FUNCTION_SELECT_INTERFACE:
            return "URB_FUNCTION_SELECT_INTERFACE";
        case URB_FUNCTION_ABORT_PIPE:
            return "URB_FUNCTION_ABORT_PIPE";
        case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL:
            return "URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL";
        case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL:
            return "URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL";
        case URB_FUNCTION_GET_FRAME_LENGTH:
            return "URB_FUNCTION_GET_FRAME_LENGTH";
        case URB_FUNCTION_SET_FRAME_LENGTH:
            return "URB_FUNCTION_SET_FRAME_LENGTH";
        case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER:
            return "URB_FUNCTION_GET_CURRENT_FRAME_NUMBER";
        case URB_FUNCTION_CONTROL_TRANSFER:
            return "URB_FUNCTION_CONTROL_TRANSFER";
        case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
            return "URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER";
        case URB_FUNCTION_ISOCH_TRANSFER:
            return "URB_FUNCTION_ISOCH_TRANSFER";
        case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
            return "URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE";
		case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
            return "URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE";
        case URB_FUNCTION_SET_FEATURE_TO_DEVICE:
            return "URB_FUNCTION_SET_FEATURE_TO_DEVICE";
        case URB_FUNCTION_SET_FEATURE_TO_INTERFACE:
            return "URB_FUNCTION_SET_FEATURE_TO_INTERFACE";
        case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:
            return "URB_FUNCTION_SET_FEATURE_TO_ENDPOINT";
        case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:
            return "URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE";
        case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:
            return "URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE";
        case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:
            return "URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT";
        case URB_FUNCTION_GET_STATUS_FROM_DEVICE:
            return "URB_FUNCTION_GET_STATUS_FROM_DEVICE";
        case URB_FUNCTION_GET_STATUS_FROM_INTERFACE:
            return "URB_FUNCTION_GET_STATUS_FROM_INTERFACE";
        case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:
            return "URB_FUNCTION_GET_STATUS_FROM_ENDPOINT";
        case URB_FUNCTION_RESERVED_0X0016:
            return "URB_FUNCTION_RESERVED_0X0016";
        case URB_FUNCTION_VENDOR_DEVICE:
            return "URB_FUNCTION_VENDOR_DEVICE";
        case URB_FUNCTION_VENDOR_INTERFACE:
            return "URB_FUNCTION_VENDOR_INTERFACE";
        case URB_FUNCTION_VENDOR_ENDPOINT:
            return "URB_FUNCTION_VENDOR_ENDPOINT";
        case URB_FUNCTION_CLASS_DEVICE:
            return "URB_FUNCTION_CLASS_DEVICE";
        case URB_FUNCTION_CLASS_INTERFACE:
            return "URB_FUNCTION_CLASS_INTERFACE";
        case URB_FUNCTION_CLASS_ENDPOINT:
            return "URB_FUNCTION_CLASS_ENDPOINT";
        case URB_FUNCTION_RESERVE_0X001D:
            return "URB_FUNCTION_RESERVE_0X001D";
        case URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL:
            return "URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL";
        case URB_FUNCTION_CLASS_OTHER:
            return "URB_FUNCTION_CLASS_OTHER";
        case URB_FUNCTION_VENDOR_OTHER:
            return "URB_FUNCTION_VENDOR_OTHER";
        case URB_FUNCTION_GET_STATUS_FROM_OTHER:
            return "URB_FUNCTION_GET_STATUS_FROM_OTHER";
        case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:
            return "URB_FUNCTION_CLEAR_FEATURE_TO_OTHER";
        case URB_FUNCTION_SET_FEATURE_TO_OTHER:
            return "URB_FUNCTION_SET_FEATURE_TO_OTHER";
		case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
            return "URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT";
        case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
            return "URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT";
        case URB_FUNCTION_GET_CONFIGURATION:
            return "URB_FUNCTION_GET_CONFIGURATION";
        case URB_FUNCTION_GET_INTERFACE:
            return "URB_FUNCTION_GET_INTERFACE";
        case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
            return "URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE";
        case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
            return "URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE";
        case URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR:
            return "URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR";
        case URB_FUNCTION_RESERVE_0X002B:
            return "URB_FUNCTION_RESERVE_0X002B";
        case URB_FUNCTION_RESERVE_0X002C:
            return "URB_FUNCTION_RESERVE_0X002C";
        case URB_FUNCTION_RESERVE_0X002D:
            return "URB_FUNCTION_RESERVE_0X002D";
        case URB_FUNCTION_RESERVE_0X002E:
            return "URB_FUNCTION_RESERVE_0X002E";
        case URB_FUNCTION_RESERVE_0X002F:
            return "URB_FUNCTION_RESERVE_0X002F";
        case URB_FUNCTION_SYNC_RESET_PIPE:
            return "URB_FUNCTION_SYNC_RESET_PIPE";
        case URB_FUNCTION_SYNC_CLEAR_STALL:
            return "URB_FUNCTION_SYNC_CLEAR_STALL";

        default:
            return "UNKNOWN URB FUNCTION";
    }
}
#endif // DBG

#pragma code_seg()


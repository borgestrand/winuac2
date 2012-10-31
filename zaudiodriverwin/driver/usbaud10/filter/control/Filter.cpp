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
 * @file       Filter.cpp
 * @brief      Device control filter implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "Filter.h"
#include "Tables.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "CTRL_FILTER: "

/*****************************************************************************
 * Referenced forward
 */

#pragma code_seg("PAGE")

/*****************************************************************************
 * CControlFilter::DispatchCreate()
 *****************************************************************************
 *//*!
 * @brief
 * This is the Create dispatch for the filter.  It creates the CControlFilter 
 * and associates it with the KS filter via the bag.
 * @param
 * KsFilter Pointer to the KSFILTER structure representing the AVStream
 * filter.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CControlFilter::
DispatchCreate 
(
    IN		PKSFILTER	KsFilter,
	IN		PIRP		Irp
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CControlFilter::DispatchCreate]"));

	NTSTATUS ntStatus;

	CControlFilter * Filter = new(NonPagedPool,'tSvA') CControlFilter(NULL);

	if (Filter)
	{
		Filter->AddRef();

		ntStatus = Filter->Init(KsFilter);

		if (NT_SUCCESS(ntStatus))
		{
			//
			// Add the item to the object bag if we were successful. Whenever the device goes 
			// away, the bag is cleaned up and we will be freed.
			//
			// For backwards compatibility with DirectX 8.0, we must grab the device mutex 
			// before doing this.  For Windows XP, this is not required, but it is still safe.
			//
			KsFilterAcquireControl(KsFilter);

			ntStatus = KsAddItemToObjectBag(KsFilter->Bag, Filter, (PFNKSFREE)CControlFilter::Destruct);

			KsFilterReleaseControl(KsFilter);
		}

		if (NT_SUCCESS(ntStatus)) 
		{
			// Keeping this object...
			Filter->AddRef();

			KsFilter->Context = PVOID(Filter);
		}

		// Release the private reference.
		Filter->Release();
	}
	else
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

/*****************************************************************************
 * CControlFilter::Destruct()
 *****************************************************************************
 *//*!
 * @brief
 * This is the free callback for the bagged filter.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CControlFilter object.
 * @return
 * None.
 */
VOID
CControlFilter::
Destruct 
(
	IN		PVOID	Self
)
{
    PAGED_CODE();

	CControlFilter * Filter = (CControlFilter *)(Self);

	Filter->Release();
}

/*****************************************************************************
 * CControlFilter::NonDelegatingQueryInterface()
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
CControlFilter::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CControlFilter::NonDelegatingQueryInterface]"));

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
 * CControlFilter::~CControlFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CControlFilter::
~CControlFilter
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[~CControlFilter::~CControlFilter]"));

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
 * CControlFilter::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initializes the miniport.
 * @details
 * The caller of @b Init should run at IRQL_PASSIVE_LEVEL.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CControlFilter::
Init
(
    IN		PKSFILTER	KsFilter
)
{
    PAGED_CODE();

    ASSERT(KsFilter);

    _DbgPrintF(DEBUGLVL_BLAB,("[CControlFilter::Init]"));

	m_KsFilter = KsFilter;

	m_KsAdapter = PKSADAPTER(KsFilterGetDevice(KsFilter)->Context);
	m_KsAdapter->AddRef();

	m_UsbDevice = m_KsAdapter->GetUsbDevice();
	m_UsbDevice->AddRef();

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CControlFilter::GetPropertyHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Device control property handler.
 * @details
 * This routine gets called whenever this filter gets a property
 * request with KSPROSETPID_DeviceControl and a property set value. It is not
 * a node property but a filter property (you don't have to specify a node).
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS 
CControlFilter::
GetPropertyHandler
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CControlFilter::GetPropertyHandler]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PVOID Instance = PVOID(Request+1);

	ULONG InstanceSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSPROPERTY);

	CControlFilter * that = (CControlFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    switch (Request->Id)
	{
		case KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_DEVICE_DESCRIPTOR))
			{
				PUSB_DEVICE_DESCRIPTOR Descriptor = NULL;

				ntStatus = that->m_UsbDevice->GetDeviceDescriptor(&Descriptor);

				if (NT_SUCCESS(ntStatus))
				{
					RtlCopyMemory(Value, Descriptor, sizeof(USB_DEVICE_DESCRIPTOR));
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_DEVICE_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CONFIGURATION_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_CONFIGURATION_DESCRIPTOR))
			{
				PUSB_CONFIGURATION_DESCRIPTOR Descriptor = NULL;

				ntStatus = that->m_UsbDevice->GetConfigurationDescriptor(&Descriptor);

				if (NT_SUCCESS(ntStatus))
				{
        			// validate and get the output parameter
					if (ValueSize >= Descriptor->wTotalLength)
					{
						RtlCopyMemory(Value, Descriptor, Descriptor->wTotalLength);

						ValueSize = Descriptor->wTotalLength;

					}
					else if (ValueSize >= Descriptor->bLength)
					{
						RtlCopyMemory(Value, Descriptor, Descriptor->bLength);

						ValueSize = Descriptor->bLength;
					}
					else
					{
						ValueSize = Descriptor->wTotalLength;

						ntStatus = STATUS_BUFFER_TOO_SMALL;
					}
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_CONFIGURATION_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_INTERFACE_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_INTERFACE_DESCRIPTOR))
			{
				if (InstanceSize >= sizeof(INTERFACE_PARAMETERS))
				{
					PINTERFACE_PARAMETERS Parameters = PINTERFACE_PARAMETERS(Instance);

					PUSB_INTERFACE_DESCRIPTOR Descriptor = NULL;

					ntStatus = that->m_UsbDevice->GetInterfaceDescriptor
								(
									Parameters->InterfaceNumber,
									Parameters->AlternateSetting,
									Parameters->InterfaceClass,
									Parameters->InterfaceSubClass,
									Parameters->InterfaceProtocol,
									&Descriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						RtlCopyMemory(Value, Descriptor, sizeof(USB_INTERFACE_DESCRIPTOR));
					}
				}
				else
				{
					ntStatus = STATUS_INVALID_PARAMETER;
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_INTERFACE_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_ENDPOINT_DESCRIPTOR:
		{
        	// validate and get the output parameter
			if (ValueSize >= sizeof(USB_ENDPOINT_DESCRIPTOR))
			{
				if (InstanceSize >= sizeof(ENDPOINT_PARAMETERS))
				{
					PENDPOINT_PARAMETERS Parameters = PENDPOINT_PARAMETERS(Instance);

					PUSB_ENDPOINT_DESCRIPTOR Descriptor = NULL;

					ntStatus = that->m_UsbDevice->GetEndpointDescriptor
								(
									Parameters->InterfaceNumber,
									Parameters->AlternateSetting,
									Parameters->EndpointIndex,
									&Descriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						RtlCopyMemory(Value, Descriptor, sizeof(USB_ENDPOINT_DESCRIPTOR));
					}
				}
				else
				{
					ntStatus = STATUS_INVALID_PARAMETER;
				}
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}

			ValueSize = sizeof(USB_ENDPOINT_DESCRIPTOR);
		}
		break;

		case KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR:
		{
			if (InstanceSize >= sizeof(STRING_PARAMETERS))
			{
				PSTRING_PARAMETERS Parameters = PSTRING_PARAMETERS(Instance);

   				// validate and get the output parameter
				if (ValueSize >= sizeof(USB_STRING_DESCRIPTOR))
				{
					PUSB_STRING_DESCRIPTOR Descriptor = PUSB_STRING_DESCRIPTOR(Value);

					RtlZeroMemory(Descriptor, ValueSize);

					Descriptor->bLength = (ValueSize <= 254) ? UCHAR(ValueSize) : 254;
					Descriptor->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

					ntStatus = that->m_UsbDevice->GetStringDescriptor
								(
									Parameters->Index,
									Parameters->LanguageId,
									Descriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						ValueSize = Descriptor->bLength;
					}
					else if (ntStatus == STATUS_BUFFER_TOO_SMALL)
					{
						ValueSize = 254; // Maximum string descriptor size.
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR:
		{
			if (InstanceSize >= sizeof(CLASS_INTERFACE_PARAMETERS))
			{
				PCLASS_INTERFACE_PARAMETERS Parameters = PCLASS_INTERFACE_PARAMETERS(Instance);

				PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

				ntStatus = that->m_UsbDevice->GetInterfaceDescriptor
							(
								Parameters->InterfaceNumber,
								Parameters->AlternateSetting,
								-1, -1, -1,
								&InterfaceDescriptor
							);

				if (NT_SUCCESS(ntStatus))
				{
					PUSB_INTERFACE_DESCRIPTOR CsInterfaceDescriptor = NULL;

					ntStatus = that->m_UsbDevice->GetClassInterfaceDescriptor
								(
									Parameters->InterfaceNumber,
									Parameters->AlternateSetting,
									Parameters->ClassSpecificDescriptorType,
									&CsInterfaceDescriptor
								);

					if (NT_SUCCESS(ntStatus))
					{
						if (CsInterfaceDescriptor->bDescriptorType == USB_AUDIO_CS_INTERFACE)
						{
							switch (InterfaceDescriptor->bInterfaceSubClass)
							{
								case USB_AUDIO_SUBCLASS_AUDIOCONTROL:
								{
									PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR(CsInterfaceDescriptor);

        							// validate and get the output parameter
									if (ValueSize >= CsAcInterfaceDescriptor->wTotalLength)
									{
										RtlCopyMemory(Value, CsAcInterfaceDescriptor, CsAcInterfaceDescriptor->wTotalLength);

										ValueSize = CsAcInterfaceDescriptor->wTotalLength;
									}
									else if (ValueSize >= CsAcInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsAcInterfaceDescriptor, CsAcInterfaceDescriptor->bLength);

										ValueSize = CsAcInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsAcInterfaceDescriptor->wTotalLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;

								case USB_AUDIO_SUBCLASS_AUDIOSTREAMING:
								{
									PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR(CsInterfaceDescriptor);

        							// validate and get the output parameter
									if (ValueSize >= CsAsInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsAsInterfaceDescriptor, CsAsInterfaceDescriptor->bLength);

										ValueSize = CsAsInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsAsInterfaceDescriptor->bLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;

								case USB_AUDIO_SUBCLASS_MIDISTREAMING:
								{
									PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR CsMsInterfaceDescriptor = PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR(CsInterfaceDescriptor);

        							// validate and get the output parameter
									if (ValueSize >= CsMsInterfaceDescriptor->wTotalLength)
									{
										RtlCopyMemory(Value, CsMsInterfaceDescriptor, CsMsInterfaceDescriptor->wTotalLength);

										ValueSize = CsMsInterfaceDescriptor->wTotalLength;

									}
									else if (ValueSize >= CsMsInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsMsInterfaceDescriptor, CsMsInterfaceDescriptor->bLength);

										ValueSize = CsMsInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsMsInterfaceDescriptor->wTotalLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;

								default:
								{
        							// validate and get the output parameter
									if (ValueSize >= CsInterfaceDescriptor->bLength)
									{
										RtlCopyMemory(Value, CsInterfaceDescriptor, CsInterfaceDescriptor->bLength);

										ValueSize = CsInterfaceDescriptor->bLength;
									}
									else
									{
										ValueSize = CsInterfaceDescriptor->bLength;

										ntStatus = STATUS_BUFFER_TOO_SMALL;
									}
								}
								break;
							}
						}
						else
						{
							ntStatus = STATUS_INVALID_PARAMETER;
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR:
		{
			if (InstanceSize >= sizeof(CLASS_ENDPOINT_PARAMETERS))
			{
				PCLASS_ENDPOINT_PARAMETERS Parameters = PCLASS_ENDPOINT_PARAMETERS(Instance);

				PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

				ntStatus = that->m_UsbDevice->GetClassEndpointDescriptor
							(
								Parameters->InterfaceNumber,
								Parameters->AlternateSetting,
								Parameters->EndpointAddress,
								Parameters->ClassSpecificDescriptorType,
								&EndpointDescriptor
							);

				if (NT_SUCCESS(ntStatus))
				{
					if (EndpointDescriptor->bDescriptorType == USB_AUDIO_CS_ENDPOINT)
					{
						PUSB_ENDPOINT_DESCRIPTOR CsEndpointDescriptor = PUSB_ENDPOINT_DESCRIPTOR(EndpointDescriptor);

        				// validate and get the output parameter
						if (ValueSize >= CsEndpointDescriptor->bLength)
						{
							RtlCopyMemory(Value, CsEndpointDescriptor, CsEndpointDescriptor->bLength);
						}
						else
						{
							ntStatus = STATUS_BUFFER_TOO_SMALL;
						}

						ValueSize = CsEndpointDescriptor->bLength;
					}
					else
					{
						ntStatus = STATUS_INVALID_PARAMETER;
					}
				}
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND:
		{
			if (InstanceSize >= sizeof(CUSTOM_COMMAND_PARAMETERS))
			{
				PCUSTOM_COMMAND_PARAMETERS Parameters = PCUSTOM_COMMAND_PARAMETERS(Instance);

				ntStatus = that->m_UsbDevice->CustomCommand
							(
								Parameters->RequestType,
								Parameters->Request,
								Parameters->Value,
								Parameters->Index,
								Value,
								ValueSize,
								&ValueSize,
								TRUE
							);
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;
    }

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CControlFilter::SetPropertyHandler()
 *****************************************************************************
 *//*!
 * @brief
 * Device control property handler.
 * @details
 * This routine gets called whenever this filter gets a property
 * request with KSPROSETPID_DeviceControl and a property set value. It is not
 * a node property but a filter property (you don't have to specify a node).
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS 
CControlFilter::
SetPropertyHandler
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CControlFilter::SetPropertyHandler]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PVOID Instance = PVOID(Request+1);

	ULONG InstanceSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSPROPERTY);

	CControlFilter * that = (CControlFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    switch (Request->Id)
	{
		case KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND:
		{
			if (InstanceSize >= sizeof(CUSTOM_COMMAND_PARAMETERS))
			{
				PCUSTOM_COMMAND_PARAMETERS Parameters = PCUSTOM_COMMAND_PARAMETERS(Instance);

				ntStatus = that->m_UsbDevice->CustomCommand
							(
								Parameters->RequestType,
								Parameters->Request,
								Parameters->Value,
								Parameters->Index,
								Parameters->Buffer,
								Parameters->BufferLength,
								NULL,
								FALSE
							);
			}
			else
			{
				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		break;

		case KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK:
		{
			ntStatus = that->m_KsAdapter->SetFirmwareUpgradeLock(TRUE);
        }
		break;

		case KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK:
		{
			ntStatus = that->m_KsAdapter->SetFirmwareUpgradeLock(FALSE);
        }
		break;
	}

    return ntStatus;
}

#pragma code_seg()


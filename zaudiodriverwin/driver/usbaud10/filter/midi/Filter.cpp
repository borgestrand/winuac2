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
 * @brief      Audio filter implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "Filter.h"
#include "Factory.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "MIDI_FILTER: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiFilter::DispatchTable
 *****************************************************************************
 *//*!
 * @brief
 * This is the dispatch table for the filter.  It provides notification
 * of creation, closure, processing for the filter.
 */
KSFILTER_DISPATCH 
CMidiFilter::DispatchTable = 
{
    CMidiFilter::DispatchCreate,		// Filter Create
    NULL,								// Filter Close
    NULL,                               // Filter Process
    NULL                                // Filter Reset
};

/*****************************************************************************
 * CMidiFilter::DispatchCreate()
 *****************************************************************************
 *//*!
 * @brief
 * This is the Create dispatch for the filter.  It creates the CMidiFilter 
 * and associates it with the KS filter via the bag.
 * @param
 * KsFilter Pointer to the KSFILTER structure representing the AVStream
 * filter.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CMidiFilter::
DispatchCreate 
(
    IN		PKSFILTER	KsFilter,
	IN		PIRP		Irp
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CMidiFilter::DispatchCreate]"));

	NTSTATUS ntStatus;

	CMidiFilter * Filter = new(NonPagedPool,'aChS') CMidiFilter(NULL);

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

			ntStatus = KsAddItemToObjectBag(KsFilter->Bag, Filter, (PFNKSFREE)CMidiFilter::Destruct);

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

	_DbgPrintF(DEBUGLVL_BLAB,("[CMidiFilter::DispatchCreate] - ********************* Filter: %p", Filter));

	return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::Destruct()
 *****************************************************************************
 *//*!
 * @brief
 * This is the free callback for the bagged filter.  Not providing
 * one will call ExFreePool, which is not what we want for a constructed
 * C++ object.
 * @param
 * Self Pointer to the CMidiFilter object.
 * @return
 * None.
 */
VOID
CMidiFilter::
Destruct 
(
	IN		PVOID	Self
)
{
    PAGED_CODE();

	CMidiFilter * Filter = (CMidiFilter *)(Self);

	Filter->Release();
}

/*****************************************************************************
 * CMidiFilter::NonDelegatingQueryInterface()
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
CMidiFilter::
NonDelegatingQueryInterface
(
    IN      REFIID  Interface,
    OUT     PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::NonDelegatingQueryInterface]"));

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
 * CMidiFilter::~CMidiFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CMidiFilter::
~CMidiFilter
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::~CMidiFilter]"));

    if (m_MidiDevice)
    {
        m_MidiDevice->Release();
    }

	// clean up UsbDevice
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
 * CMidiFilter::Init()
 *****************************************************************************
 *//*!
 * @brief
 * Initializes a the filter.
 * @details
 * The caller of @b Init should run at IRQL_PASSIVE_LEVEL.
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise, the method
 * returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
Init
(
    IN		PKSFILTER	KsFilter
)
{
    PAGED_CODE();

    ASSERT(KsFilter);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::Init]"));

	m_KsFilter = KsFilter;

	m_FilterFactory = (CMidiFilterFactory *)(KsFilter->Context); // only on init.

	m_KsAdapter = PKSADAPTER(KsFilterGetDevice(KsFilter)->Context);
	m_KsAdapter->AddRef();

	m_UsbDevice = m_KsAdapter->GetUsbDevice();
	m_UsbDevice->AddRef();

	m_MidiDevice = m_KsAdapter->GetMidiDevice();
	m_MidiDevice->AddRef();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	if (!NT_SUCCESS(ntStatus))
    {
        //
        // clean up our mess
        //

		// clean up AudioDevice
		if (m_MidiDevice)
		{
			m_MidiDevice->Release();
			m_MidiDevice = NULL;
		}

		// clean up UsbDevice
		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}

		// clean up KsAdapter
        if (m_KsAdapter)
        {
            m_KsAdapter->Release();
            m_KsAdapter = NULL;
        }
    }

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetDescription()
 *****************************************************************************
 *//*!
 * @brief
 * Gets the filter description.
 * @details
 * Gets a pointer to a filter description. It provides a location
 * to deposit a pointer in filter's description structure. This is the
 * placeholder for the FromNode or ToNode fields in connections which
 * describe connections to the filter's pins.
 * @param
 * OutKsFilterDescriptor Pointer to the filter description.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS 
CMidiFilter::
GetDescription
(
	OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutKsFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetDescription]"));

	NTSTATUS ntStatus = m_FilterFactory->GetFilterDescription(OutKsFilterDescriptor);

	return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::ValidateFormat()
 *****************************************************************************
 *//*!
 * @brief
 * Validates a MIDI format.
 * @details
 * Match the specified @em Format against the supported format.
 * @param
 * PinId Identifier of the pin.
 * @param
 * Capture TRUE for an input (capture) channel, and FALSE for an output
 * (playback) channel.
 * @param
 * Format Pointer to a KSDATAFORMAT structure indicating the format to match.
 * @return
 * Returns STATUS_SUCCESS if the specified @em Format is supported.
 * Otherwise, returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
ValidateFormat
(
    IN      ULONG           PinId,
    IN      BOOLEAN         Capture,
    IN      PKSDATAFORMAT   Format
)
{
    PAGED_CODE();

    ASSERT(Format);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::ValidateFormat]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

	return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::FindPin()
 *****************************************************************************
 *//*!
 * @brief
 */
PFILTER_PIN_DESCRIPTOR 
CMidiFilter::
FindPin
(
	IN		ULONG	PinId
)
{
	return m_FilterFactory->FindPin(PinId);
}

/*****************************************************************************
 * CMidiFilter::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
PNODE_DESCRIPTOR 
CMidiFilter::
FindNode
(
	IN		ULONG	NodeId
)
{
	return m_FilterFactory->FindNode(NodeId);
}

#include <stdio.h>
/*****************************************************************************
 * CMidiFilter::FindPinName()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
FindPinName
(
	IN		ULONG		PinId,
	IN OUT	PVOID		Buffer,
	IN		ULONG		BufferSize,
	OUT		ULONG *		OutPinNameLength	OPTIONAL
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::FindPinName]"));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = FindPin(PinId);

	if (Pin)
	{
		struct
		{
			UCHAR	bLength;
			UCHAR	bDescriptorType;
			WCHAR	bString[126];
		} ProductNameDescriptor;

		RtlZeroMemory(&ProductNameDescriptor, sizeof(ProductNameDescriptor));

		ProductNameDescriptor.bLength = sizeof(ProductNameDescriptor);

		struct
		{
			UCHAR	bLength;
			UCHAR	bDescriptorType;
			WCHAR	bString[32];
		} JackNameDescriptor;

		RtlZeroMemory(&JackNameDescriptor, sizeof(JackNameDescriptor));

		JackNameDescriptor.bLength = sizeof(JackNameDescriptor);

		USHORT LanguageId = m_KsAdapter->GetLanguageId();

		// Find the product name, if available.
		UCHAR iProduct = 0;

		PUSB_DEVICE_DESCRIPTOR DeviceDescriptor = NULL;

		if (NT_SUCCESS(m_UsbDevice->GetDeviceDescriptor(&DeviceDescriptor)))
		{
			iProduct = DeviceDescriptor->iProduct;

			if (iProduct)
			{
				m_UsbDevice->GetStringDescriptor(iProduct, LanguageId, PUSB_STRING_DESCRIPTOR(&ProductNameDescriptor));
			}
		}

		// Find the jack name, if available.
		UCHAR iJack = Pin->iJack();

		if (iJack)
		{
			m_UsbDevice->GetStringDescriptor(iJack, LanguageId, PUSB_STRING_DESCRIPTOR(&JackNameDescriptor));
		}

		// Determine if there is a possibility that there is a clash of names.
		// Add an index if it happened.
		ULONG iIndex = 0;

		if (iProduct && !iJack)
		{
			KSPIN_DESCRIPTOR_EX Descriptor; Pin->GetDescriptor(&Descriptor);

			if (Descriptor.PinDescriptor.DataRangesCount)
			{
				PKSDATARANGE * DataRanges = (PKSDATARANGE*)Descriptor.PinDescriptor.DataRanges;

				if (DataRanges)
				{
					PKSDATARANGE_MUSIC_EX DataRangeMusicEx = PKSDATARANGE_MUSIC_EX(DataRanges[0]);

					iIndex = DataRangeMusicEx->CableNumber;
				}
			}
		}

		// Determine the pin name length.
		ULONG PinNameLength = 0;

		UCHAR JackType = Pin->JackType();

		if (JackType == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
		{
			if (iProduct && iJack)
			{
				PinNameLength = ProductNameDescriptor.bLength + JackNameDescriptor.bLength;
			}
			else if (iProduct)
			{
				PinNameLength = ProductNameDescriptor.bLength;

				if (iIndex)
				{
					PinNameLength += wcslen(L" []") + (((iIndex+1)/10)+1)*2 + sizeof(WCHAR);
				}
			}
			else if (iJack)
			{
				PinNameLength = JackNameDescriptor.bLength;
			}
		}
		else
		{
			if (iJack)
			{
				PinNameLength = JackNameDescriptor.bLength;
			}
		}

		// If there is a pin name available for use...
		if (PinNameLength)
		{
			if (BufferSize >= PinNameLength)
			{
				PWCHAR PinName = PWCHAR(Buffer);

				if (JackType == USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED)
				{
					if (iProduct && iJack)
					{
						wcscpy(PinName, ProductNameDescriptor.bString);
						wcscat(PinName, L" ");
						wcscat(PinName, JackNameDescriptor.bString);
					}
					else if (iProduct)
					{
						if (iIndex)
						{
							swprintf(PinName, L"%s [%d]", ProductNameDescriptor.bString, iIndex+1);
						}
						else
						{
							wcscpy(PinName, ProductNameDescriptor.bString);
						}
					}
					else if (iJack)
					{
						wcscpy(PinName, JackNameDescriptor.bString);
					}
				}
				else
				{
					if (iJack)
					{
						wcscpy(PinName, JackNameDescriptor.bString);
					}
				}
				
				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_BUFFER_OVERFLOW;
			}

			if (OutPinNameLength)
			{
				*OutPinNameLength = PinNameLength;
			}
		}
		else
		{
			ntStatus = STATUS_NOT_FOUND;
		}
	}

    return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiFilter::PinPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter pin properties.
 */
DEFINE_KSPROPERTY_TABLE(CMidiFilter::PinPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_PIN_NAME,			// Id
		CMidiFilter::GetPinName,		// GetPropertyHandler or GetSupported
		sizeof(KSP_PIN),				// MinProperty
		0,								// MinData
		NULL,							// SetPropertyHandler or SetSupported
		NULL,							// Values
		0,								// RelationsCount
		NULL,							// Relations
		CMidiFilter::SupportPinName,	// SupportHandler
		0								// SerializedSize
	)
};	

/*****************************************************************************
 * CMidiFilter::ControlPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter properties.
 */
DEFINE_KSPROPERTY_TABLE(CMidiFilter::ControlPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR,			// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_DEVICE_DESCRIPTOR),			// MinProperty
		sizeof(USB_DEVICE_DESCRIPTOR),						// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CONFIGURATION_DESCRIPTOR,	// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CONFIGURATION_DESCRIPTOR),		// MinProperty
		sizeof(USB_CONFIGURATION_DESCRIPTOR),				// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_INTERFACE_DESCRIPTOR,		// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_INTERFACE_DESCRIPTOR),			// MinProperty
		sizeof(USB_INTERFACE_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_ENDPOINT_DESCRIPTOR,		// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_ENDPOINT_DESCRIPTOR),			// MinProperty
		sizeof(USB_ENDPOINT_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR,			// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_STRING_DESCRIPTOR),			// MinProperty
		sizeof(USB_STRING_DESCRIPTOR),						// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR,// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR),	// MinProperty
		sizeof(USB_INTERFACE_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR,	// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR),	// MinProperty
		sizeof(USB_ENDPOINT_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND,			// Id
		CMidiFilter::GetDeviceControl,						// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CUSTOM_COMMAND),				// MinProperty
		0,													// MinData
		CMidiFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK,		// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		0,													// MinData
		CMidiFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK,	// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		0,													// MinData
		CMidiFilter::SetDeviceControl,						// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	)
};	

#ifdef ENABLE_DIRECTMUSIC_SUPPORT
/*****************************************************************************
 * CMidiFilter::SynthClockPropertyTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Synth clock property items.
 */
DEFINE_KSPROPERTY_TABLE(CMidiFilter::SynthClockPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_SYNTH_MASTERCLOCK,			// Id
		CMidiFilter::GetSynthMasterClock,		// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),						// MinProperty
		sizeof(ULONGLONG),						// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CMidiFilter::SupportSynthMasterClock,	// SupportHandler
		0										// SerializedSize
	)
};
#endif // ENABLE_DIRECTMUSIC_SUPPORT

/*****************************************************************************
 * CMidiFilter::PropertySetTable[]
 *****************************************************************************
 *//*!
 * @brief
 * Filter property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(CMidiFilter::PropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Pin,								// Set
		SIZEOF_ARRAY(CMidiFilter::PinPropertyTable),	// PropertiesCount
		CMidiFilter::PinPropertyTable,					// PropertyItem
		0,												// FastIoCount
		NULL											// FastIoTable
	),
	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_SynthClock,							// Set
		SIZEOF_ARRAY(CMidiFilter::SynthClockPropertyTable),	// PropertiesCount
		CMidiFilter::SynthClockPropertyTable,				// PropertyItem
		0,													// FastIoCount
		NULL												// FastIoTable
	),
	#endif // ENABLE_DIRECTMUSIC_SUPPORT
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_DeviceControl,							// Set
		SIZEOF_ARRAY(CMidiFilter::ControlPropertyTable),	// PropertiesCount
		CMidiFilter::ControlPropertyTable,					// PropertyItem
		0,													// FastIoCount
		NULL												// FastIoTable
	)
};

/*****************************************************************************
 * CMidiFilter::AutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Filter automation table.
 */
DEFINE_KSAUTOMATION_TABLE(CMidiFilter::AutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(CMidiFilter::PropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

/*****************************************************************************
 * CMidiFilter::Categories[]
 *****************************************************************************
 *//*!
 * @brief
 * List of filter categories.
 */
GUID 
CMidiFilter::Categories[4] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_CAPTURE),
    STATICGUIDOF(KSCATEGORY_MIDICONTROL)
};

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiFilter::SupportPinName()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
SupportPinName
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::SupportPinName]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PFILTER_PIN_DESCRIPTOR Pin = MidiFilter->FindPin(Request->PinId);

 	if (Pin)
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
										     KSPROPERTY_TYPE_GET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_LPWSTR;
			Description->PropTypeSet.Flags = 0;
			Description->MembersListCount  = 0;
			Description->Reserved          = 0;

			// set the return value size
			ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

			ntStatus = STATUS_SUCCESS;
		}
		else if (ValueSize >= sizeof(ULONG))
		{
			// if return buffer can hold a ULONG, return the access flags
			PULONG AccessFlags = PULONG(Value);

			*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
						   KSPROPERTY_TYPE_GET;

			// set the return value size
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_SUCCESS;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetPinName()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetPinName
(
	IN		PIRP		Irp,
	IN		PKSP_PIN	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetPinName]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = MidiFilter->FindPinName(Request->PinId, Value, ValueSize, &ValueSize);

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetDeviceControl()
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
CMidiFilter::
GetDeviceControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetDeviceControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PVOID Instance = PVOID(Request+1);

	ULONG InstanceSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSPROPERTY);

	CMidiFilter * that = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

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
 * CMidiFilter::SetDeviceControl()
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
CMidiFilter::
SetDeviceControl
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::SetDeviceControl]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PVOID Instance = PVOID(Request+1);

	ULONG InstanceSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSPROPERTY);

	CMidiFilter * that = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

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

/*****************************************************************************
 * CMidiFilter::SupportMidiElementCapability()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
SupportMidiElementCapability
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::SupportMidiElementCapability]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= sizeof(ULONG))
		{
			// if return buffer can hold a ULONG, return the access flags
			PULONG AccessFlags = PULONG(Value);

			*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
						   KSPROPERTY_TYPE_GET;

			// set the return value size
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_SUCCESS;
		}
		else
		{
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetMidiElementCapability()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetMidiElementCapability
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetMidiElementCapability]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
        if (ValueSize >= sizeof(BOOL))
        {
            PBOOL Supported = PBOOL(Value);

			if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_GM_Hardware))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_GM1) ||
							 Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_GM2);
			}
			else if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_GS_Hardware))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_GS);
			}
			else if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_XG_Hardware))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_XG);
			}
			else if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_XG_Capable))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_XG);
			}
			else if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_GS_Capable))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_GS);
			}
			else if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_DLS1))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_DLS1);
			}
			else if (IsEqualGUIDAligned(Request->Property.Set, GUID_DMUS_PROP_DLS2))
			{
				*Supported = Node->SupportCapability(USB_AUDIO_MIDI_EL_CAPABILITY_DLS2);
			}
			else
			{
				*Supported = FALSE;
			}

			ntStatus = STATUS_SUCCESS;
        } 
		else
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

		ValueSize = sizeof(BOOL);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

#ifdef ENABLE_DIRECTMUSIC_SUPPORT
/*****************************************************************************
 * CMidiFilter::GetSynthCaps()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetSynthCaps
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetSynthCaps]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= sizeof(SYNTHCAPS))
		{
			PSYNTHCAPS SynthCaps = PSYNTHCAPS(Value);

			// Guid.
			switch (Node->Jack()->DescriptorSubtype())
			{
				case USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK:
				{
					SynthCaps->Guid = CLSID_MiniportDriverDMusUARTCapture;
				}
				break;

				case USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK:
				{
					SynthCaps->Guid = CLSID_MiniportDriverDMusUART;
				}
				break;

				default:
				{
					SynthCaps->Guid = GUID_NULL;
				}
				break;
			}

			// Description.
			UCHAR JackID = Node->JackID();

			for (ULONG PinId=0; ; PinId++)
			{
				PFILTER_PIN_DESCRIPTOR Pin = MidiFilter->FindPin(PinId);

				if (Pin)
				{
					if (Pin->JackID() == JackID)
					{
						MidiFilter->FindPinName(PinId, SynthCaps->Description, sizeof(SynthCaps->Description), NULL);
						break;
					}
				}
				else
				{
					break;
				}
			}

			//Flags.
			SynthCaps->Flags = SYNTH_PC_EXTERNAL;
			//MemorySize.
			SynthCaps->MemorySize = 0;
			//MaxChannelGroups
			SynthCaps->MaxChannelGroups = 1;
			//MaxVoices
			SynthCaps->MaxVoices = 0xFFFFFFFF;
			//MaxAudioChannels
			SynthCaps->MaxAudioChannels = 0xFFFFFFFF;
			//EffectFlags
			SynthCaps->EffectFlags = SYNTH_EFFECT_NONE;

			ntStatus = STATUS_SUCCESS;
		} 
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(SYNTHCAPS);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetSynthPortParameters()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetSynthPortParameters
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetSynthPortParameters]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= sizeof(SYNTH_PORTPARAMS))
		{
			PSYNTH_PORTPARAMS Params = PSYNTH_PORTPARAMS(Value);

			RtlCopyMemory(Params, Request+1, sizeof(SYNTH_PORTPARAMS));

			if (Params->ValidParams & ~SYNTH_PORTPARAMS_CHANNELGROUPS)
			{
				Params->ValidParams &= SYNTH_PORTPARAMS_CHANNELGROUPS;
			}

			if (!(Params->ValidParams & SYNTH_PORTPARAMS_CHANNELGROUPS))
			{
				Params->ChannelGroups = 1;
			}
			else if (Params->ChannelGroups != 1)
			{
				Params->ChannelGroups = 1;
			}

			ntStatus = STATUS_SUCCESS;
		} 
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(SYNTH_PORTPARAMS);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetSynthChannelGroups()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetSynthChannelGroups
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetSynthChannelGroups]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= sizeof(ULONG))
		{
			PULONG ChannelGroups = PULONG(Value);

			*ChannelGroups = 1;

			ntStatus = STATUS_SUCCESS;
		} 
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(ULONG);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::SetSynthChannelGroups()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
SetSynthChannelGroups
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::SetSynthChannelGroups]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= sizeof(ULONG))
		{
			ntStatus = STATUS_SUCCESS;
		} 
	}

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetSynthLatencyClock()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetSynthLatencyClock
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetSynthLatencyClock]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= sizeof(ULONGLONG))
		{
			PULONGLONG Latency = PULONGLONG(Value);

			LARGE_INTEGER PerformanceFrequency; 
			LARGE_INTEGER PerformanceCounter = KeQueryPerformanceCounter(&PerformanceFrequency);

			// Calculate the presentation time in 100ns.
			LONGLONG PresentationTime = LONGLONG(DOUBLE(PerformanceCounter.QuadPart) * 10000000 / DOUBLE(PerformanceFrequency.QuadPart));

			// Set the latency to 1 MIDI byte -> 1/31250 * 10bits = 320us => 3200 * 100ns.
			*Latency = PresentationTime + 3200;

			ntStatus = STATUS_SUCCESS;
		} 
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(ULONGLONG);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::SupportSynthMasterClock()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
SupportSynthMasterClock
(
	IN		PIRP			Irp,
	IN		PKSPROPERTY		Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::SupportSynthMasterClock]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (MidiFilter->m_FilterFactory->IsDirectMusicCapable())
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
											KSPROPERTY_TYPE_GET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_I8;
			Description->PropTypeSet.Flags = 0;
			Description->MembersListCount  = 0;
			Description->Reserved          = 0;

			// set the return value size
			ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

			ntStatus = STATUS_SUCCESS;
		}
		else if (ValueSize >= sizeof(ULONG))
		{
			// if return buffer can hold a ULONG, return the access flags
			PULONG AccessFlags = PULONG(Value);

			*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
						KSPROPERTY_TYPE_GET;

			// set the return value size
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_SUCCESS;
		}
	}
	else
	{
		ntStatus = STATUS_NOT_SUPPORTED;
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetSynthMasterClock()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS 
CMidiFilter::
GetSynthMasterClock
(
	IN		PIRP		Irp,
	IN		PKSPROPERTY	Request,
	IN OUT	PVOID		Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    //_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetSynthMasterClock]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if (MidiFilter->m_FilterFactory->IsDirectMusicCapable())
	{
		// validate and get the output parameter
		if (ValueSize >= sizeof(ULONGLONG))
		{
			PULONGLONG MasterClockTime = PULONGLONG(Value);

			LARGE_INTEGER PerformanceFrequency; 
			LARGE_INTEGER PerformanceCounter = KeQueryPerformanceCounter(&PerformanceFrequency);

			// Calculate the master clock time in 100ns.
			*MasterClockTime = LONGLONG(DOUBLE(PerformanceCounter.QuadPart) * 10000000 / DOUBLE(PerformanceFrequency.QuadPart));

			ntStatus = STATUS_SUCCESS;
		}
		else
		{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
		}

		ValueSize = sizeof(ULONGLONG);
	}
	else
	{
		ntStatus = STATUS_NOT_SUPPORTED;
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}
#endif // ENABLE_DIRECTMUSIC_SUPPORT

/*****************************************************************************
 * CMidiFilter::SupportCpuResources()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
SupportCpuResources
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::SupportCpuResources]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
		if (ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
		{
			// if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
			PKSPROPERTY_DESCRIPTION Description = PKSPROPERTY_DESCRIPTION(Value);

			Description->AccessFlags       = KSPROPERTY_TYPE_BASICSUPPORT |
										     KSPROPERTY_TYPE_GET;
			Description->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION);
			Description->PropTypeSet.Set   = KSPROPTYPESETID_General;
			Description->PropTypeSet.Id    = VT_I4;
			Description->PropTypeSet.Flags = 0;
			Description->MembersListCount  = 0;
			Description->Reserved          = 0;

			// set the return value size
			ValueSize = sizeof(KSPROPERTY_DESCRIPTION);

			ntStatus = STATUS_SUCCESS;
		}
		else if (ValueSize >= sizeof(ULONG))
		{
			// if return buffer can hold a ULONG, return the access flags
			PULONG AccessFlags = PULONG(Value);

			*AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
						   KSPROPERTY_TYPE_GET;

			// set the return value size
			ValueSize = sizeof(ULONG);

			ntStatus = STATUS_SUCCESS;
		}
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

/*****************************************************************************
 * CMidiFilter::GetCpuResources()
 *****************************************************************************
 *//*!
 * @brief
 * @return
 * Returns STATUS_SUCCESS if the call was successful. Otherwise,
 * the method returns an appropriate error code.
 */
NTSTATUS
CMidiFilter::
GetCpuResources
(
	IN		PIRP			Irp,
	IN		PKSNODEPROPERTY	Request,
	IN OUT	PVOID			Value
)
{
    PAGED_CODE();

    ASSERT(Request);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilter::GetCpuResources]"));

	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ValueSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	CMidiFilter * MidiFilter = (CMidiFilter*)(KsGetFilterFromIrp(Irp)->Context);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	PNODE_DESCRIPTOR Node = MidiFilter->FindNode(Request->NodeId);

 	if (Node)
	{
        if (ValueSize >= sizeof(ULONG))
        {
            *(PULONG(Value)) = KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU;

			ntStatus = STATUS_SUCCESS;
        } 
		else
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

		ValueSize = sizeof(ULONG);
	}

	Irp->IoStatus.Information = ULONG_PTR(ValueSize);

    return ntStatus;
}

#pragma code_seg()

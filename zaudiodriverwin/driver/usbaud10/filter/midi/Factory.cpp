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

/*! @brief Debug module name. */
#define STR_MODULENAME "MIDI_FILTER_FACTORY: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CreateMidiFilterFactory()
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
CreateMidiFilterFactory
(
    IN		PKSDEVICE	KsDevice,
	IN		PWCHAR		RefString,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CMidiFilterFactory::Instantiate]"));

	NTSTATUS ntStatus;

	CMidiFilterFactory * FilterFactory = new(NonPagedPool,'rCcP') CMidiFilterFactory(NULL);

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

			CMidiFilterFactory::SetupFriendlyName(KsAdapter, KsFilterDescriptor, RefString, Parameter1, Parameter2);

			KsAcquireDevice(KsDevice);

			PKSFILTERFACTORY KsFilterFactory = NULL;

			ntStatus = KsCreateFilterFactory
							(
								KsDevice->FunctionalDeviceObject, 
								KsFilterDescriptor, 
								RefString,
								NULL,
								0,
								CMidiFilterFactory::SleepCallback,
								CMidiFilterFactory::WakeCallback,
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

			ntStatus = KsAddItemToObjectBag(KsDevice->Bag, FilterFactory, (PFNKSFREE)CMidiFilterFactory::Destruct);

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

#include <stdio.h>
/*****************************************************************************
 * CMidiFilterFactory::SetupFriendlyName()
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
CMidiFilterFactory::
SetupFriendlyName
(
	IN		PKSADAPTER				KsAdapter,
	IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
	IN		PWCHAR					RefString,
	IN		PVOID					Parameter1,
	IN		PVOID					Parameter2
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterFactory::SetupFriendlyName]"));

	PUSB_DEVICE UsbDevice = KsAdapter->GetUsbDevice();

	PMIDI_TOPOLOGY MidiTopology = PMIDI_TOPOLOGY(Parameter1);

	PMIDI_CABLE MidiCable = PMIDI_CABLE(Parameter2);

	UCHAR iProduct = 0;

	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor = NULL;

	NTSTATUS ntStatus = UsbDevice->GetDeviceDescriptor(&DeviceDescriptor);

	if (NT_SUCCESS(ntStatus))
	{
		iProduct = DeviceDescriptor->iProduct;
	}

	if (iProduct)
	{
		struct
		{
			UCHAR	bLength;
			UCHAR	bDescriptorType;
			WCHAR	bString[126];
		} ProductNameDescriptor;

		RtlZeroMemory(&ProductNameDescriptor, sizeof(ProductNameDescriptor));

		ProductNameDescriptor.bLength = sizeof(ProductNameDescriptor);

		USHORT LanguageId = KsAdapter->GetLanguageId();

		UsbDevice->GetStringDescriptor(iProduct, LanguageId, PUSB_STRING_DESCRIPTOR(&ProductNameDescriptor));

		UCHAR iJack = 0, iIndex = 0;

		if (MidiCable)
		{
			UCHAR AssociatedJackID = MidiCable->AssociatedJackID();

			for (ULONG i=0; ; i++)
			{
				PMIDI_JACK Jack = NULL;

				if (MidiTopology->ParseJacks(i, &Jack))
				{
					if (Jack->JackID() == AssociatedJackID)
					{
						iJack = Jack->iJack();
						break;
					}
				}
				else
				{
					break;
				}
			}

			iIndex = MidiCable->CableNumber();
		}

		struct
		{
			UCHAR	bLength;
			UCHAR	bDescriptorType;
			WCHAR	bString[32];
		} JackNameDescriptor;

		RtlZeroMemory(&JackNameDescriptor, sizeof(JackNameDescriptor));

		JackNameDescriptor.bLength = sizeof(JackNameDescriptor);

		if (iJack)
		{
			UsbDevice->GetStringDescriptor(iJack, LanguageId, PUSB_STRING_DESCRIPTOR(&JackNameDescriptor));
		}

		WCHAR FriendlyName[256]; FriendlyName[0] = 0;

		if (iProduct && iJack)
		{
			wcscpy(FriendlyName, ProductNameDescriptor.bString);
			wcscat(FriendlyName, L" ");
			wcscat(FriendlyName, JackNameDescriptor.bString);
		}
		else if (iProduct)
		{
			if (iIndex)
			{
				swprintf(FriendlyName, L"%s [%d]", ProductNameDescriptor.bString, iIndex+1);
			}
			else
			{
				wcscpy(FriendlyName, ProductNameDescriptor.bString);
			}
		}

		for (ULONG i = 0; i < KsFilterDescriptor->CategoriesCount; i++)
		{
			KsAdapter->SetSubDeviceParameter(RefString, KsFilterDescriptor->Categories[i], L"FriendlyName", REG_SZ, FriendlyName, (wcslen(FriendlyName)+1)*sizeof(WCHAR));
		}
	}
}

/*****************************************************************************
 * CMidiFilterFactory::SleepCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CMidiFilterFactory::
SleepCallback 
(
	IN		PKSFILTERFACTORY	KsFilterFactory,
	IN		DEVICE_POWER_STATE	PowerState
)
{
	PAGED_CODE();

	CMidiFilterFactory * FilterFactory = (CMidiFilterFactory *)(KsFilterFactory->Context);

	FilterFactory->PowerChangeNotify(PowerState);
}

/*****************************************************************************
 * CMidiFilterFactory::WakeCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CMidiFilterFactory::
WakeCallback 
(
	IN		PKSFILTERFACTORY	KsFilterFactory,
	IN		DEVICE_POWER_STATE	PowerState
)
{
	PAGED_CODE();

	CMidiFilterFactory * FilterFactory = (CMidiFilterFactory *)(KsFilterFactory->Context);

	FilterFactory->PowerChangeNotify(PowerState);
}

/*****************************************************************************
 * CMidiFilterFactory::Destruct()
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
CMidiFilterFactory::
Destruct 
(
	IN		PVOID	Self
)
{
	CMidiFilterFactory * FilterFactory = (CMidiFilterFactory *)(Self);

	FilterFactory->Release();
}

/*****************************************************************************
 * CMidiFilterFactory::NonDelegatingQueryInterface()
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
CMidiFilterFactory::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CMidiFilterFactory::NonDelegatingQueryInterface]"));

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
 * CMidiFilterFactory::CMidiFilterFactory()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */

/*****************************************************************************
 * CMidiFilterFactory::~CMidiFilterFactory()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CMidiFilterFactory::
~CMidiFilterFactory
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[~CMidiFilterFactory::~CMidiFilterFactory]"));

	if (m_MidiDevice)
	{
		m_MidiDevice->Release();
	}

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
 * CMidiFilterFactory::Init()
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
CMidiFilterFactory::
Init
(
	IN		PKSDEVICE	KsDevice,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
)
{
    PAGED_CODE();

    ASSERT(KsDevice);

    _DbgPrintF(DEBUGLVL_BLAB,("[CMidiFilterFactory::Init]"));

	m_KsDevice = KsDevice;

	m_KsAdapter = PKSADAPTER(KsDevice->Context);
	m_KsAdapter->AddRef();

	m_UsbDevice = m_KsAdapter->GetUsbDevice();
	m_UsbDevice->AddRef();

	m_MidiDevice = m_KsAdapter->GetMidiDevice();
	m_MidiDevice->AddRef();

	m_MidiTopology = PMIDI_TOPOLOGY(Parameter1);

	m_MidiCable = PMIDI_CABLE(Parameter2);

	m_DevicePowerState = PowerDeviceD0;

	#ifdef ENABLE_DIRECTMUSIC_SUPPORT
	// Determine if this filter is capable of supporting DirectMusic.
	LONG DirectMusicCapable = 0; // Disabled by default.

	WCHAR ConfigurationFile[MAX_PATH] = {0};

	if (NT_SUCCESS(m_KsAdapter->RegistryReadFromDriverSubKey(L"Configuration", L"CFGFILE", ConfigurationFile, sizeof(ConfigurationFile), NULL, NULL)))
	{
		PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor; m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

		CHAR SectionName[64]; sprintf(SectionName, "USB\\VID_%04X&PID_%04X.%03X.Midi.DirectMusic", UsbDeviceDescriptor->idVendor, UsbDeviceDescriptor->idProduct, UsbDeviceDescriptor->bcdDevice);

		DirectMusicCapable = DrvProfileGetLong(SectionName, "Enable", -1, ConfigurationFile);

		if (DirectMusicCapable == -1)
		{
			sprintf(SectionName, "USB\\VID_%04X&PID_%04X.Midi.DirectMusic", UsbDeviceDescriptor->idVendor, UsbDeviceDescriptor->idProduct);

			DirectMusicCapable = DrvProfileGetLong(SectionName, "Enable", 0, ConfigurationFile);
		}
	}

	m_DirectMusicCapable = (DirectMusicCapable != 0);
	#endif // ENABLE_DIRECTMUSIC_SUPPORT

	// Figure out what the filter descriptor here...
	NTSTATUS ntStatus = m_MidiFilterDescriptor.Initialize(m_MidiDevice, m_MidiTopology, m_MidiCable, m_DirectMusicCapable);

	if (NT_SUCCESS(ntStatus))
	{
		m_KsFilterDescriptor.Dispatch				= &CMidiFilter::DispatchTable;
		m_KsFilterDescriptor.AutomationTable		= &CMidiFilter::AutomationTable;
		m_KsFilterDescriptor.Version				= KSFILTER_DESCRIPTOR_VERSION;
		m_KsFilterDescriptor.Flags					= 0;
		m_KsFilterDescriptor.ReferenceGuid			= NULL;
		m_KsFilterDescriptor.PinDescriptorsCount	= m_MidiFilterDescriptor.PinCount();
		m_KsFilterDescriptor.PinDescriptorSize      = sizeof(KSPIN_DESCRIPTOR_EX);
		m_KsFilterDescriptor.PinDescriptors         = m_MidiFilterDescriptor.Pins();
		m_KsFilterDescriptor.CategoriesCount		= m_MidiFilterDescriptor.CategoryCount();
		m_KsFilterDescriptor.Categories				= m_MidiFilterDescriptor.Categories();
		m_KsFilterDescriptor.NodeDescriptorsCount   = m_MidiFilterDescriptor.NodeCount();
		m_KsFilterDescriptor.NodeDescriptorSize     = sizeof(KSNODE_DESCRIPTOR);
		m_KsFilterDescriptor.NodeDescriptors        = m_MidiFilterDescriptor.Nodes();
		m_KsFilterDescriptor.ConnectionsCount		= m_MidiFilterDescriptor.ConnectionCount();
		m_KsFilterDescriptor.Connections			= m_MidiFilterDescriptor.Connections();
		m_KsFilterDescriptor.ComponentId			= m_MidiFilterDescriptor.ComponentId();
	}

	if (!NT_SUCCESS(ntStatus))
	{
		// Cleanup the mess...
		if (m_MidiDevice)
		{
			m_MidiDevice->Release();
			m_MidiDevice = NULL;
		}

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
 * CMidiFilterFactory::GetDescription()
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
CMidiFilterFactory::
GetFilterDescription
(
	OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutKsFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterFactory::GetDescription]"));

    *OutKsFilterDescriptor = PKSFILTER_DESCRIPTOR(&m_KsFilterDescriptor);

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CMidiFilterFactory::PowerChangeNotify()
 *****************************************************************************
 *//*!
 * @brief
 * Change power state for the device.
 * @details
 * The PortCls system driver calls the miniport's @b PowerChangeNotify method
 * to notify it of changes in the power state. The purpose of this call is
 * to give the miniport an opportunity to save any hardware-specific context
 * just before powering down or to restore a previously saved context just
 * after powering up.
 *
 * The miniport can write to the hardware registers or on-board memory during
 * the PowerChangeNotify call. If the system is powering down (making a state
 * transition away from PowerDeviceD0), the PortCls system driver calls
 * @b PowerChangeNotify before it calls IAdapterPowerManagement::PowerChangeState
 * and after it has paused any active audio data streams. This gives the
 * miniport an opportunity to save any hardware-specific device context
 * before the device powers down. For example, a WavePci miniport might
 * need to save its DMA registers if the power down occurs during a sequence
 * of scatter/gather data transfers. If the system is powering up (making a
 * state transition toward PowerDeviceD0), PortCls calls @b PowerChangeNotify
 * after it calls @b PowerChangeState and before it restarts any paused audio
 * data streams. This gives the miniport an opportunity to restore a
 * previously saved context after the device has powered up.
 *
 * The code for this method should reside in paged memory.
 * @param
 * NewPowerState Current power state. 
 * @return
 * None
 */
VOID
CMidiFilterFactory::
PowerChangeNotify
(
    IN      DEVICE_POWER_STATE	NewPowerState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiFilterFactory::PowerChangeNotify]"));

    if (m_DevicePowerState != NewPowerState)
    {
        switch (m_DevicePowerState)
        {
            case PowerDeviceD0:
            {
                switch (NewPowerState)
                {
                    case PowerDeviceD1:
                    case PowerDeviceD2:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D0->D1/D2"));
                        // Power State Transition
						m_MidiDevice->PowerStateChange(NewPowerState);
                        m_DevicePowerState = NewPowerState;
                        break;
                    case PowerDeviceD3:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D0->D3"));
                        // Power State Transition
						m_MidiDevice->PowerStateChange(NewPowerState);
                        m_DevicePowerState = NewPowerState;
                        break;
                    default:
                        break;
                }
            }
            break;
            case PowerDeviceD1:
            case PowerDeviceD2:
            {
                switch (NewPowerState)
                {
                    case PowerDeviceD0:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D1/D2->D0"));
                        // Power State Transition
						m_MidiDevice->PowerStateChange(NewPowerState);
                        m_DevicePowerState = NewPowerState;
                        break;
                    default:
                        break;
                }
            }
            break;
            case PowerDeviceD3:
            {
                switch (NewPowerState)
                {
                    case PowerDeviceD0:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D3->D0"));
                        // Power State Transition
						m_MidiDevice->PowerStateChange(NewPowerState);
                        m_DevicePowerState = NewPowerState;
                        break;
                    default:
                        break;
                }
            }
            break;
            default:
                _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : Unknown current device power state"));
                break;
        }
    }
}

/*****************************************************************************
 * CMidiFilterFactory::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
PFILTER_PIN_DESCRIPTOR 
CMidiFilterFactory::
FindPin
(
	IN		ULONG	PinId
)
{
	return m_MidiFilterDescriptor.FindFilterPin(PinId);
}

/*****************************************************************************
 * CMidiFilterFactory::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
PNODE_DESCRIPTOR 
CMidiFilterFactory::
FindNode
(
	IN		ULONG	NodeId
)
{
	return m_MidiFilterDescriptor.FindNode(NodeId);
}

/*****************************************************************************
 * CMidiFilterFactory::IsDirectMusicCapable()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CMidiFilterFactory::
IsDirectMusicCapable
(	void
)
{
	return m_DirectMusicCapable;
}

#pragma code_seg()


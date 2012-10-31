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
*//*
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

/*! @brief Debug module name. */
#define STR_MODULENAME "AUDIO_FILTER_FACTORY: "

/*****************************************************************************
 * Referenced forward
 */

#pragma code_seg("PAGE")

/*****************************************************************************
 * CreateAudioFilterFactory()
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
CreateAudioFilterFactory
(
    IN		PKSDEVICE	KsDevice,
	IN		PWCHAR		RefString,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilterFactory::Instantiate]"));

	NTSTATUS ntStatus;

	CAudioFilterFactory * FilterFactory = new(NonPagedPool,'rCcP') CAudioFilterFactory(NULL);

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

			CAudioFilterFactory::SetupFriendlyName(KsAdapter, KsFilterDescriptor, RefString);

			KsAcquireDevice(KsDevice);

			PKSFILTERFACTORY KsFilterFactory = NULL;

			ntStatus = KsCreateFilterFactory
							(
								KsDevice->FunctionalDeviceObject, 
								KsFilterDescriptor, 
								RefString,
								NULL,
								0,
								CAudioFilterFactory::SleepCallback,
								CAudioFilterFactory::WakeCallback,
								&KsFilterFactory
							);

			if (NT_SUCCESS(ntStatus))
			{
				KsFilterFactory->Context = PVOID(FilterFactory);

				FilterFactory->m_KsFilterFactory = KsFilterFactory;
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

			ntStatus = KsAddItemToObjectBag(KsDevice->Bag, FilterFactory, (PFNKSFREE)CAudioFilterFactory::Destruct);

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
 * CAudioFilterFactory::SetupFriendlyName()
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
CAudioFilterFactory::
SetupFriendlyName
(
	IN		PKSADAPTER				KsAdapter,
	IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
	IN		PWCHAR					RefString
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterFactory::SetupFriendlyName]"));

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
 * CAudioFilterFactory::SleepCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioFilterFactory::
SleepCallback 
(
	IN		PKSFILTERFACTORY	KsFilterFactory,
	IN		DEVICE_POWER_STATE	PowerState
)
{
	PAGED_CODE();

	CAudioFilterFactory * FilterFactory = (CAudioFilterFactory *)(KsFilterFactory->Context);

	FilterFactory->PowerChangeNotify(PowerState);
}

/*****************************************************************************
 * CAudioFilterFactory::WakeCallback()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioFilterFactory::
WakeCallback 
(
	IN		PKSFILTERFACTORY	KsFilterFactory,
	IN		DEVICE_POWER_STATE	PowerState
)
{
	PAGED_CODE();

	CAudioFilterFactory * FilterFactory = (CAudioFilterFactory *)(KsFilterFactory->Context);

	FilterFactory->PowerChangeNotify(PowerState);
}

/*****************************************************************************
 * CAudioFilterFactory::Destruct()
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
CAudioFilterFactory::
Destruct 
(
	IN		PVOID	Self
)
{
	CAudioFilterFactory * FilterFactory = (CAudioFilterFactory *)(Self);

	FilterFactory->Release();
}

/*****************************************************************************
 * CAudioFilterFactory::NonDelegatingQueryInterface()
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
CAudioFilterFactory::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilterFactory::NonDelegatingQueryInterface]"));

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
 * CAudioFilterFactory::~CAudioFilterFactory()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CAudioFilterFactory::
~CAudioFilterFactory
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[~CAudioFilterFactory::~CAudioFilterFactory]"));

	if (m_AudioDevice)
	{
		m_AudioDevice->SetInterruptHandler(NULL, NULL);
		m_AudioDevice->Release();
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
 * CAudioFilterFactory::Init()
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
CAudioFilterFactory::
Init
(
	IN		PKSDEVICE	KsDevice,
	IN		PVOID		Parameter1,
	IN		PVOID		Parameter2
)
{
    PAGED_CODE();

    ASSERT(KsDevice);

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioFilterFactory::Init]"));

	m_KsDevice = KsDevice;

	m_KsAdapter = PKSADAPTER(KsDevice->Context);
	m_KsAdapter->AddRef();

	m_UsbDevice = m_KsAdapter->GetUsbDevice();
	m_UsbDevice->AddRef();

	m_AudioDevice = m_KsAdapter->GetAudioDevice();
	m_AudioDevice->AddRef();

	m_DevicePowerState = PowerDeviceD0;

	// Figure out what the filter descriptor here...
	NTSTATUS ntStatus = m_AudioFilterDescriptor.Initialize(m_AudioDevice);

	if (NT_SUCCESS(ntStatus))
	{
		m_KsFilterDescriptor.Dispatch				= &CAudioFilter::DispatchTable;
		m_KsFilterDescriptor.AutomationTable		= &CAudioFilter::AutomationTable;
		m_KsFilterDescriptor.Version				= KSFILTER_DESCRIPTOR_VERSION;
		m_KsFilterDescriptor.Flags					= 0;
		m_KsFilterDescriptor.ReferenceGuid			= NULL;
		m_KsFilterDescriptor.PinDescriptorsCount	= m_AudioFilterDescriptor.PinCount();
		m_KsFilterDescriptor.PinDescriptorSize      = sizeof(KSPIN_DESCRIPTOR_EX);
		m_KsFilterDescriptor.PinDescriptors         = m_AudioFilterDescriptor.Pins();
		m_KsFilterDescriptor.CategoriesCount		= m_AudioFilterDescriptor.CategoryCount();
		m_KsFilterDescriptor.Categories				= m_AudioFilterDescriptor.Categories();
		m_KsFilterDescriptor.NodeDescriptorsCount   = m_AudioFilterDescriptor.NodeCount();
		m_KsFilterDescriptor.NodeDescriptorSize     = sizeof(KSNODE_DESCRIPTOR);
		m_KsFilterDescriptor.NodeDescriptors        = m_AudioFilterDescriptor.Nodes();
		m_KsFilterDescriptor.ConnectionsCount		= m_AudioFilterDescriptor.ConnectionCount();
		m_KsFilterDescriptor.Connections			= m_AudioFilterDescriptor.Connections();
		m_KsFilterDescriptor.ComponentId			= m_AudioFilterDescriptor.ComponentId();
	}

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = m_AudioDevice->SetInterruptHandler(InterruptHandler, this);
	}

	if (!NT_SUCCESS(ntStatus))
	{
		// Cleanup the mess...
		if (m_AudioDevice)
		{
			m_AudioDevice->SetInterruptHandler(NULL, NULL);
			m_AudioDevice->Release();
			m_AudioDevice = NULL;
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
 * CAudioFilterFactory::GetDescription()
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
CAudioFilterFactory::
GetFilterDescription
(
	OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutKsFilterDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterFactory::GetDescription]"));

    *OutKsFilterDescriptor = PKSFILTER_DESCRIPTOR(&m_KsFilterDescriptor);

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioFilterFactory::PowerChangeNotify()
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
CAudioFilterFactory::
PowerChangeNotify
(
    IN      DEVICE_POWER_STATE	NewPowerState
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioFilterFactory::PowerChangeNotify]"));

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
						m_AudioDevice->PowerStateChange(NewPowerState);
                        m_DevicePowerState = NewPowerState;
                        break;
                    case PowerDeviceD3:
                        _DbgPrintF(DEBUGLVL_TERSE,("PowerChangeNotify : D0->D3"));
                        // Power State Transition
						m_AudioDevice->PowerStateChange(NewPowerState);
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
						m_AudioDevice->PowerStateChange(NewPowerState);
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
						m_AudioDevice->PowerStateChange(NewPowerState);
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

#pragma code_seg()

/*****************************************************************************
 * CAudioFilterFactory::FindPin()
 *****************************************************************************
 *//*!
 * @brief
 */
PFILTER_PIN_DESCRIPTOR 
CAudioFilterFactory::
FindPin
(
	IN		ULONG	PinId
)
{
	return m_AudioFilterDescriptor.FindFilterPin(PinId);
}

/*****************************************************************************
 * CAudioFilterFactory::FindNode()
 *****************************************************************************
 *//*!
 * @brief
 */
PNODE_DESCRIPTOR 
CAudioFilterFactory::
FindNode
(
	IN		ULONG	NodeId
)
{
	return m_AudioFilterDescriptor.FindNode(NodeId);
}

/*****************************************************************************
 * CAudioFilterFactory::AddEventHandler()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CAudioFilterFactory::
AddEventHandler
(
	IN		AUDIO_EVENT_HANDLER_ROUTINE	EventHandlerRoutine,
	IN		PVOID						EventHandlerContext,
	OUT		PVOID *						OutHandle
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

    CAudioEventHandler * EventHandlerObject = new(NonPagedPool) CAudioEventHandler();

	if (EventHandlerObject)
	{
		ntStatus = EventHandlerObject->Init(EventHandlerRoutine, EventHandlerContext);

		if (NT_SUCCESS(ntStatus))
		{
			m_EventHandlerList.Lock();
			
			m_EventHandlerList.Put(EventHandlerObject);

			m_EventHandlerList.Unlock();
		}
		else
		{
			EventHandlerObject->Destruct();

			EventHandlerObject = NULL;
		}
	}
	else
	{
		ntStatus = STATUS_NO_MEMORY;
	}

    *OutHandle = (PVOID)EventHandlerObject;

	return ntStatus;
}

/*****************************************************************************
 * CAudioFilterFactory::RemoveEventHandler()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioFilterFactory::
RemoveEventHandler
(
	IN		PVOID	Handle
)
{
	ASSERT(Handle);

	CAudioEventHandler * EventHandlerObject = (CAudioEventHandler*)(Handle);

	m_EventHandlerList.Lock();

	m_EventHandlerList.Remove(EventHandlerObject);

	m_EventHandlerList.Unlock();

	EventHandlerObject->Destruct();
}

/*****************************************************************************
 * CAudioFilterFactory::ServiceEvent()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CAudioFilterFactory::
ServiceEvent
(
	IN		PVOID	InterruptFilter,
	IN		GUID *	Set,
	IN		ULONG	EventId,
	IN		BOOL	PinEvent,
	IN		ULONG	PinId,
	IN		BOOL	NodeEvent,
	IN		ULONG	NodeId
)
{
	m_EventHandlerList.Lock();

	for (CAudioEventHandler * Handler = m_EventHandlerList.First(); Handler; Handler = m_EventHandlerList.Next(Handler))
	{
		Handler->Service(InterruptFilter, Set, EventId, PinEvent, PinId, NodeEvent, NodeId);
	}

	m_EventHandlerList.Unlock();
}

/*****************************************************************************
 * CAudioFilterFactory::InterruptHandler()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CAudioFilterFactory::
InterruptHandler
(
	IN		PVOID						Context,
	IN		AUDIO_INTERRUPT_ORIGINATOR	Originator,
	IN		UCHAR						OriginatorID,
	IN		PVOID						InterruptFilter
)
{
	CAudioFilterFactory * that = (CAudioFilterFactory *)(Context);

	that->HandleInterrupt(Originator, OriginatorID, InterruptFilter);
}

/*****************************************************************************
 * CAudioFilterFactory::HandleInterrupt()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID
CAudioFilterFactory::
HandleInterrupt
(
	IN		AUDIO_INTERRUPT_ORIGINATOR	Originator,
	IN		UCHAR						OriginatorID,
	IN		PVOID						InterruptFilter
)
{
	switch (Originator)
	{
		case AUDIO_INTERRUPT_ORIGINATOR_AC_TERMINAL:
		{
			ULONG PinCount = m_AudioFilterDescriptor.PinCount();

			for (ULONG i=0; i<PinCount; i++)
			{
				PFILTER_PIN_DESCRIPTOR Pin = m_AudioFilterDescriptor.FindFilterPin(i);

				if (Pin && Pin->TerminalID() == OriginatorID)
				{
					ServiceEvent(InterruptFilter, (GUID*)&KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, TRUE, Pin->PinId(), FALSE, ULONG(-1));
					break;
				}
			}
		}
		break;

		case AUDIO_INTERRUPT_ORIGINATOR_AC_UNIT:
		{
			ULONG NodeCount = m_AudioFilterDescriptor.NodeCount();

			for (ULONG i=0; i<NodeCount; i++)
			{
				PNODE_DESCRIPTOR Node = m_AudioFilterDescriptor.FindNode(i);

				if (Node && Node->UnitID() == OriginatorID)
				{
					ServiceEvent(InterruptFilter, (GUID*)&KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, FALSE, ULONG(-1), TRUE, Node->NodeId());
					break;
				}
			}
		}
		break;

		case AUDIO_INTERRUPT_ORIGINATOR_AC_FUNCTION:
		{
			ServiceEvent(InterruptFilter, (GUID*)&KSEVENTSETID_AudioControlChange, KSEVENT_CONTROL_CHANGE, FALSE, ULONG(-1), FALSE, ULONG(-1));
		}
		break;

		case AUDIO_INTERRUPT_ORIGINATOR_AS_INTERFACE:
		{
			//TODO:
		}
		break;

		case AUDIO_INTERRUPT_ORIGINATOR_AS_ENDPOINT:
		{
			//TODO:
		}
		break;

		default:
		{
		}
		break;
	}
}

#pragma code_seg()


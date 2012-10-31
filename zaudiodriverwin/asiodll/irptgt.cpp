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
 * @file       irptgt.cpp
 * @brief      This is the implementation file for the C++ base class which 
 *			   abstracts common functionality in CKsFilter and CKsPin classes.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "ksaudio.h"

#define STR_MODULENAME "IRPTGT: "


/*****************************************************************************
 * CKsIrpTarget::CKsIrpTarget()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsIrpTarget::
CKsIrpTarget
(
	IN		HANDLE	Handle
)
{
    _DbgPrintF(DEBUGLVL_TERSE,("[CKsIrpTarget::CKsIrpTarget]"));

	m_Handle = Handle;
}

/*****************************************************************************
 * CKsIrpTarget::GetHandle()
 *****************************************************************************
 *//*!
 * @brief
 */
HANDLE 
CKsIrpTarget::
GetHandle
(	void
)
{ 
    _DbgPrintF(DEBUGLVL_TERSE,("[CKsIrpTarget::GetHandle]"));

	return m_Handle; 
}

/*****************************************************************************
 * CKsIrpTarget::IsValidHandle()
 *****************************************************************************
 *//*!
 * @brief
 * Quick and Dirty check to see if the handle is good.  Only checks against
 * a small number of known bad values.
 * @return
 * Returns TRUE unless the handle is NULL or INVALID_HANDLE_VALUE.
 */
BOOL 
CKsIrpTarget::
IsValidHandle
(
	IN		HANDLE	Handle
)
{
    BOOL ValidHandle = !((Handle == NULL) || (Handle == INVALID_HANDLE_VALUE));

    return ValidHandle;
}

/*****************************************************************************
 * CKsIrpTarget::SafeCloseHandle()
 *****************************************************************************
 *//*!
 * @brief
 * Safely closes a file handle.
 */
BOOL 
CKsIrpTarget::
SafeCloseHandle
(
	IN OUT	HANDLE&	Handle
)
{
    BOOL Success = TRUE;

    if (IsValidHandle(Handle))
    {
        Success = CloseHandle(Handle);

        Handle = INVALID_HANDLE_VALUE;
    }

    return Success;
}

/*****************************************************************************
 * CKsIrpTarget::Close()
 *****************************************************************************
 *//*!
 * @brief
 * Close the Irp Target.
 */
BOOL CKsIrpTarget::Close()
{
    BOOL Success = SafeCloseHandle(m_Handle);
    
    return Success;
}

/*****************************************************************************
 * CKsIrpTarget::SynchronizedIoctl()
 *****************************************************************************
 *//*!
 * @brief
 * Synchronized I/O control.
 * @param
 * Handle The file handle to send the I/O control to.
 * @param
 * CtlCode The I/O control code to send.
 * @param
 * InputBuffer Pointer to the input buffer.
 * @param
 * InputBufferSize Size in bytes of the input buffer.
 * @param
 * OutputBuffer Pointer to the output buffer.
 * @param
 * OutputBufferSize Size in bytes of the output buffer.
 * @param
 * OutBytesReturned The number of bytes written to the output buffer.
 * @return
 * Returns S_OK on success, otherwise appropriate error code.
 */
HRESULT 
CKsIrpTarget::
SynchronizedIoctl
(
	IN		HANDLE	Handle,
	IN		ULONG	CtlCode,
	IN		PVOID	InputBuffer,
	IN		ULONG	InputBufferSize,
	OUT		PVOID	OutputBuffer,
	OUT		ULONG	OutputBufferSize,
	OUT		ULONG *	OutBytesReturned
)
{
    HRESULT hr = S_OK;

    ULONG BytesReturned;

    if (!OutBytesReturned)
    {
        OutBytesReturned = &BytesReturned;
    }

    if (!IsValidHandle(Handle))
    {
        hr = E_FAIL;

		_DbgPrintF(DEBUGLVL_ERROR,("[CKsIrpTarget::SynchronizedIoctl] - Invalid Handle"));
    }
    
    OVERLAPPED Overlapped;

	if (SUCCEEDED(hr))
    {
        ZeroMemory(&Overlapped, sizeof(OVERLAPPED));

        Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (Overlapped.hEvent)
        {
            // Flag the event by setting the low-order bit so we
            // don't get completion port notifications.
            // Really! - see the description of the lpOverlapped parameter in
            // the docs for GetQueuedCompletionStatus
            Overlapped.hEvent = (HANDLE)((DWORD_PTR)Overlapped.hEvent | 0x1);
        }
        else
        {
            hr = E_OUTOFMEMORY;

			_DbgPrintF(DEBUGLVL_ERROR,("[CKsIrpTarget::SynchronizedIoctl] - CreateEvent failed"));
        }
    }

    if (SUCCEEDED(hr))
    {
        BOOL Result = DeviceIoControl(Handle, CtlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, OutBytesReturned, &Overlapped);

        if (!Result)
        {
            DWORD w32Error = GetLastError();

            if (ERROR_IO_PENDING == w32Error)
            {
                // Wait for completion
                DWORD Wait = ::WaitForSingleObject(Overlapped.hEvent, INFINITE);

                ASSERT(WAIT_OBJECT_0 == Wait);

                if (Wait != WAIT_OBJECT_0)
                {
                    hr = E_FAIL;

					_DbgPrintF(DEBUGLVL_ERROR,("[CKsIrpTarget::SynchronizedIoctl] - WaitForSingleObject failed Wait: 0x%08x", Wait));
                }
            }
            else if (((ERROR_INSUFFICIENT_BUFFER == w32Error) || (ERROR_MORE_DATA == w32Error)) &&
					 (IOCTL_KS_PROPERTY == CtlCode) &&
					 (OutputBufferSize == 0))
            {
                hr = S_OK;

                Result = TRUE;
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if (!Result) 
		{
			*OutBytesReturned = 0;
		}

        SafeCloseHandle(Overlapped.hEvent);
    }
    
    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::PropertySetSupport()
 *****************************************************************************
 *//*!
 * @brief
 * Query to see if a property set is supported.
 * @param
 * The GUID of the property set.
 * @return
 * S_OK if the Property set is supported. S_FALSE if the property set is 
 * unsupported.
 */
HRESULT 
CKsIrpTarget::
PropertySetSupport
(
	IN		REFGUID	PropertySet
)
{
	KSPROPERTY KsProperty;

    ZeroMemory(&KsProperty, sizeof(KsProperty));

    KsProperty.Set   = PropertySet;
    KsProperty.Id    = 0;
    KsProperty.Flags = KSPROPERTY_TYPE_SETSUPPORT;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle, 
						IOCTL_KS_PROPERTY, 
						&KsProperty, 
						sizeof(KsProperty), 
						NULL, 
						0, 
						NULL
					);
    
    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::PropertyBasicSupport()
 *****************************************************************************
 *//*!
 * @brief
 * Get the basic support information for this KSPROPERTY.
 * @param
 * PropertySet The guid of the property set.
 * @param
 * PropertyId The property in the property set.
 * @param
 * OutSupport The support information for the property.
 * @return
 * Returns S_OK on success, otherwise appropriate error code.
 */
HRESULT 
CKsIrpTarget::
PropertyBasicSupport
(
    IN		REFGUID	PropertySet,
    IN		ULONG   PropertyId,
    OUT		ULONG *	OutSupport
)
{
    KSPROPERTY KsProperty;

    ZeroMemory(&KsProperty, sizeof(KsProperty));

    KsProperty.Set   = PropertySet;
    KsProperty.Id    = PropertyId;
    KsProperty.Flags = KSPROPERTY_TYPE_BASICSUPPORT;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_PROPERTY,
						&KsProperty,
						sizeof(KsProperty),
						OutSupport,
						sizeof(ULONG),
						NULL
					);
    
    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::GetPropertySimple()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a simple property.
 */
HRESULT 
CKsIrpTarget::
GetPropertySimple
(
    IN		REFGUID	PropertySet,
    IN		ULONG   PropertyId,
    OUT		PVOID   Value,
    IN		ULONG   ValueSize,
    IN		PVOID	Instance		OPTIONAL,
    IN		ULONG	InstanceSize	OPTIONAL
)
{
    HRESULT hr = S_OK;

	ULONG PropertySize = sizeof(KSPROPERTY) + InstanceSize;

    PKSPROPERTY KsProperty = (PKSPROPERTY)new BYTE[PropertySize];

    if (NULL == KsProperty)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        ZeroMemory(KsProperty, sizeof(KSPROPERTY));

        KsProperty->Set   = PropertySet;
        KsProperty->Id    = PropertyId;
        KsProperty->Flags = KSPROPERTY_TYPE_GET;

        if (Instance)
        {
            CopyMemory(PUCHAR(KsProperty) + sizeof(KSPROPERTY), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					KsProperty,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }    

    //cleanup memory
	if (KsProperty)
	{
		delete[] (BYTE*)KsProperty;
	}

    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::GetPropertyMulti()
 *****************************************************************************
 *//*!
 * @brief
 * Multiple items request.  The function allocates memory for the caller.  
 * It is the caller's responsiblity to free this memory.
 */
HRESULT 
CKsIrpTarget::
GetPropertyMulti
(
    IN		REFGUID				PropertySet,
    IN		ULONG				PropertyId,
    OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem
)
{
    ULONG MultipleItemSize = 0;

	KSPROPERTY KsProperty;

    ZeroMemory(&KsProperty, sizeof(KsProperty));

    KsProperty.Set   = PropertySet;
    KsProperty.Id    = PropertyId;
    KsProperty.Flags = KSPROPERTY_TYPE_GET;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_PROPERTY,
						&KsProperty,
						sizeof(KSPROPERTY),
						NULL,
						0,
						&MultipleItemSize
					);

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        *OutKsMultipleItem = (PKSMULTIPLE_ITEM) new BYTE[MultipleItemSize];

        if (NULL == *OutKsMultipleItem)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					&KsProperty,
					sizeof(KSPROPERTY),
					*OutKsMultipleItem,
					MultipleItemSize,
					NULL
				);
    }
    
    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::GetPropertyMulti()
 *****************************************************************************
 *//*!
 * @brief
 * Multiple items request for when the input is not a property. The function 
 * allocates memory for the  caller.  It is the caller's responsiblity to 
 * free this memory.
 */
HRESULT 
CKsIrpTarget::
GetPropertyMulti
(
    IN		REFGUID				PropertySet,
    IN		ULONG				PropertyId,
    IN		PVOID				Data,
    IN		ULONG				DataSize,
    OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem
)
{
    ULONG MultipleItemSize = 0;

	KSPROPERTY KsProperty;

    ZeroMemory(&KsProperty, sizeof(KsProperty));

    KsProperty.Set   = PropertySet;
    KsProperty.Id    = PropertyId;
    KsProperty.Flags = KSPROPERTY_TYPE_GET;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_PROPERTY,
						&KsProperty,
						sizeof(KSPROPERTY),
						NULL,
						0,
						&MultipleItemSize
					);

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        *OutKsMultipleItem = (PKSMULTIPLE_ITEM) new BYTE[MultipleItemSize];

        if (NULL == *OutKsMultipleItem)
        {
            hr = E_OUTOFMEMORY;
        }
    }


    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					Data,
					DataSize,
					*OutKsMultipleItem,
					MultipleItemSize,
					NULL
				);
    }

    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::GetNodePropertySimple()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a simple node property.
 */
HRESULT 
CKsIrpTarget::
GetNodePropertySimple
(
    IN		ULONG	NodeId,
    IN		REFGUID	PropertySet,
    IN		ULONG	PropertyId,
    OUT		PVOID	Value,
    IN		ULONG	ValueSize,
    IN		PVOID	Instance		OPTIONAL,
    IN		ULONG	InstanceSize	OPTIONAL
)
{
	HRESULT hr = S_OK;
    
	ULONG PropertySize = sizeof(KSNODEPROPERTY) + InstanceSize;

    PKSNODEPROPERTY KsNodeProperty = (PKSNODEPROPERTY)new BYTE[PropertySize];

    if (NULL == KsNodeProperty)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        ZeroMemory(KsNodeProperty, sizeof(KSNODEPROPERTY));

        KsNodeProperty->Property.Set = PropertySet;
        KsNodeProperty->Property.Id = PropertyId;
        KsNodeProperty->Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
        KsNodeProperty->NodeId = NodeId;
        KsNodeProperty->Reserved = 0;

        if (Instance)
        {
            CopyMemory(PUCHAR(KsNodeProperty) + sizeof(KSNODEPROPERTY), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					KsNodeProperty,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }    

	//cleanup memory
	if (KsNodeProperty)
	{
		delete[] (BYTE*)KsNodeProperty;
	}

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::GetNodePropertyChannel()
 *****************************************************************************
 *//*!
 * @brief
 * Gets a simple node channel property.
 */
HRESULT 
CKsIrpTarget::
GetNodePropertyChannel
(
    IN		ULONG	NodeId,
    IN		ULONG	Channel,
    IN		REFGUID	PropertySet,
    IN		ULONG	PropertyId,
    OUT		PVOID	Value,
    IN		ULONG	ValueSize,
    IN		PVOID	Instance 		OPTIONAL,
    IN		ULONG	InstanceSize	OPTIONAL
)
{
	HRESULT hr = S_OK;

	ULONG PropertySize = sizeof(KSNODEPROPERTY_AUDIO_CHANNEL) + InstanceSize;

    PKSNODEPROPERTY_AUDIO_CHANNEL KsNodePropertyChannel = (PKSNODEPROPERTY_AUDIO_CHANNEL)new BYTE[PropertySize];

    if (NULL == KsNodePropertyChannel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        ZeroMemory(KsNodePropertyChannel, sizeof(KSNODEPROPERTY_AUDIO_CHANNEL));

        KsNodePropertyChannel->NodeProperty.Property.Set = PropertySet;
        KsNodePropertyChannel->NodeProperty.Property.Id = PropertyId;
        KsNodePropertyChannel->NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
        KsNodePropertyChannel->NodeProperty.NodeId = NodeId;
        KsNodePropertyChannel->NodeProperty.Reserved = 0;
        KsNodePropertyChannel->Channel = Channel;
        KsNodePropertyChannel->Reserved = 0;

        if (Instance)
        {
            CopyMemory(PUCHAR(KsNodePropertyChannel) + sizeof(KSNODEPROPERTY_AUDIO_CHANNEL), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					KsNodePropertyChannel,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }    

    //cleanup memory
	if (KsNodePropertyChannel)
	{
		delete[] (BYTE*)KsNodePropertyChannel;
	}

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::SetPropertySimple()
 *****************************************************************************
 *//*!
 * @brief
 * Set the value of a simple (non-multi) property.
 */
HRESULT 
CKsIrpTarget::
SetPropertySimple
(
    IN		REFGUID	PropertySet,
    IN		ULONG	PropertyId,
    IN		PVOID	Value,
    IN		ULONG	ValueSize,
    IN		PVOID	Instance 		OPTIONAL,
    IN		ULONG	InstanceSize	OPTIONAL
)
{
    HRESULT hr = S_OK;

    ULONG PropertySize = sizeof(KSPROPERTY) + InstanceSize;

    PKSPROPERTY KsProperty = (PKSPROPERTY)new BYTE[PropertySize];

    if (NULL == KsProperty)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        KsProperty->Set    = PropertySet; 
        KsProperty->Id     = PropertyId;       
        KsProperty->Flags  = KSPROPERTY_TYPE_SET;

        if (Instance)
        {
            memcpy((PBYTE)KsProperty + sizeof(KSPROPERTY), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					KsProperty,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }

    //cleanup memory
	if (KsProperty)
	{
		delete[] (BYTE*)KsProperty;
	}

    return hr;
}

/*****************************************************************************
 * CKsIrpTarget::SetPropertyMulti()
 *****************************************************************************
 *//*!
 * @brief
 * ...
 */
HRESULT 
CKsIrpTarget::
SetPropertyMulti
(
    IN		REFGUID				PropertySet,
    IN		ULONG				PropertyId,
    OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem
)
{
    ULONG MultipleItemSize = 0;

	KSPROPERTY KsProperty;

    KsProperty.Set    = PropertySet; 
    KsProperty.Id     = PropertyId;       
    KsProperty.Flags  = KSPROPERTY_TYPE_SET;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_PROPERTY,
						&KsProperty,
						sizeof(KSPROPERTY),
						NULL,
						0,
						&MultipleItemSize
					);

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        *OutKsMultipleItem = (PKSMULTIPLE_ITEM) new BYTE[MultipleItemSize];

        if (NULL == *OutKsMultipleItem)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr) && MultipleItemSize)
    {
        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					&KsProperty,
					sizeof(KSPROPERTY),
					*OutKsMultipleItem,
					MultipleItemSize,
					NULL
				);
    }

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::SetNodePropertySimple()
 *****************************************************************************
 *//*!
 * @brief
 * Set the value of a simple (non-multi) node property.
 */
HRESULT 
CKsIrpTarget::
SetNodePropertySimple
(
    IN		ULONG	NodeId,
    IN		REFGUID	PropertySet,
    IN		ULONG	PropertyId,
    IN		PVOID	Value,
    IN		ULONG	ValueSize,
    IN		PVOID	Instance 		OPTIONAL,
    IN		ULONG	InstanceSize	OPTIONAL
)
{
    HRESULT hr = S_OK;

	ULONG PropertySize = sizeof(KSNODEPROPERTY) + InstanceSize;

    PKSNODEPROPERTY KsNodeProperty = (PKSNODEPROPERTY)new BYTE[PropertySize];
    
    if (NULL == KsNodeProperty)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        KsNodeProperty->Property.Set    = PropertySet; 
        KsNodeProperty->Property.Id     = PropertyId;       
        KsNodeProperty->Property.Flags  = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
        KsNodeProperty->NodeId          = NodeId;
        KsNodeProperty->Reserved        = 0;

        if (Instance)
        {
            CopyMemory((PBYTE)KsNodeProperty + sizeof(KSNODEPROPERTY), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					KsNodeProperty,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }

    //cleanup memory
	if (KsNodeProperty)
	{
		delete[] (BYTE*)KsNodeProperty;
	}

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::SetNodePropertyChannel()
 *****************************************************************************
 *//*!
 * @brief
 * Set the value of a simple (non-multi) node channel property.
 */
HRESULT 
CKsIrpTarget::
SetNodePropertyChannel
(
    IN		ULONG	NodeId,
    IN		ULONG	Channel,
    IN		REFGUID	PropertySet,
    IN		ULONG	PropertyId,
    IN		PVOID	Value,
    IN		ULONG	ValueSize,
    IN		PVOID	Instance 		OPTIONAL,
    IN		ULONG	InstanceSize	OPTIONAL
)
{
    HRESULT hr = S_OK;

    ULONG PropertySize = sizeof(KSNODEPROPERTY_AUDIO_CHANNEL) + InstanceSize;

	PKSNODEPROPERTY_AUDIO_CHANNEL KsNodePropertyChannel = (PKSNODEPROPERTY_AUDIO_CHANNEL)new BYTE[PropertySize];
    
    if (NULL == KsNodePropertyChannel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        KsNodePropertyChannel->NodeProperty.Property.Set    = PropertySet; 
        KsNodePropertyChannel->NodeProperty.Property.Id     = PropertyId;       
        KsNodePropertyChannel->NodeProperty.Property.Flags  = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
        KsNodePropertyChannel->NodeProperty.NodeId          = NodeId;
        KsNodePropertyChannel->NodeProperty.Reserved        = 0;
        KsNodePropertyChannel->Channel						= Channel;
        KsNodePropertyChannel->Reserved						= 0;

        if (Instance)
        {
            CopyMemory((PBYTE)KsNodePropertyChannel + sizeof(KSNODEPROPERTY_AUDIO_CHANNEL), Instance, InstanceSize);
        }

        hr = SynchronizedIoctl
				(
					m_Handle,
					IOCTL_KS_PROPERTY,
					KsNodePropertyChannel,
					PropertySize,
					Value,
					ValueSize,
					NULL
				);
    }

    //cleanup memory
	if (KsNodePropertyChannel)
	{
		delete[] (BYTE*)KsNodePropertyChannel;
	}

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::EnableEvent()
 *****************************************************************************
 *//*!
 * @brief
 * Enable the KS event.
 */
HRESULT
CKsIrpTarget::
EnableEvent
(
	IN		REFGUID			EventSet,
	IN		ULONG			EventId,
	IN		PKSEVENTDATA	EventData,
	IN		ULONG			EventDataSize
)
{
	KSEVENT Event;

	Event.Set = EventSet;
	Event.Id = EventId;
	Event.Flags = KSEVENT_TYPE_ENABLE;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_ENABLE_EVENT,
						&Event,
						sizeof(KSEVENT),
						EventData,
						EventDataSize,
						NULL
					);

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::EnableNodeEvent()
 *****************************************************************************
 *//*!
 * @brief
 * Enable the KS event for the specified node.
 */
HRESULT
CKsIrpTarget::
EnableNodeEvent
(
	IN		ULONG			NodeId,
	IN		REFGUID			EventSet,
	IN		ULONG			EventId,
	IN		PKSEVENTDATA	EventData,
	IN		ULONG			EventDataSize
)
{
	KSE_NODE KseNode;

	KseNode.Event.Set = EventSet;
	KseNode.Event.Id = EventId;
	KseNode.Event.Flags = KSEVENT_TYPE_ENABLE | KSEVENT_TYPE_TOPOLOGY;
	KseNode.NodeId = NodeId;
	KseNode.Reserved = 0;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_ENABLE_EVENT,
						&KseNode,
						sizeof(KSE_NODE),
						EventData,
						EventDataSize,
						NULL
					);

	return hr;
}

/*****************************************************************************
 * CKsIrpTarget::DisableEvent()
 *****************************************************************************
 *//*!
 * @brief
 * Disable the KS event.
 */
HRESULT
CKsIrpTarget::
DisableEvent
(
	IN		PKSEVENTDATA	EventData,
	IN		ULONG			EventDataSize
)
{
    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_DISABLE_EVENT,
						EventData,
						EventDataSize,
						NULL,
						0,
						NULL
					);

	return hr;
}


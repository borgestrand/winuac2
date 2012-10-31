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
 * @file       filter.cpp
 * @brief      This is the implementation file for C++ classes that expose 
 *             functionality of KS filters.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "ksaudio.h"
#include "filter.h"

#define STR_MODULENAME "KSFILTER: "


/*****************************************************************************
 * CKsFilter::InternalInit()
 *****************************************************************************
 *//*!
 * @brief
 * Internal Initialization Function.  It zero's out the strings and calls
 * initialzie on all of the lists.
 */
HRESULT 
CKsFilter::
InternalInit
(	void
)
{
    m_FilterType = KS_TECHNOLOGY_TYPE_UNKNOWN;

		// Zero out the strings
    ZeroMemory(m_SymbolicLink, sizeof(m_SymbolicLink));

    // Initialize the Lists
    HRESULT hr = m_ListCaptureSinkPins.Initialize(1);

    if (SUCCEEDED(hr))
    {
        hr = m_ListCaptureSourcePins.Initialize(1);

        if (FAILED(hr))
		{
	        _DbgPrintF(DEBUGLVL_ERROR,("Failed to Initialize m_ListCaptureSourcePins"));
		}
    }
        

    if (SUCCEEDED(hr))
    {
        hr = m_ListNoCommPins.Initialize(1);

        if (FAILED(hr))
		{
	        _DbgPrintF(DEBUGLVL_ERROR,("Failed to Initialize m_ListNoCommPins"));
		}
    }

    if (SUCCEEDED(hr))
    {
        hr = m_ListPins.Initialize(1);

        if (FAILED(hr))
		{
	        _DbgPrintF(DEBUGLVL_ERROR,("Failed to Initialize m_ListPins"));
		}
    }

    if (SUCCEEDED(hr))
    {
        hr = m_ListRenderSinkPins.Initialize(1);

        if (FAILED(hr))
		{
	        _DbgPrintF(DEBUGLVL_ERROR,("Failed to Initialize m_ListRenderSinkPins"));
		}
    }

    if (SUCCEEDED(hr))
    {
        hr = m_ListRenderSourcePins.Initialize(1);

        if (FAILED(hr))
		{
	        _DbgPrintF(DEBUGLVL_ERROR,("Failed to Initialize m_ListRenderSourcePins"));
		}
    }

    if (SUCCEEDED(hr))
    {
        hr = m_ListNodes.Initialize(1);

        if (FAILED(hr))
		{
	        _DbgPrintF(DEBUGLVL_ERROR,("Failed to Initialize m_ListNodes"));
		}
    }

    return hr;
}

/*****************************************************************************
 * CKsFilter::CKsFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsFilter::
CKsFilter 
(
    IN		LPCTSTR		SymbolicLink,
    IN		LPCTSTR		FriendlyName,
    OUT		HRESULT *	OutHResult
)
:	CKsIrpTarget(INVALID_HANDLE_VALUE)
{
    HRESULT hr = InternalInit();

    if (NULL == SymbolicLink)
    {
        hr = E_INVALIDARG;

        _DbgPrintF(DEBUGLVL_ERROR,("SymbolicLink cannot be NULL"));
    }

    if (SUCCEEDED(hr))
    {
        _tcsncpy(m_SymbolicLink, SymbolicLink, MAX_PATH);
        
		_tcsncpy(m_FriendlyName, FriendlyName, MAX_PATH);
    }
    
    *OutHResult = hr;
}

/*****************************************************************************
 * CKsFilter::DestroyLists()
 *****************************************************************************
 *//*!
 * @brief
 * Dumps the contents of the lists.
 */
HRESULT 
CKsFilter::
DestroyLists
(	void
)
{
    HRESULT hr = S_OK;

    CKsNode * KsNode = NULL;

    // Clear the node list
    while (m_ListNodes.RemoveHead(&KsNode))
    {
        delete KsNode;
    }

	CKsPin* Pin = NULL;

    // clean pins
    while (m_ListPins.RemoveHead(&Pin))
    {
        delete Pin;
    }

    // empty shallow copy lists
    while (m_ListRenderSinkPins.RemoveHead(&Pin));
    while (m_ListRenderSourcePins.RemoveHead(&Pin));
    while (m_ListCaptureSinkPins.RemoveHead(&Pin));
    while (m_ListCaptureSourcePins.RemoveHead(&Pin));
    while (m_ListNoCommPins.RemoveHead(&Pin));
    
    return hr;
}

/*****************************************************************************
 * CKsFilter::~CKsFilter()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CKsFilter::
~CKsFilter
(	void
)
{
    DestroyLists();

    SafeCloseHandle(m_Handle);
}

/*****************************************************************************
 * CKsFilter::Instantiate()
 *****************************************************************************
 *//*!
 * @brief
 * Instantiates the filter.
 */
HRESULT 
CKsFilter::
Instantiate
(	void
)
{
    HRESULT hr = S_OK;

    m_Handle = CreateFile
				(
					m_SymbolicLink,
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
					NULL
				);

    if (!IsValidHandle(m_Handle))
    {
        hr = E_FAIL;
    }
    
    if (FAILED(hr))
    {
        DWORD w32Error = GetLastError();

        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::Instantiate] - CreateFile failed for device %s.  ErrorCode = 0x%08x", m_SymbolicLink, w32Error));
    }
    
    return hr;
}

/*****************************************************************************
 * CKsFilter::EnumeratePins()
 *****************************************************************************
 *//*!
 * @brief
 * Enumerates pins.
 */
HRESULT 
CKsFilter::
EnumeratePins
(	void
)
{
	ULONG NumberOfPinFactory = 0;

    HRESULT hr = GetPinPropertySimple
					(
						0,
						KSPROPSETID_Pin,
						KSPROPERTY_PIN_CTYPES,
						&NumberOfPinFactory,
						sizeof(ULONG)
					);
        
    if (SUCCEEDED(hr))
    {
        // Loop through the pins
        for (ULONG PinId = 0; PinId < NumberOfPinFactory; PinId++)
        {
            // create a new CKsPin
            hr = S_OK;

            CKsPin* Pin = new CKsPin(this, PinId, &hr);

            if (FAILED(hr))
            {
		        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::EnumeratePins] - Failed to construct pin"));

                goto break_loop;
            }
            else if (NULL == Pin)
            {
		        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::EnumeratePins] - Failed to create pin"));

				hr = E_OUTOFMEMORY;

                goto break_loop;
            }

            if (NULL == m_ListPins.AddTail(Pin))
            {
		        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::EnumeratePins] - Failed to add pin to list"));

                goto break_loop;
            }
            continue;

		break_loop:

            delete Pin;
            Pin = NULL;
        }
    }

    hr = ClassifyPins();

    BOOL ViableFilter = m_ListCaptureSinkPins.GetCount()    ||
						m_ListCaptureSourcePins.GetCount()  ||
						m_ListRenderSinkPins.GetCount()     ||
						m_ListRenderSourcePins.GetCount()   ||
						m_ListNoCommPins.GetCount();

    if (!ViableFilter)
    {
        hr = E_FAIL;

        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::EnumeratePins] - Filter is not viable. It has no pins."));
    }
    
    return hr;
}

/*****************************************************************************
 * CKsFilter::ClassifyPins()
 *****************************************************************************
 *//*!
 * @brief
 * Classify pins.
 */
HRESULT 
CKsFilter::
ClassifyPins
(	void
)
{
    HRESULT hr = S_OK;

    CKsPin* Pin = NULL;

	LISTPOS ListPosition = m_ListPins.GetHeadPosition();

    while (SUCCEEDED(hr) && m_ListPins.GetNext(ListPosition, &Pin))
    {
	    KSPIN_COMMUNICATION Communication;

        hr = Pin->GetCommunication(&Communication);

        if (FAILED(hr))
        {
            break;
        }
        
	    KSPIN_DATAFLOW DataFlow;
    
        hr = Pin->GetDataFlow(&DataFlow);

        if (FAILED(hr))
        {
            break;
        }
       
        if (KSPIN_DATAFLOW_IN == DataFlow)
        {
            switch (Communication)
            {
                case KSPIN_COMMUNICATION_SINK:
				{
                    if (NULL == m_ListRenderSinkPins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                case KSPIN_COMMUNICATION_SOURCE:
				{
                    if (NULL == m_ListCaptureSourcePins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                case KSPIN_COMMUNICATION_BOTH:
				{
                    if (NULL == m_ListRenderSinkPins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    if (NULL == m_ListCaptureSourcePins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                case KSPIN_COMMUNICATION_NONE:
				{
                    if (NULL == m_ListNoCommPins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                default:
				{
			        _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::ClassifyPins] - Pin communication type not recognized."));
				}
                break;
            }
        }
        else
        {
            switch (Communication)
            {
                case KSPIN_COMMUNICATION_SINK:
				{
                    if (NULL == m_ListCaptureSinkPins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                case KSPIN_COMMUNICATION_SOURCE:
				{
                    if (NULL == m_ListRenderSourcePins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                case KSPIN_COMMUNICATION_BOTH:
				{
                    if (NULL == m_ListCaptureSinkPins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }

					if (NULL == m_ListRenderSourcePins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                case KSPIN_COMMUNICATION_NONE:
				{
                    if (NULL == m_ListNoCommPins.AddTail(Pin))
                    {
                        hr = E_OUTOFMEMORY;
                    }
				}
                break;

                default:
				{
			        _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::ClassifyPins] - Pin communication type not recognized."));
				}
                break;
            }
        }
    }
    
    return hr;
}

/*****************************************************************************
 * CKsFilter::GetPinPropertySimple()
 *****************************************************************************
 *//*!
 * @brief
 * Gets simple pin properties.
 */
HRESULT 
CKsFilter::
GetPinPropertySimple
(
    IN		ULONG	PinId,
    IN		REFGUID PropertySet,
    IN		ULONG   PropertyId,
    OUT		PVOID   Value,
    IN		ULONG   ValueSize
)
{
    KSP_PIN KspPin;

    KspPin.Property.Set	  = PropertySet;
    KspPin.Property.Id    = PropertyId;
    KspPin.Property.Flags = KSPROPERTY_TYPE_GET;
    KspPin.PinId          = PinId;
    KspPin.Reserved       = 0;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_PROPERTY,
						&KspPin,
						sizeof(KSP_PIN),
						Value,
						ValueSize,
						NULL
					);
    
    return hr;
}

/*****************************************************************************
 * CKsFilter::GetPinPropertyMulti()
 *****************************************************************************
 *//*!
 * @brief
 * Gets multiple pin properties.
 */
HRESULT 
CKsFilter::
GetPinPropertyMulti
(
    IN		ULONG				PinId,
    IN		REFGUID				PropertySet,
    IN		ULONG				PropertyId,
    OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem	OPTIONAL
)
{
    ULONG MultipleItemSize = 0;

    KSP_PIN KspPin;

    KspPin.Property.Set	  = PropertySet;
    KspPin.Property.Id    = PropertyId;
    KspPin.Property.Flags = KSPROPERTY_TYPE_GET;
    KspPin.PinId          = PinId;
    KspPin.Reserved       = 0;

    HRESULT hr = SynchronizedIoctl
					(
						m_Handle,
						IOCTL_KS_PROPERTY,
						&KspPin,
						sizeof(KSP_PIN),
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
					&KspPin,
					sizeof(KSP_PIN),
					*OutKsMultipleItem,
					MultipleItemSize,
					NULL
				);
    }

    return hr;
}

/*****************************************************************************
 * CKsFilter::EnumerateNodes()
 *****************************************************************************
 *//*!
 * @brief
 * Enumerates nodes.
 */
HRESULT 
CKsFilter::
EnumerateNodes
(	void
)
{
    // Clear the existing node list
    CKsNode * KsNode = NULL;

    while (m_ListNodes.RemoveHead(&KsNode))
    {
        delete KsNode;
    }

    PKSMULTIPLE_ITEM KsMultipleTopoNodes = NULL;

    HRESULT hr = GetPropertyMulti
					(
						KSPROPSETID_Topology,
						KSPROPERTY_TOPOLOGY_NODES,
						&KsMultipleTopoNodes 
					);

    if (FAILED(hr))
    {
		_DbgPrintF(DEBUGLVL_VERBOSE,("[CKsFilter::EnumerateNodes] - Failed to get property KSPROPSETID_Topology.KSPROPERTY_TOPOLOGY_NODES"));
    }
    else
    {
        if (KsMultipleTopoNodes)
        {
            ULONG NumberOfNodes = KsMultipleTopoNodes->Count;

            // Point to immediately following KsMultipleTopoNodes
            GUID * NodeType = (GUID*)(KsMultipleTopoNodes + 1);

            // Iterate through all the nodes
            for(ULONG NodeId = 0; NodeId < NumberOfNodes; NodeId++)
            {
                CKsNode * Node = new CKsNode(NodeId, NodeType[NodeId], &hr);

                if (Node)
                {
                    if (SUCCEEDED(hr))
                    {
                        // Add the node to the list
                        if (NULL == m_ListNodes.AddTail(Node))
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    if (FAILED(hr))
                    {
                        delete Node;
                    }
                }
                // Don't need to set hr if we got a NULL result, as the check below will fail
                // and set hr to E_FAIL.
            }

            if (m_ListNodes.GetCount() != NumberOfNodes)
            {
                hr = E_FAIL;

                while (m_ListNodes.RemoveHead(&KsNode))
                {
                    delete KsNode;
                }
            }
        }
    }

	if (KsMultipleTopoNodes)
	{
		delete[] (BYTE*)KsMultipleTopoNodes;
	}

    return hr;
}

/*****************************************************************************
 * CKsFilter::NumberOfNodes()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
CKsFilter::
NumberOfNodes
(	void
)
{
	return m_ListNodes.GetCount();
}

/*****************************************************************************
 * CKsFilter::ParseNodes()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CKsFilter::
ParseNodes
(
	IN		ULONG		Index,
	OUT		CKsNode **	OutNode
)
{
	ULONG i = 0;

    CKsNode * KsNode = NULL;

    LISTPOS ListPos = m_ListNodes.GetHeadPosition();
    
	while (m_ListNodes.GetNext(ListPos, &KsNode) && (i <= Index))
    {
		if (i == Index)
		{
			*OutNode = (CKsNode*)KsNode;

			return TRUE;
		}

		i++;
    }

    return FALSE;
}

/*****************************************************************************
 * CKsFilter::GetSymbolicLink()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsFilter::
GetSymbolicLink
(
	OUT		LPTSTR *	OutSymbolicLink
)
{
	*OutSymbolicLink = m_SymbolicLink;

	return S_OK;
}

/*****************************************************************************
 * CKsFilter::GetFriendlyName()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CKsFilter::
GetFriendlyName
(
	OUT		LPTSTR *	OutFriendlyName
)
{
	*OutFriendlyName = m_FriendlyName;

	return S_OK;
}

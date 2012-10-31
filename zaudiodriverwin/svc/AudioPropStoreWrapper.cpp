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
/**
*******************************************************************************
 
@file       AudioPropStoreWrapper.cpp
 
@brief      Implementation of the Audio Service Property Store Wrapper
 
@author     Tan Kee Seng
 
@remarks    

 
Exception:
 
Created:    10/10/2006
 
$Date: 2007/07/20 08:20:52 $
 
$Revision: 1.1.2.1 $
 
 
*******************************************************************************
Copyright (C) Creative Technology, Ltd., 1999-2006. All rights reserved.
*******************************************************************************
*/

#include <float.h>

#include "AudioPropStoreWrapper.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
///////////////////////////////////////////////////////////////////////////////
CAudioPropStoreWrapper::CAudioPropStoreWrapper(void)
{
    m_pEndptID = NULL;
    m_pIPolicyConfigClient = NULL;
    m_nRefCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
CAudioPropStoreWrapper::~CAudioPropStoreWrapper(void)
{
    if(m_pIPolicyConfigClient)
    {
        m_pIPolicyConfigClient->Release();
        m_pIPolicyConfigClient = NULL;
    }

    if(m_pEndptID)
    {
        delete[] m_pEndptID;
        m_pEndptID = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
// IUnknown Implementation
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CAudioPropStoreWrapper::QueryInterface(
    IN REFIID riid ,
    OUT LPVOID* ppvObject)
{
    *ppvObject = NULL;

    if (riid == __uuidof(IUnknown))
    {
        *ppvObject = static_cast<PVOID>(this);
    }
    else if (riid == __uuidof(IAudioPropStore))
    {
        *ppvObject = static_cast<IAudioPropStore*>(this);
    }

    if (*ppvObject)
    {
        AddRef();
        return S_OK;
    }

    // Requested Interface not supported
    *ppvObject = NULL ;
    return E_NOINTERFACE ;
}//QueryInterface method

//////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) 
CAudioPropStoreWrapper::AddRef(VOID)
{
    return InterlockedIncrement(&m_nRefCount) ;
}

//////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) 
CAudioPropStoreWrapper::Release(VOID)
{
    LONG nRefCount=0;
    nRefCount = InterlockedDecrement(&m_nRefCount) ;

    if (nRefCount == 0)
    {
        delete this;
    }
    return nRefCount;
}

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CAudioPropStoreWrapper::GetValue(
    IN  REFPROPERTYKEY key,
    OUT PROPVARIANT *pv)
{
    return m_pIPolicyConfigClient->GetPropertyValue(m_pEndptID, m_storeType, key, pv);
}

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CAudioPropStoreWrapper::SetValue(
    IN  REFPROPERTYKEY key,
    IN  REFPROPVARIANT propvar)
{
    return m_pIPolicyConfigClient->SetPropertyValue(m_pEndptID, m_storeType, key, propvar);
}

///////////////////////////////////////////////////////////////////////////////
HRESULT
CAudioPropStoreWrapper::Init(
    IN  LPCWSTR              pEndptID,
    IN  IPolicyConfigClient* pIPolicyCfgClient,
    IN  UINT32               storeType)
{
    UINT32 lenStr = 0;
    HRESULT hRes = E_FAIL;

    lenStr = wcslen(pEndptID);
    m_pEndptID = new WCHAR[lenStr+1];
    if(m_pEndptID)
    {
        wcscpy(m_pEndptID, pEndptID);
        m_pIPolicyConfigClient = pIPolicyCfgClient;
        if(m_pIPolicyConfigClient)
        {
            m_pIPolicyConfigClient->AddRef();
            m_storeType = storeType;
            hRes = S_OK;
        }
    }

    return hRes;
}

///////////////////////////////////////////////////////////////////////////////

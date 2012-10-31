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
 
@file       AudioPropStoreWrapper.h
 
@brief      Definition of the Audio Service Property Store Wrapper
 
@author     Tan Kee Seng
 
@remarks    

 
Created:    10/10/2006
 
$Date: 2007/07/20 08:20:52 $
 
$Revision: 1.1.2.1 $
 
 
*******************************************************************************

*******************************************************************************
*/
#ifndef _AUDIOPROPSTOREWRAPPER_H_
#define _AUDIOPROPSTOREWRAPPER_H_

#include "AudioPropStore.h"
#include "PolicyConfigClientIDL.h"

class CAudioPropStoreWrapper :
    public IAudioPropStore
{
public:
    // constructor
    CAudioPropStoreWrapper();
    virtual ~CAudioPropStoreWrapper();    // destructor

public:
    // IUnknown Methods
    STDMETHOD(QueryInterface)( 
        IN REFIID riid , 
        OUT LPVOID* ppvObject);
    
    STDMETHOD_(ULONG, AddRef)(VOID);
    
    STDMETHOD_(ULONG, Release)(VOID);

    // IAudioPropStore
    STDMETHOD (GetValue)(
        IN  REFPROPERTYKEY key,
        OUT PROPVARIANT *pv);
    
    STDMETHOD (SetValue)(
        IN  REFPROPERTYKEY key,
        IN  REFPROPVARIANT propvar);

    HRESULT
    Init(
        IN  LPCWSTR              pEndptID,
        IN  IPolicyConfigClient* pIPolicyCfgClient,
        IN  UINT32               storeType);

private:
    LPWSTR                  m_pEndptID;
    IPolicyConfigClient*    m_pIPolicyConfigClient;
    UINT32                  m_storeType;
    LONG                    m_nRefCount;        // Reference count

};

#endif

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
 * @file       factory.cpp
 * @brief      ASIO class factory implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "factory.h"

#define STR_MODULENAME "Factory: "


/*****************************************************************************
 * CClassFactory::m_lServerLocks
 *****************************************************************************
 */
LONG CClassFactory::m_lServerLocks = 0;

/*****************************************************************************
 * CClassFactory::~CClassFactory()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CClassFactory::
~CClassFactory
(   void
)
{
    _DbgPrintF(DEBUGLVL_TERSE,("[CClassFactory::~CClassFactory]"));
}

/*****************************************************************************
 * CClassFactory::NonDelegatingQueryInterface()
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
 * Returns S_OK if the interface is found. Otherwise, returns E_NOINTERFACE.
 */
STDMETHODIMP_(HRESULT)
CClassFactory::
NonDelegatingQueryInterface
(
    IN		REFIID	Interface,
    OUT		PVOID * Object
)
{
    ASSERT(Object);

    if (IsEqualGUID(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    if (IsEqualGUID(Interface, IID_IClassFactory))
    {
        *Object = PVOID((IClassFactory*)(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

/*****************************************************************************
 * CClassFactory::CreateInstance()
 *****************************************************************************
 *//*!
 * @brief
 */
STDMETHODIMP
CClassFactory::
CreateInstance
(
	IN		PUNKNOWN	UnknownOuter,
	IN		REFIID		Interface,
	OUT		PVOID *		Object
)
{
    ASSERT(Object);

    HRESULT hr;

	CAsioDriver * AsioDriver = new CAsioDriver(UnknownOuter);

    if (AsioDriver)
    {
		AsioDriver->AddRef();

		hr = AsioDriver->Setup(m_ClassId);
		
		if (SUCCEEDED(hr))
		{
			hr = AsioDriver->QueryInterface(Interface, Object);
		}

		AsioDriver->Release();
    }
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

/*****************************************************************************
 * CClassFactory::LockServer()
 *****************************************************************************
 *//*!
 * @brief
 */
STDMETHODIMP
CClassFactory::
LockServer
(
	IN		BOOL	Lock
)
{
	if (Lock)
	{
		InterlockedIncrement(&m_lServerLocks);
	}
	else 
	{
		InterlockedDecrement(&m_lServerLocks);
	}

	return S_OK;
}

/*****************************************************************************
 * CClassFactory::Init()
 *****************************************************************************
 *//*!
 * @brief
 */
HRESULT 
CClassFactory::
Init
(
    IN      REFCLSID	RefClsId
)
{
	m_ClassId = RefClsId;

	return S_OK;
}

/*****************************************************************************
 * CClassFactory::IsLocked()
 *****************************************************************************
 *//*!
 * @brief
 */
BOOL 
CClassFactory::
IsLocked
(	void
) 
{
    return (m_lServerLocks > 0);
}

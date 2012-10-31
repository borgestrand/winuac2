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
 * @file       factory.h
 * @brief      ASIO class factory private definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _CLASS_FACTORY_H_
#define _CLASS_FACTORY_H_

#include "asiodll.h"
#include "asiodrv.h"

/*****************************************************************************
 * Constants
 */
// {C72A0D87-2FF8-491f-9507-AD0AC0CE5FF3}
//FIXME: If we are going to support 16-bit & 32-bit, then the CLSID below
//would be used as follow: the LS byte of CLSID.Data1 is used to store the
//bit depth, and all the code the register/unregister & class factory code
//would use that mechanism to determine what is the bit depth to use.
//DEFINE_GUID(CLSID_ASIO_DRIVER, 
//0xc72a0d87, 0x2ff8, 0x491f, 0x95, 0x7, 0xad, 0xa, 0xc0, 0xce, 0x5f, 0xf3);

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CClassFactory
 *****************************************************************************
 * @brief
 * ASIO class factory object.
 */
class CClassFactory
:	public IClassFactory,
	public CUnknown
{
private:
	GUID			m_ClassId;

	static LONG		m_lServerLocks;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CClassFactory);
    ~CClassFactory();

    /*****************************************************************************
     * IClassFactory implementation
     */
    STDMETHODIMP CreateInstance
	(
		IN		PUNKNOWN	UnknownOuter,
		IN		REFIID		Interface,
		OUT		PVOID *		Object
	);

    STDMETHODIMP LockServer
	(
		IN		BOOL	Lock
	);

    /*****************************************************************************
     * CClassFactory methods
     */
	HRESULT Init
	(
	    IN      REFCLSID	RefClsId
	);

	/*************************************************************************
     * Static
     */
    static 
	BOOL IsLocked
	(	void
	); 

	/*************************************************************************
     * Friends
     */
};

#endif // _CLASS_FACTORY_H_

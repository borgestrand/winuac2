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
 * @file       Factory.h
 * @brief      Device control filter factory private definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _CONTROL_FACTORY_PRIVATE_H_
#define _CONTROL_FACTORY_PRIVATE_H_

#include "IKsAdapter.h"

/*****************************************************************************
 * Defines
 */

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CControlFilterFactoryFactory
 *****************************************************************************
 * @brief
 * Device control filter.
 * @details
 * This object is associated with the device and is created when the device 
 * is started.  The class inherits CUnknown so it automatically gets reference 
 * counting and aggregation support.
 */
class CControlFilterFactory
:	public CUnknown
{
private:
    PKSDEVICE				m_KsDevice;				/*!< @brief The AVStream device we're associated with. */
    PKSADAPTER				m_KsAdapter;			/*!< @brief Pointer to the IKsAdapter interface. */

	PUSB_DEVICE				m_UsbDevice;			/*!< @brief Pointer to the USB device object. */

	/*************************************************************************
     * CControlFilterFactory methods
     */
	static
	VOID SetupFriendlyName
	(
		IN		PKSADAPTER				KsAdapter,
		IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
		IN		PWCHAR					RefString
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in FACTORY.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in FACTORY.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
	DEFINE_STD_CONSTRUCTOR(CControlFilterFactory);
    ~CControlFilterFactory();

    /*************************************************************************
     * CControlFilterFactory methods
     */
	NTSTATUS Init
	(
		IN		PKSDEVICE	KsDevice,
		IN		PVOID		Parameter1,
		IN		PVOID		Parameter2
	);

	NTSTATUS GetFilterDescription
	(
		OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
	);

	/*************************************************************************
     * Static
     */
	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	/*************************************************************************
     * Friends
     */
	friend
	NTSTATUS CreateControlFilterFactory
	(
		IN		PKSDEVICE	KsDevice,
		IN		PWCHAR		RefString,
		IN		PVOID		Parameter1,
		IN		PVOID		Parameter2
	);
};

#endif  //  _CONTROL_FACTORY_PRIVATE_H_

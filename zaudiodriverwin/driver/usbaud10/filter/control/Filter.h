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
 * @file       Filter.h
 * @brief      Device control filter private definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _CONTROL_FILTER_PRIVATE_H_
#define _CONTROL_FILTER_PRIVATE_H_

#include "IKsAdapter.h"

#include "PrvProp.h"

/*****************************************************************************
 * Defines
 */

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CControlFilter
 *****************************************************************************
 * @brief
 * Device control filter.
 * @details
 * This object is associated with the device and is created when the device 
 * is started.  The class inherits CUnknown so it automatically gets reference 
 * counting and aggregation support.
 */
class CControlFilter
:	public CUnknown
{
private:
    PKSFILTER				m_KsFilter;				/*!< @brief The AVStream filter we're associated with. */
    PKSADAPTER				m_KsAdapter;			/*!< @brief Pointer to the IKsAdapter interface. */

	PUSB_DEVICE				m_UsbDevice;			/*!< @brief Pointer to the USB device object. */

	/*************************************************************************
     * CControlFilter methods
     *
     * These are private member functions used internally by the object.  See
     * FILTER.CPP for specific descriptions.
     */

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in FILTER.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in FILTER.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
	DEFINE_STD_CONSTRUCTOR(CControlFilter);
    ~CControlFilter();

    /*************************************************************************
     * CControlFilter methods
     */
	NTSTATUS Init
	(
		IN		PKSFILTER	KsFilter
	);

	/*************************************************************************
     * Static
     */
    static
    NTSTATUS DispatchCreate 
	(
        IN		PKSFILTER	KsFilter,
		IN		PIRP		Irp
    );

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	static
	NTSTATUS GetPropertyHandler
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetPropertyHandler
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);
};

#endif  //  _CONTROL_FILTER_PRIVATE_H_

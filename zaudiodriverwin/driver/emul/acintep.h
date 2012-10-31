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
 * @file	   acintep.h
 * @brief	   AC interrupt endpoint definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __AC_INTERRUPT_ENDPOINT_H__
#define __AC_INTERRUPT_ENDPOINT_H__

#include "endpoint.h"

/*****************************************************************************
 * Classes
 */
class CUsbConfiguration;

/*****************************************************************************
 *//*! @class CAcInterruptEndpoint
 *****************************************************************************
 */
class CAcInterruptEndpoint
:	public CUsbEndpoint
{
private:
	CUsbConfiguration *			m_UsbConfiguration;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CAcInterruptEndpoint() : CUsbEndpoint() {}
    /*! @brief Destructor. */
	~CAcInterruptEndpoint();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAcInterruptEndpoint public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *				UsbDevice,
		IN		CUsbConfiguration *			UsbConfiguration,
		IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor,
		IN		PUSB_ENDPOINT_DESCRIPTOR	EndpointDescriptor
	);

    /*************************************************************************
	 * The other USB-Audio specification descriptions.
     */
	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CAcInterruptEndpoint>;
};

typedef CAcInterruptEndpoint * PAC_INTERRUPT_ENDPOINT;

#endif // __AC_INTERRUPT_ENDPOINT_H__

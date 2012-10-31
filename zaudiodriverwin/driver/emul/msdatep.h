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
 * @file	   msdatep.h
 * @brief	   MS bulk data endpoint definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __MS_BULK_DATA_ENDPOINT_H__
#define __MS_BULK_DATA_ENDPOINT_H__

#include "endpoint.h"

/*****************************************************************************
 * Classes
 */
class CUsbConfiguration;

/*****************************************************************************
 *//*! @class CMsBulkDataEndpoint
 *****************************************************************************
 */
class CMsBulkDataEndpoint
:	public CUsbEndpoint
{
private:
	CUsbConfiguration *								m_UsbConfiguration;
	PUSB_AUDIO_10_CS_MS_DATA_ENDPOINT_DESCRIPTOR	m_CsMsEndpointDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMsBulkDataEndpoint() : CUsbEndpoint() {}
    /*! @brief Destructor. */
	~CMsBulkDataEndpoint();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMsBulkDataEndpoint public methods
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
	friend class CList<CMsBulkDataEndpoint>;
};

typedef CMsBulkDataEndpoint * PMS_BULK_DATA_ENDPOINT;

#endif // __MS_BULK_DATA_ENDPOINT_H__

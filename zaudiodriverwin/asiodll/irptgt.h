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
 * @file       irptgt.h
 * @brief      This is the header file for the C++ base class which abstracts
 *			   common functionality in CKsFilter and CKsPin classes.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_IRP_TARGET_H_
#define _KS_IRP_TARGET_H_

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CKsIrpTarget
 *****************************************************************************
 * @brief
 * This is a base class for controlling Ks file objects, i.e. filters and pins
 * CKsFilter and CKsPin derive from this class.
 */
class CKsIrpTarget
{
protected:
    HANDLE		m_Handle;     // Allocated handle of the filter instance.

public:
    // Support
    HRESULT PropertySetSupport
	(
        IN		REFGUID	PropertySet
	);

    HRESULT PropertyBasicSupport
	(
        IN		REFGUID	PropertySet,
        IN		ULONG   PropertyId,
        OUT		ULONG *	OutSupport
	);

    // GET
    HRESULT GetPropertySimple
	(
        IN		REFGUID	PropertySet,
        IN		ULONG   PropertyId,
        OUT		PVOID   Value,
        IN		ULONG   ValueSize,
        IN		PVOID	Instance = NULL		OPTIONAL,
        IN		ULONG	InstanceSize = 0	OPTIONAL
	);
    
    HRESULT GetPropertyMulti
	(
        IN		REFGUID				PropertySet,
        IN		ULONG				PropertyId,
        OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem
	);
        
    HRESULT GetPropertyMulti
	(
        IN		REFGUID				PropertySet,
        IN		ULONG				PropertyId,
        IN		PVOID				Data,
        IN		ULONG				DataSize,
        OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem
	);
            
    HRESULT GetNodePropertySimple
	(
        IN		ULONG	NodeId,
        IN		REFGUID	PropertySet,
        IN		ULONG	PropertyId,
        OUT		PVOID	Value,
        IN		ULONG	ValueSize,
        IN		PVOID	Instance = NULL		OPTIONAL,
        IN		ULONG	InstanceSize = 0	OPTIONAL
	);

    HRESULT GetNodePropertyChannel
	(
        IN		ULONG	NodeId,
        IN		ULONG	Channel,
        IN		REFGUID	PropertySet,
        IN		ULONG	PropertyId,
        OUT		PVOID	Value,
        IN		ULONG	ValueSize,
        IN		PVOID	Instance = NULL		OPTIONAL,
        IN		ULONG	InstanceSize = 0	OPTIONAL
	);

    // Set
    HRESULT SetPropertySimple
	(
        IN		REFGUID	PropertySet,
        IN		ULONG	PropertyId,
        IN		PVOID	Value,
        IN		ULONG	ValueSize,
        IN		PVOID	Instance = NULL		OPTIONAL,
        IN		ULONG	InstanceSize = 0	OPTIONAL
	);
        
    HRESULT SetPropertyMulti
	(
        IN		REFGUID				PropertySet,
        IN		ULONG				PropertyId,
        OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem
	);
    
    HRESULT SetNodePropertySimple
	(
        IN		ULONG	NodeId,
        IN		REFGUID	PropertySet,
        IN		ULONG	PropertyId,
        IN		PVOID	Value,
        IN		ULONG	ValueSize,
        IN		PVOID	Instance = NULL		OPTIONAL,
        IN		ULONG	InstanceSize = 0	OPTIONAL
	);

    HRESULT SetNodePropertyChannel
	(
        IN		ULONG	NodeId,
        IN		ULONG	Channel,
        IN		REFGUID	PropertySet,
        IN		ULONG	PropertyId,
        IN		PVOID	Value,
        IN		ULONG	ValueSize,
        IN		PVOID	Instance = NULL		OPTIONAL,
        IN		ULONG	InstanceSize = 0	OPTIONAL
	);

	// Events
	HRESULT EnableEvent
	(
		IN		REFGUID			EventSet,
		IN		ULONG			EventId,
		IN		PKSEVENTDATA	EventData,
		IN		ULONG			EventDataSize
	);

	HRESULT EnableNodeEvent
	(
		IN		ULONG			NodeId,
		IN		REFGUID			EventSet,
		IN		ULONG			EventId,
		IN		PKSEVENTDATA	EventData,
		IN		ULONG			EventDataSize
	);

	HRESULT DisableEvent
	(
		IN		PKSEVENTDATA	EventData,
		IN		ULONG			EventDataSize
	);

public:
	/*************************************************************************
     * Constructor
     */  
    CKsIrpTarget
	(
		IN		HANDLE	Handle
	);

	/*************************************************************************
     * CKsIrpTarget methods
     */  
	HANDLE GetHandle
	(	void
	);
	
	BOOL Close
	(	void
	);

	/*************************************************************************
     * Static
     */  
    static 
	HRESULT SynchronizedIoctl
	(
		IN		HANDLE	Handle,
		IN		ULONG	CtlCode,
		IN		PVOID	InputBuffer,
		IN		ULONG	InputBufferSize,
		OUT		PVOID	OutputBuffer,
		OUT		ULONG	OutputBufferSize,
		OUT		ULONG *	OutBytesReturned
	);

	static 
	BOOL SafeCloseHandle
	(
		IN OUT	HANDLE&	Handle
	);

	static 
	BOOL IsValidHandle
	(
		IN		HANDLE	Handle
	);
};

#endif //_KS_IRP_TARGET_H_

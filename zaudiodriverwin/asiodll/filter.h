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
 * @file       filter.h
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS filters.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_FILTER_H_
#define _KS_FILTER_H_


/*****************************************************************************
 * Classes
 */
class CKsPin;
class CKsNode;

/*****************************************************************************
 *//*! @class CKsFilter
 *****************************************************************************
 * @brief
 * This is the base class for classes that proxy KS filters from user mode.
 * @details
 * Basic usage is:
 * Instantiate a CKsFilter (or derived class):
 * - Call Instantiate, which creates a file object (instantiates the KS filter).
 * whose handle is stored as m_Handle.
 * - Call EnumeratePins, EnumerateNodes, to deduce the filter's topology.
 * - Call CKsIrpTarget functions to get/set properties.
 */
class CKsFilter 
:	public	CKsIrpTarget
{
private:
    TCHAR		m_SymbolicLink[MAX_PATH];   // Filter Path
	
	TCHAR		m_FriendlyName[MAX_PATH];
    
	/*************************************************************************
     * CKsFilter priavate methods
     */  
    virtual 
	HRESULT DestroyLists
	(	void
	);

    HRESULT InternalInit
	(	void
	);

protected:
    // Lists of nodes
    TList<CKsNode>      m_ListNodes;

     // This is the "ROOT LIST" for all pins
    TList<CKsPin>		m_ListPins;

    // These lists only contain copies of the pointers in m_listPins.
    // Don't delete the memory that they point
    TList<CKsPin>		m_ListRenderSinkPins;
    TList<CKsPin>		m_ListRenderSourcePins;
    TList<CKsPin>		m_ListCaptureSinkPins;
    TList<CKsPin>		m_ListCaptureSourcePins;
    TList<CKsPin>		m_ListNoCommPins;

    KS_TECHNOLOGY_TYPE	m_FilterType;

	/*************************************************************************
     * CKsFilter protected methods
     */  
    HRESULT ClassifyPins
	(	void
	);

public:
	/*************************************************************************
     * Constructor/destructor.
     */  
    CKsFilter
	(
        IN		LPCTSTR		SymbolicLink,
        IN		LPCTSTR		FriendlyName,
        OUT		HRESULT *	OutHResult
	);

    virtual ~CKsFilter
	(	void
	);
    
	/*************************************************************************
     * CKsFilter public methods
     */  
	virtual 
	HRESULT Instantiate
	(	void
	);
    
	virtual 
	HRESULT EnumerateNodes
	(	void
	);
    
	virtual 
	HRESULT EnumeratePins
	(	void
	);

	ULONG NumberOfNodes
	(	void
	);

	BOOL ParseNodes
	(
		IN		ULONG		Index,
		OUT		CKsNode **	OutNode
	);

	HRESULT GetSymbolicLink
	(
		OUT		LPTSTR *	OutGetSymbolicLink
	);

	HRESULT GetFriendlyName
	(
		OUT		LPTSTR *	OutFriendlyName
	);

	HRESULT GetPinPropertySimple
	(
        IN		ULONG	PinId,
        IN		REFGUID PropertySet,
        IN		ULONG   PropertyId,
        OUT		PVOID   Value,
        IN		ULONG   ValueSize
	);

    HRESULT GetPinPropertyMulti
	(
        IN		ULONG				PinId,
        IN		REFGUID				PropertySet,
        IN		ULONG				PropertyId,
        OUT		PKSMULTIPLE_ITEM *	OutKsMultipleItem	OPTIONAL
	);

	/*************************************************************************
     * Friends
     */  
	friend class CKsPin;
	friend class CKsAudPin;
	friend class CKsNode;
};

#endif //_KS_FILTER_H_

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
 * @file       enum.h
 * @brief      This is the header file for the C++ class which encapsulates
 *			   the SetupDi functions needed to enumerate KS filters.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_ENUM_H_
#define _KS_ENUM_H_

#include "ksaudio.h"

/*****************************************************************************
 * Classes
 */
class CKsFilter;

/*****************************************************************************
 *//*! @class CKsEnumerator
 *****************************************************************************
 * @brief
 * This is a utility class used for enumerating KS filters.
 * @details
 * Basic usage is:
 * - Instantiate a CKsEnumerator.
 * - Call EnumFilters, which enumerates a list of filters.
 * - Pick a filter to use from m_ListFilters.
 * - Either call RemoveFilterFromList to remove the filter from the list,
 * or duplicate the filter and use the duplicate copy.
 */
class CKsEnumerator
{
public:
	TList<CKsFilter>	m_ListFilters; // This is the list of the filters

public:
	/*****************************************************************************
	 * Constructor/destructor
	 */
    CKsEnumerator
	(
        OUT		HRESULT *	OutHResult
	);

    virtual ~CKsEnumerator
	(	void
	);

	/*****************************************************************************
	 * CKsEnumerator public methods
	 */
    virtual 
	VOID DestroyLists
	(	void
	);   

    HRESULT EnumerateFilters
	(   
        IN		KS_TECHNOLOGY_TYPE	FilterType,
        IN		GUID *				Categories,
        IN		ULONG				CategoriesCount,
        IN		BOOL				NeedPins,			// = TRUE // Enumerates devices for sysaudio.
        IN		BOOL				NeedNodes,			// = TRUE
        IN		BOOL				Instantiate			// = TRUE // Should we instantiate.
	);

    HRESULT RemoveFilterFromList
	(
        IN		CKsFilter *	KsFilter
	);
};

#endif // _KS_ENUM_H_

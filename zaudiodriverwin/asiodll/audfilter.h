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
 * @file       audfilter.h
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS filters that support PCM audio formats.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_AUDIO_FILTER_H_
#define _KS_AUDIO_FILTER_H_

#include "ksaudio.h"

/*****************************************************************************
 * Classes
 */
class CKsAudioPin;
class CKsAudioRenderPin;
class CKsAudioCapturePin;

/*****************************************************************************
 *//*! @class CKsAudioFilter
 *****************************************************************************
 * @brief
 * This is the base class for an audio filter.
 */
class CKsAudioFilter 
:	public	CKsFilter
{
protected:
	/*****************************************************************************
	 * Constructor
	 */
    CKsAudioFilter
    (
        IN		LPCTSTR		SymbolicLink,
        IN		LPCTSTR		FriendlyName,
        OUT		HRESULT *	OutHResult
    );

public:
    
	/*****************************************************************************
	 * CKsAudioFilter public methods
	 */
	virtual 
	HRESULT EnumeratePins
	(	void
	);
    
};

/*****************************************************************************
 *//*! @class CKsAudioRenderFilter
 *****************************************************************************
 * @brief
 * This is the base class for an audio render filter.
 */
class CKsAudioRenderFilter 
:	public	CKsAudioFilter
{
private:

public:
	/*****************************************************************************
	 * Constructor
	 */
    CKsAudioRenderFilter
    (
        IN		LPCTSTR		SymbolicLink,
        IN		LPCTSTR		FriendlyName,
        OUT		HRESULT *	OutHResult
    );

	/*****************************************************************************
	 * CKsAudioRenderFilter public methods
	 */
	ULONG NumberOfRenderPins
	(	void
	);

	BOOL ParseRenderPins
	(
		IN		ULONG					Index,
		OUT		CKsAudioRenderPin **	OutAudioRenderPin
	);

    CKsAudioRenderPin * FindViablePin
	(
		IN		PWAVEFORMATEX	WaveFormatEx,
		IN		ULONG			MatchingCriteria = KS_FORMAT_MATCHING_CRITERIA_DEFAULT
	);

	CKsAudioRenderPin * CreateRenderPin
	(
		IN		PWAVEFORMATEX	WaveFormatEx,
		IN		BOOL			Looped
	);
    
	BOOL CanCreateRenderPin
	(
		IN		PWAVEFORMATEX	WaveFormatEx
	);
};

/*****************************************************************************
 *//*! @class CKsAudioCaptureFilter
 *****************************************************************************
 * @brief
 * This is the base class for an audio capture filter.
 */
class CKsAudioCaptureFilter 
:	public	CKsAudioFilter
{
private:

public:
	/*****************************************************************************
	 * Constructor
	 */
    CKsAudioCaptureFilter
    (
        IN		LPCTSTR		SymbolicLink,
        IN		LPCTSTR		FriendlyName,
        OUT		HRESULT *	OutHResult
    );

	/*****************************************************************************
	 * CKsAudioCaptureFilter public methods
	 */
	ULONG NumberOfCapturePins
	(	void
	);

	BOOL ParseCapturePins
	(
		IN		ULONG					Index,
		OUT		CKsAudioCapturePin **	OutAudioCapturePin
	);

	CKsAudioCapturePin * FindViablePin
	(
		IN		PWAVEFORMATEX	WaveFormatEx,
		IN		ULONG			MatchingCriteria = KS_FORMAT_MATCHING_CRITERIA_DEFAULT
	);

	HRESULT GetCapturePin
	(
		OUT		CKsAudioCapturePin **	OutCapturePin
	);

    CKsAudioCapturePin * CreateCapturePin
	(
		IN		PWAVEFORMATEX	WaveFormatEx,
		IN		BOOL			Looped
	);
};

#endif //_KS_AUDIO_FILTER_H_

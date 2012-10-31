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
 * @file       audpin.h
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS pins that support PCM audio formats.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_AUDIO_PIN_H_
#define _KS_AUDIO_PIN_H_

#include "ksaudio.h"

/*****************************************************************************
 * Classes
 */
class CKsNode;
class CKsAudioFilter;

/*****************************************************************************
 *//*! @class CKsAudioPin
 *****************************************************************************
 * @brief
 * This is the base class for an audio pin.
 */
class CKsAudioPin 
:	public	CKsPin
{
private:
    TList<CKsNode>              m_ListNodes;

    PWAVEFORMATEX				m_WaveFormatEx;
    PKSDATAFORMAT_WAVEFORMATEX  m_KsDataFormatWfx; // Just a reference into m_KsPinConnect

	/*****************************************************************************
	 * CKsAudioPin private methods
	 */
    HRESULT Init
	(	void
	);
 
    CKsNode * GetNode
	(
		IN		ULONG	NodeId
	);

protected:
    CKsAudioFilter *	m_KsAudioFilter;

public: 
    TList<KSDATARANGE_AUDIO>	m_ListDataRanges;

	/*****************************************************************************
	 * Constructor/destructor
	 */
    CKsAudioPin
	(
		IN		CKsAudioFilter *	KsAudioFilter, 
		IN		ULONG				PinId, 
		OUT		HRESULT *			OutHResult
	);

    virtual ~CKsAudioPin
	(	void
	);

	/*****************************************************************************
	 * CKsAudioPin public methods
	 */
    HRESULT SetFormat
	(
		IN		PWAVEFORMATEX	WaveFormatEx
	);
    
	const 
	PWAVEFORMATEX GetFormat
	(	void
	) 
	{
		return m_WaveFormatEx;
	}

    HRESULT GetPosition
	(
		OUT		KSAUDIO_POSITION *	OutPosition
	);
    
	HRESULT SetPosition
	(
		IN		PKSAUDIO_POSITION	Position
	);

    BOOL IsFormatSupported
	(
		IN		PWAVEFORMATEX	WaveFormatEx,
		IN		ULONG			MatchingCriteria = KS_FORMAT_MATCHING_CRITERIA_DEFAULT
	);

	BOOL ParseDataRanges
	(
		IN		ULONG					Index,
		OUT		PKSDATARANGE_AUDIO *	OutDataRangeAudio
	);
};

/*****************************************************************************
 *//*! @class CKsAudioRenderPin
 *****************************************************************************
 * @brief
 * This is the base class for an audio render pin.
 */
class CKsAudioRenderPin 
:	public	CKsAudioPin
{
public:
	/*****************************************************************************
	 * Constructor/destructor
	 */
    CKsAudioRenderPin
	(
		IN		CKsAudioFilter *	KsAudioFilter, 
		IN		ULONG				PinId, 
		OUT		HRESULT *			OutHResult
	);

    virtual ~CKsAudioRenderPin
	(	void
	);

};

/*****************************************************************************
 *//*! @class CKsAudioCapturePin
 *****************************************************************************
 * @brief
 * This is the base class for an audio capture pin.
 */
class CKsAudioCapturePin 
:	public	CKsAudioPin
{
public:
	/*****************************************************************************
	 * Constructor/destructor
	 */
    CKsAudioCapturePin
	(
		IN		CKsAudioFilter *	KsAudioFilter, 
		IN		ULONG				PinId, 
		OUT		HRESULT *			OutHResult
	);

    virtual ~CKsAudioCapturePin
	(	void
	);
};

#endif // _KS_AUDIO_PIN_H_

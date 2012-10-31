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
*//*
 *****************************************************************************
 *//*!
 * @file       Formats.h
 * @brief      Wave formats definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-16-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _FORMATS_PRIVATE_H_
#define _FORMATS_PRIVATE_H_

/*****************************************************************************
 * Defines.
 */
/*! @brief AC3 wave format tag. */
#define WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092
//@{
/*! @brief AC3 format GUID definition. */
#define STATIC_KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF\
	DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_DOLBY_AC3_SPDIF)
DEFINE_GUIDSTRUCT("00000092-0000-0010-8000-00aa00389b71",KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF);
#define KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_DOLBY_AC3_SPDIF)
//@}

/*! @brief WMA-over-S/PDIF format tag. */
#define WAVE_FORMAT_WMA_OVER_SPDIF  0x0164
//@{
/*! @brief WMA-over-S/PDIF format GUID definition. */
#define STATIC_KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF\
    DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_WMA_OVER_SPDIF)
DEFINE_GUIDSTRUCT("00000164-0000-0010-8000-00aa00389b71",KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF);
#define KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_WMA_OVER_SPDIF)
//@}

/*! @brief Wave format extensible tag. */
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
//@{
/*! @brief Wave format extensible GUID definition. */
#define STATIC_KSDATAFORMAT_SUBTYPE_EXTENSIBLE\
	DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_EXTENSIBLE)
DEFINE_GUIDSTRUCT("0000fffe-0000-0010-8000-00aa00389b71",KSDATAFORMAT_SUBTYPE_EXTENSIBLE);
#define KSDATAFORMAT_SUBTYPE_EXTENSIBLE DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_EXTENSIBLE)
//@}

#define KSDATAFORMAT_SUBTYPE_UNKNOWN	KSDATAFORMAT_SUBTYPE_WAVEFORMATEX

#endif // _FORMATS_PRIVATE_H_

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
 * @file	   Convert.h
 * @brief	   This file defines the copy-conversion routines used to convert
 *			   from one format to another.
 *
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __CONVERT_H__
#define __CONVERT_H__

// TODO: Optimize these routines... Unrolling the loop will probably be 
// good enough...

/*****************************************************************************
 * Copy32_16()
 *****************************************************************************
 * @brief
 * Convert 32-bit samples to 16-bit samples.
 */
static 
VOID 
Copy32_16
(
	IN		PSHORT	Dst, 
	IN		PLONG	Src, 
	IN		ULONG	NumberOfSamples
)
{
	for (ULONG i=0; i<NumberOfSamples; i++)
	{
		Dst[i] = (SHORT)(Src[i]>>16);
	}
}

/*****************************************************************************
 * Copy32_24()
 *****************************************************************************
 * @brief
 * Convert 32-bit samples to 24-bit samples.
 */
static 
VOID 
Copy32_24
(
	IN		PUCHAR	Dst, 
	IN		PLONG	Src, 
	IN		ULONG	NumberOfSamples
)
{
	for (ULONG i=0; i<NumberOfSamples; i++)
	{
		Dst[0] = UCHAR((Src[i]&0x0000FF00)>>8);
		Dst[1] = UCHAR((Src[i]&0x00FF0000)>>16);
		Dst[2] = UCHAR((Src[i]>>24));

		Dst+=3;
	}
}

/*****************************************************************************
 * Copy24_16()
 *****************************************************************************
 * @brief
 * Convert 24-bit samples to 16-bit samples.
 */
static 
VOID 
Copy24_16
(
	IN		PSHORT	Dst, 
	IN		PUCHAR	Src, 
	IN		ULONG	NumberOfSamples
)
{
	for (ULONG i=0; i<NumberOfSamples; i++)
	{
		Dst[i] = (SHORT(Src[2])<<8) | Src[1];

		Src+=3;
	}
}

/*****************************************************************************
 * Copy24_32()
 *****************************************************************************
 * @brief
 * Convert 24-bit samples to 32-bit samples.
 */
static 
VOID 
Copy24_32
(
	IN		PLONG	Dst, 
	IN		PUCHAR	Src, 
	IN		ULONG	NumberOfSamples
)
{
	for (ULONG i=0; i<NumberOfSamples; i++)
	{
		Dst[i] = (LONG(Src[2])<<24) | (LONG(Src[1])<<16) | (LONG(Src[0])<<8);

		Src+=3;
	}
}

/*****************************************************************************
 * Copy16_24()
 *****************************************************************************
 * @brief
 * Convert 16-bit samples to 24-bit samples.
 */
static 
VOID 
Copy16_24
(
	IN		PUCHAR	Dst, 
	IN		PSHORT	Src, 
	IN		ULONG	NumberOfSamples
)
{
	for (ULONG i=0; i<NumberOfSamples; i++)
	{
		Dst[0] = 0;
		Dst[1] = UCHAR(Src[i]&0xFF);
		Dst[2] = UCHAR(Src[i]>>8);

		Dst+=3;
	}
}

/*****************************************************************************
 * Copy16_32()
 *****************************************************************************
 * @brief
 * Convert 16-bit samples to 32-bit samples.
 */
static 
VOID 
Copy16_32
(
	IN		PLONG	Dst, 
	IN		PSHORT	Src, 
	IN		ULONG	NumberOfSamples
)
{
	for (ULONG i=0; i<NumberOfSamples; i++)
	{
		Dst[i] = LONG(Src[i])<<16;
	}
}

/*****************************************************************************
 * FindConversionRoutine()
 *****************************************************************************
 * @brief
 * Find a conversion routine that converts from one bit depth to another.
 */
static
AUDIO_CONVERSION_ROUTINE
FindConversionRoutine
(
	IN		ULONG	FromBitPerSample,
	IN		ULONG	ToBitPerSample
)
{
	//DbgPrint("Converter: %d -> %d bits\n", FromBitPerSample, ToBitPerSample);

	AUDIO_CONVERSION_ROUTINE ConversionRoutine = NULL;

	switch (ToBitPerSample)
	{
		case 16:
		{
			switch (FromBitPerSample)
			{
				case 24:
				{
					ConversionRoutine = (AUDIO_CONVERSION_ROUTINE)Copy24_16;
				}
				break;

				case 32:
				{
					ConversionRoutine = (AUDIO_CONVERSION_ROUTINE)Copy32_16;
				}
				break;
			}
		}
		break;
		
		case 24:
		{
			switch (FromBitPerSample)
			{
				case 16:
				{
					ConversionRoutine = (AUDIO_CONVERSION_ROUTINE)Copy16_24;
				}
				break;

				case 32:
				{
					ConversionRoutine = (AUDIO_CONVERSION_ROUTINE)Copy32_24;
				}
				break;
			}
		}
		break;

		case 32:
		{
			switch (FromBitPerSample)
			{
				case 16:
				{
					ConversionRoutine = (AUDIO_CONVERSION_ROUTINE)Copy16_32;
				}
				break;

				case 24:
				{
					ConversionRoutine = (AUDIO_CONVERSION_ROUTINE)Copy24_32;
				}
				break;
			}
		}
		break;
	}

	return ConversionRoutine;
}

#endif // __CONVERT_H__

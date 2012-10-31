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
 * @file       ksaudio.h
 * @brief      KS audio header files & definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_AUDIO_H_
#define _KS_AUDIO_H_

#ifdef __cplusplus
// WDM.H does not play well with C++.
extern "C"
{
#include <wdm.h>
}
#else
#include <wdm.h>
#endif

#include <windef.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <stdunk.h>
#include <ks.h>
#include <ksmedia.h>
#include <punknown.h>
#include <drmk.h>

DEFINE_GUID(GUID_DMUS_PROP_GM_Hardware, 0x178f2f24, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_GS_Hardware, 0x178f2f25, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_XG_Hardware, 0x178f2f26, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_XG_Capable,  0x6496aba1, 0x61b0, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(GUID_DMUS_PROP_GS_Capable,  0x6496aba2, 0x61b0, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(GUID_DMUS_PROP_DLS1,        0x178f2f27, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_DLS2,        0xf14599e5, 0x4689, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);

//
// Stuff that are in Vista WDK that are not in the older DDKs.
//

// USB-MIDI nodes
#ifndef STATIC_KSNODETYPE_MIDI_JACK
#define STATIC_KSNODETYPE_MIDI_JACK\
    0x265e0c3f, 0xfa39, 0x4df3, 0xab, 0x04, 0xbe, 0x01, 0xb9, 0x1e, 0x29, 0x9a
DEFINE_GUIDSTRUCT("265E0C3F-FA39-4df3-AB04-BE01B91E299A", KSNODETYPE_MIDI_JACK);
#define KSNODETYPE_MIDI_JACK DEFINE_GUIDNAMED(KSNODETYPE_MIDI_JACK)
#endif // STATIC_KSNODETYPE_MIDI_JACK

#ifndef STATIC_KSNODETYPE_MIDI_ELEMENT
#define STATIC_KSNODETYPE_MIDI_ELEMENT\
    0x01c6fe66, 0x6e48, 0x4c65, 0xac, 0x9b, 0x52, 0xdb, 0x5d, 0x65, 0x6c, 0x7e
DEFINE_GUIDSTRUCT("01C6FE66-6E48-4c65-AC9B-52DB5D656C7E", KSNODETYPE_MIDI_ELEMENT);
#define KSNODETYPE_MIDI_ELEMENT DEFINE_GUIDNAMED(KSNODETYPE_MIDI_ELEMENT)
#endif // STATIC_KSNODETYPE_MIDI_ELEMENT

#ifndef STATIC_KSNODETYPE_PARAMETRIC_EQUALIZER
#define STATIC_KSNODETYPE_PARAMETRIC_EQUALIZER\
    0x19bb3a6a, 0xce2b, 0x4442, 0x87, 0xec, 0x67, 0x27, 0xc3, 0xca, 0xb4, 0x77
DEFINE_GUIDSTRUCT("19BB3A6A-CE2B-4442-87EC-6727C3CAB477", KSNODETYPE_PARAMETRIC_EQUALIZER);
#define KSNODETYPE_PARAMETRIC_EQUALIZER DEFINE_GUIDNAMED(KSNODETYPE_PARAMETRIC_EQUALIZER)
#endif // STATIC_KSNODETYPE_PARAMETRIC_EQUALIZER

#ifndef STATIC_KSNODETYPE_UPDOWN_MIX
#define STATIC_KSNODETYPE_UPDOWN_MIX\
    0xb7edc5cf, 0x7b63, 0x4ee2, 0xa1, 0x0, 0x29, 0xee, 0x2c, 0xb6, 0xb2, 0xde
DEFINE_GUIDSTRUCT("B7EDC5CF-7B63-4ee2-A100-29EE2CB6B2DE", KSNODETYPE_UPDOWN_MIX);
#define KSNODETYPE_UPDOWN_MIX DEFINE_GUIDNAMED(KSNODETYPE_UPDOWN_MIX)
#endif // STATIC_KSNODETYPE_UPDOWN_MIX

#ifndef STATIC_KSNODETYPE_DYN_RANGE_COMPRESSOR
#define STATIC_KSNODETYPE_DYN_RANGE_COMPRESSOR\
    0x8c8a6a8, 0x601f, 0x4af8, 0x87, 0x93, 0xd9, 0x5, 0xff, 0x4c, 0xa9, 0x7d
DEFINE_GUIDSTRUCT("08C8A6A8-601F-4af8-8793-D905FF4CA97D", KSNODETYPE_DYN_RANGE_COMPRESSOR);
#define KSNODETYPE_DYN_RANGE_COMPRESSOR DEFINE_GUIDNAMED(KSNODETYPE_DYN_RANGE_COMPRESSOR)
#endif // STATIC_KSNODETYPE_DYN_RANGE_COMPRESSOR

#if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_LONGHORN)
enum 
{
    KSPROPERTY_AUDIO_PEQ_MAX_BANDS = KSPROPERTY_AUDIO_PREFERRED_STATUS+1,
    KSPROPERTY_AUDIO_PEQ_NUM_BANDS,
    KSPROPERTY_AUDIO_PEQ_BAND_CENTER_FREQ,
    KSPROPERTY_AUDIO_PEQ_BAND_Q_FACTOR,
    KSPROPERTY_AUDIO_PEQ_BAND_LEVEL,
    KSPROPERTY_AUDIO_CHORUS_MODULATION_RATE,
    KSPROPERTY_AUDIO_CHORUS_MODULATION_DEPTH,
    KSPROPERTY_AUDIO_REVERB_TIME,
    KSPROPERTY_AUDIO_REVERB_DELAY_FEEDBACK
};
#endif // !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_LONGHORN)

#endif // _KS_AUDIO_H_
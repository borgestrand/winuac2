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
/**********************************************************

  usbaudio.h
  
  History:
    6/10/2001 - created (av)
    
  Description:
    declarations and definitions for USB Audio Device class
 
**********************************************************/

#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__

#include <pshpack1.h>

// ---------------- audio device class codes -------------------

// audio interface class code
#define USB_CLASS_CODE_AUDIO								0x01

// audio interface subclass codes
#define USB_AUDIO_SUBCLASS_UNDEFINED						0x00
#define USB_AUDIO_SUBCLASS_AUDIOCONTROL						0x01
#define USB_AUDIO_SUBCLASS_AUDIOSTREAMING					0x02
#define USB_AUDIO_SUBCLASS_MIDISTREAMING					0x03

// audio interface protocol codes
#define USB_AUDIO_PROTOCOL_UNDEFINED						0x00

// audio class-specific descriptor types
#define USB_AUDIO_CS_UNDEFINED								0x20
#define USB_AUDIO_CS_DEVICE									0x21
#define USB_AUDIO_CS_CONFIGURATION							0x22
#define USB_AUDIO_CS_STRING									0x23
#define USB_AUDIO_CS_INTERFACE								0x24
#define USB_AUDIO_CS_ENDPOINT								0x25

// audio class-specific Audio Control interface descriptor subtypes
#define USB_AUDIO_AC_DESCRIPTOR_UNDEFINED					0x00
#define USB_AUDIO_AC_DESCRIPTOR_HEADER						0x01
#define USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL				0x02
#define USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL				0x03
#define USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT					0x04
#define USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT				0x05
#define USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT				0x06
#define USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT				0x07
#define USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT				0x08

// audio class-specific Audio Streaming interface descriptor subtypes
#define USB_AUDIO_AS_DESCRIPTOR_UNDEFINED					0x00
#define USB_AUDIO_AS_DESCRIPTOR_GENERAL						0x01
#define USB_AUDIO_AS_DESCRIPTOR_FORMAT_TYPE					0x02
#define USB_AUDIO_AS_DESCRIPTOR_FORMAT_SPECIFIC				0x03

// audio class-specific MIDI Streaming interface descriptor subtypes
#define USB_AUDIO_MS_DESCRIPTOR_UNDEFINED					0x00
#define USB_AUDIO_MS_DESCRIPTOR_HEADER						0x01
#define USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK				0x02
#define USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK				0x03
#define USB_AUDIO_MS_DESCRIPTOR_ELEMENT						0x04

// audio class-specific endpoint descriptor subtypes
#define USB_AUDIO_EP_DESCRIPTOR_UNDEFINED					0x00
#define USB_AUDIO_EP_DESCRIPTOR_GENERAL						0x01

// processing unit process types
#define USB_AUDIO_PROCESS_UNDEFINED							0x00
#define USB_AUDIO_PROCESS_UPMIX_DOWNMIX						0x01
#define USB_AUDIO_PROCESS_DOLBY_PROLOGIC					0x02
#define USB_AUDIO_PROCESS_3D_STEREO_EXTENDER				0x03
#define USB_AUDIO_PROCESS_REVERBERATION						0x04
#define USB_AUDIO_PROCESS_CHORUS							0x05
#define USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION			0x06

// audio class-specific request codes
#define USB_AUDIO_REQUEST_UNDEFINED							0x00
#define USB_AUDIO_REQUEST_SET_CUR							0x01
#define USB_AUDIO_REQUEST_GET_CUR							0x81
#define USB_AUDIO_REQUEST_SET_MIN							0x02
#define USB_AUDIO_REQUEST_GET_MIN							0x82
#define USB_AUDIO_REQUEST_SET_MAX							0x03
#define USB_AUDIO_REQUEST_GET_MAX							0x83
#define USB_AUDIO_REQUEST_SET_RES							0x04
#define USB_AUDIO_REQUEST_GET_RES							0x84
#define USB_AUDIO_REQUEST_SET_MEM							0x05
#define USB_AUDIO_REQUEST_GET_MEM							0x85
#define USB_AUDIO_REQUEST_GET_STAT							0xFF

// audio data endpoint attributes
#define USB_AUDIO_DATA_EP_ATTR_SAMPLING_FREQUENCY			0x01  // sampling frequency control is supported
#define USB_AUDIO_DATA_EP_ATTR_PITCH						0x02  // pitch control is supported
#define USB_AUDIO_DATA_EP_ATTR_MAX_PACKETS_ONLY				0x80  // partial packets must be padded

// "lock delay" unit constants for audio data endpoints
#define USB_AUDIO_DATA_EP_DELAY_UNIT_UNDEFINED				0x00
#define USB_AUDIO_DATA_EP_DELAY_UNIT_MILLISECONDS			0x01
#define USB_AUDIO_DATA_EP_DELAY_UNIT_SAMPLES				0x02

// MIDI jack types
#define USB_AUDIO_MIDI_JACK_TYPE_UNDEFINED					0x00
#define USB_AUDIO_MIDI_JACK_TYPE_EMBEDDED					0x01
#define USB_AUDIO_MIDI_JACK_TYPE_EXTERNAL					0x02

// ------------------ control selector codes -------------------

// terminal control selectors 
#define USB_AUDIO_TE_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_TE_CONTROL_COPY_PROTECT					0x01

// feature unit control selectors
#define USB_AUDIO_FU_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_FU_CONTROL_MUTE							0x01
#define USB_AUDIO_FU_CONTROL_VOLUME							0x02
#define USB_AUDIO_FU_CONTROL_BASS							0x03
#define USB_AUDIO_FU_CONTROL_MID							0x04
#define USB_AUDIO_FU_CONTROL_TREBLE							0x05
#define USB_AUDIO_FU_CONTROL_GRAPHIC_EQ						0x06
#define USB_AUDIO_FU_CONTROL_AUTOMATIC_GAIN					0x07
#define USB_AUDIO_FU_CONTROL_DELAY							0x08
#define USB_AUDIO_FU_CONTROL_BASS_BOOST						0x09
#define USB_AUDIO_FU_CONTROL_LOUDNESS						0x0A

// feature unit "supported feature" bits
#define USB_AUDIO_FU_FEATURE_MUTE							0x0001
#define USB_AUDIO_FU_FEATURE_VOLUME							0x0002
#define USB_AUDIO_FU_FEATURE_BASS							0x0004
#define USB_AUDIO_FU_FEATURE_MID							0x0008
#define USB_AUDIO_FU_FEATURE_TREBLE							0x0010
#define USB_AUDIO_FU_FEATURE_GRAPHIC_EQ						0x0020
#define USB_AUDIO_FU_FEATURE_AUTOMATIC_GAIN					0x0040
#define USB_AUDIO_FU_FEATURE_DELAY							0x0080
#define USB_AUDIO_FU_FEATURE_BASS_BOOST						0x0100
#define USB_AUDIO_FU_FEATURE_LOUDNESS						0x0200

// up/down-mix processing unit control selectors
#define USB_AUDIO_UD_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_UD_CONTROL_ENABLE							0x01
#define USB_AUDIO_UD_CONTROL_MODE_SELECT					0x02

// Dolby Prologic processing unit control selectors
#define USB_AUDIO_DP_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_DP_CONTROL_ENABLE							0x01
#define USB_AUDIO_DP_CONTROL_MODE_SELECT					0x02

// 3D stereo extender processing unit control selectors
#define USB_AUDIO_3D_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_3D_CONTROL_ENABLE							0x01
#define USB_AUDIO_3D_CONTROL_SPACIOUSNESS					0x02

// reverberation processing unit control selectors
#define USB_AUDIO_RV_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_RV_CONTROL_ENABLE							0x01
#define USB_AUDIO_RV_CONTROL_TYPE							0x02
#define USB_AUDIO_RV_CONTROL_LEVEL							0x03
#define USB_AUDIO_RV_CONTROL_TIME							0x04
#define USB_AUDIO_RV_CONTROL_FEEDBACK						0x05

// chorus processing unit control selectors
#define USB_AUDIO_CH_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_CH_CONTROL_ENABLE							0x01
#define USB_AUDIO_CH_CONTROL_LEVEL							0x02
#define USB_AUDIO_CH_CONTROL_RATE							0x03
#define USB_AUDIO_CH_CONTROL_DEPTH							0x04

// dynamic range compressor processing unit control selectors
#define USB_AUDIO_DR_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_DR_CONTROL_ENABLE							0x01
#define USB_AUDIO_DR_CONTROL_COMPRESSION_RATIO				0x02
#define USB_AUDIO_DR_CONTROL_MAX_AMPLITUDE					0x03
#define USB_AUDIO_DR_CONTROL_THRESHOLD						0x04
#define USB_AUDIO_DR_CONTROL_ATTACK_TIME					0x05
#define USB_AUDIO_DR_CONTROL_RELEASE_TIME					0x06

// extension unit control selectors
#define USB_AUDIO_XU_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_XU_CONTROL_ENABLE							0x01

// endpoint control selectors
#define USB_AUDIO_EP_CONTROL_UNDEFINED						0x00
#define USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY				0x01
#define USB_AUDIO_EP_CONTROL_PITCH							0x02

// MIDI transfer endpoint control selectors
#define USB_AUDIO_MIDI_EP_CONTROL_UNDEFINED					0x00
#define USB_AUDIO_MIDI_EP_CONTROL_ASSOCIATION				0x01

// MIDI element capabilities
#define USB_AUDIO_MIDI_EL_CAPABILITY_UNDEFINED				0x00
#define USB_AUDIO_MIDI_EL_CAPABILITY_CUSTOM 				0x01
#define USB_AUDIO_MIDI_EL_CAPABILITY_CLOCK					0x02
#define USB_AUDIO_MIDI_EL_CAPABILITY_MTC					0x03
#define USB_AUDIO_MIDI_EL_CAPABILITY_MMC					0x04
#define USB_AUDIO_MIDI_EL_CAPABILITY_GM1					0x05
#define USB_AUDIO_MIDI_EL_CAPABILITY_GM2					0x06
#define USB_AUDIO_MIDI_EL_CAPABILITY_GS						0x07
#define USB_AUDIO_MIDI_EL_CAPABILITY_XG						0x08
#define USB_AUDIO_MIDI_EL_CAPABILITY_EFX					0x09
#define USB_AUDIO_MIDI_EL_CAPABILITY_PATCH_BAY				0x0A
#define USB_AUDIO_MIDI_EL_CAPABILITY_DLS1					0x0B
#define USB_AUDIO_MIDI_EL_CAPABILITY_DLS2					0x0C

// format type codes
#define USB_AUDIO_FORMAT_TYPE_UNDEFINED						0x00
#define USB_AUDIO_FORMAT_TYPE_I								0x01
#define USB_AUDIO_FORMAT_TYPE_II							0x02
#define USB_AUDIO_FORMAT_TYPE_III							0x03

// format codes
// Type I
#define USB_AUDIO_FORMAT_TYPE_I_UNDEFINED					0x0000
#define USB_AUDIO_FORMAT_TYPE_I_PCM							0x0001
#define USB_AUDIO_FORMAT_TYPE_I_PCM8						0x0002
#define USB_AUDIO_FORMAT_TYPE_I_IEEE_FLOAT					0x0003
#define USB_AUDIO_FORMAT_TYPE_I_ALAW						0x0004
#define USB_AUDIO_FORMAT_TYPE_I_MULAW						0x0005
// Type II
#define USB_AUDIO_FORMAT_TYPE_II_UNDEFINED					0x1000
#define USB_AUDIO_FORMAT_TYPE_II_MPEG						0x1001
#define USB_AUDIO_FORMAT_TYPE_II_AC3						0x1002
// Type III
#define USB_AUDIO_FORMAT_TYPE_III_UNDEFINED					0x2000
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_AC3				0x2001
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER_1		0x2002
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER_23	0x2003  // this is not a
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_NOEXT		0x2003  // mistake
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_EXT			0x2004
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER_1_LS	0x2005
#define USB_AUDIO_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER_23_LS	0x2006

// MPEG capabilities
#define USB_MPEG_CAPABILITY_SUPPORT_LAYER_1					0x0001
#define USB_MPEG_CAPABILITY_SUPPORT_LAYER_2					0x0002
#define USB_MPEG_CAPABILITY_SUPPORT_LAYER_3					0x0004
#define USB_MPEG_CAPABILITY_MPEG_1_ONLY						0x0008
#define USB_MPEG_CAPABILITY_MPEG_1_DUAL_CHANNEL				0x0010
#define USB_MPEG_CAPABILITY_MPEG_2_SECOND_STEREO			0x0020
#define USB_MPEG_CAPABILITY_MPEG_2_7_DOT_1					0x0040
#define USB_MPEG_CAPABILITY_ADAPTIVE_MULTICHANNEL			0x0080
#define USB_MPEG_CAPABILITY_MULTILINGUAL_UNSUPPORTED		0x0000
#define USB_MPEG_CAPABILITY_MULTILINGUAL_AT_FS				0x0100
#define USB_MPEG_CAPABILITY_MULTILINGUAL_RESERVED			0x0200
#define USB_MPEG_CAPABILITY_MULTILINGUAL_AT_FS_AND_HALF_FS	0x0300

// MPEG features
#define USB_MPEG_FEATURE_DYN_RANGE_CTRL_UNSUPPORTED			0x0000
#define USB_MPEG_FEATURE_DYN_RANGE_CTRL_NONSCALABLE			0x0010
#define USB_MPEG_FEATURE_DYN_RANGE_CTRL_WITH_COMMON_BOOST	0x0020
#define USB_MPEG_FEATURE_DYN_RANGE_CTRL_WITH_SEPARATE_BOOST	0x0030

// AC3 features
#define USB_AC3_FEATURE_RF_MODE								0x0001
#define USB_AC3_FEATURE_LINE_MODE							0x0002
#define USB_AC3_FEATURE_CUSTOM_0_MODE						0x0004
#define USB_AC3_FEATURE_CUSTOM_1_MODE						0x0008
#define USB_AC3_FEATURE_DYN_RANGE_CTRL_UNSUPPORTED			0x0000
#define USB_AC3_FEATURE_DYN_RANGE_CTRL_NONSCALABLE			0x0010
#define USB_AC3_FEATURE_DYN_RANGE_CTRL_WITH_COMMON_BOOST	0x0020
#define USB_AC3_FEATURE_DYN_RANGE_CTRL_WITH_SEPARATE_BOOST	0x0030

// --- terminal types
// USB terminal types
#define USB_TERMINAL_USB_UNDEFINED									0x0100
#define USB_TERMINAL_USB_STREAMING									0x0101
#define USB_TERMINAL_USB_VENDOR_SPECIFIC							0x01FF

// input terminal types
#define USB_TERMINAL_INPUT_UNDEFINED								0x0200
#define USB_TERMINAL_INPUT_MICROPHONE								0x0201
#define USB_TERMINAL_INPUT_DESKTOP_MICROPHONE						0x0202
#define USB_TERMINAL_INPUT_PERSONAL_MICROPHONE						0x0203
#define USB_TERMINAL_INPUT_OMNIDIRECTIONAL_MICROPHONE				0x0204
#define USB_TERMINAL_INPUT_MICROPHONE_ARRAY							0x0205
#define USB_TERMINAL_INPUT_PROCESSING_MICROPHONE_ARRAY				0x0206

// output terminal types
#define USB_TERMINAL_OUTPUT_UNDEFINED								0x0300
#define USB_TERMINAL_OUTPUT_SPEAKER									0x0301
#define USB_TERMINAL_OUTPUT_HEADPHONES								0x0302
#define USB_TERMINAL_OUTPUT_HEAD_MOUNTED_DISPLAY_AUDIO				0x0303
#define USB_TERMINAL_OUTPUT_DESKTOP_SPEAKER							0x0304
#define USB_TERMINAL_OUTPUT_ROOM_SPEAKER							0x0305
#define USB_TERMINAL_OUTPUT_COMMUNICATION_SPEAKER					0x0306
#define USB_TERMINAL_OUTPUT_LFE_SPEAKER								0x0307

// bi-directional terminal types
#define USB_TERMINAL_BIDIRECTIONAL_UNDEFINED						0x0400
#define USB_TERMINAL_BIDIRECTIONAL_HANDSET							0x0401
#define USB_TERMINAL_BIDIRECTIONAL_HEADSET							0x0402
#define USB_TERMINAL_BIDIRECTIONAL_SPEAKERPHONE						0x0403
#define USB_TERMINAL_BIDIRECTIONAL_SPEAKERPHONE_ECHO_SUPPRESSING	0x0404
#define USB_TERMINAL_BIDIRECTIONAL_SPEAKERPHONE_ECHO_CANCELLING		0x0405

// telephony terminal types
#define USB_TERMINAL_TELEPHONY_UNDEFINED							0x0500
#define USB_TERMINAL_TELEPHONY_PHONE_LINE							0x0501
#define USB_TERMINAL_TELEPHONY_TELEPHONE							0x0502
#define USB_TERMINAL_TELEPHONY_DOWN_LINE_PHONE						0x0503

// external terminal types
#define USB_TERMINAL_EXTERNAL_UNDEFINED								0x0600
#define USB_TERMINAL_EXTERNAL_ANALOG_CONNECTOR						0x0601
#define USB_TERMINAL_EXTERNAL_DIGITAL_AUDIO_INTERFACE				0x0602
#define USB_TERMINAL_EXTERNAL_LINE_CONNECTOR						0x0603
#define USB_TERMINAL_EXTERNAL_LEGACY_AUDIO_CONNECTOR				0x0604
#define USB_TERMINAL_EXTERNAL_SPDIF_INTERFACE						0x0605
#define USB_TERMINAL_EXTERNAL_1394_DA_STREAM						0x0606
#define USB_TERMINAL_EXTERNAL_1394_DV_STREAM_SOUNDTRACK				0x0607

// embedded function terminal types
#define USB_TERMINAL_EMBEDDED_UNDEFINED								0x0700
#define USB_TERMINAL_EMBEDDED_LEVEL_CALIBRATION_NOISE_SOURCE		0x0701
#define USB_TERMINAL_EMBEDDED_EQUALIZATION_SOURCE					0x0702
#define USB_TERMINAL_EMBEDDED_CD_PLAYER								0x0703
#define USB_TERMINAL_EMBEDDED_DAT									0x0704
#define USB_TERMINAL_EMBEDDED_DCC									0x0705
#define USB_TERMINAL_EMBEDDED_MINIDISK								0x0706
#define USB_TERMINAL_EMBEDDED_ANALOG_TAPE							0x0707
#define USB_TERMINAL_EMBEDDED_PHONOGRAPH							0x0708
#define USB_TERMINAL_EMBEDDED_VCR_AUDIO								0x0709
#define USB_TERMINAL_EMBEDDED_VIDEO_DISK_AUDIO						0x070A
#define USB_TERMINAL_EMBEDDED_DVD_AUDIO								0x070B
#define USB_TERMINAL_EMBEDDED_TV_TUNER_AUDIO						0x070C
#define USB_TERMINAL_EMBEDDED_SATELLITE_RECEIVER_AUDIO				0x070D
#define USB_TERMINAL_EMBEDDED_CABLE_TUNER_AUDIO						0x070E
#define USB_TERMINAL_EMBEDDED_DSS_AUDIO								0x070F
#define USB_TERMINAL_EMBEDDED_RADIO_RECEIVER						0x0710
#define USB_TERMINAL_EMBEDDED_RADIO_TRANSMITTER						0x0711
#define USB_TERMINAL_EMBEDDED_MULTI_TRACK_RECORDER					0x0712
#define USB_TERMINAL_EMBEDDED_SYNTHESIZER							0x0713
// -------- miscellaneous USB audio-related structures ---------

// status word - specifies data format returned via status interrupt endpoint
#define USB_AUDIO_STATUS_TYPE_ORIGINATOR_AC_INTERFACE		0x00
#define USB_AUDIO_STATUS_TYPE_ORIGINATOR_AS_INTERFACE		0x01
#define USB_AUDIO_STATUS_TYPE_ORIGINATOR_AS_ENDPOINT		0x02
#define USB_AUDIO_STATUS_TYPE_MEMORY_CHANGED				0x40
#define USB_AUDIO_STATUS_TYPE_INTERRUPT_PENDING				0x80


typedef struct _USB_AUDIO_STATUS_WORD {
  UCHAR bmStatusType;		// bitmap, see bit definitions above
  UCHAR bOriginator;		// ID of the originator
} USB_AUDIO_STATUS_WORD, *PUSB_AUDIO_STATUS_WORD;


// channel cluster format
#define USB_AUDIO_CHANNEL_LEFT_FRONT						0x0001
#define USB_AUDIO_CHANNEL_RIGHT_FRONT						0x0002
#define USB_AUDIO_CHANNEL_CENTER_FRONT						0x0004
#define USB_AUDIO_CHANNEL_LFE								0x0008
#define USB_AUDIO_CHANNEL_LEFT_SURROUND						0x0010
#define USB_AUDIO_CHANNEL_RIGTH_SURROUND					0x0020
#define USB_AUDIO_CHANNEL_LEFT_CENTER						0x0040
#define USB_AUDIO_CHANNEL_RIGHT_CENTER						0x0080
#define USB_AUDIO_CHANNEL_SURROUND							0x0100
#define USB_AUDIO_CHANNEL_SIDE_LEFT							0x0200
#define USB_AUDIO_CHANNEL_SIDE_RIGHT						0x0400
#define USB_AUDIO_CHANNEL_TOP								0x0800

// USB-MIDI event packet
typedef struct _USB_MIDI_EVENT_PACKET {
	struct {
		UCHAR	CodeIndexNumber:4;	// indicates the classification of the bytes in MIDI[x] fields.
		UCHAR	CableNumber:4;		// a value ranging from 0x0 to 0xF indicating the number assignment 
									// of the Embedded MIDI Jack associated with the endpoint that is 
									// transferring the data
	};
	UCHAR	MIDI[3]; // actual MIDI event.
} USB_MIDI_EVENT_PACKET, *PUSB_MIDI_EVENT_PACKET;

// Code index number
#define CODE_INDEX_NUMBER_MISCELLANEOUS				0x0
#define CODE_INDEX_NUMBER_CABLE_EVENT				0x1
#define CODE_INDEX_NUMBER_2_BYTE_SYSTEM_COMMON		0x2
#define CODE_INDEX_NUMBER_3_BYTE_SYSTEM_COMMON		0x3
#define CODE_INDEX_NUMBER_SYSEX_START_OR_CONTINUE	0x4
#define CODE_INDEX_NUMBER_1_BYTE_SYSTEM_COMMON		0x5
#define CODE_INDEX_NUMBER_1_BYTE_SYSEX_END			0x5
#define CODE_INDEX_NUMBER_2_BYTE_SYSEX_END			0x6
#define CODE_INDEX_NUMBER_3_BYTE_SYSEX_END			0x7
#define CODE_INDEX_NUMBER_NOTE_OFF					0x8
#define CODE_INDEX_NUMBER_NOTE_ON					0x9
#define CODE_INDEX_NUMBER_POLYKEY_PRESSURE			0xA
#define CODE_INDEX_NUMBER_CONTROL_CHANGE			0xB
#define CODE_INDEX_NUMBER_PROGRAM_CHANGE			0xC
#define CODE_INDEX_NUMBER_CHANNEL_PRESSURE			0xD
#define CODE_INDEX_NUMBER_PITCH_BEND_CHANGE			0xE
#define CODE_INDEX_NUMBER_1_BYTE					0xF

// -------------------- USB audio descriptors --------------------

typedef struct _USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR {
  UCHAR  bNrChannels;		// number of logical channels
  USHORT wChannelConfig;	// bitmap specifying which channels are present
  UCHAR  iChannelNames;		// optional index of the first non-predefined channel string descriptor
} USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR, *PUSB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR;

// common audio header descriptor
typedef struct _USB_AUDIO_COMMON_DESCRIPTOR {
    UCHAR  bLength;				/* size of this descriptor */
    UCHAR  bDescriptorType;		/* descriptor type */
    UCHAR  bDescriptorSubtype;	/* sub descriptor type */
} USB_AUDIO_COMMON_DESCRIPTOR, *PUSB_AUDIO_COMMON_DESCRIPTOR;

// class-specific audio control interface header descriptor
typedef struct _USB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR {
  UCHAR  bLength;				/* size of this descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_HEADER */
  USHORT bcdADC;				/* audio device spec release number in BCD */
  USHORT wTotalLength;			/* combined length of this desriptor header and all Unit and Terminal descriptors */
  UCHAR  bInCollection;			/* number of AudioStreaming and MIDIStreaming interfaces in the collection */
  UCHAR  baInterfaceNr[1];		/* array of interface numbers for streaming interfaces in the collection */
} USB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR, *PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR;

// type-common format descriptor
typedef struct _USB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR {
  UCHAR  bLength;				/* size of this descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AS_DESCRIPTOR_FORMAT_TYPE */
  UCHAR  bFormatType;			/* FORMAT_TYPE_I/II/III */
} USB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR, *PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR;

// type-I format descriptor
typedef struct _USB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR {
  UCHAR  bLength;				/* size of this descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AS_DESCRIPTOR_FORMAT_TYPE */
  UCHAR  bFormatType;			/* must be FORMAT_TYPE_I */
  UCHAR  bNrChannels;			/* number of physical channels in the stream */
  UCHAR  bSubframeSize;			/* number of bytes occupied by a single audio subframe */
  UCHAR  bBitResolution;		/* number of actually used bits in an audio subframe */
  UCHAR  bSamFreqType;			/* 0 - continuous, 1..255 - number of discrete sampling frequencies to follow */
  UCHAR  tSamFreq[1][3];		/* array of supported sampling frequencies */
} USB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR, *PUSB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR;

// type-II format descriptor
typedef struct _USB_AUDIO_TYPE_II_FORMAT_DESCRIPTOR {
  UCHAR  bLength;				/* size of this descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AS_DESCRIPTOR_FORMAT_TYPE */
  UCHAR  bFormatType;			/* must be FORMAT_TYPE_II */
  USHORT wMaxBitrate;			/* max number of bits per second (expressed in kbps) this interface can handle */
  USHORT wSamplesPerFrame;		/* number of PCM audio samples in one encoded audio frame */
  UCHAR  bSamFreqType;			/* 0 - continuous, 1..255 - number of discrete sampling frequencies to follow */
  UCHAR  tSamFreq[1][3];		/* array of supported sampling frequencies */
} USB_AUDIO_TYPE_II_FORMAT_DESCRIPTOR, *PUSB_AUDIO_TYPE_II_FORMAT_DESCRIPTOR;

// type-III format descriptor
typedef struct _USB_AUDIO_TYPE_III_FORMAT_DESCRIPTOR {
  UCHAR  bLength;				/* size of this descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AS_DESCRIPTOR_FORMAT_TYPE */
  UCHAR  bFormatType;			/* must be FORMAT_TYPE_III */
  UCHAR  bNrChannels;			/* number of physical channels in the stream, must be set to 2 */
  UCHAR  bSubframeSize;			/* number of bytes occupied by a single audio subframe, must be set to 2 */
  UCHAR  bSamFreqType;			/* 0 - continuous, 1..255 - number of discrete sampling frequencies to follow */
  UCHAR  bBitResolution;		/* number of actually used bits in an audio subframe */
  UCHAR  tSamFreq[1][3];		/* array of supported sampling frequencies */
} USB_AUDIO_TYPE_III_FORMAT_DESCRIPTOR, *PUSB_AUDIO_TYPE_III_FORMAT_DESCRIPTOR;

// Common format-specific descriptor
typedef struct _USB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR {
  UCHAR  bLength;				// size of this descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_FORMAT_SPECIFIC
  USHORT wFormatTag;			// must be USB_AUDIO_FORMAT_TYPE_II_MPEG
  USHORT bmMPEGCapabilities;	// specific MPEG capabilities
  UCHAR  bmMPEGFeatures;		// specific MPEG features
} USB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR, *PUSB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR;

// MPEG format-specific descriptor
typedef struct _USB_AUDIO_MPEG_FORMAT_DESCRIPTOR {
  UCHAR  bLength;				// size of this descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_FORMAT_SPECIFIC
  USHORT wFormatTag;			// must be USB_AUDIO_FORMAT_TYPE_II_MPEG
  USHORT bmMPEGCapabilities;	// specific MPEG capabilities
  UCHAR  bmMPEGFeatures;		// specific MPEG features
} USB_AUDIO_MPEG_FORMAT_DESCRIPTOR, *PUSB_AUDIO_MPEG_FORMAT_DESCRIPTOR;

// AC3 format-specific descriptor
typedef struct _USB_AUDIO_AC3_FORMAT_DESCRIPTOR {
  UCHAR  bLength;				// size of this descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_FORMAT_SPECIFIC
  USHORT wFormatTag;			// must be USB_AUDIO_FORMAT_TYPE_II_AC3
  ULONG	 bmBSID;				// supported bitstream ID modes
  UCHAR  bmAC3Features;			// specific AC3 features
} USB_AUDIO_AC3_FORMAT_DESCRIPTOR, *PUSB_AUDIO_AC3_FORMAT_DESCRIPTOR;

// common entity descriptor
typedef struct _USB_AUDIO_COMMON_ENTITY_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* sub descriptor type */
  UCHAR  bEntityID;				/* unique ID of the entity */
} USB_AUDIO_COMMON_ENTITY_DESCRIPTOR, *PUSB_AUDIO_COMMON_ENTITY_DESCRIPTOR;

// common terminal descriptor
typedef struct _USB_AUDIO_COMMON_TERMINAL_DESCRIPTOR {
  UCHAR  bLength;				// size of this descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL
  UCHAR  bTerminalID;			// unique ID for the terminal within the audio function
  USHORT wTerminalType;			// type of the terminal
  UCHAR  bAssocTerminal;		// ID of the input terminal to which this output terminal is associated 
} USB_AUDIO_COMMON_TERMINAL_DESCRIPTOR, *PUSB_AUDIO_COMMON_TERMINAL_DESCRIPTOR;

// input terminal descriptor
typedef struct _USB_AUDIO_INPUT_TERMINAL_DESCRIPTOR {
  UCHAR  bLength;				// size of this descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL
  UCHAR  bTerminalID;			// unique ID for the terminal within the audio function
  USHORT wTerminalType;			// type of the terminal
  UCHAR  bAssocTerminal;		// ID of the output terminal to which this input terminal is associated 
  UCHAR  bNrChannels;			// number of logical output channels in the terminal's output channel cluster
  USHORT wChannelConfig;		// describes the spatial location of the logical channels
  UCHAR  iChannelNames;			// index of a string descriptor for the name of the first logical channel
  UCHAR  iTerminal;				// index of a string descriptor for the input terminal
} USB_AUDIO_INPUT_TERMINAL_DESCRIPTOR, *PUSB_AUDIO_INPUT_TERMINAL_DESCRIPTOR;


// output terminal descriptor
typedef struct _USB_AUDIO_OUTPUT_TERMINAL_DESCRIPTOR {
  UCHAR  bLength;				// size of this descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL
  UCHAR  bTerminalID;			// unique ID for the terminal within the audio function
  USHORT wTerminalType;			// type of the terminal
  UCHAR  bAssocTerminal;		// ID of the input terminal to which this output terminal is associated 
  UCHAR  bSourceID;				// ID of the unit or terminal to which this terminal is connected
  UCHAR  iTerminal;				// index of a string descriptor for the input terminal
} USB_AUDIO_OUTPUT_TERMINAL_DESCRIPTOR, *PUSB_AUDIO_OUTPUT_TERMINAL_DESCRIPTOR;


// common unit descriptor
typedef struct _USB_AUDIO_COMMON_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* sub descriptor type */
  UCHAR  bUnitID;				/* unique ID of the unit */
} USB_AUDIO_COMMON_UNIT_DESCRIPTOR, *PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR;

// feature unit descriptor
#define DEFINE_USB_FEATURE_DESCRIPTOR(unit_id, num_channels, controls_size) \
typedef struct _USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  UCHAR  bSourceID;				/* ID of units or terminal to which this feature unit is connected */ \
  UCHAR  bControlSize;			/* size (in bytes) of elements in bmaControls array */ \
  UCHAR  bmaControls[num_channels+1][controls_size]; /* controls available for different channels */ \
  UCHAR  iFeature;				/* index of the string descriptor for this feature unit */ \
} USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_FEATURE_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_FEATURE_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  UCHAR  bSourceID;				/* ID of units or terminal to which this feature unit is connected */
  UCHAR  bControlSize;			/* size (in bytes) of elements in bmaControls array */
  //UCHAR  bmaControls[num_channels+1][controls_size]; /* controls available for different channels */
  //UCHAR  iFeature;				/* index of the string descriptor for this feature unit */
} USB_AUDIO_FEATURE_UNIT_DESCRIPTOR, *PUSB_AUDIO_FEATURE_UNIT_DESCRIPTOR;

#define USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET		6
#define USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_IFEATURE_OFFSET(control_size, num_channels)	(USB_AUDIO_FEATURE_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+(num_channels+1)*control_size)

// mixer unit descriptor
#define DEFINE_USB_MIXER_DESCRIPTOR(unit_id, num_input_pins, controls_size) \
typedef struct _USB_AUDIO_MIXER_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  UCHAR  bNrInPins;				/* number of input pins */ \
  UCHAR  baSourceID[num_input_pins];/* array of IDs of Units or Terminals to which input pins are connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bmControls[controls_size];	/* control bit matrix */ \
  UCHAR  iMixer;				/* index of the string descriptor for this mixer */ \
} USB_AUDIO_MIXER_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_MIXER_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_MIXER_UNIT_DESCRIPTOR_ {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  UCHAR  bNrInPins;				/* number of input pins */
  //UCHAR  baSourceID[num_input_pins];/* array of IDs of Units or Terminals to which input pins are connected */
  //UCHAR  bNrChannels;			/* number of logical output channels */
  //USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  //UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  //UCHAR  bmControls[controls_size];	/* control bit matrix */
  //UCHAR  iMixer;				/* index of the string descriptor for this mixer */
} USB_AUDIO_MIXER_UNIT_DESCRIPTOR, *PUSB_AUDIO_MIXER_UNIT_DESCRIPTOR;

#define USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET		5
#define USB_AUDIO_MIXER_UNIT_DESCRIPTOR_CLUSTER_OFFSET(num_input_pins)		(USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BASOURCEID_OFFSET+num_input_pins)
#define USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(num_input_pins)	(USB_AUDIO_MIXER_UNIT_DESCRIPTOR_CLUSTER_OFFSET(num_input_pins)+4)
#define USB_AUDIO_MIXER_UNIT_DESCRIPTOR_IMIXER_OFFSET(num_input_pins, control_size)	(USB_AUDIO_MIXER_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(num_input_pins)+control_size)

// selector unit descriptor
#define DEFINE_USB_SELECTOR_DESCRIPTOR(unit_id, num_input_pins) \
typedef struct _USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  UCHAR  bNrInPins;				/* number of input pins */ \
  UCHAR  baSourceID[num_input_pins];/* array of IDs of Units or Terminals to which input pins are connected */ \
  UCHAR  iSelector;				/* index of the string descriptor for this selector */ \
} USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  UCHAR  bNrInPins;				/* number of input pins */
  //UCHAR  baSourceID[num_input_pins];/* array of IDs of Units or Terminals to which input pins are connected */
  //UCHAR  iSelector;				/* index of the string descriptor for this selector */
} USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR, *PUSB_AUDIO_SELECTOR_UNIT_DESCRIPTOR;

#define USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_BASOURCEID_OFFSET		5
#define USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_ISELECTOR_OFFSET(num_input_pins)	(USB_AUDIO_SELECTOR_UNIT_DESCRIPTOR_BASOURCEID_OFFSET+num_input_pins)

// common processing unit descriptor
typedef struct _USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */ 
  USHORT wProcessType;			/* process type */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for up/down-mix unit */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
} USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR, *PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR;

#define USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)

// up/down-mix unit descriptor
#define DEFINE_USB_UP_DOWNMIX_DESCRIPTOR(unit_id, controls_size, num_modes) \
typedef struct _USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_UPMIX_DOWNMIX */ \
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for up/down-mix unit */ \
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */ \
  UCHAR  bNrModes;				/* number of supported modes */ \
  USHORT waModes[num_modes];	/* specifies active logical channels for each mode */ \
} USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_UPMIX_DOWNMIX */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for up/down-mix unit */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */
  //UCHAR  bNrModes;				/* number of supported modes */
  //USHORT waModes[num_modes];	/* specifies active logical channels for each mode */
} USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR, *PUSB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR;

#define USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)
#define USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_BNRMODES_OFFSET(control_size)	(USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)+1)
#define USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_WAMODES_OFFSET(control_size)	(USB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR_BNRMODES_OFFSET(control_size)+1)


// Dolby Prologic unit descriptor
#define DEFINE_USB_PROLOGIC_DESCRIPTOR(unit_id, controls_size, num_modes) \
typedef struct _USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_DOLBY_PROLOGIC */ \
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for Dolby Prologic unit */ \
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */ \
  UCHAR  bNrModes;				/* number of supported modes, a maximum of 3 modes is possible */ \
  USHORT waModes[num_modes];	/* specifies active logical channels for each mode */ \
} USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_DOLBY_PROLOGIC */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for Dolby Prologic unit */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */
  //UCHAR  bNrModes;				/* number of supported modes, a maximum of 3 modes is possible */
  //USHORT waModes[num_modes];	/* specifies active logical channels for each mode */
} USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR, *PUSB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR;

#define USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)
#define USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_BNRMODES_OFFSET(control_size)	(USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)+1)
#define USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_WAMODES_OFFSET(control_size)	(USB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR_BNRMODES_OFFSET(control_size)+1)


// 3D-stereo extender unit descriptor
#define DEFINE_USB_3D_EXTENDER_DESCRIPTOR(unit_id, controls_size) \
typedef struct _USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_3D_STEREO_EXTENDER */ \
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for 3D stereo extender */ \
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */ \
} USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_3D_STEREO_EXTENDER */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for 3D stereo extender */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */
} USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR, *PUSB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR;

#define USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)


// reverberation unit descriptor
#define DEFINE_USB_REVERBERATION_DESCRIPTOR(unit_id, controls_size) \
typedef struct _USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_REVERBERATION */ \
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for reverberation process */ \
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */ \
} USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_REVERBERATION */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for reverberation process */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */
} USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR, *PUSB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR;

#define USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)


// chorus unit descriptor
#define DEFINE_USB_CHORUS_DESCRIPTOR(unit_id, controls_size) \
typedef struct _USB_AUDIO_CHORUS_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_CHORUS */ \
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for chorus process */ \
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */ \
} USB_AUDIO_CHORUS_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_CHORUS_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_CHORUS_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_CHORUS */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for chorus process */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */
} USB_AUDIO_CHORUS_UNIT_DESCRIPTOR, *PUSB_AUDIO_CHORUS_UNIT_DESCRIPTOR;

#define USB_AUDIO_CHORUS_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_CHORUS_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_CHORUS_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)


// dynamic range compressor unit descriptor
#define DEFINE_USB_DYNAMIC_RANGE_COMPRESSOR_DESCRIPTOR(unit_id, controls_size) \
typedef struct _USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION */ \
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for dynamic range compression process */ \
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */ \
} USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wProcessType;			/* must be USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION */
  UCHAR  bNrInPins;				/* number of input pins, must be 1 for dynamic range compression process */
  UCHAR  bSourceID;				/* ID of the unit or terminal to which input pin is connected */
  UCHAR  bNrChannels;			/* number of logical output channels */
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iProcessing;			/* index of the string descriptor for this processing unit */
} USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR, *PUSB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR;

#define USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET	13
#define USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR_IPROCESSING_OFFSET(control_size)	(USB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET+control_size)


// extension unit descriptor
#define DEFINE_USB_EXTENSION_UNIT_DESCRIPTOR(unit_id, num_input_pins, controls_size) \
typedef struct _USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_##unit_id {\
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT */ \
  UCHAR  bUnitID;				/* unique ID of the unit */ \
  USHORT wExtensionCode;		/* vendor-specific code identifying extension unit */ \
  UCHAR  bNrInPins;				/* number of input pins */ \
  UCHAR  baSourceID[num_input_pins]; /* IDs of the units or terminals to which input pins are connected */ \
  UCHAR  bNrChannels;			/* number of logical output channels */ \
  USHORT wChannelConfig;		/* describes spatial location of the logical channels */ \
  UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */ \
  UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */ \
  UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */ \
  UCHAR  iExtension;			/* index of the string descriptor for this extension unit */ \
} USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT */
  UCHAR  bUnitID;				/* unique ID of the unit */
  USHORT wExtensionCode;		/* vendor-specific code identifying extension unit */
  UCHAR  bNrInPins;				/* number of input pins */
  //UCHAR  baSourceID[num_input_pins]; /* IDs of the units or terminals to which input pins are connected */
  //UCHAR  bNrChannels;			/* number of logical output channels */
  //USHORT wChannelConfig;		/* describes spatial location of the logical channels */
  //UCHAR  iChannelNames;			/* index of the string descriptor for the first channel */
  //UCHAR  bControlSize;			/* size (in bytes) of the bmControls field */
  //UCHAR  bmControls[controls_size];	/* a bit set to 1 indicates that the corresponding control is supported */
  //UCHAR  iExtension;			/* index of the string descriptor for this extension unit */
} USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR, *PUSB_AUDIO_EXTENSION_UNIT_DESCRIPTOR;

#define USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BASOURCEID_OFFSET		7
#define USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_CLUSTER_OFFSET(num_input_pins)		(USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BASOURCEID_OFFSET+num_input_pins)
#define USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(num_input_pins)	(USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_CLUSTER_OFFSET(num_input_pins)+4)
#define USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(num_input_pins)	(USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BCONTROLSIZE_OFFSET(num_input_pins)+1)
#define USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_IEXTENSION_OFFSET(num_input_pins, control_size)	(USB_AUDIO_EXTENSION_UNIT_DESCRIPTOR_BMCONTROLS_OFFSET(num_input_pins)+control_size)

// class-specific audio streaming interface descriptor
typedef struct _USB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR {
  UCHAR  bLength;				// size of the descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;    // must be USB_AUDIO_AS_DESCRIPTOR_GENERAL
  UCHAR  bTerminalLink;			// the ID of the terminal to which the endpoint of this interface is connected
  UCHAR  bDelay;				// delay introduced by the data path
  USHORT wFormatTag;			// audio data format
} USB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR, *PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR;


// standard (but expanded) AS endpoint descriptor
typedef struct _USB_AUDIO_ENDPOINT_DESCRIPTOR {
  UCHAR  bLength;				// size of the descriptor
  UCHAR  bDescriptorType;		// must be USB_ENDPOINT_DESCRIPTOR_TYPE
  UCHAR  bEndpointAddress;		// endpoint address
  UCHAR  bmAttributes;			// attributes
  USHORT wMaxPacketSize;		// max packet size
  UCHAR  bInterval;				// polling interval
  
  // audio device-specific fields
  UCHAR  bRefresh;				// used by sync endpoints
  UCHAR  bSynchAddress;			// address of the sync endpoint
} USB_AUDIO_ENDPOINT_DESCRIPTOR, *PUSB_AUDIO_ENDPOINT_DESCRIPTOR;


// class-specific AS isochronous audio data endpoint descriptor
typedef struct _USB_AUDIO_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR {
  UCHAR  bLength;				// size of the descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_ENDPOINT
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_EP_DESCRIPTOR_GENERAL
  UCHAR  bmAttributes;			// endpoint attributes (see above)
  UCHAR  bLockDelayUnits;		// units used for wLockDelay field, must be 0 for async endpoints
  USHORT wLockDelay;			// time to lock internal clock recovery, must be 0 for async
} USB_AUDIO_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR, *PUSB_AUDIO_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR;


// class-specific MIDI streaming interface descriptor
typedef struct _USB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR {
  UCHAR  bLength;				// size of the descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;    // must be USB_AUDIO_MS_DESCRIPTOR_HEADER
  USHORT bcdMSC;				// MIDIStreaming subclass release number in BCD
  USHORT wTotalLength;			// total number of bytes returned for the class-specific MIDI streaming I/F
  								// descriptor (includes the combined length of this descriptor header and all
  								// Jack and Element descriptors)
} USB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR, *PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR;

// Common MIDI jack descriptor
typedef struct _USB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR {
  UCHAR  bLength;				// size of the descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK
  UCHAR  bJackType;				// must be MIDI_JACK_TYPE_EMBEDDED or MIDI_JACK_TYPE_EXTERNAL
  UCHAR  bJackID;				// unique ID for the jack within the USB-MIDI function
} USB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR, *PUSB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR;


// MIDI IN jack descriptor
typedef struct _USB_AUDIO_MIDI_IN_JACK_DESCRIPTOR {
  UCHAR  bLength;				// size of the descriptor
  UCHAR  bDescriptorType;		// must be USB_AUDIO_CS_INTERFACE
  UCHAR  bDescriptorSubtype;	// must be USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK
  UCHAR  bJackType;				// must be MIDI_JACK_TYPE_EMBEDDED or MIDI_JACK_TYPE_EXTERNAL
  UCHAR  bJackID;				// unique ID for the jack within the USB-MIDI function
  UCHAR  iJack;					// index of a string descriptor for the MIDI IN Jack
} USB_AUDIO_MIDI_IN_JACK_DESCRIPTOR, *PUSB_AUDIO_MIDI_IN_JACK_DESCRIPTOR;


// MIDI OUT jack descriptor
#define DEFINE_USB_MIDI_OUT_JACK_DESCRIPTOR(unit_id, num_input_pins) \
typedef struct _USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_##unit_id { \
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK */ \
  UCHAR  bJackType;				/* must be MIDI_JACK_TYPE_EMBEDDED or MIDI_JACK_TYPE_EXTERNAL */ \
  UCHAR  bJackID;				/* unique ID for the jack within the USB-MIDI function */ \
  UCHAR  bNrInputPins;			/* number of input pins of this MIDI OUT Jack */ \
  /* source structure */ \
  struct { \
  UCHAR  baSourceID;			/* ID of the entity to which N-th pin of this MIDI OUT Jack is connected */ \
  UCHAR  baSourcePin;			/* output pin number of the entity to which N-th pin of this MIDI OUT Jack is connected */ \
  } saSource[num_input_pins]; \
  UCHAR  iJack;					/* index of a string descriptor for the MIDI OUT Jack */ \
} USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_##unit_id, *PUSB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK */
  UCHAR  bJackType;				/* must be MIDI_JACK_TYPE_EMBEDDED or MIDI_JACK_TYPE_EXTERNAL */
  UCHAR  bJackID;				/* unique ID for the jack within the USB-MIDI function */
  UCHAR  bNrInputPins;			/* number of input pins of this MIDI OUT Jack */
  /* source structure */
  //struct {
  //UCHAR  baSourceID;			/* ID of the entity to which N-th pin of this MIDI OUT Jack is connected */
  //UCHAR  baSourcePin;			/* output pin number of the entity to which N-th pin of this MIDI OUT Jack is connected */
  //} saSource[1];
  //UCHAR  iJack;					/* index of a string descriptor for the MIDI OUT Jack */
} USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR, *PUSB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR;

typedef struct _USB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR {
  UCHAR  bSourceID;				/* ID of the entity to which N-th pin of this MIDI OUT Jack is connected */
  UCHAR  bSourcePin;			/* output pin number of the entity to which N-th pin of this MIDI OUT Jack is connected */
} USB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR, *PUSB_AUDIO_MIDI_SOURCE_ID_PIN_PAIR;

#define USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_SOURCE_OFFSET				(6)
#define USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_IJACK_OFFSET(num_input_pins)	(USB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR_SOURCE_OFFSET+(2*num_input_pins))

// MIDI Element descriptor
#define DEFINE_USB_MIDI_ELEMENT_DESCRIPTOR(unit_id, num_input_pins, cap_size) \
typedef struct _USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_##unit_id { \
  UCHAR  bLength;				/* size of the descriptor */ \
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */ \
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_MS_DESCRIPTOR_MIDI_ELEMENT */ \
  UCHAR  bElementID;			/* unique ID for the Element within the USB-MIDI function */ \
  UCHAR  bNrInputPins;			/* number of input pins of this Element */ \
  /* source structure */ \
  struct { \
  UCHAR  baSourceID;			/* ID of the entity to which N-th pin of this Element is connected */ \
  UCHAR  baSourcePin;			/* output pin number of the entity to which N-th pin of this Element is connected */ \
  } saSource[num_input_pins]; \
  UCHAR  bNrOutputPins;			/* number of output pins of this element */ \
  UCHAR  bInTerminalLink;		/* the terminal ID of the Input Terminal to which this Element is connected */ \
  UCHAR  bOutTerminalLink;		/* the terminal ID of the Output Terminal to which this Element is connected */ \
  UCHAR  bElCapsSize;			/* size (in bytes) of the bmElementCaps field */ \
  UCHAR  bmElementCaps[cap_size];  /* element capabilities */ \
  UCHAR  iElement;				/* index of a string descriptor for the MIDI OUT Jack */ \
} USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_##unit_id, *PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_##unit_id

typedef struct _USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_INTERFACE */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_MS_DESCRIPTOR_MIDI_ELEMENT */
  UCHAR  bElementID;			/* unique ID for the Element within the USB-MIDI function */
  UCHAR  bNrInputPins;			/* number of input pins of this Element */ \
  /* source structure */
  //struct {
  //UCHAR  baSourceID;			/* ID of the entity to which N-th pin of this Element is connected */
  //UCHAR  baSourcePin;			/* output pin number of the entity to which N-th pin of this Element is connected */
  //} saSource[num_input_pins];
  //UCHAR  bNrOutputPins;			/* number of output pins of this element */
  //UCHAR  bInTerminalLink;		/* the terminal ID of the Input Terminal to which this Element is connected */
  //UCHAR  bOutTerminalLink;		/* the terminal ID of the Output Terminal to which this Element is connected */
  //UCHAR  bElCapsSize;			/* size (in bytes) of the bmElementCaps field */ \
  //UCHAR  bmElementCaps[cap_size];  /* element capabilities */
  //UCHAR  iElement;				/* index of a string descriptor for the MIDI OUT Jack */
} USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR, *PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR;

#define USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_SOURCE_OFFSET		5

typedef struct _USB_AUDIO_MIDI_ELEMENT_INFORMATION {
  UCHAR  bNrOutputPins;			/* number of output pins of this element */
  UCHAR  bInTerminalLink;		/* the terminal ID of the Input Terminal to which this Element is connected */
  UCHAR  bOutTerminalLink;		/* the terminal ID of the Output Terminal to which this Element is connected */
  UCHAR  bElCapsSize;			/* size (in bytes) of the bmElementCaps field */ \
  UCHAR  bmElementCaps[1];		/* element capabilities */
} USB_AUDIO_MIDI_ELEMENT_INFORMATION, *PUSB_AUDIO_MIDI_ELEMENT_INFORMATION;

#define USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_INFORMATION_OFFSET(num_input_pins)		(USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_SOURCE_OFFSET+(2*num_input_pins))
#define USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_IELEMENT_OFFSET(num_input_pins, cap_size)	(USB_AUDIO_MIDI_ELEMENT_DESCRIPTOR_SOURCE_OFFSET+sizeof(USB_AUDIO_MIDI_ELEMENT_INFORMATION)-1+(2*num_input_pins)+cap_size)

// audio class-specific MS bulk data endpoint descriptor
typedef struct _USB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR {
  UCHAR  bLength;				/* size of the descriptor */
  UCHAR  bDescriptorType;		/* must be USB_AUDIO_CS_ENDPOINT */
  UCHAR  bDescriptorSubtype;	/* must be USB_AUDIO_EP_DESCRIPTOR_GENERAL */
  UCHAR  bNumEmbMIDIJacks;		/* number of embedded MIDI jacks */
  UCHAR  baAssocJackID[1];		/* IDs of the embedded jacks associated with this endpoint */
} USB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR, *PUSB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR;

#include <poppack.h>

#endif
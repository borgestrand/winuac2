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
/****************************************************************************
 * Copyright (C) 1999  Creative Technology, Ltd.  All rights reserved.
 *
 ****************************************************************************
 *  File:		midenum.h
 *
 *  Standard enumerations and defines used by any MIDI state machine
 *
 ****************************************************************************
 */

#ifndef __MIDIENUM_H
#define __MIDIENUM_H

#define MAX_NRPN_PALLETTE 2
#define MAX_NRPN_PARAM 27

enum enMIDIChannel
{
  emcChannel1,
  emcChannel2,
  emcChannel3,
  emcChannel4,
  emcChannel5,
  emcChannel6,
  emcChannel7,
  emcChannel8,
  emcChannel9,
  emcChannel10,
  emcChannel11,
  emcChannel12,
  emcChannel13,
  emcChannel14,
  emcChannel15,
  emcChannel16,
  emcEndRealChannels,
  emcViennaChannel=emcEndRealChannels,
  emcAuditionChannel,
  emcEndChannel
};

#define   NOP          0xF8     // No Operation
#define   MEASURE_END  0xF9     // Measure End
#define   DATA_END     0xFC     // Data End

//// MPU Messages ////

#define REQUEST_TRACK(x)           (0xF0 + ((x) - 1))
/*
#define   REQUEST_TRACK1  0xF0  // Track Data Request for Track 1
#define   REQUEST_TRACK2  0xF1
#define   REQUEST_TRACK3  0xF2
#define   REQUEST_TRACK4  0xF3
#define   REQUEST_TRACK5  0xF4
#define   REQUEST_TRACK6  0xF5
#define   REQUEST_TRACK7  0xF6
#define   REQUEST_TRACK8  0xF7  // Track Data Request for Track 8
*/

#define   TIME_OUT     0xF8     // Timing Overflow
#define   CONDUCTOR    0xF9     // Conductor Data Request
#define   ALL_END      0xFC     // All End
#define   CLOCK_OUT    0xFD     // Clock to Host
#define   MPU_ACK      0xFE     // Acknowledge
#define   SYS_MESSAGE  0xFF     // System Message

///////////////////////// MPU Commands ////////////////////////////
//   A full control summary is given below, but only most common   //
//   commands are defined.  The table contains every bit-mapped    //
//   possibility for the START, STOP, and CONTINUE commands:       //
/////////////////////////////////////////////////////////////////////
/* Command      MIDI      Play     Record      Comment
 _______      ____      ____     ______      _______

 00           -         -        -           Not Used
 01           Stop      -        -
 02           Start     -        -
 03           Continue  -        -
 04           -         Stop     -
 05           Stop      Stop     -           Stop Play
 06           Start     Stop     -
 07           Continue  Stop     -
 08           -         Start    -
 09           Stop      Start    -
 0A           Start     Start    -           Start Play
 0B           Continue  Start    -           Continue Play
 0C to 0F     -         -        -           Not Used

 10           -         -       Stop
 11           Stop      -       Stop         Stop Record
 12           Start     -       Stop
 13           Continue  -       Stop
 14           -         Stop    Stop
 15           Stop      Stop    Stop         Stop over-dub
 16           Start     Stop    Stop
 17           Continue  Stop    Stop
 18           -         Start   Stop
 19           Stop      Start   Stop
 1A           Start     Start   Stop
 1B           Continue  Start   Stop
 1C to 1F     -         -       -            Not Used

 20           -         -       Standby      Record Standby
 21           Stop      -       Standby
 22           Start     -       Start        Start Recording
 23           Continue  -       Start        Continue Recording
 24           -         Stop    Standby
 25           Stop      Stop    Standby
 26           Start     Stop    Start
 27           Continue  Stop    Start
 28           -         Start   Standby
 29           Stop      Start   Standby
 2A           Start     Start   Start        Start over-dub
 2B           Continue  Start   Start        Start over-dub
 2C to 2F     -         -       -            Not Used			  */
///////////////////////////////////////////////////////////////////


#define   STOP_PLAY         0x05
#define   START_PLAY        0x0A
#define   CONTINUE_PLAY     0x0B

#define   STOP_RECORD       0x11
#define   RECORD_STANDBY    0x20
#define   START_RECORD      0x22
#define   CONTINUE_RECORD   0x23

#define   STOP_OVERDUB      0x15
#define   START_OVERDUB     0x2A
#define   CONTINUE_OVERDUB  0x2B


/////////////////////////////////////////////////////////////////////
//     These Command Switches function for just one direction.     //
//     If you wish to escape from these modes, you must issue      //
//     the RESET_401 command.                                      //
/////////////////////////////////////////////////////////////////////

#define   NOTES_OFF        0x30         // All Notes Off
#define   NO_REALTIME      0x32         // No Real Time
#define   THRU_CHANL_OFF   0x33         // Thru : Off on Channels
#define   WITH_TIME        0x34         // With Timing Byte : on
#define   MODE_MESSAGES    0x35         // Mode Message : on
#define   EXCLUSIVE_THRU   0x36         // Exclusive Thru : on
#define   COMMON_THRU      0x38         // Common to Host : on
#define   REAL_THRU        0x39         // Real Time to Host : on
#define   UART             0x3F         // UART Mode Everything Disabled

//// Channel reference table numbers are normally computed ////
/*
40 - 4F A  Sets the Channel Reference for Table A
50 - 5F B
60 - 6F C
70 - 7F D
*/

#define   INTERNAL_CLOCK   0x80         // Internal Clock
#define   FSK_CLOCK        0x81         // FSK Clock
#define   MIDI_CLOCK       0x82         // MIDI Clock
#define   METRO_ON_WOUT    0x83         // Metronome : on - w/o Accents
#define   METRO_OFF        0x84         // Metronome : off
#define   METRO_ON_WITH    0x85         // Metronome : on - with Accents
#define   BENDER_OFF       0x86         // Bender : off
#define   BENDER_ON        0x87         // Bender : on
#define   THRU_OFF         0x88         // MIDI Thru : off
#define   THRU_ON          0x89         // MIDI Thru : on
#define   DSTOP_MODE_OFF   0x8A         // Data in Stop Mode : off
#define   DSTOP_MODE_ON    0x8B         // Data in Stop Mode : on
#define   MEASURE_END_OFF  0x8C         // Send Measure End : off
#define   MEASURE_END_ON   0x8D         // Send Measure End : on
#define   CONDUCTOR_OFF    0x8E         // Conductor : off
#define   CONDUCTOR_ON     0x8F         // Conductor : on
#define   REAL_TIME_OFF    0x90         // Real Time Affecton : off
#define   REAL_TIME_ON     0x91         // Real Time Affection : on
#define   FSK_INTERNAL     0x92         // FSK to Internal
#define   FSK_MIDI         0x93         // FSK to MIDI
#define   CLOCK_OFF        0x94         // Clock to Host : off
#define   CLOCK_ON         0x95         // Clock to Host : on
#define   EXCLUSIVE_OFF    0x96         // Exclusive to Host : off
#define   EXCLUSIVE_ON     0x97         // Exclusive to Host : on

#define   CHANA_OFF        0x98         // Reference Table A : off
#define   CHANA_ON         0x99         // A : on
#define   CHANB_OFF        0x9A         // B : off
#define   CHANB_ON         0x9B         // B : on
#define   CHANC_OFF        0x9C         // C : off
#define   CHANC_ON         0x9D         // C : on
#define   CHAND_OFF        0x9E         // D : off
#define   CHAND_ON         0x9F         // D : on

//// Reading Data ////

#define REQUEST_TRACK_COUNTER(x)   (0xA0 + (x))

#define   REQUEST_REC_CNT  0xAB         // Request Record Counter
#define   REQUEST_VER      0xAC         // Request Version
#define   REQUEST_REV      0xAD         // Request Revision
#define   REQUEST_TEMPO    0xAF         // Request Tempo

#define   SYNTH_CONTROL    0xB0         // Control Starts at Channel 1
#define   RESET_RTEMPO     0xB1         // Resets Relative Tempo
#define   CLEAR_PCOUNT     0xB8         // Clear Play Counters
#define   CLEAR_PMAP       0xB9         // Clear Play Map - All Notes Off
#define   CLEAR_REC        0xBA         // Clear Record Counter

#define PROGRAM_CONTROL    0xC0         // Program Change Starts at Ch. 1

#define TIMEBASE(x)      (0xC2 + (((x) - 48) / 24))

#define WANTTO_SEND_DATA(x)               (0xD0 + ((x) - 1))

#define   WS_SYS           0xDF         // Want to Send System Message

//// Set Conditions and Values (follow by 1 byte of data)  ////

#define   SET_TEMPO          0xE0       // Set Tempo
#define   SET_RTEMPO         0xE1       // Relative Tempo
#define   SET_GRAD           0xE2       // Set Graduation
#define   CLOCKS_PER_BEAT    0xE4       // MIDI Clocks per Metronome Beep
#define   BEATS_PER_MEASURE  0xE6       // Number of Beats per Measure
#define   INTERNAL_CLOCKS    0xE7       // int * 4 / Clock to Host
#define   ACTIVATE_TRACK     0xEC       // Active Tracks on/off
#define   SEND_PCOUNT        0xED       // Send Play Counter on/off
#define   CHAN_ON1_8         0xEE       // Acceptable Channels 1-8 on/off
#define   CHAN_ON9_16        0xEF       // Acceptable Channels 9-16 on/off

#define   RESET_MPU401       0xFF       // Reset the MPU-401


/////////////////////////////////////////////////////////////////////
//  Channel Voice Messages:  Byte Format 1xxx nnnn where nChannel //
//  Note:  Add the Channel Number to the Status Byte to send the   //
//    respective channel the Voice Message,e.g.: 0x90 | 3  0x93   //
//    means send a Note On message to channel 4.                   //
/////////////////////////////////////////////////////////////////////
//
//                       Status Byte      Data Byte1      Data Byte2
//                       -----------      ----------      ----------
#define NOTE_OFF          0x80    //      Note Number     Release Velocity
#define NOTE_ON           0x90    //      Note Number     Attack Velocity
#define POLYKEY_PRESSURE  0xA0    //      Note Number     Pressure Value
#define CONTROL_CHANGE    0xB0    //      Controller ID   Controller Value
#define PROGRAM_CHANGE    0xC0    //      Program Number  ----------
#define CHANNEL_PRESSURE  0xD0    //      Pressure Value  ----------
#define PITCH_WHEEL       0xE0    //      Pitch Bend LSB  Pitch Wheel MSB

///// Control Change: 14-Bit Controller ID Numbers  /////
//  Note:  The format for Control Change (0xB0) is 1011 nnnn where nChannel.
//         Byte 2:  0ccc cccc where cController ID Number.
//         Byte 3:  0vvv vvvv where vController Value (0-127).
//                          Controller Number:  MSB (Byte 2)
//                          --------------------------------
#define BANK_SELECT          0x00
#define MOD_WHEEL            0x01
#define BREATH_CONTROL       0x02

#define FOOT_CONTROL         0x04
#define PORTAMENTO_TIME      0x05
#define DATA_ENTRY_MSB       0x06
#define VOLUME               0x07
#define BALANCE              0x08

#define PAN_CONTROL          0x0A
#define EXPRESSION_CONTROL   0x0B

#define GENERAL_CONTROL0     0x10
#define GENERAL_CONTROL1     0x11
#define GENERAL_CONTROL2     0x12
#define GENERAL_CONTROL3     0x13

// Note:  The following Controller LSB's can apply to Controllers 0-31,
//        but as you can see above, not all of them are defined.  The
//        LSB is sent as another complete Control Change message to achieve
//        high-resolution of the Controller.  The LSB is optional...
//                          Controller Number:  Optional LSB (Byte 2)
//                          --------------------------------
#define CONTROLLER_LSB(x)   (0x20 + (x)) // LSB value for Controller x


/////////// Control Change: 7-Bit Controller ID Numbers  ////////////
//  Note:  The format for Control Change (0xB0) is 1011 nnnn where //
//         nChannel.                                              //
//         Byte 2:  0ccc cccc where cController ID Number.        //
//         Byte 3:  0vvv vvvv where vController Value.            //
/////////////////////////////////////////////////////////////////////
//                          Controller Number:  MSB (Byte 2)
//                          --------------------------------
#define CC_SUSTAIN              0x40
#define CC_PORTAMENTO           0x41   // On - Off
#define CC_SOSTENUTO            0x42   // On - Off
#define CC_SOFT_PEDAL           0x43

#define HOLD_2               0x45   // On - Off

#define GS_HARMONIC_CONTENT  0x47
#define GS_ATTACK            0x48
#define GS_RELEASE           0x49
#define GS_BRIGHTNESS        0x4A

#define GENERAL_CONTROL5     0x50
#define GENERAL_CONTROL6     0x51
#define GENERAL_CONTROL7     0x52
#define GENERAL_CONTROL8     0x53

#define REVERB_DEPTH         0x5B
#define TREMOLO_DEPTH        0x5C
#define CHORUS_DEPTH         0x5D
#define CELESTE_DEPTH        0x5E
#define PHASER_DEPTH         0x5F

///// Parameter Value /////

#define DATA_INCREMENT       0x60
#define DATA_DECREMENT       0x61

///// Parameter Selection /////

#define NONREGISTERED_LSB     0x62  // Non-Registered Parameter Number LSB
#define NONREGISTERED_MSB     0x63  // Non-Registered Parameter Number MSB
#define REGISTERED_LSB        0x64  // Registered Parameter Number LSB
#define REGISTERED_MSB        0x65  // Registered Parameter Number MSB

///// Reserved for Channnel Mode Messages /////

#define ALL_SOUND_OFF         0x78
#define RESET_CONTROLLERS     0x79
#define LOCAL_CONTROL         0x7A  // On - Off
#define ALL_NOTES_OFF         0x7B
#define OMNI_OFF              0x7C
#define OMNI_ON               0x7D
#define MONO_ON               0x7E
#define POLY_ON               0x7F


///// System Common Messages /////
//
//                                    Data Byte1         Data Byte2
//                                    ----------         ----------
#define MTC_QUARTER_FRAME     0xF1  // Message Type       -
#define SONG_POS_POINTER      0xF2  // Song Pos LSB       Song Pos MSB
#define SONG_SELECT           0xF3  // Song ID Number     -

#define F4					  0xF4  // Byte1			  Byte2 ...
				
#define TUNE_REQUEST          0xF6  // -                  -
#define EOX                   0xF7  // -                  -


///// System Real-Time Messages /////

#define TIMING_CLOCK          0xF8

#define START                 0xFA
#define CONTINUE              0xFB
#define STOP                  0xFC

#define ACTIVE_SENSING        0xFE
#define SYSTEM_RESET          0xFF


///// System Exclusive Messages /////
// There is more which can be defined here...

#define SYSEX      0xF0     // File Format: <0xF0><length><bytes><0xF7>
//      EOX        0xF7     // End of Exclusive
#define SYSEX_ESC  0xF7     // Another form of a Sysex event with
			    //   format: <0xF7><length><bytes>

#define BYTE3_MAN_ID        0x00  // If the next data byte == 0
#define BYTE1_MAN_ID        0x7C  // If the next data byte == (0x01..0x7C)
#define NONREALTIME_SYSEX   0x7E
#define REALTIME_SYSEX      0x7F

// ID's supported
#define GENERAL_ID			0x00
#define EMU_ID              0x18
#define ROLAND_ID           0x41
#define EMU8200_ID          0x40
#define PROTEUS_MPS_ID      0x08

// If ID is GENERAL_ID...
#define CREATIVE_ID_1		0x20
#define CREATIVE_ID_2		0x21

// Roland ID's
#define ROL_SC_1            0x10
#define ROL_SC_2            0x42
#define ROL_SC_3            0x12

// Devices
#define ALL_DEVICES         0x7F

// Universal Function IDs
#define UNIV_MASTER         0x04
#define UNIV_GM             0x09
#define UNIV_VOLUME         0x01
#define UNIV_FILEREF        0x0B
#define GM_ON               0x01
#define GM_OFF              0x02

// Function IDs
#define PROT_MODE_SELECT    0x00
#define PROT_EFFECT_SELECT  0x01
#define PROT_PARAM_CHANGE   0x02
#define ROL_FX_SELECT1      0x40
#define ROL_FX_SELECT2      0x01

// File reference selections
#define FILEREF_OPEN          0x01
#define FILEREF_SELECT        0x02
#define FILEREF_OPENANDSELECT 0x03
#define FILEREF_CLOSE         0x04

// Extended data IDs for File reference SysEx
// Per Tom Savell, approved by MMA 2001-01-04
#define FILEREF_EXT_BANKOFFS_ID_1 0x00
#define FILEREF_EXT_BANKOFFS_ID_2 0x01

// Effects Selections
#define PROT_REVERB         0x00
#define PROT_CHORUS         0x01
#define PROT_QSOUND         0x02
#define ROL_REVERB          0x30
#define ROL_CHORUS          0x38

// Emu 8200 Routine IDs
#define EMU_SYSEX_SETTING   0x00
#define EMU_DIAG            0x01
#define EMU_EDIT            0x02
#define EMU_SYNTH           0x03
#define EMU_EFFECT_SELECT   0x04
#define EMU_EDIT_PRETRANS   0x05
#define EMU_NAME_DUMP       0x06
#define EMU_EQ_SETTING      0x07

// Emu8200 setting (EMU_SYSEX_SETTING) functions
#define EMU_SET_DEVICE_ID   0x00
#define EMU_SET_BASE_CHAN   0x01

// Emu 8200 Limited Edit IDs
#define EMU_QUERY_EDIT      0x7E

// Emu 8200 Synthesizer Modes
#define MODE_GENERAL_MIDI 0
#define MODE_GENERAL_SYNTH 1
#define MODE_MT32 2
#define MODE_FULL_CUSTOM 3
#define MODE_DEFAULT (MODE_FULL_CUSTOM)   // Emu8200 initializes with this
#define MODE_MAX_VALUE (MODE_FULL_CUSTOM) // Parser ignores if greater

// Emu 8200 Universal Functions
#define EMU_SYSEX_NEWFUNC   0x7E
#define EMU_SYSEX_RETURN    0x7F

// Emu 8200 Diagnostic Functions
#define EMU_DIAG_SINE_ON    0x00
#define EMU_DIAG_SINE_OFF   0x01
#define EMU_DIAG_SROM_TEST  0x02
#define EMU_DIAG_SRAM_TEST  0x03
#define EMU_DIAG_TRAM_TEST  0x04
#define EMU_DIAG_PROM_TEST  0x05
#define EMU_DIAG_PRAM_TEST  0x06
#define EMU_DIAG_VERS_TEST  0x07

// Emu8200 Name Types
#define EMU_NAME_BANK         0x00
#define EMU_NAME_BANK_PROGRAM 0x01
#define EMU_NAME_DRUM         0x02
#define EMU_NAME_DRUM_PROGRAM 0x03

// Emu8200 EQ Setting Types
#define EMU_EQ_BASS           0x00
#define EMU_EQ_TREBLE         0x01

// Creative routine IDS
#define CREATIVE_SFMS_FUNCTION 0x5F
#define CREATIVE_SF_LOAD_FUNCTION 0x10

#define CREATIVE_SF_LOAD_BANK      0x00
#define CREATIVE_SF_LOAD_PRESET    0x01
#define CREATIVE_SF_UNLOAD_BANK    0x02
#define CREATIVE_SF_UNLOAD_PRESET  0x03
#define CREATIVE_SF_LOAD_RESET_ALL 0x7F

// RPNs
#define RPN_GM_PALLETTE				0x0
#define RPN_PITCH_BEND_SENSITIVITY	0x0
#define RPN_FINE_TUNING				0x1
#define RPN_KEY_OFFSET				0x2

#define RPN_3D_PALLETTE				0x3D
#define RPN_3D_AZIMUTH				0x00
#define RPN_3D_ELEVATION			0x01
#define RPN_3D_GAIN					0x02
#define RPN_3D_DISTANCE_RATIO		0x03
#define RPN_3D_MAX_DISTANCE			0x04
#define RPN_3D_GAIN_AT_MAX_DISTANCE	0x05
#define RPN_3D_REF_DISTANCE_RATIO	0x06
#define RPN_3D_PAN_SPREAD			0x07
#define RPN_3D_PAN_ROLL				0x08


// NRPNs
#define NRPN_ROLAND         0x01
#define NRPN_SOUNDFONT		0x78
#define NRPN_PERF_DLT       0x7D
#define NRPN_PERF_ABS       0x7E
#define NRPN_SBAWE			0x7F

///// Universal Real-Time SysEx /////
// Not implemented at this time....


///// MIDI File Definitions /////

#define META_EVENT  0xFF         // File Format: <0xFF><type><length><bytes>

///// Other Defines /////

#define MIDICHANNELS     0x21    // The number of MIDI channels
#define REALMIDICHANNELS 0x20

#define RANGETOP       127       // Top of the MIDI Range
#define RANGEBOTTOM    0         // Bottom of the MIDI Range

#define MIDI_CLOCKS_PER_BEAT  24
#define MASTER_CLOCK_RATE     96
/*
#define NO_MPU       0           // These #defineants allow the program to
#define STANDBY      1           //   point to the correct MIDI parser
#define PLAY_MODE    2           //   and its subsequent supporting routines
#define RECORD_MODE  3
*/
///// Channel Defines (channels 0x00 thru 0x0F /////

#define CHANNEL_BASE(x)   (--(x))
#define GET_CHANNEL(x)    ((x) & 0x0F)


#endif

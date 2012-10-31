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
#ifndef __ASIO_DLL_VER_H
#define __ASIO_DLL_VER_H

#include <windows.h>

/** The CTVERSION structure is a simple structure that defines a major and minor version.
  */
typedef struct ctVersionTag
{
	USHORT usMajor;	/**< Major version value */
	USHORT usMinor;   /**< Minor version value */
} CTVERSION, *PCTVERSION, **PPCTVERSION;

#define CTMARKMAGIC 0x10301968

/** The CTFILEVER structure is used as a binary signature for all UDA compiled files that
    is guaranteed to be consistent across all binaries, regardless of the build environment.
 */
typedef struct ctFileVersionTag
{
	CHAR	 szFileVer[10];   /**< This field must hold the text "CTFILEVER" as the indicator that the file version signature begins here.*/
	USHORT usMajorVerNum;	  /**< This field holds the major version number, which is usually consistent for bi-directional compatible binary builds. */
	USHORT usMinorVerNum;   /**< This field holds the minor version number, which combined with usMajorVerNum is usually consistent for bi-directional compatible binary builds which offer similar functionality sets. */
	USHORT usCodeBuildNum;  /**< This field holds the code build number, which signifies a binary whose only difference is a build date or bug fix. */
	USHORT usLangBuildNum;  /**< This field holds the language build number, which signifies a binary build whose only difference is in language support. */
	USHORT usPlatformsSupported; /**< This field holds the platform code number, which signifies any platform restrictions the binary holds. Most UDA binaries are platform independent, and therefore have this value set to 0.*/
	USHORT usLangCode;       /**< This field holds the language code number, which signifies any language restrictions the binary holds. Most UDA binaries are langauge independent, and therefore have this value set to 0.*/
	USHORT usPad;            /**< This field pads the structure to maintain 4-byte alignment */
	ULONG  ulMarkMagic;      /**< This field marks the location where a post-build binary mark may be placed in the binary. Its value must be 0x10301968. Marking is used when distributing binaries to third parties, to facilitate tracing in the event such binaries leak. */
	ULONG  ulMarkNum;        /**< This field is the location of the mark. Standard builds must set this value to 0. Marked builds would modify this value, and then update any OS specific binary headers to do checksum fixup. A utility called CTFILEMARK is used to mark Windows binaries. */
} CTFILEVER, *PCTFILEVER, **PPCTFILEVER;

// IMPORTANT
//
// We have the notion of two sets of version numbers. One that Microsoft
//   forces upon us that in no way reflects internal development and one
//   that we have to use on our own that uses a format that is consistant
//   across all binaries and that reflects our internal development.
//
// These values must match those in the .RC and .VRC files. We plan to generate
//   a tool to do this for us in the build process. Once the tool is developed,
//   this will be the one and only file that ever needs to be updated.
//
// The Microsoft version number is always 4.07.00.xxxx for Win9xVxD drivers.
//   xxxx is some number that Microsoft assigns to us upon passing WHQL, and
//   should be greater than 95 decimal.
//
// The Creative version number starts from 1.0.0.0, and is fully defined in
//   the UDA documentation area.
//
// The DDB version number in a VxD will be set to the Microsoft values.
//
// The Microsoft binary version number must be set to the Microsoft values,
//   because that is in the WHQL spec.
//
// The Microsoft string version number will be set to reflect both.
//
// That last bit is what injects the Creative version number into the binary
//   image of the driver. It abides by a format that is consistant across
//   all Creative binaries, and is defined in 'ctdef.h' One single source file 
//   must set the __CTINSERTFILEVERSION__ flag.
//
// See WHQL and UDA documentation for more details.

#define CTMAJORVER  1
#define CTMINORVER  2
#define CTCODEBUILD 52
#define CTLANGBUILD 0

#define MSMAJORVER  6
#define MSMINORVER  0
#define MSPADVAL    1
#define MSWHQLBUILD 1

#ifndef __CTINSERTFILEVERSION__
extern
#else
static
#endif
CTFILEVER gVerInfo
#ifdef __CTINSERTFILEVERSION__
= {"CTFILEVER", CTMAJORVER, CTMINORVER, CTCODEBUILD, CTLANGBUILD, 0, 0, 0, CTMARKMAGIC, 0};
#else
;
#endif



#endif


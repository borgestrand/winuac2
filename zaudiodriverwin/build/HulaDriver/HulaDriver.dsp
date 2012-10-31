# Microsoft Developer Studio Project File - Name="HulaDriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=HulaDriver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HulaDriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HulaDriver.mak" CFG="HulaDriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HulaDriver - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "HulaDriver - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "HulaDriver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f HulaDriver.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "HulaDriver.exe"
# PROP BASE Bsc_Name "HulaDriver.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "..\ddkbuild -WNET free ..\..\driver\usbaud10"
# PROP Rebuild_Opt "-cZ"
# PROP Target_File "emusba10.sys"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "HulaDriver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f HulaDriver.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "HulaDriver.exe"
# PROP BASE Bsc_Name "HulaDriver.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "..\ddkbuild -WNET checked ..\..\driver\usbaud10"
# PROP Rebuild_Opt "-cZ"
# PROP Target_File "emusba10.sys"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "HulaDriver - Win32 Release"
# Name "HulaDriver - Win32 Debug"

!IF  "$(CFG)" == "HulaDriver - Win32 Release"

!ELSEIF  "$(CFG)" == "HulaDriver - Win32 Debug"

!ENDIF 

# Begin Group "adapter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\Guids.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\hula\Hula.rc
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\hula\HulaVer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\hula\HulaVer.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\KsAdapter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\KsAdapter.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\Main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\hula\Resource.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\hula\SOURCES
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\adapter\hula\timebomb.h
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Audio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Audio.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\AudioFifo.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Convert.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\DataProcessingConstants.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Element.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Element.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Entity.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Entity.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Jack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Jack.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Midi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Midi.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\MidiEnum.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\MidiFifo.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\MidiParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\MidiParser.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\MidiQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Profile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Profile.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\SOURCES
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Terminal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Terminal.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Unit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\Unit.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\UsbDev.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\core\UsbDev.h
# End Source File
# End Group
# Begin Group "filter"

# PROP Default_Filter ""
# Begin Group "audio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Descriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Descriptor.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Event.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Factory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Factory.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Filter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Formats.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Pin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Pin.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\SOURCES
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\audio\Tables.h
# End Source File
# End Group
# Begin Group "control"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\control\Factory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\control\Factory.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\control\Filter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\control\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\control\SOURCES
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\control\Tables.h
# End Source File
# End Group
# Begin Group "midi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Descriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Descriptor.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Factory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Factory.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Filter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Pin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Pin.h
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\SOURCES
# End Source File
# Begin Source File

SOURCE=..\..\driver\usbaud10\filter\midi\Tables.h
# End Source File
# End Group
# End Group
# End Target
# End Project

#
#   This file is part of the EMU CA0189 USB Audio Driver.
#
#   Copyright (C) 2008 EMU Systems/Creative Technology Ltd. 
#
#   This driver is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This driver is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public License
#   along with this library.   If not, a copy of the GNU Lesser General Public 
#   License can be found at <http://www.gnu.org/licenses/>.
#


clean:
    del $(O)\$(TARGETNAME).$(TARGETEXT)
    del .\emaud.inf
    del .\emuxmidi.inf

default:
	copy /Y ..\..\..\inf\hula.inf /B .\emaud.inf
 

chinese:
	copy /Y .\emaud.inf /B + ..\..\..\inf\locale\chinese.inf /B + ..\..\..\inf\strings.inf /B .\emaud.inf

french:
	copy /Y .\emaud.inf /B + ..\..\..\inf\locale\french.inf /B + ..\..\..\inf\strings.inf /B .\emaud).inf

german:
	copy /Y .\emaud.inf /B + ..\..\..\inf\locale\german.inf /B + ..\..\..\inf\strings.inf /B .\emaud.inf

japanese:
	copy /Y .\emaud.inf /B + ..\..\..\inf\locale\japanese.inf /B + ..\..\..\inf\strings.inf /B .\emaud.inf

portuguese:
	copy /Y .\emaud.inf /B + ..\..\..\inf\locale\portuguese.inf /B + ..\..\..\inf\strings.inf /B .\emaud.inf

spanish:
	copy /Y .\emaud.inf /B + ..\..\..\inf\locale\spanish.inf /B + ..\..\..\inf\strings.inf /B .\emaud.inf

BINPLACE_CMD=..\..\..\..\bin\win\scripts\binplace.cmd ALL $(_BUILDARCH) $(FREEBUILD) $(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).$(TARGETEXT) $(TARGETNAME).dat emaud.inf
_BINPLACE_TARGET_COMMAND=$(BINPLACE_CMD)
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
    del $(O)\*.ini
    del placefile

ctzapxx:
        $(HULAROOT)\bin\win\encrypt $*.ini
        move .\ctzapxx(e).ini $(O)\$*.ini
        echo $(O)\$*.ini >> placefile

ctzapxxmidi:
        $(HULAROOT)\bin\win\encrypt $*.ini
        move .\ctzapxxmidi(e).ini $(O)\$*.ini
        echo $(O)\$*.ini >> placefile

emaud:
        $(HULAROOT)\bin\win\encrypt $*.ini
        move .\emaud(e).ini $(O)\$*.ini
        echo $(O)\$*.ini >> placefile

emuxmidi:
        $(HULAROOT)\bin\win\encrypt $*.ini
        move .\emuxmidi(e).ini $(O)\$*.ini
        echo $(O)\$*.ini >> placefile

placement:
	..\..\bin\win\scripts\binplace.cmd ALL $(_BUILDARCH) $(FREEBUILD) @placefile

BINPLACE_CMD=..\..\bin\win\scripts\binplace.cmd ALL $(_BUILDARCH) $(FREEBUILD) $(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).$(TARGETEXT)
_BINPLACE_TARGET_COMMAND=$(BINPLACE_CMD)

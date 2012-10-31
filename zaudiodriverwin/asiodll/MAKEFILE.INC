#/*
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
#*/
clean:
    del $(O)\$(TARGETNAME).$(TARGETEXT)

!IF $(386)
POST_BUILD_CMD=postbld.cmd $(TARGETPATH)\$(TARGET_DIRECTORY) $(TARGETNAME) $(TARGETEXT)
BINPLACE_CMD=$(HULAROOT)\bin\win\scripts\binplace.cmd $(PACKAGE_TYPE) $(_BUILDARCH) $(FREEBUILD) $(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).$(TARGETEXT) $(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).drv $(TARGETNAME).dat
!ELSE
BINPLACE_CMD=$(HULAROOT)\bin\win\scripts\binplace.cmd $(PACKAGE_TYPE) $(_BUILDARCH) $(FREEBUILD) $(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).$(TARGETEXT) $(TARGETNAME).dat
!ENDIF
_BINPLACE_TARGET_COMMAND=$(BINPLACE_CMD)

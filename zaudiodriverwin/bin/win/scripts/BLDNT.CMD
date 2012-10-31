@rem
@rem   This file is part of the EMU CA0189 USB Audio Driver.
@rem
@rem   Copyright (C) 2008 EMU Systems/Creative Technology Ltd. 
@rem
@rem   This driver is free software; you can redistribute it and/or
@rem   modify it under the terms of the GNU Library General Public
@rem   License as published by the Free Software Foundation; either
@rem   version 2 of the License, or (at your option) any later version.
@rem
@rem   This driver is distributed in the hope that it will be useful,
@rem   but WITHOUT ANY WARRANTY; without even the implied warranty of
@rem   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
@rem   Library General Public License for more details.
@rem
@rem   You should have received a copy of the GNU Library General Public License
@rem   along with this library.   If not, a copy of the GNU Lesser General Public 
@rem   License can be found at <http://www.gnu.org/licenses/>.
@rem

@set RESET_PACKAGE_TYPE=0
@if "%PACKAGE_TYPE%"=="" (
   set RESET_PACKAGE_TYPE=1
   set PACKAGE_TYPE=nt4
)
@set RESET_PACKAGE_MANIFEST=0
@if "%PACKAGE_MANIFEST%"=="" (
   if "%i386_PACKAGE_MANIFEST%"=="" (
      if "%AMD64_PACKAGE_MANIFEST%"=="" (
         if "%IA64_PACKAGE_MANIFEST%"=="" (
            set RESET_PACKAGE_MANIFEST=1
            set PACKAGE_MANIFEST=%HULAROOT%\bin\win\scripts\package.nt4.manifest
         )
      )
   )
)
@set RESET_PACKAGE_INSTALL_DIRECTORY=0
@if "%PACKAGE_INSTALL_DIRECTORY%"=="" (
   if "%i386_PACKAGE_INSTALL_DIRECTORY%"=="" (
      if "%AMD64_PACKAGE_INSTALL_DIRECTORY%"=="" (
         if "%IA64_PACKAGE_INSTALL_DIRECTORY%"=="" (
            set RESET_PACKAGE_INSTALL_DIRECTORY=1
            if "%DDKBUILDENV%"=="fre" (
               set PACKAGE_INSTALL_DIRECTORY=%ReleaseBin%\install
            ) else (
               set PACKAGE_INSTALL_DIRECTORY=%WinSysDest%\install
            )
         )
      )
   )
)
@set RESET_PACKAGE_SYMBOL_DIRECTORY=0
@if "%PACKAGE_SYMBOL_DIRECTORY%"=="" (
   if "%i386_PACKAGE_SYMBOL_DIRECTORY%"=="" (
      if "%AMD64_PACKAGE_SYMBOL_DIRECTORY%"=="" (
         if "%IA64_PACKAGE_SYMBOL_DIRECTORY%"=="" (
            set RESET_PACKAGE_SYMBOL_DIRECTORY=1
            if "%DDKBUILDENV%"=="fre" (
               set PACKAGE_SYMBOL_DIRECTORY=%ReleaseBin%\symbols
            ) else (
               set PACKAGE_SYMBOL_DIRECTORY=%WinSysDest%\symbols
            )
         )
      )
   )
)

@set EXCLUDE_DIRS=~wdm ~w9x ~win9xvxd
@if not "%_BUILDARCH%"=="x86" (
   set EXCLUDE_DIRS=%EXCLUDE_DIRS% ~thunk
)

@build.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 %EXCLUDE_DIRS% * -nmake "/f makefile.nt"
@set EXCLUDE_DIRS=

@if %RESET_PACKAGE_SYMBOL_DIRECTORY%==1 (
   set PACKAGE_SYMBOL_DIRECTORY=
)
@set RESET_PACKAGE_SYMBOL_DIRECTORY=
@if %RESET_PACKAGE_INSTALL_DIRECTORY%==1 (
   set PACKAGE_INSTALL_DIRECTORY=
)
@set RESET_PACKAGE_INSTALL_DIRECTORY=
@if %RESET_PACKAGE_MANIFEST%==1 (
   set PACKAGE_MANIFEST=
)
@set RESET_PACKAGE_MANIFEST=
@if %RESET_PACKAGE_TYPE%==1 (
   set PACKAGE_TYPE=
)
@set RESET_PACKAGE_TYPE=

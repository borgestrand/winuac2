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

@set PackageType=%1
@set Platform=%2
@set Retail=%3
@set SourceFiles=%4 %5 %6 %7 %8 %9

@set i386=
@for /F "eol=; tokens=1 delims=, " %%i in ('echo %%386%%') do @set i386=%%i

@if "%PackageType%"=="none" goto skip_binplace
@if "%PackageType%"=="NONE" goto skip_binplace

@rem Setup the binplace tool.
@set BINPLACE_TOOL=@%HULAROOT%\bin\win\bitplace.exe
@set BINPLACE_LOG=
@set BINPLACE_OPTIONS=-e -f -:LOGPDB

@set PackageManifest=
@if "%AMD64%"=="1" (
   if not "%AMD64_PACKAGE_MANIFEST%"=="" (
      set PackageManifest=%AMD64_PACKAGE_MANIFEST%
   )
) else (
   if "%IA64%"=="1" (
      if not "%IA64_PACKAGE_MANIFEST%"=="" (
         set PackageManifest=%IA64_PACKAGE_MANIFEST%
      )
   ) else (
      if "%i386%"=="1" (
         if not "%i386_PACKAGE_MANIFEST%"=="" (
            set PackageManifest=%i386_PACKAGE_MANIFEST%
         )
      )
   )
)
@if "%PackageManifest%"=="" (
   set PackageManifest=%PACKAGE_MANIFEST%
) else (
   if not "%PACKAGE_MANIFEST%"=="" (
      set PackageManifest=%PACKAGE_MANIFEST%;%PackageManifest%
   )
)
@if "%PackageManifest%"=="" (
   goto end
)
@set BINPLACE_PLACEFILE=

@set PackageInstallDirectory=
@if "%AMD64%"=="1" (
   if not "%AMD64_PACKAGE_INSTALL_DIRECTORY%"=="" (
      set PackageInstallDirectory=%AMD64_PACKAGE_INSTALL_DIRECTORY%
   )
) else (
   if "%IA64%"=="1" (
      if not "%IA64_PACKAGE_INSTALL_DIRECTORY%"=="" (
         set PackageInstallDirectory=%IA64_PACKAGE_INSTALL_DIRECTORY%
      )
   ) else (
      if "%i386%"=="1" (
         if not "%i386_PACKAGE_INSTALL_DIRECTORY%"=="" (
            set PackageInstallDirectory=%i386_PACKAGE_INSTALL_DIRECTORY%
         )   
      )
   )
)
@if "%PackageInstallDirectory%"=="" (
   if "%PACKAGE_INSTALL_DIRECTORY%"=="" (
      if %Retail%==1 (
         set PackageInstallDirectory=%ReleaseBin%
      ) else (
         set PackageInstallDirectory=%WinSysDest%
      )
   ) else (
      set PackageInstallDirectory=%PACKAGE_INSTALL_DIRECTORY%
   )
) else (
   if not "%PACKAGE_INSTALL_DIRECTORY%"=="" (
      set PackageInstallDirectory=%PACKAGE_INSTALL_DIRECTORY%;%PackageInstallDirectory%
   )
)
@set BINPLACE_ROOTDIR=

@set PackageSymbolDirectory=
@if "%AMD64%"=="1" (
   if not "%AMD64_PACKAGE_SYMBOL_DIRECTORY%"=="" (
      set PackageSymbolDirectory=%AMD64_PACKAGE_SYMBOL_DIRECTORY%
   )
) else (
   if "%IA64%"=="1" (
      if not "%IA64_PACKAGE_SYMBOL_DIRECTORY%"=="" (
         set PackageSymbolDirectory=%IA64_PACKAGE_SYMBOL_DIRECTORY%
      )
   ) else (
      if "%i386%"=="1" (
         if not "%i386_PACKAGE_SYMBOL_DIRECTORY%"=="" (
            set PackageSymbolDirectory=%i386_PACKAGE_SYMBOL_DIRECTORY%
         )
      )   
   )
)
@if "%PackageSymbolDirectory%"=="" (
   if "%PACKAGE_SYMBOL_DIRECTORY%"=="" (
      if %Retail%==1 (
         set PackageSymbolDirectory=%ReleaseBin%
      ) else (
         set PackageSymbolDirectory=%WinSysDest%
      )
   ) else (
      set PackageSymbolDirectory=%PACKAGE_SYMBOL_DIRECTORY%
   )
) else (
   if not "%PACKAGE_SYMBOL_DIRECTORY%"=="" (
      set PackageSymbolDirectory=%PACKAGE_SYMBOL_DIRECTORY%;%PackageSymbolDirectory%
   )
)
@set BINPLACE_SYMBOLDIR=

@rem BINPLACE_SUBROOTDIR is set to "" to flatten the package. 
@rem This is to allow the manifest file fully control the layout of the package.
@set BINPLACE_SUBROOTDIR=""

@if /i "%Platform%"=="THK32" (
   set BINPLACE_EXTRASUBDIR=thunk
) else (
   set BINPLACE_EXTRASUBDIR=""
)

:parse
@if not "%PackageManifest%"=="" set BINPLACE_PLACEFILE=%PackageManifest%
@for /F "tokens=1,* delims=; " %%i in ("%PackageManifest%") do @set BINPLACE_PLACEFILE=%%i
@for /F "tokens=1,* delims=; " %%i in ("%PackageManifest%") do @set PackageManifest=%%j

@if not "%PackageInstallDirectory%"=="" set BINPLACE_ROOTDIR=%PackageInstallDirectory%
@for /F "tokens=1,* delims=; " %%i in ("%PackageInstallDirectory%") do @set BINPLACE_ROOTDIR=%%i
@for /F "tokens=1,* delims=; " %%i in ("%PackageInstallDirectory%") do @set PackageInstallDirectory=%%j

@if not "%PackageSymbolDirectory%"=="" set BINPLACE_SYMBOLDIR=%PackageSymbolDirectory%
@for /F "tokens=1,* delims=; " %%i in ("%PackageSymbolDirectory%") do @set BINPLACE_SYMBOLDIR=%%i
@for /F "tokens=1,* delims=; " %%i in ("%PackageSymbolDirectory%") do @set PackageSymbolDirectory=%%j

@rem Run binplace...
%BINPLACE_TOOL% -r %BINPLACE_ROOTDIR% -o %BINPLACE_SUBROOTDIR% -b %BINPLACE_EXTRASUBDIR%  -s %BINPLACE_SYMBOLDIR% -p %BINPLACE_PLACEFILE% %BINPLACE_OPTIONS% %SourceFiles%

@if "%PackageManifest%"=="" goto end
@goto parse

@goto end

:skip_binplace
@echo Skip Binplace:

:end
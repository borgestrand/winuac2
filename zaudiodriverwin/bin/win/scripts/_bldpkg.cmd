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


@rem
@rem Build package command
@rem
@echo Build Package Utility 0.01
@echo Copyright (c) Creative Advanced Technology Center, 2003.
@echo.

@if "%1"=="-?" goto usage
@if "%1"=="/?" goto usage
@if "%1"==""   goto usage

@rem Save & clear all the DDK specific variables.
@set #AMD64#=%AMD64%
@set #IA64#=%IA64%
@set #BUILD_OPTIONS#=%BUILD_OPTIONS%
@set #BUFFER_OVERFLOW_CHECKS#=%BUFFER_OVERFLOW_CHECKS%
@set #_CL_#=%_CL_%
@set #_LINK_#=%_LINK_%
@set #_ML_#=%_ML_%
@set #DDKBUILDENV#=%DDKBUILDENV%
@set AMD64=
@set IA64=
@set BUILD_OPTIONS=
@set BUFFER_OVERFLOW_CHECKS=
@set _CL_=
@set _LINK_=
@set _ML_=
@set DDKBUILDENV=

@set PackageManifest=
@set PackageType=wdm
@set PackagePlatform=all
@set PackageInstallDir=
@set PackageSymbolDir=
@set PackageBuildFile=
@set PackageBuildArgs=
@set PackageRetail=ON

@set _PACKAGE_MANIFEST=%PACKAGE_MANIFEST%
@set _PACKAGE_TYPE=%PACKAGE_TYPE%
@set _PACKAGE_INSTALL_DIRECTORY=%PACKAGE_INSTALL_DIRECTORY%
@set _PACKAGE_SYMBOL_DIRECTORY=%PACKAGE_SYMBOL_DIRECTORY%

@if not "%PACKAGE_MANIFEST%"=="" set PackageManifest=%PACKAGE_MANIFEST%
@if not "%PACKAGE_TYPE%"=="" set PackageType=%PACKAGE_TYPE%
@if not "%PACKAGE_INSTALL_DIRECTORY%"=="" set PackageInstallDir=%PACKAGE_INSTALL_DIRECTORY%
@if not "%PACKAGE_SYMBOL_DIRECTORY%"=="" set PackageSymbolDir=%PACKAGE_SYMBOL_DIRECTORY%

@rem Platform specific manifest file, install & symbol directories.
@set PackageManifest_x86=
@set PackageInstallDir_x86=
@set PackageSymbolDir_x86=
@set PackageManifest_AMD64=
@set PackageInstallDir_AMD64=
@set PackageSymbolDir_AMD64=
@set PackageManifest_IA64=
@set PackageInstallDir_IA64=
@set PackageSymbolDir_IA64=

@set _i386_PACKAGE_MANIFEST=%i386_PACKAGE_MANIFEST%
@set _i386_PACKAGE_INSTALL_DIRECTORY=%i386_PACKAGE_INSTALL_DIRECTORY%
@set _i386_PACKAGE_SYMBOL_DIRECTORY=%i386_PACKAGE_SYMBOL_DIRECTORY%
@set _AMD64_PACKAGE_MANIFEST=%AMD64_PACKAGE_MANIFEST%
@set _AMD64_PACKAGE_INSTALL_DIRECTORY=%AMD64_PACKAGE_INSTALL_DIRECTORY%
@set _AMD64_PACKAGE_SYMBOL_DIRECTORY=%AMD64_PACKAGE_SYMBOL_DIRECTORY%
@set _IA64_PACKAGE_MANIFEST=%IA64_PACKAGE_MANIFEST%
@set _IA64_PACKAGE_INSTALL_DIRECTORY=%IA64_PACKAGE_INSTALL_DIRECTORY%
@set _IA64_PACKAGE_SYMBOL_DIRECTORY=%IA64_PACKAGE_SYMBOL_DIRECTORY%

@if not "%i386_PACKAGE_MANIFEST%"=="" set PackageManifest_x86=%i386_PACKAGE_MANIFEST%
@if not "%i386_PACKAGE_INSTALL_DIRECTORY%"=="" set PackageInstallDir_x86=%i386_PACKAGE_INSTALL_DIRECTORY%
@if not "%i386_PACKAGE_SYMBOL_DIRECTORY%"=="" set PackageSymbolDir_x86=%i386_PACKAGE_SYMBOL_DIRECTORY%
@if not "%AMD64_PACKAGE_MANIFEST%"=="" set PackageManifest_AMD64=%AMD64_PACKAGE_MANIFEST%
@if not "%AMD64_PACKAGE_INSTALL_DIRECTORY%"=="" set PackageInstallDir_AMD64=%AMD64_PACKAGE_INSTALL_DIRECTORY%
@if not "%AMD64_PACKAGE_SYMBOL_DIRECTORY%"=="" set PackageSymbolDir_AMD64=%AMD64_PACKAGE_SYMBOL_DIRECTORY%
@if not "%IA64_PACKAGE_MANIFEST%"=="" set PackageManifest_IA64=%IA64_PACKAGE_MANIFEST%
@if not "%IA64_PACKAGE_INSTALL_DIRECTORY%"=="" set PackageInstallDir_IA64=%IA64_PACKAGE_INSTALL_DIRECTORY%
@if not "%IA64_PACKAGE_SYMBOL_DIRECTORY%"=="" set PackageSymbolDir_IA64=%IA64_PACKAGE_SYMBOL_DIRECTORY%

@set parameters=%*

:parse
@set p1=
@set p2=
@set p3=
@for /F "tokens=1 delims=, " %%i in ("%parameters%") do @set p1=%%~i
@for /F "tokens=2 delims=, " %%i IN ("%parameters%") do @set p2=%%~i
@for /F "tokens=3 delims=, " %%i IN ("%parameters%") do @set p3=%%~i

@set skip_next_parameter=0

:: PackageManifest parsing & expansion
@set manifest_args=
:manifest_parse
@if /i "%p1%"=="-manifest" (
   set skip_next_parameter=1
   if "%manifest_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_args=%manifest_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto manifest_parse
) else (
   goto skip_manifest_args
)
@set PackageManifest=%manifest_args%
@set manifest_args=
:skip_manifest_args

:: PackageType parsing & expansion
@if /i "%p1%"=="-package" (
   for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set PackageType=%%~i
   set skip_next_parameter=1
)
@if /i "%p1%"=="-platform" (
   for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set PackagePlatform=%%~i
   set skip_next_parameter=1
)

:: PackageInstallDir parsing & expansion
@set installdir_args=
:installdir_parse
@if /i "%p1%"=="-install" (
   set skip_next_parameter=1
   if "%installdir_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_args=%installdir_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto installdir_parse
) else (
   goto skip_installdir_args
)
@set PackageInstallDir=%installdir_args%
@set installdir_args=
:skip_installdir_args

:: PackageSymbolDir parsing & expansion
@set symboldir_args=
:symboldir_parse
@if /i "%p1%"=="-symbols" (
   set skip_next_parameter=1
   if "%symboldir_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_args=%symboldir_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto symboldir_parse
) else (
   goto skip_symboldir_args
)
@set PackageSymbolDir=%symboldir_args%
@set symboldir_args=
:skip_symboldir_args

:: PackageBuildArgs parsing & expansion
@set build_args=
:build_parse
@if /i "%p1%"=="-build" (
   set skip_next_parameter=1
   if "%build_args%"=="" (
      for /F "tokens=1 delims=: " %%i in ("%p2%") do @set build_args=%%~i
   ) else (
      for /F "tokens=1 delims=: " %%i in ("%p2%") do @set build_args=%build_args% %%~i
   )
   for /F "tokens=1,* delims=: " %%i in ("%p2%") do @set p2=%%j
   if not "%p2%"=="" goto build_parse
) else (
   goto skip_build_args
)
@set PackageBuildArgs=%build_args%
@set build_args=
:skip_build_args

:: PackageRetail parsing & expansion
@if /i "%p1%"=="-retail" (
   set PackageRetail=%p2% 
   set skip_next_parameter=1
)

:: PackageBuildFile parsing & expansion
@if "%p1%"=="-@" (
   for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set PackageBuildFile=%%~i
   set skip_next_parameter=1
)

:: SET value=name parsing & expansion
@if /i "%p1%"=="-set" (
   for /F "tokens=1,2,* delims=; " %%i in ('echo %p2%=%p3%') do @set %%~i=%%~j
   set skip_next_parameter=2
)

:: PackageManifest_x86 parsing & expansion
@set manifest_x86_args=
:manifest_x86_parse
@if /i "%p1%"=="-manifest:x86" (
   set skip_next_parameter=1
   if "%manifest_x86_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_x86_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_x86_args=%manifest_x86_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto manifest_x86_parse
) else (
   goto skip_manifest_x86_args
)
@set PackageManifest_x86=%manifest_x86_args%
@set manifest_x86_args=
:skip_manifest_x86_args

:: PackageManifest_AMD64 parsing & expansion
@set manifest_amd64_args=
:manifest_amd64_parse
@if /i "%p1%"=="-manifest:AMD64" (
   set skip_next_parameter=1
   if "%manifest_amd64_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_amd64_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_amd64_args=%manifest_amd64_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto manifest_amd64_parse
) else (
   goto skip_manifest_amd64_args
)
@set PackageManifest_AMD64=%manifest_amd64_args%
@set manifest_amd64_args=
:skip_manifest_amd64_args

:: PackageManifest_IA64 parsing & expansion
@set manifest_ia64_args=
:manifest_ia64_parse
@if /i "%p1%"=="-manifest:IA64" (
   set skip_next_parameter=1
   if "%manifest_ia64_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_ia64_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set manifest_ia64_args=%manifest_ia64_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto manifest_ia64_parse
) else (
   goto skip_manifest_ia64_args
)
@set PackageManifest_IA64=%manifest_ia64_args%
@set manifest_ia64_args=
:skip_manifest_ia64_args

:: PackageInstallDir_x86 parsing & expansion
@set installdir_x86_args=
:installdir_x86_parse
@if /i "%p1%"=="-install:x86" (
   set skip_next_parameter=1
   if "%installdir_x86_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_x86_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_x86_args=%installdir_x86_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto installdir_x86_parse
) else (
   goto skip_installdir_x86_args
)
@set PackageInstallDir_x86=%installdir_x86_args%
@set installdir_x86_args=
:skip_installdir_x86_args

:: PackageInstallDir_AMD64 parsing & expansion
@set installdir_amd64_args=
:installdir_amd64_parse
@if /i "%p1%"=="-install:AMD64" (
   set skip_next_parameter=1
   if "%installdir_amd64_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_amd64_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_amd64_args=%installdir_amd64_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto installdir_amd64_parse
) else (
   goto skip_installdir_amd64_args
)
@set PackageInstallDir_AMD64=%installdir_amd64_args%
@set installdir_amd64_args=
:skip_installdir_amd64_args

:: PackageInstallDir_IA64 parsing & expansion
@set installdir_ia64_args=
:installdir_ia64_parse
@if /i "%p1%"=="-install:IA64" (
   set skip_next_parameter=1
   if "%installdir_ia64_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_ia64_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set installdir_ia64_args=%installdir_ia64_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto installdir_ia64_parse
) else (
   goto skip_installdir_ia64_args
)
@set PackageInstallDir_IA64=%installdir_ia64_args%
@set installdir_ia64_args=
:skip_installdir_ia64_args

:: PackageSymbolDir_x86 parsing & expansion
@set symboldir_x86_args=
:symboldir_x86_parse
@if /i "%p1%"=="-install:x86" (
   set skip_next_parameter=1
   if "%symboldir_x86_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_x86_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_x86_args=%symboldir_x86_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto symboldir_x86_parse
) else (
   goto skip_symboldir_x86_args
)
@set PackageSymbolDir_x86=%symboldir_x86_args%
@set symboldir_x86_args=
:skip_symboldir_x86_args

:: PackageSymbolDir_AMD64 parsing & expansion
@set symboldir_amd64_args=
:symboldir_amd64_parse
@if /i "%p1%"=="-install:AMD64" (
   set skip_next_parameter=1
   if "%symboldir_amd64_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_amd64_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_amd64_args=%symboldir_amd64_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto symboldir_amd64_parse
) else (
   goto skip_symboldir_amd64_args
)
@set PackageSymbolDir_AMD64=%symboldir_amd64_args%
@set symboldir_amd64_args=
:skip_symboldir_amd64_args

:: PackageSymbolDir_IA64 parsing & expansion
@set symboldir_ia64_args=
:symboldir_ia64_parse
@if /i "%p1%"=="-install:IA64" (
   set skip_next_parameter=1
   if "%symboldir_ia64_args%"=="" (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_ia64_args=%%~i
   ) else (
      for /F "tokens=1,* delims=; " %%i in ('echo %p2%') do @set symboldir_ia64_args=%symboldir_ia64_args%;%%~i
   )
   for /F "tokens=1,* delims=; " %%i in ("%p2%") do @set p2=%%j
   if not "!p2!"=="" goto symboldir_ia64_parse
) else (
   goto skip_symboldir_ia64_args
)
@set PackageSymbolDir_IA64=%symboldir_ia64_args%
@set symboldir_ia64_args=
:skip_symboldir_ia64_args

:: target parsing & expansion
@if %skip_next_parameter%==0 (
   for /F "tokens=1,* delims=; " %%i in ('echo %p1%') do @set target=%%~i
) else (
   set target=
)

:: Next parameter...
@if %skip_next_parameter%==2 (
   for /F "tokens=3,* delims=, " %%i in ("%parameters%") do @set parameters=%%i %%j
) else (
   if %skip_next_parameter%==1 (
      for /F "tokens=2,* delims=, " %%i in ("%parameters%") do @set parameters=%%j
   ) else (
      for /F "tokens=1,* delims=, " %%i in ("%parameters%") do @set parameters=%%j
   )
)

:: Insert parameters from script file.
@if not "%PackageBuildFile%"=="" if not exist %PackageBuildFile% echo Warning PKG0000: File %PackageBuildFile% does not exist.
@if not "%PackageBuildFile%"=="" if not exist %PackageBuildFile% goto skip_insert_parameters
@set px=
@if not "%PackageBuildFile%"=="" for /F "eol=# tokens=* delims=, " %%i in (%PackageBuildFile%) do @set px=!px! %%i
@if not "%PackageBuildFile%"=="" set parameters=%px% %parameters%
@set PackageBuildFile=
:skip_insert_parameters

@if "%target%"=="" goto skip_build_target

@set PACKAGE_MANIFEST=%PackageManifest%
@set PACKAGE_INSTALL_DIRECTORY=%PackageInstallDir%
@set PACKAGE_SYMBOL_DIRECTORY=%PackageSymbolDir%
@set PACKAGE_TYPE=%PackageType%

@set i386_PACKAGE_MANIFEST=%PackageManifest_x86%
@set i386_PACKAGE_INSTALL_DIRECTORY=%PackageInstallDir_x86%
@set i386_PACKAGE_SYMBOL_DIRECTORY=%PackageSymbolDir_x86%
@set AMD64_PACKAGE_MANIFEST=%PackageManifest_AMD64%
@set AMD64_PACKAGE_INSTALL_DIRECTORY=%PackageInstallDir_AMD64%
@set AMD64_PACKAGE_SYMBOL_DIRECTORY=%PackageSymbolDir_AMD64%
@set IA64_PACKAGE_MANIFEST=%PackageManifest_IA64%
@set IA64_PACKAGE_INSTALL_DIRECTORY=%PackageInstallDir_IA64%
@set IA64_PACKAGE_SYMBOL_DIRECTORY=%PackageSymbolDir_IA64%

@if not exist %target% (
   echo Error PKG1001: Target %target% does not exist.
   goto skip_build_target
)
@pushd %target%

@set PackageType_WDM=0
@set PackageType_NT4=0
@set PackageType_VXD=0
@if /i "%PackageType%"=="ALL" (
   set PackageType_WDM=1
   set PackageType_NT4=1
   set PackageType_VXD=1
)
@if /i "%PackageType%"=="WDM" (
   set PackageType_WDM=1
)
@if /i "%PackageType%"=="NT4" (
   set PackageType_NT4=1
)
@if /i "%PackageType%"=="VXD" (
   set PackageType_VXD=1
)

@set Platform_x86=0
@set Platform_IA64=0
@set Platform_AMD64=0
@set BuildCommands_x86=
@set BuildCommands_IA64=
@set BuildCommands_AMD6=
@if %PackageType_WDM%==1 (
   if /i "%PackagePlatform%"=="ALL" (
      set Platform_x86=1
      set Platform_IA64=1
      set Platform_AMD64=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_x86="%BASEDIR%\bin\setenv.bat %BASEDIR% fre WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
         set BuildCommands_IA64="%BASEDIR%\bin\setenv.bat %BASEDIR% fre 64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
         set BuildCommands_AMD64="%BASEDIR%\bin\setenv.bat %BASEDIR% fre AMD64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_x86="%BASEDIR%\bin\setenv.bat %BASEDIR% chk WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
         set BuildCommands_IA64="%BASEDIR%\bin\setenv.bat %BASEDIR% chk 64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
         set BuildCommands_AMD64="%BASEDIR%\bin\setenv.bat %BASEDIR% chk AMD64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      )
   )
   if /i "%PackagePlatform%"=="x86" (
      set Platform_x86=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_x86="%BASEDIR%\bin\setenv.bat %BASEDIR% fre WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_x86="%BASEDIR%\bin\setenv.bat %BASEDIR% chk WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      )
   )
   if /i "%PackagePlatform%"=="IA64" (
      set Platform_IA64=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_IA64="%BASEDIR%\bin\setenv.bat %BASEDIR% fre 64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_IA64="%BASEDIR%\bin\setenv.bat %BASEDIR% chk 64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      )
   )
   if /i "%PackagePlatform%"=="AMD64" (
      set Platform_AMD64=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_AMD64="%BASEDIR%\bin\setenv.bat %BASEDIR% fre AMD64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_AMD64="%BASEDIR%\bin\setenv.bat %BASEDIR% chk AMD64 WNET && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld2k.cmd %PackageBuildArgs% -P"
      )
   )
)
@if %Platform_x86%==1 (
   echo Building WDM x86 %target% package...
   CMD.EXE /c %BuildCommands_x86%
)
@if %Platform_IA64%==1 (
   echo Building WDM IA64 %target% package...
   CMD.EXE /c %BuildCommands_IA64%
)
@if %Platform_AMD64%==1 (
   echo Building WDM AMD64 %target% package...
   CMD.EXE /c %BuildCommands_AMD64%
)

@set Platform_x86=0
@if %PackageType_NT4%==1 (
   if /i "%PackagePlatform%"=="ALL" (
      set Platform_x86=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_x86="%NTROOT%\bin\setenv.bat %NTROOT% free && cd /d %cd% && %HULAROOT%\bin\win\scripts\bldnt.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_x86="%NTROOT%\bin\setenv.bat %NTROOT% checked && cd /d %cd% && %HULAROOT%\bin\win\scripts\bldnt.cmd %PackageBuildArgs% -P"
      )
   )
   if /i "%PackagePlatform%"=="x86" (
      set Platform_x86=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_x86="%NTROOT%\bin\setenv.bat %NTROOT% free && cd /d %cd% && %HULAROOT%\bin\win\scripts\bldnt.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_x86="%NTROOT%\bin\setenv.bat %NTROOT% checked && cd /d %cd% && %HULAROOT%\bin\win\scripts\bldnt.cmd %PackageBuildArgs% -P"
      )
   )
)
@if %Platform_x86%==1 (
   echo Building NT4 x86 %target% package...
   CMD.EXE /c %BuildCommands_x86%
)

@set Platform_x86=0
@if %PackageType_VXD%==1 (
   if /i "%PackagePlatform%"=="ALL" (
      set Platform_x86=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_x86="%BASEDIR%\bin\9xbld.bat %BASEDIR% free && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld9x.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_x86="%BASEDIR%\bin\9xbld.bat %BASEDIR% checked && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld9x.cmd %PackageBuildArgs% -P"
      )
   )
   if /i "%PackagePlatform%"=="x86" (
      set Platform_x86=1
      if /i "%PackageRetail%"=="ON" (
         set BuildCommands_x86="%BASEDIR%\bin\9xbld.bat %BASEDIR% free && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld9x.cmd %PackageBuildArgs% -P"
      ) else (
         set BuildCommands_x86="%BASEDIR%\bin\9xbld.bat %BASEDIR% checked && cd /d %cd% && %HULAROOT%\bin\win\scripts\bld9x.cmd %PackageBuildArgs% -P"
      )
   )
)
@if %Platform_x86%==1 (
   echo Building Win9x x86 %target% package...
   CMD.EXE /c %BuildCommands_x86%
)

@popd
:skip_build_target

@if "%parameters%"=="" goto cleanup

@goto parse

:cleanup

@set PackageType_WDM=
@set PackageType_NT4=
@set PackageType_VXD=
@set Platform_x86=
@set Platform_IA64=
@set Platform_AMD64=
@set BuildCommands_x86=
@set BuildCommands_IA64=
@set BuildCommands_AMD6=

@set skip_next_parameter=
@set PackageManifest=
@set PackagePlatform=
@set PackageType=
@set PackageInstallDir=
@set PackageSymbolDir=
@set PackageBuildFile=
@set PackageBuildArgs=
@set PackageRetail=

@set PACKAGE_MANIFEST=%_PACKAGE_MANIFEST%
@set PACKAGE_TYPE=%_PACKAGE_TYPE%
@set PACKAGE_INSTALL_DIRECTORY=%_PACKAGE_INSTALL_DIRECTORY%
@set PACKAGE_SYMBOL_DIRECTORY=%_PACKAGE_INSTALL_DIRECTORY%
@set _PACKAGE_MANIFEST=
@set _PACKAGE_TYPE=
@set _PACKAGE_INSTALL_DIRECTORY=
@set _PACKAGE_SYMBOL_DIRECTORY=

@set PackageManifest_x86=
@set PackageInstallDir_x86=
@set PackageSymbolDir_x86=
@set PackageManifest_AMD64=
@set PackageInstallDir_AMD64=
@set PackageSymbolDir_AMD64=
@set PackageManifest_IA64=
@set PackageInstallDir_IA64=
@set PackageSymbolDir_IA64=

@set i386_PACKAGE_MANIFEST=%_i386_PACKAGE_MANIFEST%
@set i386_PACKAGE_INSTALL_DIRECTORY=%_i386_PACKAGE_INSTALL_DIRECTORY%
@set i386_PACKAGE_SYMBOL_DIRECTORY=%_i386_PACKAGE_SYMBOL_DIRECTORY%
@set AMD64_PACKAGE_MANIFEST=%_AMD64_PACKAGE_MANIFEST%
@set AMD64_PACKAGE_INSTALL_DIRECTORY=%_AMD64_PACKAGE_INSTALL_DIRECTORY%
@set AMD64_PACKAGE_SYMBOL_DIRECTORY=%_AMD64_PACKAGE_SYMBOL_DIRECTORY%
@set IA64_PACKAGE_MANIFEST=%_IA64_PACKAGE_MANIFEST%
@set IA64_PACKAGE_INSTALL_DIRECTORY=%_IA64_PACKAGE_INSTALL_DIRECTORY%
@set IA64_PACKAGE_SYMBOL_DIRECTORY=%_IA64_PACKAGE_SYMBOL_DIRECTORY%
@set _i386_PACKAGE_MANIFEST=
@set _i386_PACKAGE_INSTALL_DIRECTORY=
@set _i386_PACKAGE_SYMBOL_DIRECTORY=
@set _AMD64_PACKAGE_MANIFEST=
@set _AMD64_PACKAGE_INSTALL_DIRECTORY=
@set _AMD64_PACKAGE_SYMBOL_DIRECTORY=
@set _IA64_PACKAGE_MANIFEST=
@set _IA64_PACKAGE_INSTALL_DIRECTORY=
@set _IA64_PACKAGE_SYMBOL_DIRECTORY=

@rem Restore all the DDK specific variables.
@set AMD64=%#AMD64#%
@set IA64=%#IA64#%
@set BUILD_OPTIONS=%#BUILD_OPTIONS#%
@set BUFFER_OVERFLOW_CHECKS=%#BUFFER_OVERFLOW_CHECKS#%
@set _CL_=%#_CL_#%
@set _LINK_=%#_LINK_#%
@set _ML_=%#_ML_#%
@set DDKBUILDENV=%#DDKBUILDENV#%
@set #AMD64#=
@set #IA64#=
@set #BUILD_OPTIONS#=
@set #BUFFER_OVERFLOW_CHECKS#=
@set #_CL_#=
@set #_LINK_#=
@set #_ML_#=
@set #DDKBUILDENV#=

@goto end

:usage
@echo bldpkg [Options] target [target,[target,..]]
@echo.
@echo     Options:
@echo.
@echo        -?                 Display this message.
@echo.
@echo        -package ^<type^>    Specifies the package type:
@echo                           { WDM ^| NT4 ^| VXD ^| ALL }
@echo                           If this is omitted, the default is determined by
@echo                           the PACKAGE_TYPE environment variable. If the
@echo                           environment variable is not specified, the default
@echo                           is WDM.
@echo.
@echo        -platform ^<cpu^>    Specifies the platform:
@echo                           { x86 ^| IA64 ^| AMD64 ^| ALL }
@echo                           If this is omitted, the default is ALL.
@echo.
@echo        -manifest[:^<x86^|IA64^|AMD64] ^<file^>
@echo.
@echo                           Specifies the manifest file. If this is ommited, the
@echo                           default is determined by the PACKAGE_MANIFEST
@echo                           environment variable. If the environment variable is
@echo                           not specified, the default is package.wdm.manifest.
@echo.
@echo                           The [:^<x86^|IA64^|AMD64] switches specify platform
@echo                           specific manifest file.
@echo.
@echo        -install[:^<x86^|IA64^|AMD64] ^<pathdir^>
@echo.
@echo                           Specifies the destination directory where the
@echo                           package installation will be created. If this
@echo                           is omitted, the default is determined by the
@echo                           PACKAGE_INSTALL_DIRECTORY environment variable.
@echo                           If the environment variable is not specified,
@echo                           the default is .\install.
@echo.
@echo                           The [:^<x86^|IA64^|AMD64] switches specify platform
@echo                           specific package installation directory.
@echo.
@echo        -symbols[:^<x86^|IA64^|AMD64] ^<pathdir^>
@echo.
@echo                           Specifies the destination directory for symbol
@echo                           files. If this is omitted, the default is
@echo                           determined by the PACKAGE_SYMBOL_DIRECTORY
@echo                           environment variable. If the environment variable
@echo                           is not specified, the default is .\symbols.
@echo.
@echo                           The [:^<x86^|IA64^|AMD64] switches specify platform
@echo                           specific package symbols directory.
@echo.
@echo        -build ^<args^>      Specifies the argument to pass to build.exe. If
@echo                           omitted, the default is no argument. Arguments are
@echo                           separated by colon :, ie -cZ:-f.
@echo.
@echo        -retail ^<ON^|OFF^>   Specifies the retail or debug package. If
@echo                           omitted, the default is retail.
@echo.
@echo        -set ^<name=value^>   Sets an environment variable to the value specified.
@echo.
@echo        -@ ^<file^>          Specifies a file name on the bldpkg command line
@echo                           by prefixing it with an at sign (-@). When bldpkg
@echo                           sees a string beginning with this sign on its
@echo                           command line, it will take the string, remove the
@echo                           at sign, and then look for a file with this name.
@echo                           If it finds this file, it will insert its text
@echo                           into the command line at exactly the place where
@echo                           the original parameter beginning with the at sign
@echo                           had been. Because bldpkg parses parameters from
@echo                           left to right, you can use this technique along
@echo                           with multiple instances of ^<file^> to use
@echo                           bldpkg on several packages with different options
@echo                           for each, without having to type all the options
@echo                           each time.
@echo.
@echo        Non-switch parameters specify target directories.
@goto end

:end

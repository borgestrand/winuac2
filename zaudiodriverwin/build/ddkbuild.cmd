@echo off
@set REVISION=V7.0 BETA6a
@set REVDATE=2007-02-26
@set OSR_DEBUG=off
@if "%OS%"=="Windows_NT" goto :MAIN
@echo This script requires Windows NT 4.0 or later to run properly!
goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
::    This software is supplied for instructional purposes only.
::
::    OSR Open Systems Resources, Inc. (OSR) expressly disclaims any warranty
::    for this software.  THIS SOFTWARE IS PROVIDED  "AS IS" WITHOUT WARRANTY
::    OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION,
::    THE IMPLIED WARRANTIES OF MECHANTABILITY OR FITNESS FOR A PARTICULAR
::    PURPOSE.  THE ENTIRE RISK ARISING FROM THE USE OF THIS SOFTWARE REMAINS
::    WITH YOU.  OSR's entire liability and your exclusive remedy shall not
::    exceed the price paid for this material.  In no event shall OSR or its
::    suppliers be liable for any damages whatsoever (including, without
::    limitation, damages for loss of business profit, business interruption,
::    loss of business information, or any other pecuniary loss) arising out
::    of the use or inability to use this software, even if OSR has been
::    advised of the possibility of such damages.  Because some states/
::    jurisdictions do not allow the exclusion or limitation of liability for
::    consequential or incidental damages, the above limitation may not apply
::    to you.
::
::    OSR Open Systems Resources, Inc.
::    105 Route 101A Suite 19
::    Amherst, NH 03031  (603) 595-6500 FAX: (603) 595-6503
::    email bugs to: bugs@osr.com
::
::
::    MODULE:
::
::      ddkbuild.cmd
::
::    ABSTRACT:
::
::      This file allows drivers to be build with visual studio and visual studio.net
::
::    AUTHOR(S):
::
::      - OSR Open Systems Resources, Inc.
::      - Oliver Schneider (ddkwizard.assarbad.net)
::
::    REQUIREMENTS:
::
::      Environment variables that must be set.
::        %NT4BASE%  - Set this up for "-NT4" builds
::        %W2KBASE%  - Set this up for "-W2K*" builds
::        %WXPBASE%  - Set this up for "-WXP*" builds
::        %WNETBASE% - Set this up for "-WNET*" builds
::        %WLHBASE%  - Set this up for "-WLH*" builds
::        %WDF_ROOT% - Must be set if attempting to do a WDF Build.
::
::      Examples:
::        NT4BASE : could be "D:\NT4DDK"
::        W2KBASE : could be "D:\Nt50DDK"
::        WXPBASE : could be "D:\WINDDK\2600"
::        WNETBASE: could be "D:\WINDDK\3790.1830" or "C:\WINDDK\3790"
::
::    COMMAND FORMAT:
::
::      Run the script without any parameters to get the whole help content!
::      Note: "-WDF" has been tested with the 01.00.5054 version of the framework
::
::    RETURN CODES AND THEIR MEANING:
::
::       001 == Unknown build type. Check the <platform> parameter
::       002 == No WDF_ROOT given using WDF build type.
::       003 == The DDK-specific base directory variable (NT4BASE, W2KBASE, WXPBASE,
::              WNETBASE) is not set at all and could not be auto-detected!
::       004 == BASEDIR variable is empty. Check to see that the DDK-specific
::              variable is set correctly (i.e. NT4BASE, W2KBASE, WXPBASE, WNETBASE)
::       005 == No mode (checked/free) was given. Check the respective parameter!
::       006 == No DIR or SOURCES file found in the given target directory.
::       007 == No target directory given.
::       008 == Given target directory does not exist.
::
::       Note: If %OSR_ERRCODE% and %ERRORLEVEL% are equal, the return code stems
::             from one of the tools being called during the build process.
::
::    BROWSE FILES:
::
::       This procedure supports the building of BROWSE files to be used by
::       Visual Studio 6 and by Visual Studio.Net  However, the BSCfiles created
::       by bscmake for the 2 studios are not compatible. When this command procedure
::       runs, it selects the first bscmake.exe found in the path.   So, make
::       sure that the correct bscmake.exe is in the path....
::
::       Note that if using Visual Studio.NET the .BSC must be added to the project
::       in order for the project to be browsed.
::       Another alternative is the VS addon named "Visual Assist X" which will
::       parse the header files - no more need for browse files.
::
::    COMPILERS:
::
::        If you are building NT4 you should really
::        be using the VC 6 compiler.   Later versions of the DDK now contain the
::        compiler and the linker.  This procedure should use the correct compiler.
::
::    GENERAL COMMENTS:
::
::        This procedure has been cleaned up to be modular and easy to
::        understand.
::
::        As of the Server 2003 SP1 DDK ddkbuild now clears the
::        NO_BROWSE_FILE and NO_BINPLACE environment variables so that users
::        can use these features.
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: / MAIN function of the script
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:MAIN
:: Building "stack frame"
setlocal ENABLEEXTENSIONS & pushd .

:: Init some special variables
set OSR_VERSTR=OSR DDKBUILD.CMD %REVISION% (%REVDATE%) - OSR, Open Systems Resources, Inc.
set OSR_PREBUILD_SCRIPT=ddkprebld.cmd
set OSR_POSTBUILD_SCRIPT=ddkpostbld.cmd
set OSR_SETENV_SCRIPT=ddkbldenv.cmd
set OSR_ECHO=@echo DDKBLD:

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Set error messages
:: Possible codes: 1
set ERR_UnknownBuildType=Unknown type of build. Please recheck parameters.
:: Possible codes: 2
set ERR_NoWdfRoot=WDF_ROOT is not defined, are you using 00.01.5054 or later?
:: Possible codes: 3
set ERR_BaseDirNotSet=To build using type %%OSR_TARGET%% you need to set the %%%%%%BASEDIRVAR%%%%%% environment variable to point to the %%BASEDIROS%% DDK base directory!
:: Possible codes: 4
set ERR_NoBASEDIR=NT4BASE, W2KBASE, WXPBASE and/or WNETBASE environment variable(s) not set. Environment variable(s) must be set by user according to DDK version(s) installed.
:: Possible codes: 5
set ERR_BadMode=^<build type^> must be 'checked', 'free', 'chk' or 'fre' (case-insensitive).
:: Possible codes: 6
set ERR_NoTarget=Target directory must contain a SOURCES or DIRS file.
:: Possible codes: 7, 8
set ERR_NoDir=The ^<directory^> parameter must be a valid directory.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Clear the error code variable
set OSR_ERRCODE=0
set prefast_build=0

:: Turn on tracing, use %OSR_TRACE% instead of ECHO
if /i {%OSR_DEBUG%} == {on} (set OSR_TRACE=%OSR_ECHO% [TRACE]) else (set OSR_TRACE=rem)

:: Turn on echoing of current line if %OSR_DEBUG% is set to "on"
@echo %OSR_DEBUG%

:: Output version string
@echo %OSR_VERSTR%
%OSR_TRACE% ^(Current module: ^"%~f0^"^)
@echo.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Set the target platform variable
set OSR_TARGET=%~1
:: Remove any dashes in the variable
if not {%OSR_TARGET%} == {} set OSR_TARGET=%OSR_TARGET:-=%
:: Show help if the target parameter is empty after removal of the dashes
if {%OSR_TARGET%} == {} goto :USAGE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: In the build directory check for this script and call it if it exists.
:: This allows to override any global system variable setting, if desired.
if not "%3" == "" call :GetCustomEnvironment %~f3
if not {%OSR_ERRCODE%} == {0} goto :USAGE
:: Additional error handling for better usability
:: These subroutines will also attempt to locate the requested DDK!!!
set OSR_ERRCODE=3
%OSR_TRACE% Checking whether the environment variable for the build type was set
:: Calling as a subroutine has 2 advantages:
:: 1. the script does not quit if the label was not found
:: 2. we return to the line after the call and can check variables there
call :%OSR_TARGET%Check
:: If the BASEDIROS/BASEDIRVAR variable is not defined, it means the subroutine did not exist!
if not DEFINED BASEDIROS call :ShowErrorMsg 1 "%ERR_UnknownBuildType% (BASEDIROS)" & goto :USAGE
if not DEFINED BASEDIRVAR call :ShowErrorMsg 1 "%ERR_UnknownBuildType% (BASEDIRVAR)" & goto :USAGE
if not {%OSR_ERRCODE%} == {0} call :ShowErrorMsg %OSR_ERRCODE% "%ERR_BaseDirNotSet%" & goto :USAGE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
set BASEDIR=%%%BASEDIRVAR%%%
call :ResolveVar BASEDIR
:: Check for existing %BASEDIR%
if {%BASEDIR%}=={} call :ShowErrorMsg 4 "%ERR_NoBASEDIR%" & goto :USAGE
set PATH=%BASEDIR%\bin;%PATH%
%OSR_TRACE% Now jump to the initialization of the commandline
:: Calling as a subroutine has 2 advantages:
:: 1. the script does not quit if the label was not found
:: 2. we return to the line after the call and can check variables there
call :%OSR_TARGET%Build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%OSR_TRACE% We returned from the variable initialization
if not DEFINED OSR_CMDLINE call :ShowErrorMsg 1 "%ERR_UnknownBuildType% (OSR_CMDLINE)" & goto :USAGE

%OSR_TRACE% Hurrah, all the variables have been initialized, continuing
:: Proceed with common build steps
goto :CommonBuild

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Check whether the parameter makes sense and try to
:: correct it if possible
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:WLHCheck
:WLHX64Check
:WLHI64Check
:WLHNETX64Check
:WLHNETI64Check
:WLHXPCheck
:WLH2KCheck
:WLHNETCheck
set BASEDIROS=Windows Vista/Longhorn Server
set BASEDIRVAR=WLHBASE
:: Compatibility between BUILD and VS ... prevent pipes from being used
%OSR_ECHO% Clearing %%VS_UNICODE_OUTPUT%% ...
set VS_UNICODE_OUTPUT=
:: Return to caller
if DEFINED %BASEDIRVAR% goto :CommonCheckNoErrorWithReturn
call :CommonCheckMsg1
:: Try all the possible "default" locations
set BASEDIRTEMP=%SystemDrive%\WINDDK\6000
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\WINDDK\6000
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
:: Try some "odd" locations
set BASEDIRTEMP=%SystemDrive%\DDK\6000
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\DDK\6000
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
goto :CommonCheckErrorNotSupportedWithReturn

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:WNET2KCheck
:WNETXPCheck
:WNETXP64Check
:WNET64Check
:WNETI64Check
:WNETAMD64Check
:WNETX64Check
:WNETCheck

set BASEDIROS=Windows 2003 Server
set BASEDIRVAR=WNETBASE
:: Return to caller
if DEFINED %BASEDIRVAR% goto :CommonCheckNoErrorWithReturn
call :CommonCheckMsg1
:: Try all the possible "default" locations
set BASEDIRTEMP=%SystemDrive%\WINDDK\3790.1830
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%SystemDrive%\WINDDK\3790.1218
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%SystemDrive%\WINDDK\3790
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\WINDDK\3790.1830
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\WINDDK\3790.1218
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%%ProgramFiles%\WINDDK\3790
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
:: Try some "odd" locations
set BASEDIRTEMP=%SystemDrive%\DDK\3790.1830
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%SystemDrive%\DDK\3790.1218
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%SystemDrive%\DDK\3790
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\DDK\3790.1830
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\DDK\3790.1218
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\DDK\3790
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
goto :CommonCheckErrorNotDetectedWithReturn

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:WXP64Check
:WXPI64Check
:WXPCheck
:WXP2KCheck
set BASEDIROS=Windows XP
set BASEDIRVAR=WXPBASE
:: Return to caller
if DEFINED %BASEDIRVAR% goto :CommonCheckNoErrorWithReturn
call :CommonCheckMsg1
:: Try all the possible "default" locations
set BASEDIRTEMP=%SystemDrive%\WINDDK\2600
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\WINDDK\2600
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
:: Try some "odd" locations
set BASEDIRTEMP=%SystemDrive%\DDK\2600
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
set BASEDIRTEMP=%ProgramFiles%\DDK\2600
%OSR_ECHO% Trying %BASEDIRTEMP% ...
if exist "%BASEDIRTEMP%" goto :CommonCheckSetVarWithReturn
goto :CommonCheckErrorNotDetectedWithReturn

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:W2K64Check
:W2KI64Check
:W2KCheck
set BASEDIROS=Windows 2000
set BASEDIRVAR=W2KBASE
:: Return to caller
if DEFINED %BASEDIRVAR% goto :CommonCheckNoErrorWithReturn
call :CommonCheckMsg2
goto :CommonCheckErrorNotSupportedWithReturn

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:NT4Check
set BASEDIROS=Windows NT4
set BASEDIRVAR=NT4BASE
:: Return to caller
if DEFINED %BASEDIRVAR% goto :CommonCheckNoErrorWithReturn
call :CommonCheckMsg2
goto :CommonCheckErrorNotSupportedWithReturn

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:CommonCheckMsg1
echo.
%OSR_ECHO% WARNING: %%%BASEDIRVAR%%% NOT SET!
%OSR_ECHO%   Attempting to auto-detect the installation folder and set %%%BASEDIRVAR%%%.
%OSR_ECHO%   (If this fails *you* will have to set it!)
echo.
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:CommonCheckMsg2
echo.
%OSR_ECHO% WARNING:
%OSR_ECHO%   Auto-detection of the folder settings is not supported for the requested DDK.
%OSR_ECHO%   Please set %%%BASEDIRVAR%%% yourself!
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:CommonCheckSetVarWithReturn
%OSR_ECHO% Found!
echo.
set %BASEDIRVAR%=%BASEDIRTEMP%
set BASEDIRTEMP=
:: Tell the caller it was successful
:CommonCheckNoErrorWithReturn
set OSR_ERRCODE=0
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:CommonCheckErrorNotDetectedWithReturn
echo.
%OSR_ECHO% None of the usual default paths works. Set %%%BASEDIRVAR%%% manually!
:CommonCheckErrorNotSupportedWithReturn
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Initialize variables specific to the respective platform
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: NT 4.0 build using NT4 DDK
:NT4Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% "%%MSDEVDIR%%"
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: W2K build for 32bit using WXP DDK
:WXP2KBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\w2k\set2k.bat %%BASEDIR%% %%BuildMode%%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: W2K build for 64bit (Intel) using W2K DDK
:W2K64Build
:W2KI64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv64.bat %%BASEDIR%% %%BuildMode%%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: W2K build for 32bit using W2K DDK
:W2KBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WXP build for 64bit (Intel) using WXP DDK
:WXP64Build
:WXPI64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% 64
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WXP build for 32bit using WXP DDK
:WXPBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: W2K build for 32bit using WNET DDK
:WNET2KBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% W2K %%BuildMode%%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WXP build for 32bit using WNET DDK
:WNETXPBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% WXP
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WXP build for 64bit using WNET DDK
:WNETXP64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% 64 WXP
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WNET build for 64bit (Intel) using WNET DDK
:WNET64Build
:WNETI64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% 64 WNET
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WNET build for 64bit (AMD) using WNET DDK
:WNETAMD64Build
:WNETX64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% AMD64 WNET
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WNET build for 32bit using WNET DDK
:WNETBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WLH build for 32bit using WLH DDK
:WLHBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% WLH
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WLH build for 64bit (AMD) using WLH DDK
:WLHX64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% AMD64 WLH
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WLH build for 64bit (Intel) using WLH DDK
:WLHI64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% 64 WLH
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WNET build for 64bit (AMD) using WLH DDK
:WLHNETX64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% AMD64 WNET
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WNET build for 64bit (Intel) using WLH DDK
:WLHNETI64Build
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% 64 WNET
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WXP build for 32bit using WLH DDK
:WLHXPBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% WXP
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: W2K build for 32bit using WLH DDK
:WLH2KBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% W2K
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WNET build for 32bit using WLH DDK
:WLHNETBuild
set OSR_CMDLINE=%%BASEDIR%%\bin\setenv.bat %%BASEDIR%% %%BuildMode%% WNET
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: All builds go here for the rest of the procedure. Now,
:: we are getting ready to call build. The big problem
:: here is to figure our the name of the buildxxx files
:: being generated for the different platforms.
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:CommonBuild
:: Remove first command line arg
shift
call :SetMode %1
if not {%OSR_ERRCODE%} == {0} call :ShowErrorMsg %OSR_ERRCODE% "%ERR_BadMode%" & goto :USAGE
:: Resolve unresolved variable
set OSR_BUILDNAME=%OSR_TARGET% (%BuildMode%) using the %BASEDIROS% DDK and %%%BASEDIRVAR%%%

call :CheckTargets %2
if {%OSR_ERRCODE%} == {6} call :ShowErrorMsg %OSR_ERRCODE% "%ERR_NoTarget%" & goto :USAGE
if not {%OSR_ERRCODE%} == {0} call :ShowErrorMsg %OSR_ERRCODE% "%ERR_NoDir%" & goto :USAGE

:: Resolve any variables in the command line string
set OSR_CMDLINE=%OSR_CMDLINE%

pushd .
:: This external script prepares the build environment (e.g. setenv.bat)
call %OSR_CMDLINE%
popd

:: ----------------------------------------------------------------------------
:: Setting global variables for the scope of this CMD session
set NO_BROWSER_FILE=
set NO_BINPLACE=
set buildDirectory=%~f2
set buildDirectory_raw=%2
set buildDirectory_fname=%~n2
%OSR_TRACE% buildDirectory       == %buildDirectory%
%OSR_TRACE% buildDirectory_raw   == %buildDirectory_raw%
%OSR_TRACE% buildDirectory_fname == %buildDirectory_fname%

set mpFlag=-M
if {%BUILD_ALT_DIR%}=={} goto :NT4

:: W2K sets this!
set OSR_EXT=%BUILD_ALT_DIR%
set mpFlag=-MI

:NT4
if {%NUMBER_OF_PROCESSORS%}=={} set mpFlag=
if {%NUMBER_OF_PROCESSORS%}=={1} set mpFlag=

:: Set additional variables at this point or do whatever you please
@if exist "%buildDirectory%\%OSR_PREBUILD_SCRIPT%" @(
  %OSR_ECHO% ^>^> Performing pre-build steps [%OSR_PREBUILD_SCRIPT%] ...
  pushd "%buildDirectory%"
  call "%OSR_PREBUILD_SCRIPT%" > "%TEMP%\%OSR_PREBUILD_SCRIPT%.tmp"
  for /f "tokens=*" %%x in ('type "%TEMP%\%OSR_PREBUILD_SCRIPT%.tmp"') do @(
    %OSR_ECHO% %%x
  )
  if exist "%TEMP%\%OSR_PREBUILD_SCRIPT%.tmp" del /f /q "%TEMP%\%OSR_PREBUILD_SCRIPT%.tmp"
  popd
  %OSR_ECHO% ^<^< Finished pre-build steps [%OSR_PREBUILD_SCRIPT%] ...
)
:: Save the current directory (before changing into the build directory!)
:: AFTERPREBUILD
pushd .

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Determine the settings of flags, WDF and PREFAST in
:: other words what was set for %3 and beyond....
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%OSR_ECHO% %OSR_BUILDNAME%
set OSR_ARGS= + argument(s):
if not {%3} == {} set OSR_ARGS=%OSR_ARGS% %3
if not {%4} == {} set OSR_ARGS=%OSR_ARGS% %4
if not {%5} == {} set OSR_ARGS=%OSR_ARGS% %5
if /i "%OSR_ARGS%" == " + argument(s):" set OSR_ARGS=
%OSR_ECHO% Directory: %buildDirectory%%OSR_ARGS%
%OSR_ECHO% %BASEDIRVAR%: %BASEDIR%

cd /D %~s2
set bflags=-FZe
set bscFlags=

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:ContinueParsing
if {%3} == {} goto :DONE
if {%3} == {/a} goto :RebuildallFound
if /i {%3} == {-WDF} goto :WDFFound
if /i {%3} == {-PREFAST} goto :PrefastFound
set bscFlags=/n
set bflags=%bflags% %3 -e
:: Remove first arg
shift
goto :ContinueParsing

:WDFFound
shift
if /i {%BASEDIRVAR%} == {WLHBASE} goto :WDFOkay
if {%WDF_ROOT%} == {} call :ShowErrorMsg 2 "%ERR_NoWdfRoot%" & goto :USAGE
pushd .
call %WDF_ROOT%\set_wdf_env.cmd
popd
:WDFOkay
goto :ContinueParsing

:PrefastFound
shift
set prefast_build=1
goto :ContinueParsing

:RebuildallFound
shift
set bscFlags=/n
set bflags=-cfFeZ
goto :ContinueParsing
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:DONE

if exist "build%OSR_EXT%.err"   erase /f /q "build%OSR_EXT%.err"
if exist "build%OSR_EXT%.wrn2"   erase /f /q "build%OSR_EXT%.wrn"
if exist "build%OSR_EXT%.log"   erase /f /q "build%OSR_EXT%.log"
if exist "prefast%OSR_EXT%.log" erase /f /q "prefast%OSR_EXT%.log"

if not {%prefast_build%} == {0} goto :RunPrefastBuild
%OSR_ECHO% Run build %bflags% %mpFlag% for %BuildMode% version in %buildDirectory_raw%
pushd .
build  %bflags% %mpFlag%
popd
goto :BuildComplete

:RunPrefastBuild
%OSR_ECHO% Run prefast build %bflags% %mpFlag% for %BuildMode% version in %buildDirectory_raw%
setlocal ENABLEEXTENSIONS & pushd .
set PREFASTLOG=PREfast_defects_%OSR_EXT%.xml
prefast /log=%PREFASTLOG% /reset build  %bflags% %mpFlag% > NUL
if "%errorlevel%" GTR "0" set OSR_ERRCODE=%errorlevel%
prefast /log=%PREFASTLOG% list > prefast%OSR_EXT%.log
%OSR_ECHO% The PREfast logfile is ^"%prefastlog%^"!
popd & endlocal

:BuildComplete
if not {%errorlevel%} == {0} set OSR_ERRCODE=%errorlevel%

@echo %OSR_DEBUG%
:: Assume that the onscreen errors are complete!
setlocal
set WARNING_FILE_COUNT=0
if exist "build%OSR_EXT%.wrn" for /f "tokens=*" %%x in ('findstr "warning[^.][DRCLU][0-9]*" "build%OSR_EXT%.wrn"') do @(
  set /a WARNING_FILE_COUNT=%WARNING_FILE_COUNT%+1
)
if exist "build%OSR_EXT%.log" for /f "tokens=*" %%x in ('findstr "warning[^.][DRCLU][0-9]*" "build%OSR_EXT%.log"') do @(
  set /a WARNING_FILE_COUNT=%WARNING_FILE_COUNT%+1
)
if not {%WARNING_FILE_COUNT%} == {0} (
  %OSR_ECHO% ================ Build warnings =======================
  if exist "build%OSR_EXT%.wrn" for /f "tokens=*" %%x in ('findstr "warning[^.][DRCLU][0-9]*" "build%OSR_EXT%.wrn"') do @(
    %OSR_ECHO% %%x
  )
  if exist "build%OSR_EXT%.log" for /f "tokens=*" %%x in ('findstr "warning[^.][DRCLU][0-9]*" "build%OSR_EXT%.log"') do @(
    %OSR_ECHO% %%x
  )
)
set WARNING_FILE_COUNT_PRE=0
if exist "prefast%OSR_EXT%.log" for /f "tokens=*" %%x in ('findstr "warning[^.][CLU]*" "prefast%OSR_EXT%.log"') do @(
  set /a WARNING_FILE_COUNT_PRE=%WARNING_FILE_COUNT_PRE%+1
)
:: Reset if this is no PREfast build
if {%prefast_build%} == {0} set WARNING_FILE_COUNT_PRE=0
if not {%WARNING_FILE_COUNT_PRE%} == {0} (
  %OSR_ECHO% =============== PREfast warnings ======================
  if exist "prefast%OSR_EXT%.log" for /f "tokens=*" %%x in ('findstr "warning[^.][CLU]*" "prefast%OSR_EXT%.log"') do @(
    %OSR_ECHO% %%x
  )
)
set /a WARNING_FILE_COUNT=%WARNING_FILE_COUNT%+%WARNING_FILE_COUNT_PRE%
if not {%WARNING_FILE_COUNT%} == {0} (
  %OSR_ECHO% =======================================================
)
endlocal
@echo.
%OSR_ECHO% Build complete
%OSR_ECHO% Building browse information files
if exist "buildbrowse.cmd" call "buildbrowse.cmd" & goto :postBuildSteps
set sbrlist=sbrList.txt
if not exist sbrList%CPU%.txt goto :sbrDefault
set sbrlist=sbrList%CPU%.txt

:sbrDefault
if not exist %sbrlist% goto :postBuildSteps
:: Prepend blank space
if not {%bscFlags%} == {} set bscFlags= %bscFlags%
:: bscmake%bscFlags% prevents a double blank space ...
bscmake%bscFlags% @%sbrlist%

:: Perform whatever post-build steps
:postBuildSteps
:: Restore the current directory (after changing into the build directory!)
:: Search upwards for "AFTERPREBUILD" to find the corresponding PUSHD
popd
@if exist "%buildDirectory%\%OSR_POSTBUILD_SCRIPT%" @(
  %OSR_ECHO% ^>^> Performing post-build steps [%OSR_POSTBUILD_SCRIPT%] ...
  pushd "%buildDirectory%"
  call "%OSR_POSTBUILD_SCRIPT%" > "%TEMP%\%OSR_POSTBUILD_SCRIPT%.tmp"
  for /f "tokens=*" %%x in ('type "%TEMP%\%OSR_POSTBUILD_SCRIPT%.tmp"') do @(
    %OSR_ECHO% %%x
  )
  if exist "%TEMP%\%OSR_POSTBUILD_SCRIPT%.tmp" del /f /q "%TEMP%\%OSR_POSTBUILD_SCRIPT%.tmp"
  popd
  %OSR_ECHO% ^<^< Finished post-build steps [%OSR_POSTBUILD_SCRIPT%] ...
)
goto :END
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: \ MAIN function of the script
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  / GetCustomEnvironment
::    First parameter is the "directory" that supposedly contains the SOURCES
::    or DIRS file (and the build scripts)
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:GetCustomEnvironment
pushd .
call :CheckTargets %1
@if not {%OSR_ERRCODE%} == {0} @(
  echo.
  %OSR_ECHO% The target directory seemed to not contain a DIRS or SOURCES file
  %OSR_ECHO% when trying to set a custom environment! Quitting.
  set buildDirectory=%~f1
  if {%OSR_ERRCODE%} == {6} call :ShowErrorMsg %OSR_ERRCODE% "%ERR_NoTarget%" & goto :GetCustomEnvironment_ret
  call :ShowErrorMsg %OSR_ERRCODE% "%ERR_NoDir%" & goto :GetCustomEnvironment_ret
  goto :GetCustomEnvironment_ret
)
@if exist "%1\%OSR_SETENV_SCRIPT%" @(
  %OSR_ECHO% ^>^> Setting custom environment variables [%OSR_SETENV_SCRIPT%] ...
  pushd "%1"
  for /f "tokens=*" %%x in ('call "%OSR_SETENV_SCRIPT%"') do @(
    %OSR_ECHO% %%x
  )
  popd
  %OSR_ECHO% ^<^< Finished setting custom environment variables [%OSR_SETENV_SCRIPT%] ...
)
:GetCustomEnvironment_ret
popd
goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  \ GetCustomEnvironment
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  / SetMode
::    Subroutine to validate the mode of the build passed in. It must be free,
::    FREE, fre, FRE or checked, CHECKED, chk, CHK. Anything else is an error.
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:SetMode
set BuildMode=
if /i {%OSR_TARGET%} == {WLH2K} goto :SetModeWLH2K
for %%f in (free fre) do if /i {%%f} == {%1} set BuildMode=free
for %%f in (checked chk) do if /i {%%f} == {%1} set BuildMode=checked
goto :SetModeCommonEnd
:SetModeWLH2K
for %%f in (free fre) do if /i {%%f} == {%1} set BuildMode=f
for %%f in (checked chk) do if /i {%%f} == {%1} set BuildMode=c
:SetModeCommonEnd
%OSR_TRACE% Mode set to ^"%BuildMode%^"
if {%BuildMode%} == {} set OSR_ERRCODE=5
goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  \ SetMode
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: / CheckTargets subroutine
::   Subroutine to validate that the target directory exists and that there is
::   either a DIRS or SOURCES and MakeFile in it.
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:CheckTargets
:: Building "stack frame"
setlocal & pushd . & set OSR_ERRCODE=0
if not {%1} == {} goto :CheckTargets1
set OSR_ERRCODE=7
goto :CheckTargets_ret
:CheckTargets1
if exist "%1" goto :CheckTargets2
set OSR_ERRCODE=8
goto :CheckTargets_ret
:CheckTargets2
if not exist "%1\DIRS" goto :CheckTargets3
set OSR_ERRCODE=0
goto :CheckTargets_ret
:CheckTargets3
if exist "%1\SOURCES" goto :CheckTargets4
set OSR_ERRCODE=6
goto :CheckTargets_ret
:CheckTargets4
if exist "%1\MAKEFILE" goto :CheckTargets5
set OSR_ERRCODE=6
goto :CheckTargets_ret
:CheckTargets5
set OSR_ERRCODE=0
:CheckTargets_ret
:: Cleaning "stack frame" and returning error code into global scope
popd & endlocal & set OSR_ERRCODE=%OSR_ERRCODE%
goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: \ CheckTargets subroutine
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: / ResolveVar subroutine
::   There is only one parameter, the name of the variable to be resolved!
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:ResolveVar
:: Get the name of the variable we are working with
setlocal ENABLEEXTENSIONS & set VAR_NAME=%1
set VAR_TEMPRET2=%%%VAR_NAME%%%
:ResolveVarLoop
set VAR_TEMPRET1=%VAR_TEMPRET2%
set VAR_TEMPRET2=%VAR_TEMPRET1%
for /f "tokens=*" %%i in ('echo %VAR_TEMPRET1%') do (
  set VAR_TEMPRET2=%%i
)
if not "%VAR_TEMPRET1%" == "%VAR_TEMPRET2%" goto :ResolveVarLoop
:: Re-export the variable out of the local scope
endlocal & set %VAR_NAME%=%VAR_TEMPRET1%
goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: / ResolveVar subroutine
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: / ErrorWithUsage subroutine
::   This one will take the passed in parameters and build a nice error
::   message which is returned to the user along with the usage hints.
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:ShowErrorMsg
@set OSR_ERRCODE=%~1
@set OSR_ERRMSG=%~2
@set OSR_ERRMSG=%OSR_ERRMSG:'="%
@set OSR_ERRMSG=ERROR #%OSR_ERRCODE%: %OSR_ERRMSG%
@echo.
%OSR_ECHO% %OSR_ERRMSG%
if DEFINED buildDirectory %OSR_ECHO% -^> Target directory: %buildDirectory%
goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: \ ErrorWithUsage subroutine
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Usage output
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:USAGE
@echo.
@echo Syntax:
@echo -------
@echo ddkbuild ^<platform^> ^<build type^> ^<directory^> [flags] [-WDF] [-PREFAST]
@echo.
@echo Values for ^<platform^>:
@echo.
@echo      -------------------------------------------------------------
@echo       Target platform and DDK   ^| Miscellaneous
@echo      ---------------------------^|---------------------------------
@echo       Platform   ^| DDK   ^| CPU  ^| Base directory ^| Platform alias
@echo      ------------^|-------^|------^|----------------^|----------------
@echo       -W2K       ^| W2K   ^| x86  ^| %%W2KBASE%%      ^|
@echo       -W2K64     ^| W2K   ^| IA64 ^| %%W2KBASE%%      ^| -W2KI64
@echo       -WXP       ^| WXP   ^| x86  ^| %%WXPBASE%%      ^|
@echo       -WXP64     ^| WXP   ^| IA64 ^| %%WXPBASE%%      ^| -WXPI64
@echo       -WXP2K     ^| W2K   ^| x86  ^| %%WXPBASE%%      ^|
@echo       -WNET      ^| WNET  ^| x86  ^| %%WNETBASE%%     ^|
@echo       -WNET64    ^| WNET  ^| IA64 ^| %%WNETBASE%%     ^| -WNETI64
@echo       -WNETXP    ^| WXP   ^| x86  ^| %%WNETBASE%%     ^|
@echo       -WNETXP64  ^| WXP   ^| IA64 ^| %%WNETBASE%%     ^|
@echo       -WNETAMD64 ^| WNET  ^| x64  ^| %%WNETBASE%%     ^| -WNETX64
@echo       -WNET2K    ^| W2K   ^| x86  ^| %%WNETBASE%%     ^|
@echo       -WLH       ^| WLH   ^| x86  ^| %%WLHBASE%%      ^|
@echo       -WLH2K     ^| W2K   ^| x86  ^| %%WLHBASE%%      ^|
@echo       -WLHXP     ^| WXP   ^| x86  ^| %%WLHBASE%%      ^|
@echo       -WLHNET    ^| WNET  ^| x86  ^| %%WLHBASE%%      ^|
@echo       -WLHNETI64 ^| WNET  ^| IA64 ^| %%WLHBASE%%      ^|
@echo       -WLHNETX64 ^| WNET  ^| x64  ^| %%WLHBASE%%      ^|
@echo       -WLHI64    ^| WLH   ^| IA64 ^| %%WLHBASE%%      ^|
@echo       -WLHX64    ^| WLH   ^| x64  ^| %%WLHBASE%%      ^|
@echo       -NT4       ^| NT4   ^| x86  ^| %%NT4BASE%%      ^|
@echo      -------------------------------------------------------------
@echo.
@echo Values for ^<build type^>:
@echo        checked, chk    indicates a checked build
@echo        free, fre       indicates a free build
@echo.
@echo Remaining parameters ("opt!" = optional parameter):
@echo        ^<directory^>     path to build directory, try . (cwd)
@echo        [flags]   opt!  any flags you think should be passed to build (try /a
@echo                        for clean)
@echo       -WDF       opt!  performs a WDF build
@echo       -PREFAST   opt!  performs a PREFAST build
@echo.
@echo Special files:
@echo        The build target directory (where the DIRS or SOURCES file resides) can
@echo        contain the following files:
@echo        - %OSR_PREBUILD_SCRIPT%
@echo          Allows to include a step before the BUILD tool from the DDK is called
@echo          but after the environment for the respective DDK has been set!
@echo        - %OSR_POSTBUILD_SCRIPT%
@echo          Allows to include a step after the BUILD tool from the DDK is called,
@echo          so the environment is still available to the script.
@echo        - %OSR_SETENV_SCRIPT%
@echo          Allows to set (or override) _any_ environment variables that may exist
@echo          in the global environment. Thus you can set the base directory for the
@echo          DDK from inside this script, making your project more self-contained.
@echo.
@echo        DDKBUILD will only handle those files which exist, so you may choose to
@echo        use none, one or multiple of these script files.
@echo        (All scripts execute inside there current directory. Consider this!)
@echo.
@echo Examples:
@echo       ^"ddkbuild -NT4 checked .^" (for NT4 BUILD)
@echo       ^"ddkbuild -WXP64 chk .^"
@echo       ^"ddkbuild -WXP chk c:\projects\myproject^"
@echo       ^"ddkbuild -WNET64 chk .^"      (IA64 build)
@echo       ^"ddkbuild -WNETAMD64 chk .^"   (AMD64/EM64T build)
@echo       ^"ddkbuild -WNETXP chk . -cZ -WDF^"
@echo       ^"ddkbuild -WNETXP chk . -cZ -PREFAST^"
@echo.
@echo       In order for this procedure to work correctly for each platform, it
@echo       requires an environment variable to be set up for certain platforms.
@echo       The environment variables are as follows:
@echo.
@echo       %%NT4BASE%%  - Set this up for ^"-NT4^" builds
@echo       %%W2KBASE%%  - Set this up for ^"-W2K^" and ^"-W2K64^" builds
@echo       %%WXPBASE%%  - Set this up for ^"-WXP^", ^"-WXP64^", ^"-WXP2K^" builds
@echo       %%WNETBASE%% - Set this up for ^"-WNET*^" builds
@echo       %%WLHBASE%%  - Set this up for ^"-WLH*^" builds
@echo.
@echo       %%WDF_ROOT%% must be set if attempting to do a WDF Build.
@echo.
@echo.
@echo   %OSR_VERSTR%
@echo   -^> report any problems found to info@osr.com or assarbad.net/contact
@echo.

:END
popd & endlocal & exit /b %OSR_ERRCODE%

@set SOURCEDIR=%1
@set SOURCENAME=%2
@set SOURCEEXT=%3

@rem Make a copy of *.dll to *.drv...
@copy /Y %SOURCEDIR%\%SOURCENAME%.%SOURCEEXT% %SOURCEDIR%\%SOURCENAME%.drv

:done

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


#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdlib.h>


DEFINE_GUID(CLSID_MEDIA,
0x4d36e96c, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x0, 0x2b, 0xe1, 0x03, 0x18);

#define SUCCESS 0
#define ERR_FAIL 1

BOOL OptionSilent = FALSE;

CHAR InfPath[MAX_PATH] = {0};
CHAR DDInstallSection[MAX_PATH] = {0};

BOOL OptionOsVersionCheck = FALSE;
CHAR ExpectedOsVersion[32] = {0};

BOOL OptionOsBuildNumberCheck = FALSE;
DWORD ExpectedOsBuildNumber = 0;

BOOL OptionOsServicePackCheck = FALSE;
DWORD ExpectedOsServicePack = 0;

LPTSTR *
CommandLineToArgv
(
    IN      LPCTSTR lpCmdLine,
    OUT     DWORD * pNumArgs
);

BOOL
InitializeOptions
(
    IN      LPSTR   lpCmdLine
)
{
	BOOL Success1 = FALSE;
	BOOL Success2 = FALSE;

    DWORD ArgC = 0;

    LPTSTR * ArgV = CommandLineToArgv(lpCmdLine, &ArgC);

    if (ArgV)
    {
        for (DWORD i=0; i<ArgC; i++)
        {
            if (_stricmp(ArgV[i], "/s")==0)
            {
                OptionSilent = TRUE;
            }
            else if (_stricmp(ArgV[i],"/silent")==0)
            {
                OptionSilent = TRUE;
            }
            else if (_stricmp(ArgV[i],"/path")==0)
            {
				if (((i+1)<ArgC) && (ArgV[i+1]))
				{
					strcpy(InfPath, ArgV[i+1]);

					Success1 = TRUE;
					i++;
				}
            }
            else if (_stricmp(ArgV[i],"/path:relative")==0)
            {
				if (((i+1)<ArgC) && (ArgV[i+1]))
				{
					GetCurrentDirectory(MAX_PATH, InfPath);
					strcat(InfPath, "\\");
					strcat(InfPath, ArgV[i+1]);

					Success1 = TRUE;
					i++;
				}
            }
            else if (_stricmp(ArgV[i],"/ddinstall")==0)
            {
				if (((i+1)<ArgC) && (ArgV[i+1]))
				{
					strcpy(DDInstallSection, ArgV[i+1]);

					Success2 = TRUE;
					i++;
				}
            }
            else if (_stricmp(ArgV[i],"/os")==0)
            {
				if (((i+1)<ArgC) && (ArgV[i+1]))
				{
					strcpy(ExpectedOsVersion, ArgV[i+1]);

					OptionOsVersionCheck = TRUE;
					i++;
				}
            }
            else if (_stricmp(ArgV[i],"/buildnumber")==0)
            {
				if (((i+1)<ArgC) && (ArgV[i+1]))
				{
					ExpectedOsBuildNumber = atoi(ArgV[i+1]);

					OptionOsBuildNumberCheck = TRUE;
					i++;
				}
            }
            else if (_stricmp(ArgV[i],"/sp")==0)
            {
				if (((i+1)<ArgC) && (ArgV[i+1]))
				{
					ExpectedOsServicePack = atoi(ArgV[i+1]);

					OptionOsServicePackCheck = TRUE;
					i++;
				}
            }
        }

        GlobalFree(HGLOBAL(ArgV));
    }

	return (Success1 && Success2);
}

UINT FileCallback(PVOID Context, UINT Notification, UINT_PTR Param1, UINT_PTR Param2)
{
	return SetupDefaultQueueCallback(Context, Notification, Param1, Param2);
}

int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	if (!InitializeOptions(lpCmdLine)) 
	{
		if (!OptionSilent)
		{
			MessageBox(NULL, "Please specify the correct parameters:\r\n" \
							 "/path <Full path to the INF>\r\n" \
							 "/path:relative <Relative path to the INF from the current directory>\r\n" \
							 "/ddinstall <DDInstall section name to be processed>\r\n" \
 							 "/os <Expected OS version in the form of x.y where x is MajorVersion, y is Minor version, ie 5.0 for Windows 2000>\r\n" \
 							 "/buildnumber <Expected OS build number, ie 2195 for Windows 2000>\r\n" \
 							 "/sp <Expected OS service pack number>\r\n", 
							 "Pre-Installer",
							 MB_OK);

		}

		return ERR_FAIL;
	}

	if ((OptionOsVersionCheck) || (OptionOsBuildNumberCheck) || (OptionOsServicePackCheck))
	{
		OSVERSIONINFO OsVersionInfo; 
		OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if (GetVersionEx(&OsVersionInfo))
		{
			if (OptionOsVersionCheck)
			{
				CHAR Version[32];

				sprintf(Version, "%d.%d", OsVersionInfo.dwMajorVersion, OsVersionInfo.dwMinorVersion);

				if (_stricmp(Version, ExpectedOsVersion))
				{
					if (!OptionSilent)
					{
						CHAR ErrorMsg[256];
						sprintf(ErrorMsg, "OS version %s doesn't match the expected %s", Version, ExpectedOsVersion);

						MessageBox(NULL, ErrorMsg, "Pre-Installer", MB_OK);
					}

					return ERR_FAIL;
				}
			}

			if (OptionOsBuildNumberCheck)
			{
				if (OsVersionInfo.dwBuildNumber != ExpectedOsBuildNumber)
				{
					if (!OptionSilent)
					{
						CHAR ErrorMsg[256];
						sprintf(ErrorMsg, "OS build number %d doesn't match the expected %d", OsVersionInfo.dwBuildNumber, ExpectedOsBuildNumber);

						MessageBox(NULL, ErrorMsg, "Pre-Installer", MB_OK);
					}

					return ERR_FAIL;
				}
			}

			if (OptionOsServicePackCheck)
			{
				CHAR ServicePack[32];

				sprintf(ServicePack, "Service Pack %d", ExpectedOsServicePack);

				if (_stricmp(OsVersionInfo.szCSDVersion, ServicePack))
				{
					if (!OptionSilent)
					{
						CHAR ErrorMsg[256];
						sprintf(ErrorMsg, "OS %s doesn't match the expected %s", OsVersionInfo.szCSDVersion, ServicePack);

						MessageBox(NULL, ErrorMsg, "Pre-Installer", MB_OK);
					}

					return ERR_FAIL;
				}
			}
		}
		else
		{
			return ERR_FAIL;
		}
	}

	HINF InfHandle = SetupOpenInfFile(InfPath, NULL, INF_STYLE_WIN4 , NULL);	// Get INF Handle

	if (InfHandle != INVALID_HANDLE_VALUE)
	{
		HSPFILEQ QueueHandle = SetupOpenFileQueue();

		if (QueueHandle != INVALID_HANDLE_VALUE)
		{
			SetupInstallFilesFromInfSection(InfHandle, NULL, QueueHandle, DDInstallSection, NULL, SP_COPY_FORCE_NEWER);
																		//Copies the files

			PVOID Context = SetupInitDefaultQueueCallback(NULL);

			if (Context)
			{
				SetupCommitFileQueue(NULL, QueueHandle, FileCallback, Context);

				SetupTermDefaultQueueCallback(Context);
			}

			SetupCloseFileQueue(QueueHandle);
		}

		// Do not do the following as we do not want to install the drivers yet. When the device is turned on,
		// Windows PnP will take care of the rest of the installation process.
		#if 0
		CHAR DDInstallServicesSection[MAX_PATH];

		strcpy(DDInstallServicesSection, DDInstallSection);
		strcat(DDInstallServicesSection, ".Services");

		SetupInstallServicesFromInfSection(InfHandle, DDInstallServicesSection, SPSVCINST_TAGTOFRONT);

		HKEY RegKey = SetupDiOpenClassRegKey((LPGUID)&CLSID_MEDIA, KEY_ALL_ACCESS);

		if (RegKey != INVALID_HANDLE_VALUE)
		{
			SetupInstallFromInfSection(NULL, InfHandle, DDInstallSection, SPINST_REGISTRY, RegKey, NULL, NULL,
										NULL, NULL, NULL, NULL);		// Does the AddReg, Del Reg Stuff

			RegCloseKey(RegKey);
		}
		#endif // 0

		SetupCloseInfFile(InfHandle);
	}

	if (SetupCopyOEMInf(InfPath, NULL, SPOST_PATH, NULL, NULL ,0, NULL, NULL))
	{
		return SUCCESS;
	}
	else
	{
		DWORD err = GetLastError();

		return ERR_FAIL;
	}
}

#define ISWWHITE(ch) ((ch)==' ')

// Returns the # of arguments.
static UINT
ParseCommandLine
(
    IN      LPCTSTR     psrc,
    OUT     LPTSTR *    pdstout
)
{
    UINT    argcount = 1;       // discovery of arg0 is unconditional, below
    LPTSTR  pdst     = *pdstout;
    BOOL    fDoWrite = (pdst != NULL);

    BOOL    fInQuotes;
    int     iSlash;

    /* A quoted program name is handled here. The handling is much
    simpler than for other arguments. Basically, whatever lies
    between the leading double-quote and next one, or a terminal null
    character is simply accepted. Fancier handling is not required
    because the program name must be a legal NTFS/HPFS file name.
    Note that the double-quote characters are not copied, nor do they
    contribute to numchars.
    This "simplification" is necessary for compatibility reasons even
    though it leads to mishandling of certain cases.  For example,
    "c:\tests\"test.exe will result in an arg0 of c:\tests\ and an
    arg1 of test.exe.  In any rational world this is incorrect, but
    we need to preserve compatibility.
    */
    LPCTSTR pStart = psrc;
    BOOL    skipQuote = FALSE;

    if (*psrc == '\"')
    {
        // scan from just past the first double-quote through the next
        // double-quote, or up to a null, whichever comes first
        while ((*(++psrc) != '\"') && (*psrc != '\0'))
            continue;

        skipQuote = TRUE;
    }
    else
    {
        /* Not a quoted program name */
        while (!ISWWHITE(*psrc) && *psrc != '\0')
            psrc++;
    }

    // We have now identified arg0 as pStart (or pStart+1 if we have a leading
    // quote) through psrc-1 inclusive
    if (skipQuote)
        pStart++;

    while (pStart < psrc)
    {
        if (fDoWrite)
            *pdst = *pStart;
        pStart++;
        pdst++;
    }
    // And terminate it.
    if (fDoWrite)
        *pdst = '\0';

    pdst++;
    // if we stopped on a double-quote when arg0 is quoted, skip over it
    if (skipQuote && *psrc == '\"')
        psrc++;

    while ( *psrc != '\0')
    {
LEADINGWHITE:

        // The outofarg state.
        while (ISWWHITE(*psrc))
            psrc++;

        if (*psrc == '\0')
            break;
        else
            if (*psrc == '#')
            {
                while (*psrc != '\0' && *psrc != '\n')
                    psrc++;     // skip to end of line

                goto LEADINGWHITE;
            }

        argcount++;
        fInQuotes = FALSE;

        while ((!ISWWHITE(*psrc) || fInQuotes) && *psrc != '\0')
        {
            switch (*psrc)
            {
            case '\\':
                iSlash = 0;
                while (*psrc == '\\')
                {
                    iSlash++;
                    psrc++;
                }

                if (*psrc == '\"')
                {
                    for ( ; iSlash >= 2; iSlash -= 2)
                    {
                        if (fDoWrite)
                            *pdst = '\\';
                        pdst++;
                    }

                    if (iSlash & 1)
                    {
                        if (fDoWrite)
                            *pdst = *psrc;
                        psrc++;
                        pdst++;
                    }
                    else
                    {
                        fInQuotes = !fInQuotes;
                        psrc++;
                    }
                }
                else
                    for ( ; iSlash > 0; iSlash--)
                    {
                        if (fDoWrite)
                            *pdst = '\\';
                        pdst++;
                    }
                break;

            case '\"':
                fInQuotes = !fInQuotes;
                psrc++;
                break;

            default:
                if (fDoWrite)
                    *pdst = *psrc;
                psrc++;
                pdst++;
            }
        }

        if (fDoWrite)
            *pdst = '\0';

        pdst++;
    }

    *pdstout = pdst;

    return argcount;
}

LPTSTR *
CommandLineToArgv
(
    IN      LPCTSTR lpCmdLine,
    OUT     DWORD * pNumArgs
)
{
     DWORD argcount = 0;
     LPTSTR retval = NULL;
     LPTSTR *pslot;

     LPTSTR pdst = NULL;
     argcount = ParseCommandLine(lpCmdLine, &pdst);

     // This check is because on WinCE the Application Name is not passed in as an argument to the app!
     if (argcount == 0)
     {
         *pNumArgs = 0;
         return NULL;
     }

     // Now we need alloc a buffer the size of the command line + the number of strings * DWORD
     retval = (LPTSTR)GlobalAlloc(GPTR, (argcount*sizeof(TCHAR*))/sizeof(TCHAR) + (pdst - (LPTSTR)NULL));

     if(!retval)
         return NULL;

     pdst = (LPTSTR)( argcount*sizeof(LPTSTR*) + (BYTE*)retval );
     ParseCommandLine(lpCmdLine, &pdst);
     pdst = (LPTSTR)( argcount*sizeof(LPTSTR*) + (BYTE*)retval );
     pslot = (LPTSTR*)retval;

     for (DWORD i = 0; i < argcount; i++)
     {
         *(pslot++) = pdst;

         while (*pdst != '\0')
         {
             pdst++;
         }
         pdst++;
     }

     *pNumArgs = argcount;

     return (LPTSTR*)retval;
}

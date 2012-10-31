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
/*
 *****************************************************************************
 *//*!
 * @file       Profile.cpp
 * @brief      Implementation of private profile routines.
 * @copyright  E-MU Systems, 2004.
 * @author     <put-your-name-here>.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Profile.h"

/*! @brief Debug module name. */
#define STR_MODULENAME "Profile: "

/*****************************************************************************
 * Forward Declarations
 */
NTSTATUS
OpenFile
(
	IN      LPWSTR		FilePathName,
	IN      LPSTR		Access,
	OUT     HANDLE *	OutFileHandle
);

VOID 
CloseFile
(
	IN		HANDLE	FileHandle
);

NTSTATUS
GetFileSize
(
	IN		HANDLE	FileHandle,
	OUT		ULONG *	OutFileSize
);

NTSTATUS
ReadFile
(
	IN		HANDLE	FileHandle,
	IN		ULONG	ByteOffset,
	IN		PVOID	Buffer,
	IN		ULONG	BufferLength,
	OUT		ULONG *	OutBytesRead
);

inline LPWSTR
MakeString
(
    IN      LPWSTR  Source,
    IN      BOOL    IsPath
);

inline VOID
FreeString
(
    IN      LPWSTR	Source
);

inline VOID
CopyString
(
    IN      LPWSTR  Destination,
    IN      LPWSTR  Source,
    IN      BOOL    IsPath
);

/*****************************************************************************
 * Defines
 */
#if PLATFORM_UNIX
#define EOL     "\012"
#define PATH_SEPARATOR  "/"
#define PATH_SEPARATOR_CHAR  '/'
#else
#define EOL     "\015\012"
#define PATH_SEPARATOR  "\\"
#define PATH_SEPARATOR_CHAR  '\\'
#endif

 /*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CFileReadBuffer
 *****************************************************************************
 * @brief
 * File read buffer object.
 * @details
 */
class CFileReadBuffer
{
private:
    LPWSTR				m_FilePathName;			// Path name of the file.
    HANDLE				m_FileHandle;			// File handle.
    ULONG				m_FileSize;				// Size of the file.
    BOOL				m_IsFileOpened;			// True if file is opened, otherwise false.
    CHAR				m_IoBuffer[0x100];		// Storage area to store the data read from file.
	ULONG				m_IoBufferSize;			// Size of the storage area to store the data read from file.
    LPSTR				m_ReadPtr;				// Current buffer read pointer.
    ULONG				m_BytesRead;			// Number of valid bytes from the read pointer.
    ULONG				m_TotalSize;			// Total number of bytes read from the file.
    USHORT				m_LineNumber;			// Current line number.
    USHORT				m_NumberOfNullChar;		// Number of null characters in file.
    LPSTR				m_CommentToEOL;			// Comment to EOL string.	
    ULONG				m_CommentToEOLLength;	// Length of EOL string in characters.
    BOOL				m_IsEof;				// True if EOF, otherwise false.

	BOOL _ReadBuffer
	(	void
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
	CFileReadBuffer();
	~CFileReadBuffer();

    /*************************************************************************
     * File I/O methods.
     */
	BOOL Open
	(
		IN      LPWSTR  FilePathName,
		IN      LPSTR   CommentToEOL
	);
	BOOL Setup
	(
		IN      LPWSTR  FilePathName,
		IN      LPSTR   CommentToEOL
	);
	VOID Close
	(	void
	);
	LPSTR ReadLine
	(	void
	);
};

 /*************************************************************************
 * CFileReadBuffer::CFileReadBuffer()
 *************************************************************************
 * Constructor.
 */
CFileReadBuffer::
CFileReadBuffer
(	void	
)
{
	PAGED_CODE();

    m_FilePathName = NULL;
	m_FileHandle = NULL;
	m_FileSize = 0;
	m_IsFileOpened = FALSE;
	m_ReadPtr = NULL;
    m_BytesRead = 0;
    m_TotalSize = 0;
    m_LineNumber = 0;
	m_NumberOfNullChar = 0;
	m_CommentToEOL = NULL;
    m_CommentToEOLLength = 0;
	m_IsEof = FALSE;

	RtlZeroMemory(m_IoBuffer, sizeof(m_IoBuffer));
	m_IoBufferSize = sizeof(m_IoBuffer);
 }

/*************************************************************************
 * CFileReadBuffer::~CFileReadBuffer()
 *************************************************************************
 * Destructor.
 */
CFileReadBuffer::
~CFileReadBuffer
(	void
)
{
	PAGED_CODE();
}

/*************************************************************************
 * CFileReadBuffer::Open()
 *************************************************************************
 */
BOOL 
CFileReadBuffer::
Open
(
	IN      LPWSTR  FilePathName,
	IN      LPSTR   CommentToEOL
)
{
	PAGED_CODE();

	BOOL status = FALSE;

    if (!m_IsFileOpened)
    {
		if (Setup(FilePathName, CommentToEOL))
		{
			status = TRUE;
		}
	}

    return status;
}

/*************************************************************************
 * CFileReadBuffer::Setup()
 *************************************************************************
 */
BOOL 
CFileReadBuffer::
Setup
(
	IN      LPWSTR  FilePathName,
	IN      LPSTR   CommentToEOL
)
{
	PAGED_CODE();

    ASSERT(!m_IsFileOpened);
    ASSERT(m_FileHandle == NULL);
    ASSERT(m_FilePathName == NULL);

    if (!NT_SUCCESS(OpenFile(FilePathName, "rb", &m_FileHandle)))
    {
        return FALSE;
    }

    m_LineNumber = 0;
    m_NumberOfNullChar = 0;
    m_TotalSize = 0;
    m_IsEof = FALSE;
    m_CommentToEOL = CommentToEOL;
    m_CommentToEOLLength = strlen(CommentToEOL);

	GetFileSize(m_FileHandle, &m_FileSize);

    if (!_ReadBuffer())
    {
        CloseFile(m_FileHandle);

        m_FileHandle = NULL;

        return FALSE;          // zero byte file
    }

    m_FilePathName = MakeString(FilePathName, TRUE);
	
    m_IsFileOpened = TRUE;

    return TRUE;
}

/*************************************************************************
 * CFileReadBuffer::Close()
 *************************************************************************
 */
VOID
CFileReadBuffer::
Close
(	void
)
{
	PAGED_CODE();

	ASSERT(m_IsFileOpened);
	ASSERT(m_FileHandle != NULL);
	ASSERT(m_FilePathName != NULL);

	CloseFile(m_FileHandle);

	m_IsFileOpened = FALSE;
	m_FileHandle = NULL;

	FreeString(m_FilePathName);
	m_FilePathName = NULL;
}

/*************************************************************************
 * CFileReadBuffer::ReadLine()
 *************************************************************************
 */
LPSTR
CFileReadBuffer::
ReadLine
(	void
)
{
	PAGED_CODE();

	ASSERT(m_FileHandle != NULL);

	if (m_BytesRead == 0)
	{
		if (m_IsEof)
		{
			return NULL;
		}

		if (!_ReadBuffer())
		{
			return NULL;
		}
	}

	LPSTR Line = m_ReadPtr;
	LPSTR p = Line;

	LPSTR End = &p[m_BytesRead];
	LPSTR Continuation = NULL;

	LPSTR Comment = NULL;
	BOOL IsInComment = FALSE;

	//  scan through line forward
	while (p < End)
	{
		switch (*p)
		{
			case ' ':
			case '\t':
			case '\r':
				*p = ' ';
				break;

			case '\\':
				Continuation = p; // remember continuation character
				break;

			case '\n': //  Are we at an end of line?
			case '\0':
				if (*p == '\n')
				{
					m_LineNumber++;
				}

				if (IsInComment)
				{
					memset(Comment, ' ', p-Comment-1);

					IsInComment = FALSE;
				}

				if (Continuation == NULL)
				{
					goto eol; // bail out if single line, else combine multiple lines...
				}              

				*Continuation = ' '; // remove continuation char
				Continuation = NULL; // eat only one line per continuation

				*p = ' '; // join the lines with blanks
				break;

			default:
				//  See if the character we're examining begins the
				//  comment-to-EOL string.
				if ((*p == m_CommentToEOL[0]) &&
					(!strncmp(p, m_CommentToEOL, m_CommentToEOLLength)) &&
					(!IsInComment))
				{
					IsInComment = TRUE;

					Comment = p;
				}

				Continuation = NULL;               // not a continuation character

				break;
		}

		p++;
	}

eol:
	ASSERT(m_BytesRead >= (ULONG)(p - m_ReadPtr));

	m_BytesRead -= (ULONG)(p - m_ReadPtr);
	m_ReadPtr = p;

	if (Continuation != NULL)
	{
		*Continuation = ' '; // file ended with backslash...
	}

	ASSERT((*p == '\0') || (*p == '\n'));

	if (p < End)
	{
		if (*p == '\0')
		{
			if (m_NumberOfNullChar++ == 0)
			{
				_DbgPrintF(DEBUGLVL_TERSE,("null byte at offset %lx", m_TotalSize - m_BytesRead + p - m_ReadPtr));
			}
		}

		*p = '\0'; // terminate line

		ASSERT(m_BytesRead >= 1);
		m_BytesRead--; // account for newline (or null)
		m_ReadPtr++;
	}
	else
	{
		ASSERT((p == End) && (*p == '\0'));

		if ((*Line == 'Z' - 64) && (p == &Line[1]) && (m_BytesRead == 0))
		{
			Line = NULL; // found CTL-Z at end of file
		}
		else
		{
	        _DbgPrintF(DEBUGLVL_TERSE,( "last line incomplete."));
		}
	}

	BOOL IsWhiteSpace = FALSE;

	if (Line != NULL)
	{
		while (*Line == ' ')
		{
			Line++; // skip leading whitespace

			IsWhiteSpace = TRUE;
		}

		if (*p != '\0')
		{
			_DbgPrintF(DEBUGLVL_TERSE,("\"*p != '\\0'\" at offset %lx", m_TotalSize - m_BytesRead + p - m_ReadPtr));
			_DbgPrintF(DEBUGLVL_TERSE,("Line=%p(%s) p=%p(%s)\n", Line, Line, p, p));
		}

		ASSERT(*p == '\0');

		while ((p > Line) && (*--p == ' '))
		{
			*p = '\0'; // truncate trailing whitespace
		}
	}

	if (Line == NULL)
	{
		return NULL;
	}


	return Line;
}

/*************************************************************************
 * CFileReadBuffer::_ReadBuffer()
 *************************************************************************
 */
BOOL
CFileReadBuffer::
_ReadBuffer
(   void
)
{
	PAGED_CODE();

    ASSERT(!m_IsEof);

    m_ReadPtr = m_IoBuffer;

	ReadFile(m_FileHandle, m_TotalSize, m_IoBuffer, m_IoBufferSize - 1, &m_BytesRead);

    if (m_BytesRead == 0)
    {
        m_IsEof = TRUE; // no more to read

        return FALSE;
    }

    static CHAR achzeros[16] = {0};

    if ((m_TotalSize == 0) &&
        (m_BytesRead > sizeof(achzeros)) &&
        (memcmp(m_IoBuffer, achzeros, sizeof(achzeros)) == 0))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("ignoring binary file"));

        m_IsEof = TRUE;

        return FALSE;
    }

    LPSTR p1 = &m_IoBuffer[m_BytesRead - 1];

    if ((m_TotalSize + m_BytesRead) < m_FileSize)
    {
        LPSTR p2;

        do
        {
            while ((p1 > m_IoBuffer) && (*p1 != '\n'))
            {
                p1--;
            }

            p2 = p1; // save end of last complete line

            if ((p1 > m_IoBuffer) && (*p1 == '\n'))
            {
                p1--;

                if ((p1 > m_IoBuffer) && (*p1 == '\r'))
                {
                    p1--;
                }

                while ((p1 > m_IoBuffer) && ((*p1 == '\t') || (*p1 == ' ')))
                {
                    p1--;
                }
            }
        }
        while (*p1 == '\\');

        if (p1 == m_IoBuffer)
        {
            _DbgPrintF(DEBUGLVL_ERROR,("(Fatal Error) too many continuation lines."));

            return FALSE;
        }

        p1 = p2; // restore end of last complete line

        m_BytesRead = (ULONG)(p1 - m_IoBuffer + 1);
    }
    else
    {
        m_IsEof = TRUE; // no more to read
    }

    p1[1] = '\0';

    m_TotalSize += m_BytesRead;

    return TRUE;
}

/*! @brief Convert lower case character to upper case. */
#define UPCASE(ch) ((ch>=L'a')&&(ch<=L'z')) ? L'A'+(ch-L'a') : ch

/*****************************************************************************
 * PrefixUnicodeString()
 *****************************************************************************
 * Determine if a string is a prefix of another string.
 */
static
BOOLEAN
PrefixUnicodeString
(
    /*IN*/  PCWSTR  String1_,
    /*IN*/  PCWSTR  String2_,
    /*IN*/  ULONG   Offset,
    /*IN*/	BOOLEAN CaseInsensitive
)
{
	PAGED_CODE();

    UNICODE_STRING String1, String2;
    RtlInitUnicodeString(&String1, String1_);
    RtlInitUnicodeString(&String2, String2_);

    if (String1.Length <= String2.Length-Offset*sizeof(WCHAR))  // String1 is prefix of String2
    {
        for (ULONG i=0;i<String1.Length/sizeof(WCHAR);i++)
        {
            WCHAR ch1 = (CaseInsensitive) ? UPCASE(String1.Buffer[i]) : String1.Buffer[i];
            WCHAR ch2 = (CaseInsensitive) ? UPCASE(String2.Buffer[i+Offset]) : String2.Buffer[i+Offset];

            if (ch1 != ch2) return FALSE; // no match
        }

        return TRUE; // match
    }

    return FALSE; // no match
}

/*****************************************************************************
 * OpenFile()
 *****************************************************************************
 * Open a file.
 */
NTSTATUS
OpenFile
(
	IN      LPWSTR		FilePathName,
	IN      LPSTR		Access,
	OUT     HANDLE *	OutFileHandle
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    PWCHAR wFileName = PWCHAR(ExAllocatePoolWithTag(NonPagedPool, (wcslen(FilePathName)+1)*sizeof(WCHAR)+sizeof(L"\\DosDevices\\"), 'mdW'));

    if (wFileName)
    {
        RtlZeroMemory(wFileName, wcslen(FilePathName)*sizeof(WCHAR));

        if (PrefixUnicodeString(L"\\", FilePathName, 0, TRUE))
        {
            // anything with prefix '\' is assumed to be the absolute path.
            wcscpy(wFileName, L"");
        }
        else
        {
            // Attach the "\DosDevices\" to form the absolute path. Contrary to
            // what the DDK documentation says, the "\??\" doesn't work under
            // Win98.
            wcscpy(wFileName, L"\\DosDevices\\");
        }

        wcscat(wFileName, FilePathName);

        UNICODE_STRING FileName;

        RtlInitUnicodeString(&FileName, PCWSTR(wFileName));

        OBJECT_ATTRIBUTES ObjectAttributes;

        InitializeObjectAttributes(&ObjectAttributes,
                                    &FileName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

        ACCESS_MASK DesiredAccess = SYNCHRONIZE;

        DesiredAccess |= (strchr(Access, 'r')) ? GENERIC_READ : 0;
        DesiredAccess |= (strchr(Access, 'w')) ? GENERIC_WRITE : 0;

        HANDLE FileHandle;

        IO_STATUS_BLOCK IoStatusBlock;

        ntStatus = ZwOpenFile
					(
						&FileHandle,
                        DesiredAccess,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT
					);

        if (NT_SUCCESS(ntStatus))
        {
            *OutFileHandle = FileHandle;
        }

        ExFreePool(wFileName);
    }

	return ntStatus;
}

/*****************************************************************************
 * CloseFile()
 *****************************************************************************
 * Close a file.
 */
VOID 
CloseFile
(
	IN		HANDLE	FileHandle
)
{
	PAGED_CODE();

	if (FileHandle)
	{
		ZwClose(FileHandle);
	}
}

/*****************************************************************************
 * GetFileSize()
 *****************************************************************************
 * Gets a file size.
 */
NTSTATUS
GetFileSize
(
	IN		HANDLE	FileHandle,
	OUT		ULONG *	OutFileSize
)
{
	PAGED_CODE();

	ASSERT(FileHandle);
	ASSERT(OutFileSize);

	FILE_STANDARD_INFORMATION FileStdInfo;

    IO_STATUS_BLOCK IoStatusBlock;

    NTSTATUS ntStatus = ZwQueryInformationFile
						(
							FileHandle,
                            &IoStatusBlock,
                            &FileStdInfo,
                            sizeof(FILE_STANDARD_INFORMATION),
                            FileStandardInformation
						);

    if (NT_SUCCESS(ntStatus))
    {
        *OutFileSize = FileStdInfo.EndOfFile.LowPart;
    }
    else
    {
        *OutFileSize = 0;
    }

    return ntStatus;
}

/*****************************************************************************
 * ReadFile()
 *****************************************************************************
 *//*!
 * Read from a file.
 */
NTSTATUS
ReadFile
(
	IN		HANDLE	FileHandle,
	IN		ULONG	ByteOffset,
	IN		PVOID	Buffer,
	IN		ULONG	BufferLength,
	OUT		ULONG *	OutBytesRead
)
{
	PAGED_CODE();

	ASSERT(FileHandle);
	ASSERT(OutBytesRead);

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if (BufferLength)
	{
		ASSERT(Buffer);

		LARGE_INTEGER Offset;

		Offset.LowPart = ByteOffset;
		Offset.HighPart = 0;

		IO_STATUS_BLOCK IoStatus;

		ntStatus = ZwReadFile
					(
						FileHandle,
						NULL,
						NULL,
						NULL,
						&IoStatus,
						Buffer,
						BufferLength,
						&Offset,
						NULL
					);

		if (NT_SUCCESS(ntStatus))
		{
			*OutBytesRead = ULONG(IoStatus.Information);
		}
		else
		{
		    *OutBytesRead = 0;
		}
	}
	else
	{
	    *OutBytesRead = 0;

		ntStatus = STATUS_SUCCESS;
	}

    return ntStatus;
}

/*****************************************************************************
 * MakeString()
 *****************************************************************************
 */
inline LPWSTR
MakeString
(
    IN      LPWSTR  Source,
    IN      BOOL    IsPath
)
{
	PAGED_CODE();

    if (Source == NULL)
    {
        Source = L"";
    }

    LPWSTR Destination  = (LPWSTR)ExAllocatePoolWithTag(PagedPool, (wcslen(Source) + 1) * sizeof(WCHAR), 'mdW');

    CopyString(Destination, Source, IsPath);

	return Destination;
}

/*****************************************************************************
 * FreeString()
 *****************************************************************************
 */
inline VOID
FreeString
(
    IN      LPWSTR	Source
)
{
	PAGED_CODE();

    if (Source != NULL)
    {
        ExFreePool(Source);
    }
}

/*****************************************************************************
 * CopyString()
 *****************************************************************************
 * Copy a string.
 */
inline VOID
CopyString
(
    IN      LPWSTR  Destination,
    IN      LPWSTR  Source,
    IN      BOOL    IsPath
)
{
	PAGED_CODE();

    WCHAR ch;

    while ((ch = *Source++) != '\0')
    {
        #if PLATFORM_WIN32
        if (IsPath)
        {
            if ((ch >= 'A') && (ch <= 'Z') )
            {
                ch -= (WCHAR)('A' - 'a');
            }
            else
            {
                if (ch == '/')
                {
                    ch = '\\';
                }
            }
        }
        #endif  // PLATFORM_WIN32

        *Destination++ = ch;
    }

    *Destination = ch;
}

/*****************************************************************************
 * AToX()
 *****************************************************************************
 *//*!
 * @brief
 * Hex atoi with pointer bumping and success flag.
 */
inline BOOL
AToX
(
    IN OUT  LPSTR *	Str,
    OUT     ULONG * OutValue
)
{
	PAGED_CODE();

	ULONG r;
    LPSTR p = *Str;

    while (*p == ' ')
    {
        p++;
    }

    BOOL IsHexDigit = FALSE;

    int digit;

	for (r = 0; isxdigit(digit = *p); p++)
    {
        IsHexDigit = TRUE;

        if (isdigit(digit))
        {
            digit -= '0';
        }
        else if (isupper(digit))
        {
            digit -= 'A' - 10;
        }
        else
        {
            digit -= 'a' - 10;
        }

        r = (r << 4) + digit;
    }

    *Str = p;

    *OutValue = r;

    return IsHexDigit;
}

/*****************************************************************************
 * AToX()
 *****************************************************************************
 *//*!
 * @brief
 * Hex atoi with pointer bumping and success flag.
 */
inline BOOL
AToX
(
    IN OUT  LPSTR *	Str,
    OUT     USHORT * OutValue
)
{
	PAGED_CODE();

	USHORT r;
    LPSTR p = *Str;

    while (*p == ' ')
    {
        p++;
    }

    BOOL IsHexDigit = FALSE;

    int digit;

	for (r = 0; isxdigit(digit = *p); p++)
    {
        IsHexDigit = TRUE;

        if (isdigit(digit))
        {
            digit -= '0';
        }
        else if (isupper(digit))
        {
            digit -= 'A' - 10;
        }
        else
        {
            digit -= 'a' - 10;
        }

        r = (r << 4) + digit;
    }

    *Str = p;

    *OutValue = r;

    return IsHexDigit;
}

/*****************************************************************************
 * AToX()
 *****************************************************************************
 *//*!
 * @brief
 * Hex atoi with pointer bumping and success flag.
 */
inline BOOL
AToX
(
    IN OUT  LPSTR *	Str,
    OUT     UCHAR * OutValue
)
{
	PAGED_CODE();

	UCHAR r;
    LPSTR p = *Str;

    while (*p == ' ')
    {
        p++;
    }

    BOOL IsHexDigit = FALSE;

    int digit;

	for (r = 0; isxdigit(digit = *p); p++)
    {
        IsHexDigit = TRUE;

        if (isdigit(digit))
        {
            digit -= '0';
        }
        else if (isupper(digit))
        {
            digit -= 'A' - 10;
        }
        else
        {
            digit -= 'a' - 10;
        }

        r = (r << 4) + digit;
    }

    *Str = p;

    *OutValue = r;

    return IsHexDigit;
}

/*****************************************************************************
 * AToD()
 *****************************************************************************
 *//*!
 * @brief
 * Decimal atoi with pointer bumping and success flag.
 */
inline BOOL
AToD
(
    IN OUT  LPSTR *	Str,
    OUT     LONG *	OutValue
)
{
	PAGED_CODE();

	LONG r;
    LPSTR p = *Str;

    while (*p == ' ')
    {
        p++;
    }

    BOOL IsDigit = FALSE;

    int digit;

    for (r = 0; isdigit(digit = *p); p++)
    {
        IsDigit = TRUE;

        digit -= '0';

        r = (r * 10) + digit;
    }

    *Str = p;

    *OutValue = r;

    return IsDigit;
}

/*****************************************************************************
 * AToD()
 *****************************************************************************
 *//*!
 * @brief
 * Decimal atoi with pointer bumping and success flag.
 */
inline BOOL
AToD
(
    IN OUT  LPSTR *	Str,
    OUT     SHORT * OutValue
)
{
	PAGED_CODE();

	SHORT r;
    LPSTR p = *Str;

    while (*p == ' ')
    {
        p++;
    }

    BOOL IsDigit = FALSE;

    int digit;

	for (r = 0; isdigit(digit = *p); p++)
    {
        IsDigit = TRUE;

        digit -= '0';

        r = (r * 10) + digit;
    }

    *Str = p;

    *OutValue = r;

    return IsDigit;
}

/*****************************************************************************
 * AToD()
 *****************************************************************************
 *//*!
 * @brief
 * Decimal atoi with pointer bumping and success flag.
 */
inline BOOL
AToD
(
    IN OUT  LPSTR *	Str,
    OUT     CHAR *  OutValue
)
{
	PAGED_CODE();

	CHAR r;
    LPSTR p = *Str;

    while (*p == ' ')
    {
        p++;
    }

    BOOL IsDigit = FALSE;

    int digit;

    for (r = 0; isdigit(digit = *p); p++)
    {
        IsDigit = TRUE;

        digit -= '0';

        r = (r * 10) + digit;
    }

    *Str = p;

    *OutValue = r;

    return IsDigit;
}

/*****************************************************************************
 * DrvProfileGetUlong()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
DrvProfileGetUlong
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		ULONG	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	ULONG IntValue = Default;

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					line = Frb.ReadLine();

					while (line)
					{
						// New section begins, end the search.
						if (line[0] == '[') break;

						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							// Found the key ?
							if (_stricmp(KeyName, line) == 0)
							{
								// Convert to hexadecimal.
								AToX(&p, &IntValue);

								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return IntValue;
}

/*****************************************************************************
 * DrvProfileGetUshort()
 *****************************************************************************
 *//*!
 * @brief
 */
USHORT 
DrvProfileGetUshort
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		USHORT	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	USHORT IntValue = Default;

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					line = Frb.ReadLine();

					while (line)
					{
						// New section begins, end the search.
						if (line[0] == '[') break;

						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							// Found the key ?
							if (_stricmp(KeyName, line) == 0)
							{
								// Convert to hexadecimal.
								AToX(&p, &IntValue);
								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return IntValue;
}

/*****************************************************************************
 * DrvProfileGetUchar()
 *****************************************************************************
 *//*!
 * @brief
 */
UCHAR 
DrvProfileGetUchar
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		UCHAR	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	UCHAR IntValue = Default;

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					line = Frb.ReadLine();

					while (line)
					{
						// New section begins, end the search.
						if (line[0] == '[') break;

						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							// Found the key ?
							if (_stricmp(KeyName, line) == 0)
							{
								// Convert to hexadecimal.
								AToX(&p, &IntValue);

								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return IntValue;
}

/*****************************************************************************
 * DrvProfileGetLong()
 *****************************************************************************
 *//*!
 * @brief
 */
LONG 
DrvProfileGetLong
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		LONG	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	LONG IntValue = Default;

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					line = Frb.ReadLine();

					while (line)
					{
						// New section begins, end the search.
						if (line[0] == '[') break;

						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							// Found the key ?
							if (_stricmp(KeyName, line) == 0)
							{
								// Convert to decimal.
								AToD(&p, &IntValue);

								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return IntValue;
}

/*****************************************************************************
 * DrvProfileGetShort()
 *****************************************************************************
 *//*!
 * @brief
 */
SHORT 
DrvProfileGetShort
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		SHORT	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	SHORT IntValue = Default;

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					line = Frb.ReadLine();

					while (line)
					{
						// New section begins, end the search.
						if (line[0] == '[') break;

						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							// Found the key ?
							if (_stricmp(KeyName, line) == 0)
							{
								// Convert to decimal.
								AToD(&p, &IntValue);
								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return IntValue;
}

/*****************************************************************************
 * DrvProfileGetChar()
 *****************************************************************************
 *//*!
 * @brief
 */
CHAR 
DrvProfileGetChar
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		CHAR	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	CHAR IntValue = Default;

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					line = Frb.ReadLine();

					while (line)
					{
						// New section begins, end the search.
						if (line[0] == '[') break;

						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							// Found the key ?
							if (_stricmp(KeyName, line) == 0)
							{
								// Convert to decimal.
								AToD(&p, &IntValue);

								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return IntValue;
}

/*****************************************************************************
 * DrvProfileGetString()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
DrvProfileGetString
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		LPSTR	Default,		// return value if key name is not found
    IN		LPSTR	ReturnedString,	// points to destination buffer 
    IN		DWORD	Size,			// size of destination buffer 
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	strcpy(ReturnedString, Default);

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					while (line)
					{
						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							if (_stricmp(KeyName, line) == 0)
							{
								// Remove quote if found.
								CHAR * q = strchr(p, '"'); if (q) p++; q = strrchr(p, '"'); if (q) *q = 0;

								if (Size >= (strlen(p)+1))
								{
									strcpy(ReturnedString, p);
								}
								else
								{
									strncpy(ReturnedString, p, Size-1);

									ReturnedString[Size] = '\0';
								}

								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return strlen(ReturnedString);
}

/*****************************************************************************
 * DrvProfileGetUnicodeString()
 *****************************************************************************
 *//*!
 * @brief
 */
ULONG 
DrvProfileGetUnicodeString
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		PWCHAR	Default,		// return value if key name is not found
    IN		PWCHAR	ReturnedString,	// points to destination buffer 
    IN		DWORD	Size,			// size of destination buffer in characters
    IN		LPWSTR	FilePathName	// address of initialization filename
)
{
	PAGED_CODE();

	wcscpy(ReturnedString, Default);

	CFileReadBuffer Frb;

	if (Frb.Open(FilePathName, ";"))
	{
		LPSTR line = Frb.ReadLine();

		while (line)
		{
			// Begining of a section ?
			if (line[0] == '[')
			{
				line++;

				CHAR * t = strrchr(line, ']'); if (t) *t = '\0';
				
				// Is this the section that we are looking for ?
				if (_stricmp(SectionName, line) == 0)
				{
					while (line)
					{
						CHAR * p = strchr(line, '='); 
						
						if (p)
						{
							*p = 0; p++;

							if (_stricmp(KeyName, line) == 0)
							{
								// Remove quote if found.
								CHAR * q = strchr(p, '"'); if (q) p++; q = strrchr(p, '"'); if (q) *q = 0;
								
								ULONG i=0;

								while (p && (*p) && (*p=='\\') && (i<((Size-1)*2)))
								{
									p++;

									CHAR * q = strchr(p, '\\');
									
									if (q) *q = '\0';

									ULONG hex; AToX(&p, &hex);

									UCHAR * value = (PUCHAR)ReturnedString; value[i] = UCHAR(hex);

									if(q) *q = '\\';

									p = q;

									i++;
								}

								i = ((i+1)/2)*2; // round it up.

								ReturnedString[(i/2)] = 0;

								break;
							}
						}

						line = Frb.ReadLine();
					}

					// Done looking at the Section & Key.
					break;
				}
			}

			line = Frb.ReadLine();
		}

		Frb.Close();
	}

	return wcslen(ReturnedString);
}

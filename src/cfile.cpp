#include "cfile.h"
#include <assert.h>

#define ASSERT  assert

////////////////////////////////////////////////////////////////////////////
// CFile implementation

CFile::CFile()
{
    m_hFile = (UINT) hFileNull;
    m_bCloseOnDelete = FALSE;
}

CFile::CFile(int hFile)
{
    m_hFile = hFile;
    m_bCloseOnDelete = FALSE;
}


CFile::~CFile()
{
    if (m_hFile != (UINT)hFileNull && m_bCloseOnDelete)
        Close();
}


BOOL CFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

    // CFile objects are always binary and CreateFile does not need flag
    nOpenFlags &= ~(UINT)typeBinary;

    m_bCloseOnDelete = FALSE;

    ASSERT(_tcslen(lpszFileName)<_MAX_PATH);
    strcpy(m_strFileName, lpszFileName);

    ASSERT(sizeof(HANDLE) == sizeof(UINT));
    ASSERT(shareCompat == 0);

    // map read/write mode
    ASSERT((modeRead|modeWrite|modeReadWrite) == 3);
    DWORD dwAccess = 0;
    switch (nOpenFlags & 3)
    {
    case modeRead:
        dwAccess = GENERIC_READ;
        break;
    case modeWrite:
        dwAccess = GENERIC_WRITE;
        break;
    case modeReadWrite:
        dwAccess = GENERIC_READ|GENERIC_WRITE;
        break;
    default:
        ASSERT(FALSE);  // invalid share mode
    }

    // map share mode
    DWORD dwShareMode = 0;
    switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
    {
    default:
        ASSERT(FALSE);  // invalid share mode?
    case shareCompat:
    case shareExclusive:
        dwShareMode = 0;
        break;
    case shareDenyWrite:
        dwShareMode = FILE_SHARE_READ;
        break;
    case shareDenyRead:
        dwShareMode = FILE_SHARE_WRITE;
        break;
    case shareDenyNone:
        dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ;
        break;
    }

    // Note: typeText and typeBinary are used in derived classes only.

    // map modeNoInherit flag
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

    // map creation flags
    DWORD dwCreateFlag;
    if (nOpenFlags & modeCreate)
    {
        if (nOpenFlags & modeNoTruncate)
            dwCreateFlag = OPEN_ALWAYS;
        else
            dwCreateFlag = CREATE_ALWAYS;
    }
    else
        dwCreateFlag = OPEN_EXISTING;

    // attempt file creation
    HANDLE hFile = ::CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
        dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    m_hFile = (HFILE)hFile;
    m_bCloseOnDelete = TRUE;

    return TRUE;
}

UINT CFile::Read(void* lpBuf, UINT nCount)
{
    ASSERT(m_hFile != (UINT)hFileNull);

    if (nCount == 0)
        return 0;   // avoid Win32 "null-read"

    ASSERT(lpBuf != NULL);

    DWORD dwRead = 0;
    ::ReadFile((HANDLE)m_hFile, lpBuf, nCount, &dwRead, NULL);

    return (UINT)dwRead;
}

BOOL CFile::Write(const void* lpBuf, UINT nCount)
{
    ASSERT(m_hFile != (UINT)hFileNull);

    if (nCount == 0)
        return TRUE;     // avoid Win32 "null-write" option

    ASSERT(lpBuf != NULL);

    DWORD nWritten = 0;
    return ::WriteFile((HANDLE)m_hFile, lpBuf, nCount, &nWritten, NULL);
}

LONG CFile::Seek(LONG lOff, UINT nFrom)
{
    ASSERT(m_hFile != (UINT)hFileNull);
    ASSERT(nFrom == begin || nFrom == end || nFrom == current);
    ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

    return ::SetFilePointer((HANDLE)m_hFile, lOff, NULL, (DWORD)nFrom);
}

DWORD CFile::GetPosition() const
{
    ASSERT(m_hFile != (UINT)hFileNull);

    return ::SetFilePointer((HANDLE)m_hFile, 0, NULL, FILE_CURRENT);
}

BOOL CFile::Flush()
{
    return ::FlushFileBuffers((HANDLE)m_hFile);
}


BOOL CFile::Close()
{
    ASSERT(m_hFile != (UINT)hFileNull);

    BOOL bSucc = ::CloseHandle((HANDLE)m_hFile);

    m_hFile = (UINT) hFileNull;
    m_bCloseOnDelete = FALSE;
    m_strFileName[0] = _T('\0');

    return bSucc;
}



void CFile::Abort()
{
    if (m_hFile != (UINT)hFileNull)
    {
        // close but ignore errors
        ::CloseHandle((HANDLE)m_hFile);
        m_hFile = (UINT)hFileNull;
    }

    m_strFileName[0] = _T('\0');
}

BOOL CFile::LockRange(DWORD dwPos, DWORD dwCount)
{
    ASSERT(m_hFile != (UINT)hFileNull);

    return ::LockFile((HANDLE)m_hFile, dwPos, 0, dwCount, 0);
}

BOOL CFile::UnlockRange(DWORD dwPos, DWORD dwCount)
{
    ASSERT(m_hFile != (UINT)hFileNull);

    return ::UnlockFile((HANDLE)m_hFile, dwPos, 0, dwCount, 0);
}


BOOL PASCAL CFile::Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
    return ::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName);
}

BOOL PASCAL CFile::Remove(LPCTSTR lpszFileName)
{
    return ::DeleteFile((LPTSTR)lpszFileName);
}

UINT CFile::SetLength(DWORD dwNewLen)
{
    ASSERT(m_hFile != (UINT)hFileNull);

    UINT len = Seek((LONG)dwNewLen, (UINT)begin);
    ::SetEndOfFile((HANDLE)m_hFile);

    return len;
}


DWORD CFile::GetLength() const
{
    ASSERT(m_hFile != (UINT)hFileNull);

    DWORD dwLen;
    ::GetFileSize((HANDLE)m_hFile, &dwLen);

    return dwLen;
}

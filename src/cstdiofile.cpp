#include "cfile.h"
#include <assert.h>
#include <io.h>
#include <fcntl.h>

#define ASSERT  assert

////////////////////////////////////////////////////////////////////////////
// CStdioFile implementation

CStdioFile::CStdioFile()
{
    m_pStream = NULL;
}

CStdioFile::CStdioFile(FILE* pOpenStream) : CFile(hFileNull)
{
    m_pStream = pOpenStream;
    m_hFile = (UINT)_get_osfhandle(_fileno(pOpenStream));
    ASSERT(!m_bCloseOnDelete);
}

CStdioFile::~CStdioFile()
{
    if (m_pStream != NULL && m_bCloseOnDelete)
        Close();
}


BOOL CStdioFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT(lpszFileName != NULL);

    m_pStream = NULL;
    if (!CFile::Open(lpszFileName, (nOpenFlags & ~typeText)))
        return FALSE;

    ASSERT(m_hFile != hFileNull);
    ASSERT(m_bCloseOnDelete);

    char szMode[4]; // C-runtime open string
    int nMode = 0;

    // determine read/write mode depending on CFile mode
    if (nOpenFlags & modeCreate)
    {
        if (nOpenFlags & modeNoTruncate)
            szMode[nMode++] = 'a';
        else
            szMode[nMode++] = 'w';
    }
    else if (nOpenFlags & modeWrite)
        szMode[nMode++] = 'a';
    else
        szMode[nMode++] = 'r';

    // add '+' if necessary (when read/write modes mismatched)
    if (szMode[0] == 'r' && (nOpenFlags & modeReadWrite) ||
        szMode[0] != 'r' && !(nOpenFlags & modeWrite))
    {
        // current szMode mismatched, need to add '+' to fix
        szMode[nMode++] = '+';
    }

    // will be inverted if not necessary
    int nFlags = _O_RDONLY|_O_TEXT;
    if (nOpenFlags & (modeWrite|modeReadWrite))
        nFlags ^= _O_RDONLY;

    if (nOpenFlags & typeBinary)
        szMode[nMode++] = 'b', nFlags ^= _O_TEXT;
    else
        szMode[nMode++] = 't';
    szMode[nMode++] = '\0';

    // open a C-runtime low-level file handle
    int nHandle = _open_osfhandle(m_hFile, nFlags);

    // open a C-runtime stream from that handle
    if (nHandle != -1)
        m_pStream = _fdopen(nHandle, szMode);

    if (m_pStream == NULL)
    {
        CFile::Abort(); // close m_hFile
        return FALSE;
    }

    return TRUE;
}

UINT CStdioFile::Read(void* lpBuf, UINT nCount)
{
    ASSERT(m_pStream != NULL);

    if (nCount == 0)
        return 0;   // avoid Win32 "null-read"

    return fread(lpBuf, sizeof(BYTE), nCount, m_pStream);
}

BOOL CStdioFile::Write(const void* lpBuf, UINT nCount)
{
    ASSERT(m_pStream != NULL);

    return (fwrite(lpBuf, sizeof(BYTE), nCount, m_pStream) == nCount);
}

int CStdioFile::WriteString(LPCTSTR lpsz)
{
    ASSERT(lpsz != NULL);
    ASSERT(m_pStream != NULL);

    return _fputts(lpsz, m_pStream);
}

LPTSTR CStdioFile::ReadString(LPTSTR lpsz, UINT nMax)
{
    ASSERT(lpsz != NULL);
    ASSERT(m_pStream != NULL);

    return _fgetts(lpsz, nMax, m_pStream);
}


LONG CStdioFile::Seek(LONG lOff, UINT nFrom)
{
    ASSERT(nFrom == begin || nFrom == end || nFrom == current);
    ASSERT(m_pStream != NULL);

    if (fseek(m_pStream, lOff, nFrom) != 0)
        return -1;

    return ftell(m_pStream);
}

DWORD CStdioFile::GetPosition() const
{
    ASSERT(m_pStream != NULL);

    return ftell(m_pStream);
}

BOOL CStdioFile::Flush()
{
    return fflush(m_pStream) != EOF;
}

BOOL CStdioFile::Close()
{
    int nErr = 0;

    if (m_pStream != NULL)
        nErr = fclose(m_pStream);

    m_hFile = (UINT) hFileNull;
    m_bCloseOnDelete = FALSE;
    m_pStream = NULL;

    return !nErr;
}

void CStdioFile::Abort()
{
    if (m_pStream != NULL && m_bCloseOnDelete)
        fclose(m_pStream);  // close but ignore errors
    m_hFile = (UINT) hFileNull;
    m_pStream = NULL;
    m_bCloseOnDelete = FALSE;
}
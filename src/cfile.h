#include <tchar.h>
#include <windows.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
// File - raw unbuffered disk file I/O

class CFile
{
public:
// Flag values
    enum OpenFlags {
        modeRead =          0x0000,
        modeWrite =         0x0001,
        modeReadWrite =     0x0002,
        shareCompat =       0x0000,
        shareExclusive =    0x0010,
        shareDenyWrite =    0x0020,
        shareDenyRead =     0x0030,
        shareDenyNone =     0x0040,
        modeNoInherit =     0x0080,
        modeCreate =        0x1000,
        modeNoTruncate =    0x2000,
        typeText =          0x4000, // typeText and typeBinary are used in
        typeBinary =   (int)0x8000 // derived classes only
        };

    enum Attribute {
        normal =    0x00,
        readOnly =  0x01,
        hidden =    0x02,
        system =    0x04,
        volume =    0x08,
        directory = 0x10,
        archive =   0x20
        };

    enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

    enum { hFileNull = -1 };

// Constructors
    CFile();
    CFile(int hFile);

// Attributes
    UINT m_hFile;
    operator HFILE() const;

    virtual DWORD GetPosition() const;

// Operations
    virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);

    static BOOL PASCAL Rename(LPCTSTR lpszOldName,
                LPCTSTR lpszNewName);
    static BOOL PASCAL Remove(LPCTSTR lpszFileName);

    DWORD SeekToEnd();
    void SeekToBegin();


// Overridables
    virtual LONG Seek(LONG lOff, UINT nFrom);
    virtual UINT SetLength(DWORD dwNewLen);
    virtual DWORD GetLength() const;

    virtual UINT Read(void* lpBuf, UINT nCount);
    virtual BOOL Write(const void* lpBuf, UINT nCount);

    virtual BOOL LockRange(DWORD dwPos, DWORD dwCount);
    virtual BOOL UnlockRange(DWORD dwPos, DWORD dwCount);

    virtual void Abort();
    virtual BOOL Flush();
    virtual BOOL Close();

// Implementation
public:
    virtual ~CFile();

protected:
    BOOL m_bCloseOnDelete;
    TCHAR m_strFileName[_MAX_PATH];
};


// CFile
inline CFile::operator HFILE() const
    { return m_hFile; }
inline DWORD CFile::SeekToEnd()
    { return Seek(0, CFile::end); }
inline void CFile::SeekToBegin()
    { Seek(0, CFile::begin); }



/////////////////////////////////////////////////////////////////////////////
// STDIO file implementation

class CStdioFile : public CFile
{
public:
// Constructors
    CStdioFile();
    CStdioFile(FILE* pOpenStream);

// Attributes
    FILE* m_pStream;    // stdio FILE
                        // m_hFile from base class is _fileno(m_pStream)

// Operations
    // reading and writing strings
    virtual int WriteString(LPCTSTR lpsz);
    virtual LPTSTR ReadString(LPTSTR lpsz, UINT nMax);

// Implementation
public:
    virtual ~CStdioFile();
    virtual DWORD GetPosition() const;
    virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
    virtual UINT Read(void* lpBuf, UINT nCount);
    virtual BOOL Write(const void* lpBuf, UINT nCount);
    virtual LONG Seek(LONG lOff, UINT nFrom);
    virtual void Abort();
    virtual BOOL Flush();
    virtual BOOL Close();
};
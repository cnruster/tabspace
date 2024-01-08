// CFile header adapted from MFC, Microsoft
// by Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University
// https://www.researchgate.net/profile/Yiping-Cheng/research

#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <assert.h>

#ifndef AFX_INLINE
#define AFX_INLINE inline /*__forceinline*/
#endif

#define ASSERT    assert


/////////////////////////////////////////////////////////////////////////////
// CFile - raw unbuffered disk file I/O

namespace CFile
{
	// Flag values
	enum OpenFlags {
		modeRead = (int)0x00000,
		modeWrite = (int)0x00001,
		modeReadWrite = (int)0x00002,
		shareCompat = (int)0x00000,
		shareExclusive = (int)0x00010,
		shareDenyWrite = (int)0x00020,
		shareDenyRead = (int)0x00030,
		shareDenyNone = (int)0x00040,
		modeNoInherit = (int)0x00080,
#ifdef _UNICODE
		typeUnicode = (int)0x00400, // used in derived classes only
#endif
		modeCreate = (int)0x01000,
		modeNoTruncate = (int)0x02000,
		typeText = (int)0x04000, // used in derived classes only
		typeBinary = (int)0x08000, // used in derived classes only
		osNoBuffer = (int)0x10000,
		osWriteThrough = (int)0x20000,
		osRandomAccess = (int)0x40000,
		osSequentialScan = (int)0x80000,
	};

	enum Attribute {
		normal = 0x00,                // note: not same as FILE_ATTRIBUTE_NORMAL
		readOnly = FILE_ATTRIBUTE_READONLY,
		hidden = FILE_ATTRIBUTE_HIDDEN,
		system = FILE_ATTRIBUTE_SYSTEM,
		volume = 0x08,
		directory = FILE_ATTRIBUTE_DIRECTORY,
		archive = FILE_ATTRIBUTE_ARCHIVE,
		device = FILE_ATTRIBUTE_DEVICE,
		temporary = FILE_ATTRIBUTE_TEMPORARY,
		sparse = FILE_ATTRIBUTE_SPARSE_FILE,
		reparsePt = FILE_ATTRIBUTE_REPARSE_POINT,
		compressed = FILE_ATTRIBUTE_COMPRESSED,
		offline = FILE_ATTRIBUTE_OFFLINE,
		notIndexed = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
		encrypted = FILE_ATTRIBUTE_ENCRYPTED
	};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	HANDLE Open(LPCTSTR lpszFileName, UINT nOpenFlags);

	// Operations
	ULONG Seek(HANDLE hFile, LONG lOff, UINT nFrom);
	ULONG SeekToEnd(HANDLE hFile);
	void SeekToBegin(HANDLE hFile);
	LONGLONG Seek(HANDLE hFile, LONGLONG lOff, UINT nFrom);
	LONG GetPosition(HANDLE hFile);
	LONGLONG GetLength(HANDLE hFile);
	BOOL SetLength(HANDLE hFile, ULONGLONG dwNewLen);

	BOOL Read(HANDLE hFile, void* lpBuf, UINT nCount, UINT& nRead);
	BOOL Write(HANDLE hFile, const void* lpBuf, UINT nCount);
	BOOL Flush(HANDLE hFile);
	BOOL Close(HANDLE hFile);

	BOOL LockRange(HANDLE hFile, ULONGLONG dwPos, ULONGLONG dwCount);
	BOOL UnlockRange(HANDLE hFile, ULONGLONG dwPos, ULONGLONG dwCount);
};


AFX_INLINE ULONG CFile::Seek(HANDLE hFile, LONG lOff, UINT nFrom)
{
	ASSERT(hFile != INVALID_HANDLE_VALUE);
	ASSERT(nFrom == begin || nFrom == end || nFrom == current);
	ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

	return ::SetFilePointer(hFile, lOff, NULL, (UINT)nFrom);
}

AFX_INLINE ULONG CFile::SeekToEnd(HANDLE hFile)
{
	return Seek(hFile, (LONG)0, end);
}

AFX_INLINE void CFile::SeekToBegin(HANDLE hFile)
{
	Seek(hFile, (LONG)0, begin);
}

AFX_INLINE LONG CFile::GetPosition(HANDLE hFile)
{
	ASSERT(hFile != INVALID_HANDLE_VALUE);
	return ::SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}

AFX_INLINE BOOL CFile::Read(HANDLE hFile, void* lpBuf, UINT nCount, UINT& nRead)
{
	ASSERT(hFile != INVALID_HANDLE_VALUE);
	ASSERT(lpBuf != NULL);
	ASSERT(nCount != 0);

	return ::ReadFile(hFile, lpBuf, nCount, (DWORD *)&nRead, NULL);
}

AFX_INLINE BOOL CFile::Write(HANDLE hFile, const void* lpBuf, UINT nCount)
{
	ASSERT(hFile != INVALID_HANDLE_VALUE);
	ASSERT(lpBuf != NULL);
	ASSERT(nCount != 0);

	DWORD nWritten;
	return ::WriteFile(hFile, lpBuf, nCount, &nWritten, NULL)
		&& (nWritten == nCount);
}

AFX_INLINE BOOL CFile::Flush(HANDLE hFile)
{
	ASSERT(hFile != INVALID_HANDLE_VALUE);
	return ::FlushFileBuffers(hFile);
}

AFX_INLINE BOOL CFile::Close(HANDLE hFile)
{
	ASSERT(hFile != INVALID_HANDLE_VALUE);
	return ::CloseHandle(hFile);
}
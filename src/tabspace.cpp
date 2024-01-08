// Windows utility to batch convert program files to tab-space rule compliant
//
// Copyright (c)2023 Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University. All rights reserved.
// https://www.researchgate.net/profile/Yiping-Cheng/research
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <shlwapi.h>
#include <minwinbase.h>
#include "cfile.h"
#include "fabsr.h"
#include "fabsw.h"


extern "C" {
	TCHAR* strcpy_x(TCHAR* strdst, const TCHAR* strdst_end, const TCHAR* strsrc);
}

// State values
enum State {
	stInitial = (int)0x00000,
	stRightSeg = (int)0x00001,
	stExpectLF = (int)0x00002
};


#define TAB             '\t'
#define SPACE           ' '
#define CR              '\r'
#define LF              '\n'

#define PUT_BYTE_MAIN(b) \
	do { \
		if (!fabsw.PutByte(b)) { \
WRITE_FAIL: \
			_tprintf(_T("%s : conversion failed " \
				"when writing TSC file\n"), filename); \
			goto FAIL; \
		} \
	} while (0)

#define PUT_BYTE(b) \
	do { \
		if (!fabsw.PutByte(b)) { \
			goto WRITE_FAIL; \
		} \
	} while (0)

#define PUT_BYTES(b, r) \
	do { \
		if (!fabsw.PutBytes(b, r)) { \
			goto WRITE_FAIL; \
		} \
	} while (0)


static TCHAR bakfilename[MAX_PATH], tscfilename[MAX_PATH];
static FABSR fabsr;
static FABSW fabsw;

void TSConvert(LPCTSTR filename)
{
	LPCTSTR suffix_bak = _T("bak");
	LPCTSTR suffix_tsc = _T("tsc"); // tsc = tab-space converted
	LPCTSTR p, strend;
	HANDLE orgfile, tscfile;
	BOOL changed;
	enum State state;
	UINT ccws;          // count of consecutive white spaces
	BOOL resgb;         // result of fabsr.GetByte()
	BYTE byte;


	for (p = filename; *p; p++) {
		if (_T('.') == *p) {
			if (_tcsstr(p + 1, suffix_bak)) {
				_tprintf(_T("%s : not processed by Tabspace as its extension contains %s\n"),
					filename, suffix_bak);
				return;
			}
			goto MAKE_NAMES;
		}
	}

	// needs special handling when there is no "." in the file name
	suffix_bak = _T(".bak");
	suffix_tsc = _T(".tsc");

MAKE_NAMES:
	// the backup file name is the orginal file name plus "bak" or ".bak"
	strend = &bakfilename[MAX_PATH-1];
	strcpy_x(strcpy_x(bakfilename, strend, filename), strend, suffix_bak);

	// the tsc file name is the orginal file name plus "tsc" or ".tsc"
	strend = &tscfilename[MAX_PATH-1];
	strcpy_x(strcpy_x(tscfilename, strend, filename), strend, suffix_tsc);

	// now create the tsc file, it must not name-clash with existing files
	// we do not think of a different name to avoid name clash
	// because that is too rare
	if (PathFileExists(tscfilename) ||
		!(tscfile = CFile::Open(tscfilename,
			CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))) {
		_tprintf(_T("%s : conversion failed when creating TSC file\n"),
			filename);
		return;
	}

	// open the original file
	if (!(orgfile = CFile::Open(filename, CFile::modeRead | CFile::typeBinary))) {
		_tprintf(_T("%s : conversion failed when opening it\n"), filename);

ABORT_TSC:
		CFile::Close(tscfile);
		DeleteFile(tscfilename);
		return;
	}

	// now the actual conversion begins
	fabsr.Bind(orgfile);
	fabsw.Bind(tscfile);

	changed = FALSE;
	state = stInitial;
	ccws = 0;

	for (;;) {
		if ((resgb = fabsr.GetByte(byte)) < 0) {
			_tprintf(_T("%s : conversion failed when reading it\n"),
				filename);
FAIL:
			CFile::Close(orgfile);
			goto ABORT_TSC;
		}

		if (resgb == 0) { // end of file encountered
			if (state != stExpectLF) {
				if (ccws) {
					changed = TRUE; // because the trailing spaces are discarded
				}
			}
			else {
				PUT_BYTE_MAIN(LF);
			}

			break;
		}

		if (byte == CR) {
			state = stExpectLF;
			changed = TRUE; // since we adopt the UNIX standard for line breaking
			// a single LF is used. So CR will be discarded.
			// later, there is no need to set changed to TRUE because
			// it is already set here
		}
		else if (byte == LF) {
			PUT_BYTE(LF);
			if (ccws) {
				changed = TRUE;
			}
			state = stInitial;
			ccws = 0;
		}
		else if (byte == TAB) {
			if (state == stInitial) {
				ccws += 4; // 1 tab = 4 spaces, but not put them until suitable time
			}
			else if (state == stRightSeg) {
				// stRightSeg indicates after the first non-space non-tab character
				ccws += 4;
				changed = TRUE;
			}
			else {
				PUT_BYTE(LF);
				state = stInitial;
				ccws = 4; // these spaces belong to the new line
			}
		}
		else if (isspace(byte)) {
			if (state == stInitial) {
				ccws++;
				changed = TRUE;
			}
			else if (state == stRightSeg) {
				ccws++;
				if (byte != SPACE) {
					changed = TRUE;
				}
			}
			else {
				PUT_BYTE(LF);
				state = stInitial;
				ccws = 1; // this space belongs to the new line
			}
		}
		else {
			if (state == stInitial) {
				// so that indent of 2 spaces will get a tab
				PUT_BYTES(TAB, (ccws+2)/4);
				state = stRightSeg;
			}
			else if (state == stRightSeg) {
				PUT_BYTES(SPACE, ccws);
			}
			else {
				PUT_BYTE(LF);
				state = stInitial;
			}
			PUT_BYTE(byte);
			ccws = 0;
		}
	}

	CFile::Close(orgfile);

	if (!changed) {
		// the tsc file is the same as the original file, so delete it
		_tprintf(_T("%s : already tab-space compliant so left unchanged\n"),
			filename);
		goto ABORT_TSC;
	}

	if (!fabsw.EndPut()) {  // write the remaining bytes in the fabsw buffer
		_tprintf(_T("%s : conversion failed when saving TSC file\n"),
				filename);
		goto ABORT_TSC;
	}

	if (!CFile::Close(tscfile)) {
		_tprintf(_T("%s : conversion failed when closing TSC file\n"),
			filename);

		DeleteFile(tscfilename);
		return;
	}

	// if conversion successful, rename the original file to bakfilename
	// and rename the tsc file to original filename

	if (!MoveFile(filename, bakfilename)) {
		_tprintf(_T("%s : still the original file due to renaming failure,"
			" but its TSC file is %s\n"),
			filename, tscfilename);
		return;
	}

	if (!MoveFile(tscfilename, filename)) {
		_tprintf(_T("%s : now renamed as %s, whose TSC file is %s\n"),
			filename, bakfilename, tscfilename);
		return;
	}

	_tprintf(_T("%s : tab-space conversion successful\n"), filename);
}
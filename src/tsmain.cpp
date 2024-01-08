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

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>


void TSConvertFiles(LPCTSTR pattern, int level);
void TSConvert(LPCTSTR filename);


int __cdecl _tmain(int argc, TCHAR** argv)
{
	_tprintf(_T("Tabspace Code Beautifier 1.0\n"
		"Compiled on "__DATE__"\n"
		"Copyright (c)2023-2024 Yiping Cheng\n"
		"Beijing Jiaotong University, China. Email:ypcheng@bjtu.edu.cn\n"
		"https://github.com/cnruster\n"));

	if (argc < 2) {
		_tprintf(_T("\nUsage: tabspace <pattern_1> <pattern_2> ... <pattern_n>\n"
			"For example: tabspace *.c *.cpp *.h\n"));
	}
	else {
		// each argument is a pattern
		for (int i = 1; i < argc; i++) {
			TSConvertFiles(argv[i], 0);
		}
	}
}


#define MAX_RECURSION_DEPTH     24

// perform tab-space conversion to files matching the pattern
void TSConvertFiles(LPCTSTR pattern, int level)
{
	if (!_tcscmp(pattern, _T("*")) || _tcsstr(pattern, _T(".*"))) {
		_tprintf(_T("\nTabspace refuses to do this pattern: %s\n"), pattern);
		return;
	}

	{
		// display the pattern and current directory
		// this is enclosed by {} to save stack space
		// after all, this is a recursive function
		TCHAR curr_dir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, curr_dir);
		_tprintf(_T("\nTabspace beautifying %s files in %s\n"),
			pattern, curr_dir);
	}

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	// find the first file
	if (INVALID_HANDLE_VALUE == (hFind = FindFirstFile(pattern, &FindFileData))) {
		if (ERROR_FILE_NOT_FOUND == GetLastError()) {
			_tprintf(_T("No file in this directory matches %s\n"),
				pattern);
		}
		else {
			_tprintf(_T("FindFirstFile for %s failed\n"), pattern);
		}

		goto SUB_DIRS;
	}

	do {
		if (GetFileAttributes(FindFileData.cFileName)
			& FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// now this is a file, not a directory, so do conversion
		TSConvert(FindFileData.cFileName);

	} while (FindNextFile(hFind, &FindFileData));

SUB_DIRS:
	if (MAX_RECURSION_DEPTH == ++level) {
		_tprintf(_T("Maximum recursion depth %d is reached. "
			"Tabspace will not process subdirectories of this directory.\n"),
			MAX_RECURSION_DEPTH);
		return;
	}

	if (INVALID_HANDLE_VALUE == (hFind = FindFirstFileEx(_T("*"),
		FindExInfoStandard,
		&FindFileData,
		FindExSearchLimitToDirectories,
		NULL,
		0))) {
		return;
	}

	do {
		assert(_tcscmp(FindFileData.cFileName, _T("")));

		// do tab-space conversion to subdirectories
		if (_tcscmp(FindFileData.cFileName, _T(".")) &&
			_tcscmp(FindFileData.cFileName, _T("..")) &&
			!_tcsstr(FindFileData.cFileName, _T("\\")) &&
			!_tcsstr(FindFileData.cFileName, _T("/")) &&
			SetCurrentDirectory(FindFileData.cFileName)) {
			TSConvertFiles(pattern, level);
			SetCurrentDirectory(_T(".."));
		}
	} while (FindNextFile(hFind, &FindFileData));
}
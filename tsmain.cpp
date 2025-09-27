// Windows utility to batch convert program files to tab-space rule compliant
//
// Copyright (c)2023 Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University. All rights reserved.
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

UINT str_chr_count(LPCTSTR str, TCHAR chr) noexcept;
BOOL CheckPattern(LPCTSTR pattern) noexcept;
void ConvertFiles(LPCTSTR pattern, int level) noexcept;
void Convert(LPCTSTR filename) noexcept;

static const TCHAR nameTS[10] = _T("tab-space");
static const TCHAR nameAAS[18] = _T("aligned all-space");
BOOL useAAS = FALSE;
const TCHAR* rule_name = nameTS;

int __cdecl _tmain(int argc, TCHAR** argv)
{

    _tprintf(_T("Tabspace Code Beautifier 1.2\n"
                "Compiled on "__DATE__"\n"
                "Copyright (c)2023-2024 Yiping Cheng\n"
                "Beijing Jiaotong University, China. Email:ypcheng@bjtu.edu.cn\n"
                "https://github.com/cnruster\n"));

    if (argc >= 2 && !_tcscmp(argv[1], _T("/s"))) {
        ++argv;
        --argc;
        // use aligned all-space rule
        useAAS = TRUE;
        rule_name = nameAAS;
    }

    if (argc < 2) {
        _tprintf(_T("\nUsage: tabspace <pattern_1> <pattern_2> ... <pattern_n>\n"
                    "For example: tabspace *.c *.cpp *.h\n"
                    "Or to use aligned all-space rule: "
                    "tabspace /s <pattern_1> <pattern_2> ... <pattern_n>\n"));
        return -1;
    }

    TCHAR curr_dir[MAX_PATH];
    DWORD dir_len = GetCurrentDirectory(MAX_PATH, curr_dir);
    if (dir_len == 0 || dir_len >= MAX_PATH) {
        _tprintf(_T("\nGetCurrentDirectory failed\n"));
        return -2;
    }
    if (1 >= str_chr_count(curr_dir, _T('\\'))) {
        _tprintf(_T("\nTabspace refuses to work in a root"
                    " or first-level directory : %s\n"),
            curr_dir);
        return -3;
    }

    for (int i = 1; i < argc; i++) {
        if (CheckPattern(argv[i])) {
            _tprintf(_T("\nTabspace (using %s rule) beautifying %s files in %s\n"),
                rule_name, argv[i], curr_dir);
            ConvertFiles(argv[i], 0);
        }
    }
}

UINT str_chr_count(LPCTSTR str, TCHAR chr) noexcept
{
    UINT cnt = 0;
    for (LPCTSTR p = str; *p; p++) {
        if (*p == chr) {
            cnt++;
        }
    }

    return cnt;
}

BOOL CheckPattern(LPCTSTR pattern) noexcept
{
    if (_tcschr(pattern, _T('/')) || _tcschr(pattern, _T('\\'))) {
        _tprintf(_T("\nIllegal pattern %s, as it contains / or \\\n"), pattern);
        return FALSE;
    }

    if (!_tcscmp(pattern, _T("*")) || _tcsstr(pattern, _T(".*"))) {
        _tprintf(_T("\nTabspace refuses to do this pattern: %s\n"), pattern);
        return FALSE;
    }

    return TRUE;
}


// perform tabspace conversion to files matching the pattern
void ConvertFiles(LPCTSTR pattern, int level) noexcept
{
    constexpr int MAX_RECURSION_DEPTH = 24;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // find the first file
    if (INVALID_HANDLE_VALUE == (hFind = FindFirstFile(pattern, &FindFileData))) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            _tprintf(_T("FindFirstFile for %s failed\n"), pattern);
        }
    } else {
        do {
            if (GetFileAttributes(FindFileData.cFileName)
                & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }

            // now this is a file, not a directory, so do conversion
            Convert(FindFileData.cFileName);

        } while (FindNextFile(hFind, &FindFileData));
    }

    if (MAX_RECURSION_DEPTH == ++level) {
        _tprintf(_T("Maximum recursion depth %d is reached. "
                    "Tabspace will not process subdirectories of this directory.\n"),
            MAX_RECURSION_DEPTH);
        return;
    }

    if (INVALID_HANDLE_VALUE ==
        (hFind = FindFirstFileEx(_T("*"), FindExInfoStandard, &FindFileData, FindExSearchLimitToDirectories, NULL, 0))) {
        return;
    }

    do {
        assert(_tcscmp(FindFileData.cFileName, _T("")));

        // do conversion for subdirectories
        if (_tcscmp(FindFileData.cFileName, _T("."))
            && _tcscmp(FindFileData.cFileName, _T(".."))
            && SetCurrentDirectory(FindFileData.cFileName)) {
            ConvertFiles(pattern, level);
            SetCurrentDirectory(_T(".."));
        }
    } while (FindNextFile(hFind, &FindFileData));
}
// Windows utility to batch convert program files to tab-space rule compliant
//
// Copyright (c)2023 Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University. All rights reserved.
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

extern const TCHAR* rule_name;
extern BOOL useAAS;
extern FILE* orgfile;
extern FILE* tscfile;

// State values
enum State {
    stInitial = (int)0x00000,
    stRightSeg = (int)0x00001,
    stExpectLF = (int)0x00002
};

#define TAB '\t'
#define SPACE ' '
#define CR '\r'
#define LF '\n'

#define PUT_BYTE_MAIN(b)                            \
    do {                                            \
        if (fputc(b, tscfile) == EOF) {             \
        WRITE_FAIL:                                 \
            _tprintf(_T("%s : conversion failed "   \
                        "when writing TSC file\n"), \
                filename);                          \
            goto FAIL;                              \
        }                                           \
    } while (0)

#define PUT_BYTE(b)                     \
    do {                                \
        if (fputc(b, tscfile) == EOF) { \
            goto WRITE_FAIL;            \
        }                               \
    } while (0)

#define PUT_BYTES(b, r)                     \
    do {                                    \
        for (UINT i = 0; i < r; i++) {      \
            if (fputc(b, tscfile) == EOF) { \
                goto WRITE_FAIL;            \
            }                               \
        }                                   \
    } while (0)

inline BOOL IsValidFilePath(LPCTSTR filePath) noexcept
{
    DWORD fa = GetFileAttributes(filePath);
    // Check if the path exists and is a file (not a directory)
    return (fa != INVALID_FILE_ATTRIBUTES && !(fa & FILE_ATTRIBUTE_DIRECTORY));
}

inline BOOL IsSuffix(const TCHAR* strWhole, const TCHAR* strSub) noexcept
{
    assert(strWhole && strSub);

    size_t len1 = _tcslen(strWhole);
    size_t len2 = _tcslen(strSub);

    // If strSub is longer than strWhole, it cannot be a suffix
    if (len2 > len1) {
        return FALSE;
    }

    // Compare the last len2 characters of strWhole with strSub
    return _tcscmp(strWhole + len1 - len2, strSub) == 0;
}

void Convert(LPCTSTR filename) noexcept
{
    static const TCHAR suffix_bak[5] = _T(".bak");
    static const TCHAR suffix_tsc[5] = _T(".tsc"); // tsc = tab-space converted
    TCHAR bakfilename[MAX_PATH], tscfilename[MAX_PATH];
    BOOL changed;
    enum State state;
    UINT ccws; // count of consecutive white spaces
    int ch;
    BYTE byte;

    if (IsSuffix(filename, suffix_bak)) {
        _tprintf(_T("%s : not processed by Tabspace as its extension is %s\n"),
            filename, suffix_bak);
        return;
    }

    // the backup file name is the orginal file name plus ".bak"
    if (_stprintf_s(bakfilename, MAX_PATH, _T("%s%s"), filename, suffix_bak) < 0) {
        _tprintf(_T("%s : failed to make backup file name\n"), filename);
        return;
    }

    // the tsc file name is the orginal file name plus ".tsc"
    if (_stprintf_s(tscfilename, MAX_PATH, _T("%s%s"), filename, suffix_tsc) < 0) {
        _tprintf(_T("%s : failed to make tsc file name\n"), filename);
        return;
    }

    // now create the tsc file, it must not name-clash with existing files
    // we do not think of a different name to avoid name clash
    // because that is too rare
    if (IsValidFilePath(tscfilename) || (tscfile=_tfopen(tscfilename, _T("wb")))==nullptr) {
        _tprintf(_T("%s : conversion failed when creating TSC file\n"), filename);
        return;
    }

    // open the original file
    if ((orgfile = _tfopen(filename, _T("rb"))) == nullptr) {
        _tprintf(_T("%s : conversion failed when opening original file\n"), filename);
    ABORT_TSC:
        fclose(tscfile);
        DeleteFile(tscfilename);
        return;
    }

    // now the actual conversion begins
    changed = FALSE;
    state = stInitial;
    ccws = 0;

    for (;;) {
        if ((ch = fgetc(orgfile)) == EOF) {
            if (ferror(orgfile)) {
                _tprintf(_T("%s : conversion failed when reading original file\n"), filename);
            FAIL:
                fclose(orgfile);
                goto ABORT_TSC;
            }
            // end of file encountered
            if (state != stExpectLF) {
                if (ccws) {
                    changed = TRUE; // because the trailing spaces are discarded
                }
            } else {
                PUT_BYTE_MAIN(LF);
            }

            break;
        }

        byte = (BYTE)ch;
        if (byte == CR) {
            state = stExpectLF;
            changed = TRUE; // since we adopt the UNIX standard for line breaking
            // a single LF is used. So CR will be discarded.
            // later, we will not need to set changed to TRUE because
            // it is already set here
        } else if (byte == LF) {
            PUT_BYTE(LF);
            state = stInitial;
            if (ccws) {
                changed = TRUE;
                ccws = 0;
            }
        } else if (byte == TAB) {
            if (state == stInitial) {
                ccws += 4; // 1 tab = 4 spaces, but not put them until suitable time
                if (useAAS) {
                    changed = TRUE; // because it will be changed to spaces
                }
            } else if (state == stRightSeg) {
                // stRightSeg means after the first non-space non-tab character
                ccws += 4;
                changed = TRUE;
            } else {
                PUT_BYTE(LF);
                state = stInitial;
                ccws = 4; // these spaces belong to the new line
            }
        } else if (isspace(byte)) {
            if (byte != SPACE) {
                changed = TRUE; // because it will be changed to SPACE or TAB
            }
            if (state == stInitial) {
                ccws++;
                if (!useAAS) {
                    changed = TRUE; // because it will be changed to tab
                }
            } else if (state == stRightSeg) {
                ccws++;
            } else {
                PUT_BYTE(LF);
                state = stInitial;
                ccws = 1; // this space belongs to the new line
            }
        } else {
            if (state == stInitial) {
                if (useAAS) {
                    if (ccws % 4) {
                        // ccws not a multiple of 4 will be changed
                        changed = TRUE;
                        // so that indent of 2 spaces will become 4 spaces
                        ccws = ((ccws + 2) / 4) * 4;
                    }
                    PUT_BYTES(SPACE, ccws);
                } else {
                    // No need to set changed to TRUE here, because if ccws is
                    // not a multiple of 4, the original file must contain a
                    // white space in a left seg, and changed is already set to
                    // TRUE when processing that white space.
                    PUT_BYTES(TAB, (ccws + 2) / 4);
                    // so that indent of 2 spaces will get a tab
                }
                state = stRightSeg;
            } else if (state == stRightSeg) {
                PUT_BYTES(SPACE, ccws);
            } else {
                PUT_BYTE(LF);
                state = stInitial;
            }
            PUT_BYTE(byte);
            ccws = 0;
        }
    }

    fclose(orgfile);

    if (!changed) {
        // the tsc file is the same as the original file, so delete it
        _tprintf(_T("%s : already %s compliant so left unchanged\n"),
            filename, rule_name);
        goto ABORT_TSC;
    }

    if (fclose(tscfile)) { // write the remaining bytes in the tscfile buffer
        _tprintf(_T("%s : conversion failed when closing TSC file\n"), filename);
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

    _tprintf(_T("%s : %s conversion successful\n"), filename, rule_name);
}
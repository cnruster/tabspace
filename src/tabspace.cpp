#include "cfile.h"
#include <stdio.h>
#include <windows.h>

BOOL TabToSpace(LPCTSTR filename);
void TabToSpaceFiles(LPCTSTR ext);

// ��C++�е���C����д�ĺ�����������C�����ﶨ���ȫ�ֱ��������������
extern "C" {
    TCHAR* strcpy_x(TCHAR* strdst, const TCHAR* strdst_end, const TCHAR* strsrc);
};


int __cdecl _tmain(int argc, TCHAR** argv)
{
    TCHAR curr_dir[MAX_PATH];

    GetCurrentDirectory(MAX_PATH, curr_dir);
    _tprintf(_T("Current directory: %s\n"), curr_dir);

    if (argc < 2) {
        _tprintf(_T("\nConverting tab to 4 spaces in *.c files\n"));
        TabToSpaceFiles(_T("*.c"));

        _tprintf(_T("\nConverting tab to 4 spaces in *.cpp files\n"));
        TabToSpaceFiles(_T("*.cpp"));

        _tprintf(_T("\nConverting tab to 4 spaces in *.h files\n"));
        TabToSpaceFiles(_T("*.h"));
    }
    else {
        for (int i = 1; i < argc; i++) {
            _tprintf(_T("\nConverting tab to 4 spaces in %s files\n"), argv[i]);
            TabToSpaceFiles(argv[i]);
        }
    }
}

// ������һ����չ�����ļ�����tab->spaceת��
void TabToSpaceFiles(LPCTSTR ext)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // �ҵ���һ���ļ�
    hFind = FindFirstFile(ext, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        _tprintf(_T("\nFindFirstFile for %s failed. "
        "Most likely there is no such file there.\n"), ext);
        return;
    }

    _tprintf(_T("\nFindFirstFile for %s successful.\n"), ext);

    do {
        // ���ҵ����ļ�tab->space
        if (TabToSpace(FindFileData.cFileName)) {
            _tprintf(_T("Tab-space conversion for %s successful.\n"), 
                FindFileData.cFileName);
        }
        else {
            _tprintf(_T("Tab-space conversion for %s failed.\n"),
                FindFileData.cFileName);
        }
    } while (FindNextFile(hFind, &FindFileData));
}



#define READBUFSIZE     32
#define TAB             '\t'
#define SPACE           ' '

BOOL TabToSpace(LPCTSTR filename)
{
    TCHAR bakfilename[MAX_PATH], newfilename[MAX_PATH];
    LPCTSTR suffix_bak = _T("bak");
    LPCTSTR suffix_new = _T("new");
    LPCTSTR p = filename;
    LPCTSTR strend;

    while (*p) {
        if (_T('.') != *p) {
            p++;
        } else {
            goto MAKE_NAMES;
        }
    }

    // ԭ�ļ�����û��"."
    suffix_bak = _T(".bak");
    suffix_new = _T(".new");

MAKE_NAMES:
    // ����ת�����ļ��������"bak", �����һ�������ļ���
    strend = &bakfilename[MAX_PATH-1];
    strcpy_x(strcpy_x(bakfilename, strend, filename), strend, suffix_bak);

    // ����ת�����ļ��������"new", �����һ�����ļ���
    strend = &newfilename[MAX_PATH-1];
    strcpy_x(strcpy_x(newfilename, strend, filename), strend, suffix_new);


    CFile orgfile, newfile;
    BYTE readbuf[READBUFSIZE], writebuf[READBUFSIZE*4];
    UINT cbytes, i, k;
    BOOL tab_found = FALSE;

    // �����ǽ�ת������ļ����һ�����ļ���������Ҫ�򿪴����ļ�����д
    if (!newfile.Open(newfilename, CFile::modeWrite | CFile::modeCreate)) {
        _tprintf(_T("Failed creating converted file for %s.\n"), filename);
        goto FAIL1;
    }
    
    // ����ת�����ļ�
    if (!orgfile.Open(filename, CFile::modeRead)) {
        _tprintf(_T("Failed opening %s.\n"), filename);
        goto FAIL2;
    }

    // ��ʼʵ��ִ��ת��
    while (cbytes = orgfile.Read(readbuf, READBUFSIZE)) {
        k = 0;
        for (i = 0; i < cbytes; i++) {
            if (readbuf[i] == TAB) {
                tab_found = TRUE;
                // ����������ַ���tab, ����4���ո�
                writebuf[k++] = SPACE;
                writebuf[k++] = SPACE;
                writebuf[k++] = SPACE;
                writebuf[k++] = SPACE;
            }
            else {
                // ����ԭ�����
                writebuf[k++] = readbuf[i];
            }
        }

        // readbuf������writebufд���ļ�
        if (!newfile.Write(writebuf, k)) {
            // ���д��������������˳�
            orgfile.Close();
            _tprintf(_T("Failed saving converted file for %s.\n"), filename);

FAIL2:
            newfile.Abort();

FAIL1:
            return FALSE;
        }
    }

    orgfile.Close();

    if (!newfile.Close()) {
        _tprintf(_T("Failed closing converted file for %s.\n"), filename);
        return FALSE;
    }

    if (tab_found) {
        // ԭ�ļ��з�������һ��tab
        // ������Ҫ��ԭ�ļ���Ϊ�����ļ��������ļ���Ϊԭ�ļ���
        if (CFile::Rename(filename, bakfilename)) {
            if (CFile::Rename(newfilename, filename)) {
                return TRUE;
            }
            else {
                _tprintf(_T("Original file is renamed as %s. "
                    "Converted file is generated as %s.\n"),
                    bakfilename, newfilename);
                return FALSE;
            }
        }
        else  {
            _tprintf(_T("Original file keeps its name %s. "
                "Converted file is generated as %s.\n"),
                filename, newfilename);
            return FALSE;
        }
    }
    
    // ԭ�ļ���û�з���tab, ������ļ���ԭ�ļ���һģһ���ģ�ɾ�����ļ�
    CFile::Remove(newfilename);
    return TRUE;
}
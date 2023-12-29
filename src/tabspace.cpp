// Windows utility to batch convert tab to 4 spaces in files in a directory
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

#include "cfile.h"
#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>

BOOL TabToSpace(LPCTSTR filename);
void TabToSpaceFiles(LPCTSTR ext);

// 在C++中调用C语言写的函数，或引用C语言里定义的全局变量，必须有这个
extern "C" {
    TCHAR* strcpy_x(TCHAR* strdst, const TCHAR* strdst_end, const TCHAR* strsrc);
}


int __cdecl _tmain(int argc, TCHAR** argv)
{
    // 显示当前目录。用{括起来}是为了节约栈空间使用量
    {
        TCHAR curr_dir[MAX_PATH];

        GetCurrentDirectory(MAX_PATH, curr_dir);
        _tprintf(_T("Current directory: %s\n"), curr_dir);
    }

    if (argc < 2) {
        // 本工具主要服务于C,C++语言
        _tprintf(_T("\nConverting tab to 4 spaces in *.c files\n"));
        TabToSpaceFiles(_T("*.c"));

        _tprintf(_T("\nConverting tab to 4 spaces in *.cpp files\n"));
        TabToSpaceFiles(_T("*.cpp"));

        _tprintf(_T("\nConverting tab to 4 spaces in *.h files\n"));
        TabToSpaceFiles(_T("*.h"));
    }
    else {
        // 每个命令行参数为一个文件通配符
        for (int i = 1; i < argc; i++) {
            _tprintf(_T("\nConverting tab to 4 spaces in %s files\n"), argv[i]);
            TabToSpaceFiles(argv[i]);
        }
    }
}

// 将具有一种扩展名的文件进行tab->space转换
void TabToSpaceFiles(LPCTSTR ext)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // 找到第一个文件
    hFind = FindFirstFile(ext, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        _tprintf(_T("\nFindFirstFile for %s failed. "
        "Most likely there is no such file there.\n"), ext);
        return;
    }

    _tprintf(_T("\nFindFirstFile for %s successful.\n"), ext);

    do {
        // 将找到的文件tab->space
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
    LPCTSTR suffix_new = _T("tsc"); // tsc = tab-space converted
    LPCTSTR p, strend;
    CFile orgfile, newfile;
    BYTE readbuf[READBUFSIZE], writebuf[READBUFSIZE*4];
    UINT cbytes, i, k;
    BOOL tab_found = FALSE;


    for (p = filename; *p; p++) {
        if (_T('.') == *p) {
            goto MAKE_NAMES;
        }
    }

    // 原文件名中没有".", 需特殊处理
    suffix_bak = _T(".bak");
    suffix_new = _T(".tsc");

MAKE_NAMES:
    // 将需转换的文件名后面加"bak", 构造出一个备份文件名
    strend = &bakfilename[MAX_PATH-1];
    strcpy_x(strcpy_x(bakfilename, strend, filename), strend, suffix_bak);

    // 将需转换的文件名后面加"tsc", 构造出一个新文件名
    strend = &newfilename[MAX_PATH-1];
    strcpy_x(strcpy_x(newfilename, strend, filename), strend, suffix_new);

    // 我们是将转换后的文件存成一个新文件，所以需要创建此新文件用于写
    if (PathFileExists(newfilename) || //新文件名不能与现有文件重名
        !newfile.Open(newfilename, CFile::modeWrite | CFile::modeCreate)) {
        _tprintf(_T("Failed creating converted file for %s.\n"), filename);
        goto FAIL1;
    }
    
    // 打开需转换的文件
    if (!orgfile.Open(filename, CFile::modeRead)) {
        _tprintf(_T("Failed opening %s.\n"), filename);
        goto FAIL2;
    }

    // 开始实际执行转换
    while (cbytes = orgfile.Read(readbuf, READBUFSIZE)) {
        k = 0;
        for (i = 0; i < cbytes; i++) {
            if (readbuf[i] == TAB) {
                tab_found = TRUE;
                // 如果读到的字符是tab, 则变成4个空格
                writebuf[k++] = SPACE;
                writebuf[k++] = SPACE;
                writebuf[k++] = SPACE;
                writebuf[k++] = SPACE;
            }
            else {
                // 否则原样输出
                writebuf[k++] = readbuf[i];
            }
        }

        // readbuf读满后将writebuf写入文件
        if (!newfile.Write(writebuf, k)) {
            // 写发生错误，退出
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
        // 原文件中发现至少一个tab
        // 我们需要将原文件改为备份文件名，新文件改为原文件名
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
    
    // 原文件中没有发现tab, 因此新文件和原文件是一模一样的，删除新文件
    CFile::Remove(newfilename);
    return TRUE;
}
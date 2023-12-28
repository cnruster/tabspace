#include <windows.h>
#include "cfile.h"
#include <stdio.h>

BOOL TabToSpace(PSTR filename);
void TabToSpaceFiles(PSTR ext);

// 在C++中调用C语言写的函数，或引用C语言里定义的全局变量，必须有这个
extern "C" {
char* strcpy_x(char *strdst, char *strdst_end, const char *strsrc);
};


int main(int argc, char **argv)
{
    if (argc<2)
    {
        printf("\nConverting tab to 4 spaces in *.c files\n");
        TabToSpaceFiles("*.c");

        printf("\nConverting tab to 4 spaces in *.cpp files\n");
        TabToSpaceFiles("*.cpp");

        printf("\nConverting tab to 4 spaces in *.h files\n");
        TabToSpaceFiles("*.h");
    }
    else
    {
        for (int i=1; i<argc; i++)
        {
            printf("\nConverting tab to 4 spaces in %s files\n", argv[i]);
            TabToSpaceFiles(argv[i]);
        }
    }

    return 0;
}

// 将具有一种扩展名的文件进行tab->space转换
void TabToSpaceFiles(PSTR ext)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // 找到第一个文件
    hFind = FindFirstFile(ext, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        printf("\nFindFirstFile for %s failed (%d).\n", ext, GetLastError());
        return;
    }
        
    printf("\nFindFirstFile for %s successful.\n", ext);

    do
    {
        // 将找到的文件tab->space
        if (TabToSpace(FindFileData.cFileName))
        {
            printf("Tab-space conversion for %s successful.\n", FindFileData.cFileName);
        }
        else
        {
            printf("Tab-space conversion for %s failed.\n", FindFileData.cFileName);
        }
    }
    while (FindNextFile(hFind, &FindFileData)); 
}


BOOL TabToSpace(PSTR filename)
{
    char bakfilename[MAX_PATH], readbuf[32], writebuf[128];
    CStdioFile file, newfile;
    PSTR str, strend;
    UINT cbytes, i, k;

    str=filename;
    while (*str && *str!='.')
        str++;
    if (*str=='.' && strstr(str+1, "bak"))
    {
        printf("Tabspace does not process files with \"bak\" in extension.\n");
        return FALSE;
    }

    // 将需转换的文件名后面加"bak", 构造出一个备份文件名
    strend = &bakfilename[MAX_PATH-1];
    str = strcpy_x(bakfilename, strend, filename);
    strcpy_x(str, strend, "bak");
    // 将原文件改名为备份文件名
    CFile::Remove(bakfilename);
    CFile::Rename(filename, bakfilename);

    // 打开需转换的文件
    if (!file.Open(bakfilename, CFile::modeRead))
    {
        printf("Failed opening %s.\n", filename);
        return FALSE;
    }

    // 我们是将转换过的文件存成一个新文件，所以需要打开此新文件用于写
    if (!newfile.Open(filename, CFile::modeWrite|CFile::modeCreate))
    {
        file.Close();
        printf("Failed opening output file.\n");
        return FALSE;
    }

    // 开始实际执行转换
    while (cbytes = file.Read(readbuf, 32))
    {
        k = 0;
        for (i=0; i<cbytes; i++)
        {
            if (readbuf[i]=='\t')
            {
                // 如果读到的字符是tab, 则变成4个空格
                writebuf[k++]=' ';
                writebuf[k++]=' ';
                writebuf[k++]=' ';
                writebuf[k++]=' ';
            }
            else
            {
                // 否则原样输出
                writebuf[k++]=readbuf[i];
            }
        }

        // readbuf读满后将writebuf写入文件
        if (!newfile.Write(writebuf, k))
        {
            // 如果写发生错误，则错误退出
            newfile.Abort();
            file.Close();
            // 将改名的原文件重新改回来
            CFile::Rename(bakfilename, filename);
            return FALSE;
        }
    }

    newfile.Close();
    file.Close();

    return TRUE;
}

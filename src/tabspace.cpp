#include <windows.h>
#include "cfile.h"
#include <stdio.h>

BOOL TabToSpace(PSTR filename);
void TabToSpaceFiles(PSTR ext);

// ��C++�е���C����д�ĺ�����������C�����ﶨ���ȫ�ֱ��������������
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

// ������һ����չ�����ļ�����tab->spaceת��
void TabToSpaceFiles(PSTR ext)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // �ҵ���һ���ļ�
    hFind = FindFirstFile(ext, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        printf("\nFindFirstFile for %s failed (%d).\n", ext, GetLastError());
        return;
    }
        
    printf("\nFindFirstFile for %s successful.\n", ext);

    do
    {
        // ���ҵ����ļ�tab->space
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

    // ����ת�����ļ��������"bak", �����һ�������ļ���
    strend = &bakfilename[MAX_PATH-1];
    str = strcpy_x(bakfilename, strend, filename);
    strcpy_x(str, strend, "bak");
    // ��ԭ�ļ�����Ϊ�����ļ���
    CFile::Remove(bakfilename);
    CFile::Rename(filename, bakfilename);

    // ����ת�����ļ�
    if (!file.Open(bakfilename, CFile::modeRead))
    {
        printf("Failed opening %s.\n", filename);
        return FALSE;
    }

    // �����ǽ�ת�������ļ����һ�����ļ���������Ҫ�򿪴����ļ�����д
    if (!newfile.Open(filename, CFile::modeWrite|CFile::modeCreate))
    {
        file.Close();
        printf("Failed opening output file.\n");
        return FALSE;
    }

    // ��ʼʵ��ִ��ת��
    while (cbytes = file.Read(readbuf, 32))
    {
        k = 0;
        for (i=0; i<cbytes; i++)
        {
            if (readbuf[i]=='\t')
            {
                // ����������ַ���tab, ����4���ո�
                writebuf[k++]=' ';
                writebuf[k++]=' ';
                writebuf[k++]=' ';
                writebuf[k++]=' ';
            }
            else
            {
                // ����ԭ�����
                writebuf[k++]=readbuf[i];
            }
        }

        // readbuf������writebufд���ļ�
        if (!newfile.Write(writebuf, k))
        {
            // ���д��������������˳�
            newfile.Abort();
            file.Close();
            // ��������ԭ�ļ����¸Ļ���
            CFile::Rename(bakfilename, filename);
            return FALSE;
        }
    }

    newfile.Close();
    file.Close();

    return TRUE;
}

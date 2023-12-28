#include <assert.h>

// safe string copy
char* strcpy_x(char *strdst, char *strdst_end, const char *strsrc)
{
    char ch;

    assert(strdst && strsrc && strdst_end);

    if (strdst > strdst_end)
        return 0;

    for (;;)
    {
        if (strdst==strdst_end || !(ch=*strsrc))
        {
            *strdst = '\0';
            return strdst;
        }

        *strdst++ = ch;
        strsrc++;
    }
}
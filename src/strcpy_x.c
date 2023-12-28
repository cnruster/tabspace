#include <assert.h>
#include <tchar.h>

// safe string copy
TCHAR* strcpy_x(TCHAR* strdst, const TCHAR* strdst_end, const TCHAR* strsrc)
{
    TCHAR ch;

    assert(strdst && strsrc && strdst_end);

    if (strdst > strdst_end) {
        return NULL;
    }

    for (;;) {
        if (strdst == strdst_end || !(ch = *strsrc)) {
            *strdst = _T('\0');
            return strdst;
        }

        *strdst++ = ch;
        strsrc++;
    }
}
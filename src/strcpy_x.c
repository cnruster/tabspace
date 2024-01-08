// Unicode-neutral safe string copy
// by Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include <assert.h>
#include <tchar.h>

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
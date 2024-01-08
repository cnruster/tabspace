// class FABSW : File As Byte Stream - Write
// by Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include <Windows.h>
#include "cfile.h"

// set buffer length to 512 for one consideration:
// equal to disk sector size
#define BUF_LEN    512

class FABSW
{
public:
	void Bind(HANDLE hOpenFile);
	BOOL PutByte(BYTE byte);
	BOOL PutBytes(BYTE byte, UINT rep);
	BOOL PutBytes(BYTE* bytes);
	BOOL EndPut();
	UINT GetPutCount() const;

protected:
	HANDLE m_hFile;

private:
	UINT m_nPutCount;       // this stores true put count only when ended!
	UINT m_nWriteOps;       // how many write operations are performed
	UINT m_nBufPtr;         // pointer to buffer
	BYTE m_Buffer[BUF_LEN];
};

// class FABSR : File As Byte Stream - Read
// by Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include <Windows.h>
#include "cfile.h"

// set buffer length to 512 for two considerations:
// 1. equal to disk sector size
// 2. a power of 2, the compiler can generate optimized code for division
#define BUF_LEN    512

class FABSR
{
public:
    // Result values
    enum Result {
        resSuccess = (int)1,
        resEof = (int)0,
        resFailure = (int)-1
    };

    // State values
    enum State {
        stInitial = (int)0x00000,
        stNearEof = (int)0x00001,
        stEof = (int)0x00002
    };

    void Bind(HANDLE hOpenFile);
    int GetByte(BYTE& byte);
    UINT GetGetCount() const;

protected:
    HANDLE m_hFile;

private:
    State m_state;          // what state this is in
    UINT m_nGetCount;       // how many bytes have been got
    UINT m_nReadCount;      // how many bytes have been read
    BYTE m_Buffer[BUF_LEN];
};

AFX_INLINE UINT FABSR::GetGetCount() const
{
    return m_nGetCount;
}

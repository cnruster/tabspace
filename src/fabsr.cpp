// class FABSR : File As Byte Stream - Read
// by Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include "fabsr.h"


void FABSR::Bind(HANDLE hOpenFile)
{
    ASSERT(hOpenFile != NULL);

    // assumes that hOpenFile is just opened, without history of
    // calling read/write which affects file pointer
    m_hFile = hOpenFile; // must be opened in binary read-only mode!
    m_state = stInitial;
    m_nGetCount = 0;
    m_nReadCount = 0;
}

int FABSR::GetByte(BYTE& byte)
{
    if (m_nGetCount == m_nReadCount) {
        if (m_state == stInitial) {
            UINT nRead;
            if (!CFile::Read(m_hFile, m_Buffer, BUF_LEN, nRead)) {
                return resFailure;
            }

            if (nRead == 0) {
                // direct state change from initial to eof is possible
                // if file length is a multiple of BUF_LEN
                m_state = stEof;
            }
            else {
                m_nReadCount += nRead;
                if (nRead != BUF_LEN) {
                    // this is the last read
                    m_state = stNearEof;
                }
            }
        }
        else if (m_state == stNearEof) {
            m_state = stEof;
        }
    }

    if (m_state == stEof) {
        return resEof;
    }

    byte = m_Buffer[m_nGetCount++ % BUF_LEN];
    return resSuccess;
}
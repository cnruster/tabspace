// class FABSW : File As Byte Stream - Write
// by Yiping Cheng, mailto:ypcheng@bjtu.edu.cn
// Beijing Jiaotong University
// https://www.researchgate.net/profile/Yiping-Cheng/research

#include "fabsw.h"


void FABSW::Bind(HANDLE hOpenFile)
{
    ASSERT(hOpenFile != NULL);

    // assumes that hOpenFile is freshly opened, without history of
    // calling read/write which affects file pointer
    m_hFile = hOpenFile; // must be opened in binary write-only mode!
    m_nPutCount = 0;
    m_nWriteOps = 0;
    m_nBufPtr = 0;
}

BOOL FABSW::PutByte(BYTE byte)
{
    m_Buffer[m_nBufPtr++] = byte;
    if (m_nBufPtr == BUF_LEN) {
        if (!CFile::Write(m_hFile, m_Buffer, BUF_LEN)) {
            return FALSE;
        }
        m_nWriteOps++;
        m_nBufPtr = 0;
    }

    return TRUE;
}

BOOL FABSW::PutBytes(BYTE byte, UINT rep)
{
    for (UINT i = 0; i < rep; i++) {
        if (!PutByte(byte)) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL FABSW::PutBytes(BYTE* bytes)
{
    for (BYTE* p=bytes; *p; p++) {
        if (!PutByte(*p)) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL FABSW::EndPut()
{
    m_nPutCount = m_nWriteOps * BUF_LEN + m_nBufPtr;
    return (m_nBufPtr == 0)? TRUE :
        CFile::Write(m_hFile, m_Buffer, m_nBufPtr);
}

UINT FABSW::GetPutCount() const
{
    return (m_nPutCount)? m_nPutCount : // ended
        m_nWriteOps * BUF_LEN + m_nBufPtr; // in process
}

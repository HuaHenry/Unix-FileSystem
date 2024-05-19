#pragma once
#include "includes.h"

// ���ٻ�����ƿ�
class Buffer
{
public:
    enum BufFlag{
        B_BUSY = 0x2,   // ռ�ñ�־�������������ݴӴ����ж����B_BUSY��1
        B_DELWRI = 0x4, // �ӳ�д��־
    };
	// �����̵��豸�����˻�����У�ֻ�����˿��ж��к�I/O�������
	uint8_t	b_flags = 0;
	Buffer* av_forw = nullptr;
	Buffer* av_back = nullptr;
	int		b_wcount = 0;		    /* �贫�͵��ֽ��� */
	uint8_t* b_addr = nullptr;	    /* ָ��û�����ƿ�������Ļ��������׵�ַ */
	int		b_blkno = 0;		    /* �����߼���� */
};


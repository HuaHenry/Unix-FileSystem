#pragma once
#include "DiskManager.h"
#include "Buffer.h"
#include "includes.h"

// blknoָʾ�̿�ţ���0��ʼ��ţ�ÿ512�ֽ�һ���̿�

// ���������
class BufferManager
{
public:
	static const int NBUF = 15;		    // ������ƿ顢������������
public:
	BufferManager();
	// �����̿�blkno��������Ϊlength������content��������Ӧ�����У���ʼƫ��Ϊoffset���������Ƿ�ɹ�
	bool write(uint8_t* content, int blkno, int offset = 0, int length = BLOCK_SIZE);
	// �����̿�blkno����offsetƫ�ƶ�ȡ����Ϊlength�����ݵ�holder�У����سɹ���ȡ���ֽ���
	int read(uint8_t* holder, int blkno, int offset = 0, int length = BLOCK_SIZE);
    void destroy();                     // ��д����Ĭ�϶�д�����̿�
private:
	void moveBlk2Rear(Buffer* bp);	    // ��ĳһ�黺��Ų����β��LRU�㷨��
	Buffer* searchFreeBlk();		    // Ѱ��һ��δ��ռ�õĻ���飬��û���ҵ�������nullptr
	Buffer* GetBlk(int blkno);	        // ����һ�黺�棬���ڶ�д�ַ���blkno
	Buffer* Bread(int blkno);		    // ��һ�����̿鵽���棬blknoΪĿ����̿��߼����
	void Bwrite(Buffer* bp);		    // ��һ�������д�����

private:
	// bufferList ��av_forwָʾ��β�飬av_backָʾ���׿�
	Buffer bufferList;		            // ������ƿ���У����׽�㣩��������¼���������ķ��ʴ�������LRU�㷨
	Buffer m_Buf[NBUF];		            // ������ƿ�����
	uint8_t Buffers[NBUF][BLOCK_SIZE];	// ����������

	DiskManager myDisk;		            // ���̹����࣬���𡰴��̡���д����
};




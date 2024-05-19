#pragma once
#include "includes.h"
#include "BufferManager.h"
#include "Inode.h"

class BlockManager{
public:
	static const int SUPER_BLOCK_START = 200;               // ��������ʼλ��
	static const int INODE_START = 202;	                    // ��һ��inodeָ���Ŀ¼�ļ�
	static const int FILE_DATA_START = 1024;                // �ļ���������ʼλ��
	static const int BLOCK_SUM = DISK_SIZE_KB * 1024 / 512; // �̿�����

private:
	BufferManager* m_BufferManager;
	SuperBlock m_superblock;
	uint8_t buffer[BLOCK_SIZE];
public: 
	void destroy();
	void init(BufferManager*);

	void flush();		            // �����̡���ʽ��
	int allocateBlock();			// �����̿�
	void recycleBlock(int blkno);	// �����̿飨���������̿�ţ���FileManager�Ѿ�����ת����

	// ע�⣬filemanager����inodeǰ��ˢ��inode������
	// �����ļ���Ŀ¼ʱ����Ҫ����inode����
	int allocateInode();
	// ɾ���ļ���Ŀ¼ʱ����Ҫ����inode������ע�ⶥ�����ɾ��Ŀ¼ʱ������������ļ�ҲҪɾ��������
	void recycleInode(int inodeNo);
};


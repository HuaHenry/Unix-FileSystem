#include "BlockManager.h"

void BlockManager::init(BufferManager* buff) {
	m_BufferManager = buff;

	// ʹ�û�����������superblock����

	m_BufferManager->read(buffer, SUPER_BLOCK_START);
	m_superblock.s_isize = *((int*)buffer);
	m_superblock.s_fsize = *((int*)buffer + 1);
	m_superblock.s_nfree = *((int*)buffer + 2);
	for (int i = 0; i < 100; ++i) {
		m_superblock.s_free[i] = *((int*)buffer + 3 + i);
	}

	m_BufferManager->read(buffer, SUPER_BLOCK_START + 1);
	m_superblock.s_ninode = *((int*)buffer);
	for (int i = 0; i < 100; ++i) {
		m_superblock.s_inode[i] = *((int*)buffer + 1 + i);
	}
	m_superblock.s_fmod = 0;
}

// ���ǵ�����˳�����⣬���û�н�������д����������
void BlockManager::destroy() {
	if (m_superblock.s_fmod) {
		// �ڴ�superblock���޸ģ�˵����Ҫд����̣���ʱ��д�뻺��
		memset(buffer, 0, BLOCK_SIZE);
		*((int*)buffer) = m_superblock.s_isize;
		*((int*)buffer + 1) = m_superblock.s_fsize;
		*((int*)buffer + 2) = m_superblock.s_nfree;
		for (int i = 0; i < 100; ++i) {
			*((int*)buffer + 3 + i) = m_superblock.s_free[i];
		}
		m_BufferManager->write(buffer, SUPER_BLOCK_START);

		*((int*)buffer) = m_superblock.s_ninode;
		for (int i = 0; i < 100; ++i) {
			*((int*)buffer + 1 + i) = m_superblock.s_inode[i];
		}
		m_BufferManager->write(buffer, SUPER_BLOCK_START + 1);
	}
	// �����ɻ�����д�����
}

int BlockManager::allocateBlock() {
	m_superblock.s_fmod = 1;

	int ret = m_superblock.s_free[--m_superblock.s_nfree];

	if (m_superblock.s_nfree == 0) {
		// ���Ϊ�㣬˵����ǰ100�������̿���Ѿ�������ϣ���0��ָʾ���̿��ж�ȡ�µĿ����̿�
		m_BufferManager->read(buffer, ret);
		int blkno = 0;
		for (int blkNum = *((int*)buffer); blkNum > 0; --blkNum) {
			blkno = *((int*)buffer + ++m_superblock.s_nfree);
			if (blkno) {
				m_superblock.s_free[m_superblock.s_nfree - 1] = blkno;
			}
			else {
				// blknoΪ�㣬˵���Ѿ�û�п����̿飬�˳�ѭ��
				break;
			}
		}
	}

	// ���仺���ǰ��ˢ���̿�������
	memset(buffer, 0, BLOCK_SIZE);
	m_BufferManager->write(buffer, ret);

	return ret;
}

void BlockManager::recycleBlock(int blkno)
{
	m_superblock.s_fmod = 1;

	if (m_superblock.s_nfree < 100) {
		m_superblock.s_free[m_superblock.s_nfree++] = blkno;
	}
	else {
		// ��ǰsuperblock����Ŀ����̿�����
		m_BufferManager->read(buffer, blkno);
		*((int*)buffer) = 100;
		for (int i = 0; i < 100; ++i) {
			*((int*)buffer + i + 1) = m_superblock.s_free[i];
		}
		m_BufferManager->write(buffer, blkno);

		m_superblock.s_nfree = 1;
		m_superblock.s_free[0] = blkno;
	}
}

int BlockManager::allocateInode()
{
	m_superblock.s_fmod = 1;
	int ret = m_superblock.s_inode[--m_superblock.s_ninode];
	if (m_superblock.s_ninode == 0) {   // ��ǰ100������inode���Ѿ�������ϣ�����inode����ȡ��100�����е�inode
		int inodeNo = 0;
		for (int blkno = INODE_START; blkno < FILE_DATA_START; ++blkno) {
			m_BufferManager->read(buffer, blkno);
			for (int i = 0; i < BLOCK_SIZE / sizeof(DiskInode); ++i) {
				// ���d_modeΪ�㣬˵��inode��δ��ռ�ã���ȡ֮
				if (*((int*)(buffer + sizeof(DiskInode) * i)) == 0) {
					m_superblock.s_inode[m_superblock.s_ninode++] = inodeNo;
					if (m_superblock.s_ninode == 100) {
						return ret;
					}
				}
				++inodeNo;
			}
		}
	}
	return ret;
}

void BlockManager::recycleInode(int inodeNo)
{
	if (m_superblock.s_ninode < 100) {
		m_superblock.s_fmod = 1;
		m_superblock.s_inode[m_superblock.s_ninode++] = inodeNo;
	}
	// ���򣬲���ȡ�κδ�ʩ
}

void BlockManager::flush()
{
	// 1. superblock����д�루�ڴ�superblock���º���ˢ�����Խ����ã�
	m_superblock.s_isize = FILE_DATA_START - INODE_START;
	m_superblock.s_fsize = DISK_SIZE_KB * 1024 / BLOCK_SIZE;
	m_superblock.s_nfree = 100;
	// ����s_free
	for (int i = 0; i < 100; ++i) {
		m_superblock.s_free[i] = i + FILE_DATA_START;
	}
	m_superblock.s_ninode = 100;
	// ����s_inode����·����Ӧ0��inode�����ܷŵ�����inode�У�
	for (int i = 0; i < 100; ++i) {
		m_superblock.s_inode[i] = i + 1;
	}
	m_superblock.s_fmod = 1;
	destroy();	                // д��

	// 2. inode������д�루ˢ0��
	memset(buffer, 0, BLOCK_SIZE);
	for (int i = INODE_START; i < FILE_DATA_START; ++i) {
		m_BufferManager->write(buffer, i);
	}

	// 3. �ļ�����������д��

    // �����һ�����д��
	int blkno = FILE_DATA_START;
	memset(buffer, 0, BLOCK_SIZE);
	for (; blkno < BLOCK_SUM; blkno += 100) {
		// �������ӣ���i��1024��1124��1224���������̿�洢i+100��i+199��100���̿�
		*((int*)buffer) = 100;
		for (int i = 0; i < 100; ++i) {
			*((int*)buffer + i + 1) = blkno + 100 + i;
		}
		m_BufferManager->write(buffer, blkno);

		// ��һ���̿��������ӣ�ʣ��99���̿�ȫ��ˢ0
		memset(buffer, 0, BLOCK_SIZE);
		for (int i = 1; i < 100; ++i) {
			if (blkno + i >= BLOCK_SUM)	// ��ֹд���߽�
				break;
			m_BufferManager->write(buffer, blkno + i);
		}
	}
	blkno -= 100;
	
	// ���һ���̿�ʵ��д�����Ŀ��
	int actualBlockNumber = 100 - (blkno - BLOCK_SUM);
	*((int*)buffer) = actualBlockNumber;
	*((int*)buffer + actualBlockNumber + 1) = 0;
	m_BufferManager->write(buffer, blkno);
}
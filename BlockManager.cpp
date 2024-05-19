#include "BlockManager.h"

void BlockManager::init(BufferManager* buff) {
	m_BufferManager = buff;

	// 使用缓存管理类读入superblock内容

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

// 考虑到析构顺序问题，因此没有将本操作写入析构函数
void BlockManager::destroy() {
	if (m_superblock.s_fmod) {
		// 内存superblock被修改，说明需要写入磁盘，此时先写入缓存
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
	// 后续由缓存再写入磁盘
}

int BlockManager::allocateBlock() {
	m_superblock.s_fmod = 1;

	int ret = m_superblock.s_free[--m_superblock.s_nfree];

	if (m_superblock.s_nfree == 0) {
		// 如果为零，说明当前100个空闲盘块均已经分配完毕，从0号指示的盘块中读取新的空闲盘块
		m_BufferManager->read(buffer, ret);
		int blkno = 0;
		for (int blkNum = *((int*)buffer); blkNum > 0; --blkNum) {
			blkno = *((int*)buffer + ++m_superblock.s_nfree);
			if (blkno) {
				m_superblock.s_free[m_superblock.s_nfree - 1] = blkno;
			}
			else {
				// blkno为零，说明已经没有空闲盘块，退出循环
				break;
			}
		}
	}

	// 分配缓存块前，刷新盘块中内容
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
		// 当前superblock管理的空闲盘块已满
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
	if (m_superblock.s_ninode == 0) {   // 当前100个空闲inode均已经分配完毕，遍历inode区获取下100个空闲的inode
		int inodeNo = 0;
		for (int blkno = INODE_START; blkno < FILE_DATA_START; ++blkno) {
			m_BufferManager->read(buffer, blkno);
			for (int i = 0; i < BLOCK_SIZE / sizeof(DiskInode); ++i) {
				// 如果d_mode为零，说明inode块未被占用，收取之
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
	// 否则，不采取任何措施
}

void BlockManager::flush()
{
	// 1. superblock更新写入（内存superblock更新后不用刷，可以接着用）
	m_superblock.s_isize = FILE_DATA_START - INODE_START;
	m_superblock.s_fsize = DISK_SIZE_KB * 1024 / BLOCK_SIZE;
	m_superblock.s_nfree = 100;
	// 更新s_free
	for (int i = 0; i < 100; ++i) {
		m_superblock.s_free[i] = i + FILE_DATA_START;
	}
	m_superblock.s_ninode = 100;
	// 更新s_inode（根路径对应0号inode，不能放到空闲inode中）
	for (int i = 0; i < 100; ++i) {
		m_superblock.s_inode[i] = i + 1;
	}
	m_superblock.s_fmod = 1;
	destroy();	                // 写回

	// 2. inode区更新写入（刷0）
	memset(buffer, 0, BLOCK_SIZE);
	for (int i = INODE_START; i < FILE_DATA_START; ++i) {
		m_BufferManager->write(buffer, i);
	}

	// 3. 文件数据区数据写入

    // 非最后一个块的写入
	int blkno = FILE_DATA_START;
	memset(buffer, 0, BLOCK_SIZE);
	for (; blkno < BLOCK_SUM; blkno += 100) {
		// 成组链接，第i（1024，1124，1224，…）个盘块存储i+100到i+199共100个盘块
		*((int*)buffer) = 100;
		for (int i = 0; i < 100; ++i) {
			*((int*)buffer + i + 1) = blkno + 100 + i;
		}
		m_BufferManager->write(buffer, blkno);

		// 第一个盘块用于链接，剩余99个盘块全部刷0
		memset(buffer, 0, BLOCK_SIZE);
		for (int i = 1; i < 100; ++i) {
			if (blkno + i >= BLOCK_SUM)	// 防止写出边界
				break;
			m_BufferManager->write(buffer, blkno + i);
		}
	}
	blkno -= 100;
	
	// 最后一个盘块实际写入的条目数
	int actualBlockNumber = 100 - (blkno - BLOCK_SUM);
	*((int*)buffer) = actualBlockNumber;
	*((int*)buffer + actualBlockNumber + 1) = 0;
	m_BufferManager->write(buffer, blkno);
}
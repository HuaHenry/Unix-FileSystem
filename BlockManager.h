#pragma once
#include "includes.h"
#include "BufferManager.h"
#include "Inode.h"

class BlockManager{
public:
	static const int SUPER_BLOCK_START = 200;               // 超级块起始位置
	static const int INODE_START = 202;	                    // 第一块inode指向根目录文件
	static const int FILE_DATA_START = 1024;                // 文件数据区起始位置
	static const int BLOCK_SUM = DISK_SIZE_KB * 1024 / 512; // 盘块总数

private:
	BufferManager* m_BufferManager;
	SuperBlock m_superblock;
	uint8_t buffer[BLOCK_SIZE];
public: 
	void destroy();
	void init(BufferManager*);

	void flush();		            // “磁盘”格式化
	int allocateBlock();			// 分配盘块
	void recycleBlock(int blkno);	// 回收盘块（传入物理盘块号，由FileManager已经做好转换）

	// 注意，filemanager分配inode前，刷新inode中内容
	// 创建文件或目录时，需要分配inode函数
	int allocateInode();
	// 删除文件或目录时，需要回收inode函数（注意顶层操作删除目录时，下面的所有文件也要删除！！）
	void recycleInode(int inodeNo);
};


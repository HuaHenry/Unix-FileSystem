#pragma once
#include "DiskManager.h"
#include "Buffer.h"
#include "includes.h"

// blkno指示盘块号，从0开始编号，每512字节一个盘块

// 缓存管理类
class BufferManager
{
public:
	static const int NBUF = 15;		    // 缓存控制块、缓冲区的数量
public:
	BufferManager();
	// 对于盘块blkno，将长度为length的内容content拷贝到对应缓存中（起始偏移为offset），返回是否成功
	bool write(uint8_t* content, int blkno, int offset = 0, int length = BLOCK_SIZE);
	// 对于盘块blkno，从offset偏移读取长度为length的内容到holder中，返回成功读取的字节数
	int read(uint8_t* holder, int blkno, int offset = 0, int length = BLOCK_SIZE);
    void destroy();                     // 读写操作默认读写整个盘块
private:
	void moveBlk2Rear(Buffer* bp);	    // 将某一块缓存挪到队尾（LRU算法）
	Buffer* searchFreeBlk();		    // 寻找一个未被占用的缓存块，若没有找到，返回nullptr
	Buffer* GetBlk(int blkno);	        // 申请一块缓存，用于读写字符块blkno
	Buffer* Bread(int blkno);		    // 读一个磁盘块到缓存，blkno为目标磁盘块逻辑块号
	void Bwrite(Buffer* bp);		    // 将一个缓存块写入磁盘

private:
	// bufferList 中av_forw指示队尾块，av_back指示队首块
	Buffer bufferList;		            // 缓存控制块队列（队首结点），用来记录各个缓存块的访问次序，体现LRU算法
	Buffer m_Buf[NBUF];		            // 缓存控制块数组
	uint8_t Buffers[NBUF][BLOCK_SIZE];	// 缓冲区数组

	DiskManager myDisk;		            // 磁盘管理类，负责“磁盘”读写操作
};




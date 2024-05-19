#pragma once
#include "includes.h"

// 高速缓存控制块
class Buffer
{
public:
    enum BufFlag{
        B_BUSY = 0x2,   // 占用标志，当缓存块的内容从磁盘中读入后，B_BUSY置1
        B_DELWRI = 0x4, // 延迟写标志
    };
	// 单进程单设备：简化了缓存队列，只保留了空闲队列和I/O请求队列
	uint8_t	b_flags = 0;
	Buffer* av_forw = nullptr;
	Buffer* av_back = nullptr;
	int		b_wcount = 0;		    /* 需传送的字节数 */
	uint8_t* b_addr = nullptr;	    /* 指向该缓存控制块所管理的缓冲区的首地址 */
	int		b_blkno = 0;		    /* 磁盘逻辑块号 */
};


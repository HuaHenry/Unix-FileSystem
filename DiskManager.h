#pragma once
#include "includes.h"

// 磁盘管理，负责真正在“磁盘”中进行读写操作
class DiskManager
{
private:
	string diskFileName = "myDisk.img";
	FILE* disk = nullptr;

public:
	DiskManager();
	~DiskManager();
	bool writeDisk(int blkno, uint8_t* src);	// 将src中内容写入blkno盘块，返回是否成功
	bool readDisk(int blkno, uint8_t* dst);	// 读取blkno盘块中内容至dst，返回是否成功
};


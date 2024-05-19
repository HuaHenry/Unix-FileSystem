#pragma once
#include "includes.h"

// ���̹������������ڡ����̡��н��ж�д����
class DiskManager
{
private:
	string diskFileName = "myDisk.img";
	FILE* disk = nullptr;

public:
	DiskManager();
	~DiskManager();
	bool writeDisk(int blkno, uint8_t* src);	// ��src������д��blkno�̿飬�����Ƿ�ɹ�
	bool readDisk(int blkno, uint8_t* dst);	// ��ȡblkno�̿���������dst�������Ƿ�ɹ�
};


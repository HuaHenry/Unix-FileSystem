#include "DiskManager.h"
#include "FileSystem.h"
bool diskExist = false;

DiskManager::DiskManager()
{
	// 创建一个大小为size的文件作为磁盘

	// 首先尝试用rb+方式创建（不会覆盖原有内容，但是文件必须存在）
	// 如果不行，则需要用wb+方式创建（文件可以不存在，但是会覆盖原有内容）
	errno_t err = fopen_s(&disk, diskFileName.c_str(), "rb+");
	if (err || !disk) {
		err = fopen_s(&disk, diskFileName.c_str(), "wb+");
		diskExist = false;	// 设置为false，需要进行format操作
	}
	else {
		diskExist = true;	// 设置为true，则不需要进行format操作
	}

	if (err || !disk) {
		cout << "Create Disk Failed!" << endl;
		exit(EXIT_FAILURE);
	}

	int res = fseek(disk, long(DISK_SIZE_KB * 1024 - 1), SEEK_SET);
	if (res) {
		cout << "Create Disk Failed!" << endl;
		exit(EXIT_FAILURE);
	}

	char a = 0;
	fwrite(&a, 1, sizeof(a), disk);
	fseek(disk, 0, SEEK_SET);
}

DiskManager::~DiskManager()
{
	fclose(disk);
}

bool DiskManager::readDisk(int blkno, uint8_t* dst)
{
	if (blkno * BLOCK_SIZE >= DISK_SIZE_KB * 1024) {
		cout << "ERROR: Read disk out of limit!" << endl;
	}

	if (fseek(disk, blkno * BLOCK_SIZE, SEEK_SET)){
		cout << "ERROR: Read disk block " << blkno << " out of limit!" << endl;
		return false;
	}

	return fread(dst, 1, BLOCK_SIZE, disk) != 0;
}

bool DiskManager::writeDisk(int blkno, uint8_t* src)
{
	if (blkno * BLOCK_SIZE >= DISK_SIZE_KB * 1024) {
		cout << "ERROR: Write disk block " << blkno << " out of limit!" << endl;
	}

	fseek(disk, blkno * BLOCK_SIZE, SEEK_SET);
	return fwrite(src, 1, BLOCK_SIZE, disk) != 0;
}
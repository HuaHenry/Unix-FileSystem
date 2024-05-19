#pragma once
#include "includes.h"
#include "Inode.h"
#include "BufferManager.h"
#include "BlockManager.h"

// 文件路径相关，文件信息等由FileManager维护
class FileManager
{
private:
	Inode pathInode;            // 当前路径的内存Inode结点
	Inode inode;	            // 当前打开文件的内存Inode结点

	BufferManager* m_BufferManager = nullptr;
	BlockManager* m_BlockManager = nullptr;

	int bufferBlkno;
	uint8_t buffer[BLOCK_SIZE];
	int fileOffset = 0;		    // 当前打开文件的偏移量
	vector<string> paths;	    // 记录当前各个路径

	static const int FILE_ITEM_BYTES = 32;

private:
	// 在当前文件的当前偏移量上将长度为size字节的内容content写入磁盘
	void write(int size, uint8_t* content, Inode& inode);
	// 在当前文件的当前偏移量上从磁盘读长度为size字节的内容存到content，返回实际读取的字节数
	int read(int size, uint8_t* content, Inode& inode);

	// 直接从i_addr分配盘块，是当i_addr检索到盘块号为0时触发
	int allocateBlockFromIADDR(int idx, Inode& inode);
	// 从盘块分配盘块，是当索引盘块block检索到盘块号为0时触发
	int allocateBlockFromBlock(int idx);

	int blknoTransform(int logic, Inode& inode); // 执行逻辑块号与物理块号的转换(Bmap)

	Inode readInode(int inodeNo);
	void writeInode(Inode inode);

	// 在当前目录文件中查找该文件，若存在则遍历盘块回收，并回收Inode，不存在返回false
	bool deleteFile(Inode inode);
	// 删除当前目录下所有文件及索引，需要考虑递归
	bool deleteDirectory(Inode inode);

	// 对于一个目录文件，查找name对应的项，返回该项的序号
	// 如果name为空串，则是在查找是否有之前被删除的项，可以用于写入新的目录项
	int findDirItem(string name);
public:
	void init(BufferManager*, BlockManager*);

	// 将paths内容整合返回
	string renewPath();

	void setFileOffset(int offset);

	void writeToFile(int size, uint8_t* content);
	int readFromFile(int size, uint8_t* content);

	void flush();
	void destroy();

	// 将inode号对应的Inode读入并更新
	bool renewFileInode(int inodeNo);
	
	// 针对pathInode进行操作而不修改inode变量（如果有重名，则创建失败，返回false）
	// 创建时，先调用searchItemInDirectory函数进行查找
	bool createItem(string fileName, bool isDir);

	// 遍历目录文件（ls操作）
	vector<string> directoryTraverse();	
	// 在目录文件中查找表项，返回对应的inode结点号，若没有则返回-1
	int searchItemInDirectory(string itemName);	

	// 判断是文件还是索引，分别调用下面的两个函数（当前目录下对应的表项需要删除，即置空串）
	bool deleteItem(string name);

	// 进入新的目录（下一级或上一级）
	string changePath(string path);

	// 获取当前目录下该文件的大小（search，找到后读取对应inode，查看i_size）
	int getFileSize(string name);
	// 重命名（search，找到后通过偏移量修改前28字节）
	void renameFile(string oldName, string newName);
};

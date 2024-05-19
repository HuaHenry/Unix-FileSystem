#include "FileManager.h"

void FileManager::init(BufferManager* buff, BlockManager* blk)
{
	m_BufferManager = buff;
	m_BlockManager = blk;

	pathInode = readInode(0);	// 0号结点指示文件目录的根结点
	paths.push_back("/");		// 当前路径插入根路径
}

void FileManager::setFileOffset(int offset)
{
	fileOffset = offset;
}

void FileManager::writeToFile(int size, uint8_t* content)
{
	write(size, content, inode);
}

int FileManager::readFromFile(int size, uint8_t* content)
{
	return read(size, content, inode);
}

void FileManager::flush()
{
	m_BlockManager->flush();

	// 根目录inode需要进行一些操作
	inode.i_number = 0;
	inode.i_size = 2 * FILE_ITEM_BYTES;
	inode.i_mode = inode.IDIR;

	uint8_t root[FILE_ITEM_BYTES] = {};
	// .当前目录为"/"
	root[0] = '.';
	m_BufferManager->write(root, blknoTransform(0, inode), 0, FILE_ITEM_BYTES);
	// ..上一级目录也为自身
	root[1] = '.';
	m_BufferManager->write(root, blknoTransform(0, inode), FILE_ITEM_BYTES, FILE_ITEM_BYTES);

	writeInode(inode);	// 盘块转换时会申请一个新的盘块，因此这一步要放在最后

	// 当前存在的一些变量也需要更改
	paths.clear();
	paths.push_back("/");
	pathInode = readInode(0);
	fileOffset = 0;

}

void FileManager::destroy()
{
	if (pathInode.i_mode & pathInode.IUPD)
		writeInode(pathInode);
	if (inode.i_mode & inode.IUPD)
		writeInode(inode);
}

Inode FileManager::readInode(int inodeNo)
{
	Inode inode;

	static const int DiskInodeNumPerBlock = BLOCK_SIZE / sizeof(DiskInode);
	int blkno = m_BlockManager->INODE_START + inodeNo / DiskInodeNumPerBlock;
	m_BufferManager->read(buffer, blkno);
	bufferBlkno = blkno;
	int inodeOffset = inodeNo - (blkno - m_BlockManager->INODE_START) * DiskInodeNumPerBlock;
	int offset = inodeOffset * sizeof(DiskInode);

	int* diskInodePointer = (int*)(buffer + offset);
	inode.i_mode = *diskInodePointer;
	inode.i_number = inodeNo;
	inode.i_size = *(diskInodePointer + 1);
	for (int i = 0; i < 10; ++i) {
		inode.i_addr[i] = *(diskInodePointer + 2 + i);
	}

	return inode;
}

void FileManager::writeInode(Inode inode)
{
	static const int DiskInodeNumPerBlock = BLOCK_SIZE / sizeof(DiskInode);
	int blkno = m_BlockManager->INODE_START + inode.i_number / DiskInodeNumPerBlock;
	m_BufferManager->read(buffer, blkno);
	bufferBlkno = blkno;
	int inodeOffset = inode.i_number - (blkno - m_BlockManager->INODE_START) * DiskInodeNumPerBlock;
	int offset = inodeOffset * sizeof(DiskInode);

	// 将UPD标志位清除
	inode.i_mode &= ~inode.IUPD;

	int* diskInodePointer = (int*)(buffer + offset);
	*diskInodePointer = inode.i_mode;
	*(diskInodePointer + 1) = inode.i_size;
	for (int i = 0; i < 10; ++i) {
		*(diskInodePointer + 2 + i) = inode.i_addr[i];
	}

	m_BufferManager->write(buffer, blkno);
}

int FileManager::allocateBlockFromIADDR(int idx, Inode& inode)
{
	int ret = m_BlockManager->allocateBlock();	// 得到物理盘块号
	m_BufferManager->read(buffer, ret);
	bufferBlkno = ret;
	inode.i_addr[idx] = ret;
	// 确保新盘块内容均为0（不是必要）
	memset(buffer, 0, BLOCK_SIZE);
	m_BufferManager->write(buffer, ret);// 实际上在某些情况下这可能多余
	return ret;	// 返回分配的物理盘块号
}

int FileManager::allocateBlockFromBlock(int idx)	// 索引盘块中每4字节一块索引号
{
	int ret = m_BlockManager->allocateBlock();	// 得到物理盘块号
	*((int*)buffer + idx) = ret;		// 将buffer强转为int*，找到索引对应的位置，写入盘块号
	// 先将原本的block写回，再读入新的block
	m_BufferManager->write(buffer, bufferBlkno);
	m_BufferManager->read(buffer, ret);
	bufferBlkno = ret;
	// 确保新盘块内容均为0（不是必要）
	memset(buffer, 0, BLOCK_SIZE);
	m_BufferManager->write(buffer, ret);// 实际上在某些情况下这可能多余
	return ret;	// 返回分配的物理盘块号
}

int FileManager::blknoTransform(int logic, Inode& inode)
{
	int ret = 0;	// 物理盘块号
	int index1 = 0;	// 一级索引
	int index2 = 0;	// 二级索引

	const int indexNum = BLOCK_SIZE / sizeof(int); // 128

	if (logic < inode.SMALL_FILE_BLOCK) {
		// 小型文件，直接读取即可
		ret = inode.i_addr[logic];
		if (!ret) {
			// 盘块号为0，说明未分配，先分配一个磁盘盘块
			ret = allocateBlockFromIADDR(logic, inode);
		}
		// 否则，不需要操作，直接返回物理盘块号即可
		return ret;
	}
	else if (logic < inode.LARGE_FILE_BLOCK) {
		 // 在直接索引中的入口：(logic-6)/128+6
		int iaddr_ = (logic - inode.SMALL_FILE_BLOCK) / indexNum + 6;
		index1 = inode.i_addr[iaddr_];
		if (!index1) {
			// 盘块号为0，说明未分配，先分配一个磁盘盘块
			index1 = allocateBlockFromIADDR(iaddr_, inode);
			memset(buffer, 0, BLOCK_SIZE);
		}
		else {
			// 否则直接将对应盘块读入
			m_BufferManager->read(buffer, index1);
			bufferBlkno = index1;
		}
		// 在一次间接索引中的入口
		int iaddr_blk = (logic - inode.SMALL_FILE_BLOCK) % indexNum;
		// 此时buffer中含有一级索引块内容，在索引块中再读入地址
		ret = *((int*)buffer + iaddr_blk);	// 找到偏移量对应地址，强制类型转换为int*，得到int数据
		if (!ret) {
			// 盘块号为0，说明未分配，先分配一个磁盘盘块
			ret = allocateBlockFromBlock(iaddr_blk);
		}
		return ret;
	}
	else if (logic < inode.HUGE_FILE_BLOCK) {
		// 在直接索引中的入口：(logic-LARGE_FILE_BLOCK)/(128*128)+8
		int iaddr_ = (logic - inode.LARGE_FILE_BLOCK) / (indexNum * indexNum) + 8;
		index1 = inode.i_addr[iaddr_];
		if (!index1) {
			index1 = allocateBlockFromIADDR(iaddr_, inode);
			memset(buffer, 0, BLOCK_SIZE);
		}
		else {
			m_BufferManager->read(buffer, index1);
			bufferBlkno = index1;
		}
		// 在一次间接索引中的入口
		int iaddr_blk = (logic - inode.LARGE_FILE_BLOCK) / indexNum % indexNum;
		// 在一级索引对应的盘块内容中取到二级索引所在的物理盘块号
		index2 = *((int*)buffer + iaddr_blk);
		if (!index2) {
			index2 = allocateBlockFromBlock(iaddr_blk);
			memset(buffer, 0, BLOCK_SIZE);
		}
		else {
			m_BufferManager->read(buffer, index2);
			bufferBlkno = index2;
		}
		// 在二次间接索引中的入口
		iaddr_blk = (logic - inode.LARGE_FILE_BLOCK) % indexNum;
		// 从二级索引中找到物理盘块号
		ret = *((int*)buffer + iaddr_blk);
		if (!ret) {
			// 盘块号为0，说明未分配，先分配一个磁盘盘块
			ret = allocateBlockFromBlock(iaddr_blk);
		}
		return ret;
	}
	else {
		cout << "Blkno transform failed!" << endl;
		exit(EXIT_FAILURE);
	}
}

void FileManager::write(int size, uint8_t* content, Inode &inode)
{
	int offset = fileOffset % BLOCK_SIZE;		// 计算第一次的偏移
	int blkno = fileOffset / BLOCK_SIZE;		// 计算第一次对应的盘块号（逻辑盘块号）
	int length = min(size, BLOCK_SIZE - offset);// 计算第一次的长度
	// 如果文件指针指向的逻辑盘块与之前不连续，需要先给之前分配盘块
	int borderBlock = inode.i_size / BLOCK_SIZE * BLOCK_SIZE;	// 最后一个盘块对应的序号
	for (int newBlkno = borderBlock + 1; newBlkno < blkno; ++newBlkno) {
		blknoTransform(newBlkno, inode);	// 转换时，会针对没有分配的序号进行盘块分配
	}
	fileOffset += size;
	while (size > 0) {
		// 调用FileManager的地址转换，进行逻辑块号和物理块号的转换
		if (!m_BufferManager->write(content, blknoTransform(blkno, inode), offset, length)) {
			cout << "ERROR: Write buffer failed!" << endl;
			exit(EXIT_FAILURE);
		};
		// 更新参数
		size -= length;
		content += length;
		++blkno;
		length = min(BLOCK_SIZE, size);
		offset = 0;			// 由于一次写操作在逻辑上是连续的，因此后续偏移都是0
	}

	inode.i_size = max(inode.i_size, fileOffset);	// 更新inode中的文件大小
	inode.i_mode |= inode.IUPD;
}

int FileManager::read(int size, uint8_t* content, Inode& inode)
{
	if (fileOffset + size > inode.i_size) {
		size = inode.i_size - fileOffset;
	}
	int totalSize = 0;
	int offset = fileOffset % BLOCK_SIZE;		// 计算第一次的偏移
	int blkno = fileOffset / BLOCK_SIZE;		// 计算第一次对应的盘块号（逻辑盘块号）
	int length = min(size, BLOCK_SIZE - offset);// 计算第一次的长度
	if (length <= 0) {
		if (length < 0)
			cout << "ERROR: Reading exceeds the file limit!" << endl;
		return 0;
	}
	int actualSize = 0;
	while (size > 0) {
		// 调用FileManager的地址转换，进行逻辑块号和物理块号的转换
		actualSize = m_BufferManager->read(content, blknoTransform(blkno, inode), offset, length);
		totalSize += actualSize;
		fileOffset += actualSize;
		// 更新参数
		size -= actualSize;
		content += actualSize;
		++blkno;
		// 考虑可能读取超出文件大小的内容
		length = min(BLOCK_SIZE, min(size, inode.i_size - fileOffset));
		offset = 0;			// 由于一次读操作在逻辑上是连续的，因此后续偏移都是0
	}
	return totalSize;
}

bool FileManager::renewFileInode(int inodeNo)
{
	if (inode.i_number == inodeNo) {	// 如果打开的与之前相同，不需要修改
		return true;
	}

	Inode node = readInode(inodeNo);
	if ((node.i_mode & node.ITEXT) == 0) {
		// TEXT位为零，说明不是正文文件，不允许打开
		cout << "ERROR: Not allowed to open directory file!" << endl;
		return false;
	}
	// 是正文文件，才允许更新inode结点

	// 如果之前inode结点有更新标志，则需要更新
	if (inode.i_mode & inode.IUPD)
		writeInode(inode);

	inode = node;
	return true;
}

bool FileManager::createItem(string fileName, bool isDir)
{
	// 截断，最长28字节
	if (fileName.size() > 28) {
		fileName = fileName.substr(0, 28);
	}
	if (fileName == ".") {
		cout << "ERROR: Cannot use . as file or path name" << endl;
		return false;
	}
	else if (fileName == "..") {
		cout << "ERROR: Cannot use .. as file or path name" << endl;
		return false;
	}

	// 首先查找是否有重名，若有则返回false
	if (searchItemInDirectory(fileName) != -1) {
		cout << "Creation failed! Name has already existed." << endl;
		return false;
	}

	// 申请inode结点，清空原有内容，修改i_mode，写回
	int newInodeNo = m_BlockManager->allocateInode();
	Inode newInode = readInode(newInodeNo);
	newInode.i_mode = 0;
	newInode.i_number = newInodeNo;
	if (isDir)
		newInode.i_mode |= newInode.IDIR;
	else
		newInode.i_mode |= newInode.ITEXT;	// 模式为文件正文段
	newInode.i_size = 0;
	for (int i = 0; i < 10; ++i) {
		newInode.i_addr[i] = 0;
	}

	// 将这个inode结点写入当前路径文件内
	int lastOffset = fileOffset;
	int itemOffset = findDirItem("");
	// 等于-1，说明没有可以使用的，从尾部开始继续写，否则说明有被删除的部分，则覆盖之
	fileOffset = (itemOffset == -1) ? pathInode.i_size : itemOffset * FILE_ITEM_BYTES;

	{
		uint8_t item[FILE_ITEM_BYTES] = {}; // 可以不考虑尾零

		const char* ch = fileName.c_str();
		for (int i = 0; i < 28 && ch[i] != '\0'; ++i) {
			item[i] = ch[i];
		}
		*((int*)(item + 28)) = newInodeNo;
		write(FILE_ITEM_BYTES, item, pathInode);
	}
	
	// 如果创建的是路径，还需要将前0-1项的32字节填充
	if (isDir) {
		fileOffset = 0;

		uint8_t buf[FILE_ITEM_BYTES] = {};

		memcpy(buf, ".", 1);		// 写入当前路径名
		*((int*)(buf + 28)) = searchItemInDirectory(fileName);// 写入当前路径对应的inode号
		write(FILE_ITEM_BYTES, buf, newInode);

		memset(buf, 0, FILE_ITEM_BYTES);
		memcpy(buf, "..", 2);		// 写入上一路径名
		*((int*)(buf + 28)) = searchItemInDirectory(".");// 写入上一路径对应的inode号
		write(FILE_ITEM_BYTES, buf, newInode);
	}

	writeInode(newInode);
	fileOffset = lastOffset;
	return true;
}

vector<string> FileManager::directoryTraverse()
{
	// 将原本的文件偏移保存，等到使用完毕后放回
	const int lastOffset = fileOffset;
	fileOffset = 0;

	vector<string> ret;
	// 遍历整个文件，输出路径或文件名
	char buf[FILE_ITEM_BYTES + 1] = {};
	// 前两个丢弃
	read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode);
	read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode);

	while (read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode) == FILE_ITEM_BYTES) {
		if (buf[0] == '\0')	// 如果为零，说明该项已经被删除，跳过该项
			continue;

		buf[28] = 0;	// 截断后四个字节指示盘块号的内容
		ret.push_back((string)buf);
		memset(buf, 0, FILE_ITEM_BYTES + 1);
	}
	
	fileOffset = lastOffset;
	return ret;
}

int FileManager::searchItemInDirectory(string name)
{
	// 将原本的文件偏移保存，等到使用完毕后放回
	const int lastOffset = fileOffset;
	fileOffset = 0;

	// 遍历整个文件，查找路径或文件名
	char buf[FILE_ITEM_BYTES + 1] = {};
	string strBuf;
	while (read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode) == FILE_ITEM_BYTES) {
		strBuf = buf;
		if (strBuf.substr(0, 28) == name) {
			fileOffset = lastOffset;
			return *((int*)(buf + 28));
		}
		memset(buf, 0, FILE_ITEM_BYTES + 1);
	}

	fileOffset = lastOffset;
	return -1;
}

bool FileManager::deleteItem(string name)
{
	int inodeNo = searchItemInDirectory(name);
	if (inodeNo == -1) {
		cout << "ERROR: Item doesn't exist!" << endl;
		return false;
	}

	Inode toDelete = readInode(inodeNo);
	// 在当前目录文件中将这个表项删除
	int itemIndex = findDirItem(name);
	uint8_t buf[FILE_ITEM_BYTES] = {};
	fileOffset = itemIndex * FILE_ITEM_BYTES;
	write(FILE_ITEM_BYTES, buf, pathInode);
	writeInode(pathInode);

	if (toDelete.i_mode & toDelete.ITEXT){	
		// 是正文段
		return deleteFile(toDelete);
	}
	else if (toDelete.i_mode & toDelete.IDIR) {
		// 是目录
		return deleteDirectory(toDelete);
	}

	cout << "Unknown error occurred when delete item." << endl;
	return false;
}

bool FileManager::deleteFile(Inode inode) 
{
	vector<int>blocks;
    int borderBlock = inode.i_size ? inode.i_size / BLOCK_SIZE * BLOCK_SIZE + 1 : 0; // 总盘块数
    for (int i = 0; i < borderBlock; ++i)
    { // 获取所有的物理盘块号
        blocks.push_back(blknoTransform(i, inode));
    }
    memset(buffer, 0, BLOCK_SIZE);
	for (int i = 0; i < blocks.size(); ++i) {
		m_BufferManager->write(buffer, blocks[i]);
		m_BlockManager->recycleBlock(blocks[i]);
	}
	return true;
}

bool FileManager::deleteDirectory(Inode inode)
{
	bool ret = true;
	// 删除目录，不仅要回收目录文件对应的盘块号，还需要递归回收其中的其他文件（正文/目录）
	uint8_t fileItem[FILE_ITEM_BYTES] = {};
	fileOffset = 0;
	// 读取各个目录项，递归调用删除函数
	// 前两个跳过
	read(FILE_ITEM_BYTES, fileItem, inode);
	read(FILE_ITEM_BYTES, fileItem, inode);
	while (read(FILE_ITEM_BYTES, fileItem, inode) == FILE_ITEM_BYTES) {
		fileItem[28] = 0;
		string name = (char*)fileItem;
		// 反之，如果为空，说明对应的内容已经被删除
		if (name != "")
			ret &= deleteItem(name);

		memset(fileItem, 0, FILE_ITEM_BYTES);
	}
	// 删除本身自带的几个盘块
	ret &= deleteFile(inode);
	return ret;
}

int FileManager::findDirItem(string name)
{   // 将原本的文件偏移保存，等到使用完毕后放回
    const int lastOffset = fileOffset;
	fileOffset = 0;
	// 遍历整个文件，查找路径或文件名
	char buf[FILE_ITEM_BYTES + 1] = {};
    read(FILE_ITEM_BYTES, (uint8_t *)buf, pathInode);  // 前两项跳过
    read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode);
	int itemOffset = 2;
	while (read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode) == FILE_ITEM_BYTES) {
		if (buf == name) {
			fileOffset = lastOffset;
			return itemOffset;
		}
		++itemOffset;
		memset(buf, 0, FILE_ITEM_BYTES + 1);
	}
	fileOffset = lastOffset;
	return -1;
}

string FileManager::changePath(string path)
{
	if (path == ".") {
		return renewPath();
	}

	int inodeNo = searchItemInDirectory(path);
	if (inodeNo == -1) {
		cout << "ERROR: Target path doesn't exist." << endl;
		return renewPath();
	}

	// 首先判断该表项是不是目录，如果不是则修改路径失败
	Inode newInode = readInode(inodeNo);
	if ((newInode.i_mode & newInode.IDIR) == 0) {
		cout << "ERROR: Cannot change directory to a file!" << endl;
		return renewPath();
	}

	writeInode(pathInode);
	pathInode = readInode(inodeNo);
	if (path == "..") {
		if (paths.size() > 1)
			paths.pop_back();
		// 否则，上一级目录仍然为本目录
	}
	else {
		paths.push_back(path);
	}
	return renewPath();
}

string FileManager::renewPath()
{
	string ret;
	for (string str : paths) {
		ret += str + "/";
	}
	return ret;
}

int FileManager::getFileSize(string fileName)
{
	// 截断，最长28字节
	if (fileName.size() > 28) {
		fileName = fileName.substr(0, 28);
	}
	// 首先在当前目录下查找该文件
	int inodeNo = searchItemInDirectory(fileName);
	if (inodeNo == -1) {
		cout << "Cannot find file " << fileName << endl;
		return -1;
	}

	// 如果当前内存inode即为所求，则不通过readInode进行读取（因为可能有更新延迟）
	if (inodeNo == inode.i_number) {
		return inode.i_size;
	}
	else {
		Inode ansInode = readInode(inodeNo);
		return ansInode.i_size;
	}
}

void FileManager::renameFile(string oldName, string newName)
{
	// 首先截断
	if (oldName.size() > 28) {
		oldName = oldName.substr(0, 28);
	}
	if (newName.size() > 28) {
		newName = newName.substr(0, 28);
	}
	// 通过oldName查找是否有该文件，若没有则重命名失败
	int itemIndex = findDirItem(oldName);
	if (itemIndex == -1) {
		cout << "Cannot find file " << oldName << " !" << endl;
		return;
	}
	// 通过newName查找是否有同名文件，若有则重命名失败
	if (searchItemInDirectory(newName) != -1) {
		cout << "File or directory " << newName << " already exist." << endl;
		return;
	}
	// 重命名
	uint8_t buf[FILE_ITEM_BYTES] = {};
	for (int i = 0; i < newName.size(); ++i) {
		buf[i] = newName[i];
	}
	*((int*)(buf + 28)) = searchItemInDirectory(oldName);
	fileOffset = itemIndex * FILE_ITEM_BYTES;
	write(FILE_ITEM_BYTES, buf, pathInode);
}
#include "FileManager.h"

void FileManager::init(BufferManager* buff, BlockManager* blk)
{
	m_BufferManager = buff;
	m_BlockManager = blk;

	pathInode = readInode(0);	// 0�Ž��ָʾ�ļ�Ŀ¼�ĸ����
	paths.push_back("/");		// ��ǰ·�������·��
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

	// ��Ŀ¼inode��Ҫ����һЩ����
	inode.i_number = 0;
	inode.i_size = 2 * FILE_ITEM_BYTES;
	inode.i_mode = inode.IDIR;

	uint8_t root[FILE_ITEM_BYTES] = {};
	// .��ǰĿ¼Ϊ"/"
	root[0] = '.';
	m_BufferManager->write(root, blknoTransform(0, inode), 0, FILE_ITEM_BYTES);
	// ..��һ��Ŀ¼ҲΪ����
	root[1] = '.';
	m_BufferManager->write(root, blknoTransform(0, inode), FILE_ITEM_BYTES, FILE_ITEM_BYTES);

	writeInode(inode);	// �̿�ת��ʱ������һ���µ��̿飬�����һ��Ҫ�������

	// ��ǰ���ڵ�һЩ����Ҳ��Ҫ����
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

	// ��UPD��־λ���
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
	int ret = m_BlockManager->allocateBlock();	// �õ������̿��
	m_BufferManager->read(buffer, ret);
	bufferBlkno = ret;
	inode.i_addr[idx] = ret;
	// ȷ�����̿����ݾ�Ϊ0�����Ǳ�Ҫ��
	memset(buffer, 0, BLOCK_SIZE);
	m_BufferManager->write(buffer, ret);// ʵ������ĳЩ���������ܶ���
	return ret;	// ���ط���������̿��
}

int FileManager::allocateBlockFromBlock(int idx)	// �����̿���ÿ4�ֽ�һ��������
{
	int ret = m_BlockManager->allocateBlock();	// �õ������̿��
	*((int*)buffer + idx) = ret;		// ��bufferǿתΪint*���ҵ�������Ӧ��λ�ã�д���̿��
	// �Ƚ�ԭ����blockд�أ��ٶ����µ�block
	m_BufferManager->write(buffer, bufferBlkno);
	m_BufferManager->read(buffer, ret);
	bufferBlkno = ret;
	// ȷ�����̿����ݾ�Ϊ0�����Ǳ�Ҫ��
	memset(buffer, 0, BLOCK_SIZE);
	m_BufferManager->write(buffer, ret);// ʵ������ĳЩ���������ܶ���
	return ret;	// ���ط���������̿��
}

int FileManager::blknoTransform(int logic, Inode& inode)
{
	int ret = 0;	// �����̿��
	int index1 = 0;	// һ������
	int index2 = 0;	// ��������

	const int indexNum = BLOCK_SIZE / sizeof(int); // 128

	if (logic < inode.SMALL_FILE_BLOCK) {
		// С���ļ���ֱ�Ӷ�ȡ����
		ret = inode.i_addr[logic];
		if (!ret) {
			// �̿��Ϊ0��˵��δ���䣬�ȷ���һ�������̿�
			ret = allocateBlockFromIADDR(logic, inode);
		}
		// ���򣬲���Ҫ������ֱ�ӷ��������̿�ż���
		return ret;
	}
	else if (logic < inode.LARGE_FILE_BLOCK) {
		 // ��ֱ�������е���ڣ�(logic-6)/128+6
		int iaddr_ = (logic - inode.SMALL_FILE_BLOCK) / indexNum + 6;
		index1 = inode.i_addr[iaddr_];
		if (!index1) {
			// �̿��Ϊ0��˵��δ���䣬�ȷ���һ�������̿�
			index1 = allocateBlockFromIADDR(iaddr_, inode);
			memset(buffer, 0, BLOCK_SIZE);
		}
		else {
			// ����ֱ�ӽ���Ӧ�̿����
			m_BufferManager->read(buffer, index1);
			bufferBlkno = index1;
		}
		// ��һ�μ�������е����
		int iaddr_blk = (logic - inode.SMALL_FILE_BLOCK) % indexNum;
		// ��ʱbuffer�к���һ�����������ݣ������������ٶ����ַ
		ret = *((int*)buffer + iaddr_blk);	// �ҵ�ƫ������Ӧ��ַ��ǿ������ת��Ϊint*���õ�int����
		if (!ret) {
			// �̿��Ϊ0��˵��δ���䣬�ȷ���һ�������̿�
			ret = allocateBlockFromBlock(iaddr_blk);
		}
		return ret;
	}
	else if (logic < inode.HUGE_FILE_BLOCK) {
		// ��ֱ�������е���ڣ�(logic-LARGE_FILE_BLOCK)/(128*128)+8
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
		// ��һ�μ�������е����
		int iaddr_blk = (logic - inode.LARGE_FILE_BLOCK) / indexNum % indexNum;
		// ��һ��������Ӧ���̿�������ȡ�������������ڵ������̿��
		index2 = *((int*)buffer + iaddr_blk);
		if (!index2) {
			index2 = allocateBlockFromBlock(iaddr_blk);
			memset(buffer, 0, BLOCK_SIZE);
		}
		else {
			m_BufferManager->read(buffer, index2);
			bufferBlkno = index2;
		}
		// �ڶ��μ�������е����
		iaddr_blk = (logic - inode.LARGE_FILE_BLOCK) % indexNum;
		// �Ӷ����������ҵ������̿��
		ret = *((int*)buffer + iaddr_blk);
		if (!ret) {
			// �̿��Ϊ0��˵��δ���䣬�ȷ���һ�������̿�
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
	int offset = fileOffset % BLOCK_SIZE;		// �����һ�ε�ƫ��
	int blkno = fileOffset / BLOCK_SIZE;		// �����һ�ζ�Ӧ���̿�ţ��߼��̿�ţ�
	int length = min(size, BLOCK_SIZE - offset);// �����һ�εĳ���
	// ����ļ�ָ��ָ����߼��̿���֮ǰ����������Ҫ�ȸ�֮ǰ�����̿�
	int borderBlock = inode.i_size / BLOCK_SIZE * BLOCK_SIZE;	// ���һ���̿��Ӧ�����
	for (int newBlkno = borderBlock + 1; newBlkno < blkno; ++newBlkno) {
		blknoTransform(newBlkno, inode);	// ת��ʱ�������û�з������Ž����̿����
	}
	fileOffset += size;
	while (size > 0) {
		// ����FileManager�ĵ�ַת���������߼���ź������ŵ�ת��
		if (!m_BufferManager->write(content, blknoTransform(blkno, inode), offset, length)) {
			cout << "ERROR: Write buffer failed!" << endl;
			exit(EXIT_FAILURE);
		};
		// ���²���
		size -= length;
		content += length;
		++blkno;
		length = min(BLOCK_SIZE, size);
		offset = 0;			// ����һ��д�������߼����������ģ���˺���ƫ�ƶ���0
	}

	inode.i_size = max(inode.i_size, fileOffset);	// ����inode�е��ļ���С
	inode.i_mode |= inode.IUPD;
}

int FileManager::read(int size, uint8_t* content, Inode& inode)
{
	if (fileOffset + size > inode.i_size) {
		size = inode.i_size - fileOffset;
	}
	int totalSize = 0;
	int offset = fileOffset % BLOCK_SIZE;		// �����һ�ε�ƫ��
	int blkno = fileOffset / BLOCK_SIZE;		// �����һ�ζ�Ӧ���̿�ţ��߼��̿�ţ�
	int length = min(size, BLOCK_SIZE - offset);// �����һ�εĳ���
	if (length <= 0) {
		if (length < 0)
			cout << "ERROR: Reading exceeds the file limit!" << endl;
		return 0;
	}
	int actualSize = 0;
	while (size > 0) {
		// ����FileManager�ĵ�ַת���������߼���ź������ŵ�ת��
		actualSize = m_BufferManager->read(content, blknoTransform(blkno, inode), offset, length);
		totalSize += actualSize;
		fileOffset += actualSize;
		// ���²���
		size -= actualSize;
		content += actualSize;
		++blkno;
		// ���ǿ��ܶ�ȡ�����ļ���С������
		length = min(BLOCK_SIZE, min(size, inode.i_size - fileOffset));
		offset = 0;			// ����һ�ζ��������߼����������ģ���˺���ƫ�ƶ���0
	}
	return totalSize;
}

bool FileManager::renewFileInode(int inodeNo)
{
	if (inode.i_number == inodeNo) {	// ����򿪵���֮ǰ��ͬ������Ҫ�޸�
		return true;
	}

	Inode node = readInode(inodeNo);
	if ((node.i_mode & node.ITEXT) == 0) {
		// TEXTλΪ�㣬˵�����������ļ����������
		cout << "ERROR: Not allowed to open directory file!" << endl;
		return false;
	}
	// �������ļ������������inode���

	// ���֮ǰinode����и��±�־������Ҫ����
	if (inode.i_mode & inode.IUPD)
		writeInode(inode);

	inode = node;
	return true;
}

bool FileManager::createItem(string fileName, bool isDir)
{
	// �ضϣ��28�ֽ�
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

	// ���Ȳ����Ƿ��������������򷵻�false
	if (searchItemInDirectory(fileName) != -1) {
		cout << "Creation failed! Name has already existed." << endl;
		return false;
	}

	// ����inode��㣬���ԭ�����ݣ��޸�i_mode��д��
	int newInodeNo = m_BlockManager->allocateInode();
	Inode newInode = readInode(newInodeNo);
	newInode.i_mode = 0;
	newInode.i_number = newInodeNo;
	if (isDir)
		newInode.i_mode |= newInode.IDIR;
	else
		newInode.i_mode |= newInode.ITEXT;	// ģʽΪ�ļ����Ķ�
	newInode.i_size = 0;
	for (int i = 0; i < 10; ++i) {
		newInode.i_addr[i] = 0;
	}

	// �����inode���д�뵱ǰ·���ļ���
	int lastOffset = fileOffset;
	int itemOffset = findDirItem("");
	// ����-1��˵��û�п���ʹ�õģ���β����ʼ����д������˵���б�ɾ���Ĳ��֣��򸲸�֮
	fileOffset = (itemOffset == -1) ? pathInode.i_size : itemOffset * FILE_ITEM_BYTES;

	{
		uint8_t item[FILE_ITEM_BYTES] = {}; // ���Բ�����β��

		const char* ch = fileName.c_str();
		for (int i = 0; i < 28 && ch[i] != '\0'; ++i) {
			item[i] = ch[i];
		}
		*((int*)(item + 28)) = newInodeNo;
		write(FILE_ITEM_BYTES, item, pathInode);
	}
	
	// �����������·��������Ҫ��ǰ0-1���32�ֽ����
	if (isDir) {
		fileOffset = 0;

		uint8_t buf[FILE_ITEM_BYTES] = {};

		memcpy(buf, ".", 1);		// д�뵱ǰ·����
		*((int*)(buf + 28)) = searchItemInDirectory(fileName);// д�뵱ǰ·����Ӧ��inode��
		write(FILE_ITEM_BYTES, buf, newInode);

		memset(buf, 0, FILE_ITEM_BYTES);
		memcpy(buf, "..", 2);		// д����һ·����
		*((int*)(buf + 28)) = searchItemInDirectory(".");// д����һ·����Ӧ��inode��
		write(FILE_ITEM_BYTES, buf, newInode);
	}

	writeInode(newInode);
	fileOffset = lastOffset;
	return true;
}

vector<string> FileManager::directoryTraverse()
{
	// ��ԭ�����ļ�ƫ�Ʊ��棬�ȵ�ʹ����Ϻ�Ż�
	const int lastOffset = fileOffset;
	fileOffset = 0;

	vector<string> ret;
	// ���������ļ������·�����ļ���
	char buf[FILE_ITEM_BYTES + 1] = {};
	// ǰ��������
	read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode);
	read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode);

	while (read(FILE_ITEM_BYTES, (uint8_t*)buf, pathInode) == FILE_ITEM_BYTES) {
		if (buf[0] == '\0')	// ���Ϊ�㣬˵�������Ѿ���ɾ������������
			continue;

		buf[28] = 0;	// �ضϺ��ĸ��ֽ�ָʾ�̿�ŵ�����
		ret.push_back((string)buf);
		memset(buf, 0, FILE_ITEM_BYTES + 1);
	}
	
	fileOffset = lastOffset;
	return ret;
}

int FileManager::searchItemInDirectory(string name)
{
	// ��ԭ�����ļ�ƫ�Ʊ��棬�ȵ�ʹ����Ϻ�Ż�
	const int lastOffset = fileOffset;
	fileOffset = 0;

	// ���������ļ�������·�����ļ���
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
	// �ڵ�ǰĿ¼�ļ��н��������ɾ��
	int itemIndex = findDirItem(name);
	uint8_t buf[FILE_ITEM_BYTES] = {};
	fileOffset = itemIndex * FILE_ITEM_BYTES;
	write(FILE_ITEM_BYTES, buf, pathInode);
	writeInode(pathInode);

	if (toDelete.i_mode & toDelete.ITEXT){	
		// �����Ķ�
		return deleteFile(toDelete);
	}
	else if (toDelete.i_mode & toDelete.IDIR) {
		// ��Ŀ¼
		return deleteDirectory(toDelete);
	}

	cout << "Unknown error occurred when delete item." << endl;
	return false;
}

bool FileManager::deleteFile(Inode inode) 
{
	vector<int>blocks;
    int borderBlock = inode.i_size ? inode.i_size / BLOCK_SIZE * BLOCK_SIZE + 1 : 0; // ���̿���
    for (int i = 0; i < borderBlock; ++i)
    { // ��ȡ���е������̿��
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
	// ɾ��Ŀ¼������Ҫ����Ŀ¼�ļ���Ӧ���̿�ţ�����Ҫ�ݹ�������е������ļ�������/Ŀ¼��
	uint8_t fileItem[FILE_ITEM_BYTES] = {};
	fileOffset = 0;
	// ��ȡ����Ŀ¼��ݹ����ɾ������
	// ǰ��������
	read(FILE_ITEM_BYTES, fileItem, inode);
	read(FILE_ITEM_BYTES, fileItem, inode);
	while (read(FILE_ITEM_BYTES, fileItem, inode) == FILE_ITEM_BYTES) {
		fileItem[28] = 0;
		string name = (char*)fileItem;
		// ��֮�����Ϊ�գ�˵����Ӧ�������Ѿ���ɾ��
		if (name != "")
			ret &= deleteItem(name);

		memset(fileItem, 0, FILE_ITEM_BYTES);
	}
	// ɾ�������Դ��ļ����̿�
	ret &= deleteFile(inode);
	return ret;
}

int FileManager::findDirItem(string name)
{   // ��ԭ�����ļ�ƫ�Ʊ��棬�ȵ�ʹ����Ϻ�Ż�
    const int lastOffset = fileOffset;
	fileOffset = 0;
	// ���������ļ�������·�����ļ���
	char buf[FILE_ITEM_BYTES + 1] = {};
    read(FILE_ITEM_BYTES, (uint8_t *)buf, pathInode);  // ǰ��������
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

	// �����жϸñ����ǲ���Ŀ¼������������޸�·��ʧ��
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
		// ������һ��Ŀ¼��ȻΪ��Ŀ¼
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
	// �ضϣ��28�ֽ�
	if (fileName.size() > 28) {
		fileName = fileName.substr(0, 28);
	}
	// �����ڵ�ǰĿ¼�²��Ҹ��ļ�
	int inodeNo = searchItemInDirectory(fileName);
	if (inodeNo == -1) {
		cout << "Cannot find file " << fileName << endl;
		return -1;
	}

	// �����ǰ�ڴ�inode��Ϊ������ͨ��readInode���ж�ȡ����Ϊ�����и����ӳ٣�
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
	// ���Ƚض�
	if (oldName.size() > 28) {
		oldName = oldName.substr(0, 28);
	}
	if (newName.size() > 28) {
		newName = newName.substr(0, 28);
	}
	// ͨ��oldName�����Ƿ��и��ļ�����û����������ʧ��
	int itemIndex = findDirItem(oldName);
	if (itemIndex == -1) {
		cout << "Cannot find file " << oldName << " !" << endl;
		return;
	}
	// ͨ��newName�����Ƿ���ͬ���ļ���������������ʧ��
	if (searchItemInDirectory(newName) != -1) {
		cout << "File or directory " << newName << " already exist." << endl;
		return;
	}
	// ������
	uint8_t buf[FILE_ITEM_BYTES] = {};
	for (int i = 0; i < newName.size(); ++i) {
		buf[i] = newName[i];
	}
	*((int*)(buf + 28)) = searchItemInDirectory(oldName);
	fileOffset = itemIndex * FILE_ITEM_BYTES;
	write(FILE_ITEM_BYTES, buf, pathInode);
}
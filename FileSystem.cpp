#include "FileSystem.h"

void FileSystem::init(string& path)
{
	m_FileManager.init(&m_BufferManager, &m_BlockManager);
	m_BlockManager.init(&m_BufferManager);
	path = m_FileManager.renewPath();
	if (!diskExist) {	// 磁盘不存在，说明是新建的，进行一次格式化
		m_FileManager.flush();
	}
}

FileSystem::~FileSystem()
{
	m_FileManager.destroy();	// pathInode内容写回缓存
	m_BlockManager.destroy();	// superBlock内容写回缓存
	m_BufferManager.destroy();	// 再将缓存中内容写到磁盘
}

void FileSystem::_flseek(int offset)
{
	if (currentFileName == "") {
		cout << "Operation not permitted while no file is opened!" << endl;
		return;
	}
	m_FileManager.setFileOffset(offset);
}

void FileSystem::_fformat()
{
	m_FileManager.flush();
}

vector<string> FileSystem::_ls()
{
	if (currentFileName != "") {
		cout << "Operation not permitted when a file is opened!" << endl;
		return vector<string>( { ".." } );	// 返回..，代表操作失败
	}
	return m_FileManager.directoryTraverse();
}

void FileSystem::_mkdir(string dirName)
{
	if (currentFileName != "") {
		cout << "Operation not permitted when a file is opened!" << endl;
		return;
	}
	m_FileManager.createItem(dirName, true);
}

void FileSystem::_fcreat(string fileName)
{
	if (currentFileName != "") {
		cout << "Operation not permitted when a file is opened!" << endl;
		return;
	}
	m_FileManager.createItem(fileName, false);
}

bool FileSystem::_fopen(string _fileName)
{
	if (_fileName == currentFileName) {
		cout << "[ERROR]: 当前文件已打开！" << endl;        
		return false;
	}

	int inodeNo = m_FileManager.searchItemInDirectory(_fileName);
	if (inodeNo == -1) {
		cout << "[ERROR]: 文件不存在" << endl;
		return false;
	}

	bool res = m_FileManager.renewFileInode(inodeNo);
	if (res) {
		m_FileManager.setFileOffset(0);
		currentFileName = _fileName;
	}
	return res;
}

void FileSystem::_fclose()
{
	currentFileName = "";
}

int FileSystem::_fread(int length, uint8_t* content)
{
	if (currentFileName == "") {
		cout << "Operation not permitted while no file is opened!" << endl;
		return 0;
	}

	return m_FileManager.readFromFile(length, content);
}

void FileSystem::_fwrite(int length, uint8_t* content)
{
	if (currentFileName == "") {
		cout << "Operation not permitted while no file is opened!" << endl;
		return;
	}

	m_FileManager.writeToFile(length, content);
}

void FileSystem::_fdelete(string itemName)
{
	if (currentFileName != "") {
		cout << "Operation not permitted when a file is opened!" << endl;
		return;
	}

	if (itemName == ".") {
		cout << "ERROR: Cannot delete current directory!" << endl;
		return;
	}
	else if (itemName == "..") {
		cout << "ERROR: Cannot delete the upper directory!" << endl;
		return;
	}

	m_FileManager.deleteItem(itemName);
}

string FileSystem::_cd(string name)
{
	if (currentFileName != "") {
		cout << "Operation not permitted when a file is opened!" << endl;
		return "";
	}
	return m_FileManager.changePath(name);
}

int FileSystem::_size(string name)
{
	return m_FileManager.getFileSize(name);
}

void FileSystem::_rename(string oldName, string newName)
{
	if (currentFileName != "") {
		cout << "Operation not permitted when a file is opened!" << endl;
		return;
	}
	return m_FileManager.renameFile(oldName, newName);
}
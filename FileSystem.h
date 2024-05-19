#pragma once
#include "BufferManager.h"
#include "BlockManager.h"
#include "FileManager.h"
#include "Inode.h"
#include "includes.h"

class FileSystem
{
public:
	void init(string& path);

	void _fformat();
	vector<string> _ls();
	void _mkdir(string dirName);
	void _fcreat(string fileName);
	bool _fopen(string fileName);
	void _fclose();
	int _fread(int length, uint8_t* content);
	void _fwrite(int length, uint8_t* content);
	void _flseek(int offset);
	void _fdelete(string itemName);
	string _cd(string path);
	int _size(string fileName);
	void _rename(string oldName, string newName);
	~FileSystem();
private:
	string currentFileName;

	FileManager m_FileManager;		// 文件管理类
	BufferManager m_BufferManager;	// 缓存管理类
	BlockManager m_BlockManager;	// 盘块管理类
};

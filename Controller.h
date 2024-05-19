#pragma once
#include "FileSystem.h"
#include "includes.h"

class Controller
{
private:
	const static int CMD_LENGTH = 1024;
	FileSystem myFileSystem;

public:
	string path;
	string fileName;

	Controller();
	void _fformat();
	void _ls();	
	void _mkdir(string name);
	void _fcreat(string name);	
	void _fopen(string param);
	void _fclose();
	void _fread(string size, string path);
	void _fwrite(string size, string path);
	void _flseek(string param);
	void _fdelete(string param);
	void _cd(string param);

	void _import(string filePath);
	void _export(string fileName);
	void _size(string fileName);
	void _rename(string oldName, string newName);
};


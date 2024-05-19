#include "Controller.h"

void COLOR_PRINT_S(const char* s, int color)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf(s);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
}

Controller::Controller()
{
	myFileSystem.init(path);
}

void Controller::_fformat()
{
	myFileSystem._fformat();
	fileName = "";
	path = "/";	// 内部路径记录已经更新，外部手动更新
	COLOR_PRINT_S("\n[Success] ", 2);
	cout << "磁盘格式化成功" << endl;
}

void Controller::_ls()
{
	vector<string> items = myFileSystem._ls();
	if (items.size() == 1 && items[0] == "..") {
		return;
	}

	cout << items.size() << " items under this directory:" << endl;
	// 行宽58字符
	const int length = 58;
	int remainLength = length;
	for (string name : items) {
		// 剩余宽度不足以输出，换行
		if (remainLength < name.size()) {
			cout << endl;
			remainLength = length;
		}
		remainLength -= name.size();
		cout << name << ' ';
	}
	cout << endl;
}

void Controller::_mkdir(string name)
{
	/*string name;
	cin >> name;*/
	if (name.find('\\') != -1 || name.find('/') != -1 || name.find(':') != -1
		|| name.find('*') != -1 || name.find('?') != -1 || name.find('\"') != -1
		|| name.find('<') != -1 || name.find('>') != -1 || name.find('|') != -1) {
		cout << "Illegal directory name! includes: \\ / : * ? \" < > | " << endl;
		return;
	}
	myFileSystem._mkdir(name);
}

void Controller::_fcreat(string name)
{
	/*string name;
	cin >> name;*/
	if (name.find('\\') != -1 || name.find('/') != -1 || name.find(':') != -1
		|| name.find('*') != -1 || name.find('?') != -1 || name.find('\"') != -1
		|| name.find('<') != -1 || name.find('>') != -1 || name.find('|') != -1) {
		cout << "Illegal file name! includes: \\ / : * ? \" < > | " << endl;
		return;
	}
	myFileSystem._fcreat(name);
}

void Controller::_fopen(string param)
{
	/*string param;
	cin >> param;*/
	if (myFileSystem._fopen(param)) {
		fileName = param;
	}
}

void Controller::_fclose()
{
	myFileSystem._fclose();
	fileName = "";
}

void Controller::_fread(string size, string path)
{
	/*string size, path;
	cin >> size >> path;*/
	for (char ch : size) {
		if (!isdigit(ch)) {
			cout << "Illegal parameter: " << size << endl;
			return;
		}
	}
	int size_int = atoi(size.c_str());
	size_int = size_int ? size_int : CMD_LENGTH;

	uint8_t* content = new(nothrow)uint8_t[size_int]{};
	if (!content) {
		exit(OVERFLOW);
	}

	if (path == "*") {
		// 输出到CMD窗口
		int actualSize = myFileSystem._fread(size_int, content);
		actualSize = min(actualSize, size_int);
		cout << "Read " << actualSize << " bytes successfully(64 bytes per line):" << endl;
		int lineControl = 0;
		for (int i = 0; i < actualSize; ++i, ++lineControl) {
			putchar((int)content[i]);
			if (content[i] == '\n') {
				lineControl = 63;
			}
			else if (lineControl % 64 == 63) {
				cout << endl;
			}
		}
		cout << endl;
	}
	else {
		// 输出到文件
		FILE* file;
		errno_t err = fopen_s(&file, path.c_str(), "ab");
		if (err || !file) {
			cout << "ERROR: Cannot find corresponding file!" << endl;
			return;
		}
		
		int totalLength = 0;
		int actualLength = myFileSystem._fread(size_int, content);
		if (atoi(size.c_str()) == 0) {
			// 读取的文件长度为0，说明需要将整个文件读入
			while (actualLength > 0) {
				fwrite(content, 1, actualLength, file);
				totalLength += actualLength;
				actualLength = myFileSystem._fread(actualLength, content);
			}
		}
		else {
			fwrite(content, 1, actualLength, file);
			totalLength += actualLength;
		}
		cout << "Read " << totalLength << " bytes successfully." << endl;
		fclose(file);
	}
	
	delete[] content;
}

void Controller::_fwrite(string size,string path) {
	/*string size, path;
	cin >> size >> path;*/
	for (char ch : size) {
		if (!isdigit(ch)) {
			cout << "Illegal parameter: " << size << endl;
			return;
		}
	}
	int size_int = atoi(size.c_str());
	size_int = size_int ? size_int : CMD_LENGTH;

	uint8_t* content = new(nothrow)uint8_t[size_int + 1]{};
	if (!content) {
		exit(OVERFLOW);
	}

	if (path == "*") {
		cout << "Press in... (" << size_int << " bytes at most)" << endl;
		// 清空缓冲区
		cin.ignore(65536, '\n');
		string totalString;
		while (totalString.size() <= size_int) {
			cin.getline((char*)content, size_int + 1);
			/*if (string((char*)content) == "end!") break;*/
			totalString += string((char*)content) + "\r\n";
		}
		totalString = totalString.substr(0, size_int);
		myFileSystem._fwrite(size_int, (uint8_t*)totalString.c_str());
	}
	else {
		// 从文件中读取内容并写入
		FILE* file;
		errno_t err = fopen_s(&file, path.c_str(), "rb");
		if (err || !file) {
			cout << "ERROR: Cannot find corresponding file!" << endl;
			return;
		}
		
		int actualLength = fread(content, 1, size_int, file);
		if (atoi(size.c_str()) == 0) {
			// 读取的文件长度为0，说明需要将整个文件读入
			while (actualLength > 0) {
				myFileSystem._fwrite(actualLength, content);
				actualLength = fread(content, 1, size_int, file);
			}
		}
		else {
			myFileSystem._fwrite(actualLength, content);
		}

		fclose(file);
	}
	delete[] content;
}

// 不允许出现负数
void Controller::_flseek(string param)
{
	/*string param;
	cin >> param;*/
	for (char ch : param) {
		if (!isdigit(ch)) {
			cout << "Illegal parameter: " << param << endl;
			return;
		}
	}

	myFileSystem._flseek(atoi(param.c_str()));
}

void Controller::_fdelete(string param)
{
	/*string param;
	cin >> param;*/
	myFileSystem._fdelete(param);
}

void Controller::_cd(string param)
{
	/*string param;
	cin >> param;*/
	string tmp = myFileSystem._cd(param);
	if (tmp != "") {
		path = tmp;
	}
}

void Controller::_import(string filePath)
{
	string fileName;
	//cin >> filePath;
	int pos = filePath.find_last_of('/');
	if (pos >= 0 && pos < filePath.size()) {
		fileName = filePath.substr(pos);
	}
	else {
		fileName = filePath;
	}

	// 从文件中读取内容
	FILE* file;
	errno_t err = fopen_s(&file, filePath.c_str(), "rb");
	if (err || !file) {
		cout << "ERROR: Cannot find outer file!" << endl;
		return;
	}

	myFileSystem._fcreat(fileName);
	if (myFileSystem._fopen(fileName)) {
		// 如果打开成功，读入文件中全部内容

		uint8_t* content = new(nothrow)uint8_t[BLOCK_SIZE]{};
		if (!content) {
			exit(OVERFLOW);
		}

		int actualLength = fread(content, 1, BLOCK_SIZE, file);
		while (actualLength > 0) {
			myFileSystem._fwrite(actualLength, content);
			actualLength = fread(content, 1, BLOCK_SIZE, file);
		}

		delete[] content;
		fclose(file);
		myFileSystem._fclose();
	}
}

void Controller::_export(string fileName)
{
	/*string fileName;
	cin >> fileName;*/
	if (myFileSystem._fopen(fileName)) {
		FILE* file;
		errno_t err = fopen_s(&file, fileName.c_str(), "ab");
		if (err || !file) {
			cout << "ERROR: Cannot create outer file!" << endl;
			return;
		}

		uint8_t* content = new(nothrow)uint8_t[BLOCK_SIZE]{};
		if (!content) {
			exit(OVERFLOW);
		}

		int totalLength = 0;
		int actualLength = myFileSystem._fread(BLOCK_SIZE, content);
		while (actualLength > 0) {
			fwrite(content, 1, actualLength, file);
			totalLength += actualLength;
			actualLength = myFileSystem._fread(actualLength, content);
		}

		delete[] content;
		fclose(file);
		myFileSystem._fclose();
	}
}

void Controller::_size(string fileName)
{
	/*string fileName;
	cin >> fileName;*/
	int size = myFileSystem._size(fileName);
	if (size >= 0) {
		cout << "Size of " << fileName << ": " << size << " bytes." << endl;
	}
}

void Controller::_rename(string oldName, string newName)
{
	/*string oldName, newName;
	cin >> oldName >> newName;*/
	if (newName.find('\\') != -1 || newName.find('/') != -1 || newName.find(':') != -1
		|| newName.find('*') != -1 || newName.find('?') != -1 || newName.find('\"') != -1
		|| newName.find('<') != -1 || newName.find('>') != -1 || newName.find('|') != -1) {
		cout << "Illegal new file name! includes: \\ / : * ? \" < > | " << endl;
		return;
	}
	myFileSystem._rename(oldName, newName);
}
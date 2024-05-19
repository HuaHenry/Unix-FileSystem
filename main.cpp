#include "Controller.h"
#include "includes.h"
#include "help.h"

/* 一些简单的工具函数 */

// 打印系统头部信息
void printinfo() {
	cout << endl;
	cout << " ███████╗██╗██╗     ███████╗███████╗██╗   ██╗███████╗████████╗███████╗███╗   ███╗" << endl;
	cout << " ██╔════╝██║██║     ██╔════╝██╔════╝╚██╗ ██╔╝██╔════╝╚══██╔══╝██╔════╝████╗ ████║" << endl;
	cout << " █████╗  ██║██║     █████╗  ███████╗ ╚████╔╝ ███████╗   ██║   █████╗  ██╔████╔██║" << endl;
	cout << " ██╔══╝  ██║██║     ██╔══╝  ╚════██║  ╚██╔╝  ╚════██║   ██║   ██╔══╝  ██║╚██╔╝██║  ┏┓      ┳┳  *  " << endl;
	cout << " ██║     ██║███████╗███████╗███████║   ██║   ███████║   ██║   ███████╗██║ ╚═╝ ██║  ┣ ┏┓┏┓  ┃┃┏┓┓┓┏" << endl;
	cout << " ╚═╝     ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝   ╚══════╝   ╚═╝   ╚══════╝╚═╝     ╚═╝  ┻ ┗┛┛   ┗┛┛┗┗┛┗" << endl;
	cout << endl << " -------------------------------- 2151127 华洲琦 --------------------------------" << endl;
}

// 彩色文字函数
void COLOR_PRINT(const char* s, int color)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf(s);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
}

/* HELP 部分的输出打印 */
void printHELP() {
	cout << endl << "查阅具体命令的使用，请输入 \"help 命令名\"" << endl;
	cout << "help				使用手册" << endl;
	cout << "cd				改变当前路径 (支持.和..)" << endl;
	cout << "ls				显示当前目录下的所有文件和文件夹" << endl;
	cout << "mkdir				创建目录" << endl;
	cout << "fread				读取文件内容（至控制台或其他文件）" << endl;
	cout << "fwrite				写入文件内容（至控制台或其他文件）" << endl;
	cout << "open				打开一个文件" << endl;
	cout << "close				关闭一个文件" << endl;
	cout << "create				创建一个文件" << endl;
	cout << "seek				更改文件指针" << endl;
	cout << "delete				删除文件或文件夹" << endl;
	cout << "import				将外部文件引入当前目录" << endl;
	cout << "export				将当前目录文件导出到外部" << endl;
	cout << "size				展示文件大小" << endl;
	cout << "rename				重命名文件" << endl;
	cout << "format				格式化文件卷" << endl;
	cout << "exit				退出系统" << endl;
	cout << endl;
	cout << endl;
}

// 输入划分和处理
vector<string> split(string str)
{
	string buf;
	stringstream ss(str);
	vector<string> v;
	// 字符流ss 
	while (ss >> buf) {
		//转小写
		transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
		v.push_back(buf);
	}
	
	return v;
}

int main()
{
	Controller myManager;
	string demand;
	
	// 打印系统初始信息
	printinfo();
	
	while (true) {
		COLOR_PRINT("\nFS ", 14);
		cout << myManager.path.substr(0, myManager.path.size() - 1);
		if (myManager.fileName != "") {
			cout << '(' << myManager.fileName << ')';
		}
		cout << "> ";

		// 处理输入
		getline(cin, demand);
		vector<string> demand_vector = split(demand);

		// 选择指令
		if (demand_vector.empty())
			continue;
		else {
			string instruction = demand_vector[0];
			if (instruction == "format")
				myManager._fformat();
			else if (instruction == "ls")
				myManager._ls();
			else if (instruction == "mkdir") {
				if (demand_vector.size() != 2)
					cout << "mkdir：命令语法不正确，正确语法为：[mkdir 文件夹名] " << endl;
				else myManager._mkdir(demand_vector[1].c_str());
			}
			else if (instruction == "create") {
				if (demand_vector.size() != 2)
					cout << "create：命令语法不正确，正确语法为：[create 文件名] " << endl;
				else myManager._fcreat(demand_vector[1].c_str());
			}
			else if (instruction == "open") {
				if (demand_vector.size() != 2)
					cout << "open：命令语法不正确，正确语法为：[open 文件名] " << endl;
				else myManager._fopen(demand_vector[1].c_str());
			}
			else if (instruction == "close")
				myManager._fclose();
			else if (instruction == "fread") {
				if (demand_vector.size() != 3)
					cout << "fread：命令语法不正确，正确语法为：[fread 读出大小 读出路径（*表示控制台）] " << endl;
				else myManager._fread(demand_vector[1].c_str(),demand_vector[2].c_str());
			}
			else if (instruction == "fwrite") {
				if (demand_vector.size() != 3)
					cout << "fwrite：命令语法不正确，正确语法为：[fwrite 写入大小 写入路径（*表示控制台）] " << endl;
				else myManager._fwrite(demand_vector[1].c_str(), demand_vector[2].c_str());
			}
			else if (instruction == "seek") {
				if (demand_vector.size() != 2) {
					cout << "seek：命令语法不正确，正确语法为：[seek 指针新位置] " << endl;
				}
				else myManager._flseek(demand_vector[1].c_str());
			}
			else if (instruction == "delete") {
				if (demand_vector.size() != 2) {
					cout << "delete：命令语法不正确，正确语法为：[delete 文件（夹）名] " << endl;
				}
				else myManager._fdelete(demand_vector[1].c_str());
			}
			else if (instruction == "cd") {
				if (demand_vector.size() != 2) {
					cout << "cd：命令语法不正确，正确语法为：[cd 新路径] " << endl;
				}
				else myManager._cd(demand_vector[1].c_str());
			}
			else if (instruction == "import") {
				if (demand_vector.size() != 2) {
					cout << "import：命令语法不正确，正确语法为：[import 外部文件路径] " << endl;
				}
				else myManager._import(demand_vector[1].c_str());
			}
			else if (instruction == "export") {
				if (demand_vector.size() != 2) {
					cout << "export：命令语法不正确，正确语法为：[export 文件路径] " << endl;
				}
				else myManager._export(demand_vector[1].c_str());
			}
			else if (instruction == "size")
				if (demand_vector.size() != 2) {
					cout << "size：命令语法不正确，正确语法为：[size 文件名] " << endl;
				}
				else myManager._size(demand_vector[1].c_str());
			else if (instruction == "rename") {
				if (demand_vector.size() != 3) {
					cout << "rename：命令语法不正确，正确语法为：[rename 原文件名 新文件名] " << endl;
				}
				else myManager._rename(demand_vector[1].c_str(), demand_vector[2].c_str());
			}
			else if (instruction == "help") {
				if (demand_vector.size() == 2) {
					help_controller(demand_vector[1].c_str());
				}
				else if (demand_vector.size() == 1) printHELP();
				else cout << "help：命令语法不正确，正确语法为：[help (指令名)] " << endl;
			}
			else if (instruction == "exit")
				break;
			else {
				cout << "[Error]: 无法识别的命令 " << instruction << endl;
				cout << "请使用 \'help\' 命令查看使用手册" << endl;
			}
		}
	}
	cout << endl << "文件系统已安全退出，可以关闭窗口！" << endl;
	return 0;
}

//void printHelp()
//{
//	cout << "--------------------------------------------------------------" << endl;
//	cout << "format ----------- format your disk" << endl;
//	cout << "--- params: none" << endl;
//	cout << "ls --------------- show all dirs and files under current path" << endl;
//	cout << "--- params: none" << endl;
//	cout << "mkdir ------------ make dir under current dir" << endl;
//	cout << "--- params: [-s] - path name ( except \\/:*?\"<>| )" << endl;
//	cout << "create ----------- create new file under current path" << endl;
//	cout << "--- params: [-s] - file name ( except \\/:*?\"<>| )" << endl;
//	cout << "open ------------- open a file under current path" << endl;
//	cout << "--- params: [-s] - file name" << endl;
//	cout << "close ------------ close current file" << endl;
//	cout << "--- params: none" << endl;
//	cout << "fread ------------ read from file (file and cmd supported)" << endl;
//	cout << "--- params: [-n] - size, 0 for full length (cmd 1024B)" << endl;
//	cout << "            [-s] - export file path, * for cmd" << endl;
//	cout << "fwrite ----------- write to file (file and cmd supported)" << endl;
//	cout << "--- params: [-n] - size, 0 for full length (cmd 1024B)" << endl;
//	cout << "            [-s] - import file path, * for cmd" << endl;
//	cout << "seek ------------- change current file pointer (from begin)" << endl;
//	cout << "--- params: [-n] - size, from the beginning, not negative" << endl;
//	cout << "delete ----------- delete a file or dir under current path" << endl;
//	cout << "--- params: [-s] - file or dir name" << endl;
//	cout << "cd --------------- change current path (. and .. supported)" << endl;
//	cout << "--- params: [-s] - new path, . for current and .. for upper" << endl;
//	cout << "import ----------- import outer file to current path" << endl;
//	cout << "--- params: [-s] - outer file path" << endl;
//	cout << "export ----------- export file from current path to outer path" << endl;
//	cout << "--- params: [-s] - inner file name" << endl;
//	cout << "size ------------- show file size (dir file will not sum up)" << endl;
//	cout << "--- params: [-s] - file name" << endl;
//	cout << "rename ----------- rename file under current path" << endl;
//	cout << "--- params: [-s] - old file name" << endl;
//	cout << "--- params: [-s] - new file name" << endl;
//	cout << "--------------------------------------------------------------" << endl;
//}
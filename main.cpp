#include "Controller.h"
#include "includes.h"
#include "help.h"

/* һЩ�򵥵Ĺ��ߺ��� */

// ��ӡϵͳͷ����Ϣ
void printinfo() {
	cout << endl;
	cout << " ���������������[�����[�����[     ���������������[���������������[�����[   �����[���������������[�����������������[���������������[�������[   �������[" << endl;
	cout << " �����X�T�T�T�T�a�����U�����U     �����X�T�T�T�T�a�����X�T�T�T�T�a�^�����[ �����X�a�����X�T�T�T�T�a�^�T�T�����X�T�T�a�����X�T�T�T�T�a���������[ ���������U" << endl;
	cout << " �����������[  �����U�����U     �����������[  ���������������[ �^���������X�a ���������������[   �����U   �����������[  �����X���������X�����U" << endl;
	cout << " �����X�T�T�a  �����U�����U     �����X�T�T�a  �^�T�T�T�T�����U  �^�����X�a  �^�T�T�T�T�����U   �����U   �����X�T�T�a  �����U�^�����X�a�����U  ����      �ש�  *  " << endl;
	cout << " �����U     �����U���������������[���������������[���������������U   �����U   ���������������U   �����U   ���������������[�����U �^�T�a �����U  �� ��������  ��������������" << endl;
	cout << " �^�T�a     �^�T�a�^�T�T�T�T�T�T�a�^�T�T�T�T�T�T�a�^�T�T�T�T�T�T�a   �^�T�a   �^�T�T�T�T�T�T�a   �^�T�a   �^�T�T�T�T�T�T�a�^�T�a     �^�T�a  �� ������   ��������������" << endl;
	cout << endl << " -------------------------------- 2151127 ������ --------------------------------" << endl;
}

// ��ɫ���ֺ���
void COLOR_PRINT(const char* s, int color)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf(s);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
}

/* HELP ���ֵ������ӡ */
void printHELP() {
	cout << endl << "���ľ��������ʹ�ã������� \"help ������\"" << endl;
	cout << "help				ʹ���ֲ�" << endl;
	cout << "cd				�ı䵱ǰ·�� (֧��.��..)" << endl;
	cout << "ls				��ʾ��ǰĿ¼�µ������ļ����ļ���" << endl;
	cout << "mkdir				����Ŀ¼" << endl;
	cout << "fread				��ȡ�ļ����ݣ�������̨�������ļ���" << endl;
	cout << "fwrite				д���ļ����ݣ�������̨�������ļ���" << endl;
	cout << "open				��һ���ļ�" << endl;
	cout << "close				�ر�һ���ļ�" << endl;
	cout << "create				����һ���ļ�" << endl;
	cout << "seek				�����ļ�ָ��" << endl;
	cout << "delete				ɾ���ļ����ļ���" << endl;
	cout << "import				���ⲿ�ļ����뵱ǰĿ¼" << endl;
	cout << "export				����ǰĿ¼�ļ��������ⲿ" << endl;
	cout << "size				չʾ�ļ���С" << endl;
	cout << "rename				�������ļ�" << endl;
	cout << "format				��ʽ���ļ���" << endl;
	cout << "exit				�˳�ϵͳ" << endl;
	cout << endl;
	cout << endl;
}

// ���뻮�ֺʹ���
vector<string> split(string str)
{
	string buf;
	stringstream ss(str);
	vector<string> v;
	// �ַ���ss 
	while (ss >> buf) {
		//תСд
		transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
		v.push_back(buf);
	}
	
	return v;
}

int main()
{
	Controller myManager;
	string demand;
	
	// ��ӡϵͳ��ʼ��Ϣ
	printinfo();
	
	while (true) {
		COLOR_PRINT("\nFS ", 14);
		cout << myManager.path.substr(0, myManager.path.size() - 1);
		if (myManager.fileName != "") {
			cout << '(' << myManager.fileName << ')';
		}
		cout << "> ";

		// ��������
		getline(cin, demand);
		vector<string> demand_vector = split(demand);

		// ѡ��ָ��
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
					cout << "mkdir�������﷨����ȷ����ȷ�﷨Ϊ��[mkdir �ļ�����] " << endl;
				else myManager._mkdir(demand_vector[1].c_str());
			}
			else if (instruction == "create") {
				if (demand_vector.size() != 2)
					cout << "create�������﷨����ȷ����ȷ�﷨Ϊ��[create �ļ���] " << endl;
				else myManager._fcreat(demand_vector[1].c_str());
			}
			else if (instruction == "open") {
				if (demand_vector.size() != 2)
					cout << "open�������﷨����ȷ����ȷ�﷨Ϊ��[open �ļ���] " << endl;
				else myManager._fopen(demand_vector[1].c_str());
			}
			else if (instruction == "close")
				myManager._fclose();
			else if (instruction == "fread") {
				if (demand_vector.size() != 3)
					cout << "fread�������﷨����ȷ����ȷ�﷨Ϊ��[fread ������С ����·����*��ʾ����̨��] " << endl;
				else myManager._fread(demand_vector[1].c_str(),demand_vector[2].c_str());
			}
			else if (instruction == "fwrite") {
				if (demand_vector.size() != 3)
					cout << "fwrite�������﷨����ȷ����ȷ�﷨Ϊ��[fwrite д���С д��·����*��ʾ����̨��] " << endl;
				else myManager._fwrite(demand_vector[1].c_str(), demand_vector[2].c_str());
			}
			else if (instruction == "seek") {
				if (demand_vector.size() != 2) {
					cout << "seek�������﷨����ȷ����ȷ�﷨Ϊ��[seek ָ����λ��] " << endl;
				}
				else myManager._flseek(demand_vector[1].c_str());
			}
			else if (instruction == "delete") {
				if (demand_vector.size() != 2) {
					cout << "delete�������﷨����ȷ����ȷ�﷨Ϊ��[delete �ļ����У���] " << endl;
				}
				else myManager._fdelete(demand_vector[1].c_str());
			}
			else if (instruction == "cd") {
				if (demand_vector.size() != 2) {
					cout << "cd�������﷨����ȷ����ȷ�﷨Ϊ��[cd ��·��] " << endl;
				}
				else myManager._cd(demand_vector[1].c_str());
			}
			else if (instruction == "import") {
				if (demand_vector.size() != 2) {
					cout << "import�������﷨����ȷ����ȷ�﷨Ϊ��[import �ⲿ�ļ�·��] " << endl;
				}
				else myManager._import(demand_vector[1].c_str());
			}
			else if (instruction == "export") {
				if (demand_vector.size() != 2) {
					cout << "export�������﷨����ȷ����ȷ�﷨Ϊ��[export �ļ�·��] " << endl;
				}
				else myManager._export(demand_vector[1].c_str());
			}
			else if (instruction == "size")
				if (demand_vector.size() != 2) {
					cout << "size�������﷨����ȷ����ȷ�﷨Ϊ��[size �ļ���] " << endl;
				}
				else myManager._size(demand_vector[1].c_str());
			else if (instruction == "rename") {
				if (demand_vector.size() != 3) {
					cout << "rename�������﷨����ȷ����ȷ�﷨Ϊ��[rename ԭ�ļ��� ���ļ���] " << endl;
				}
				else myManager._rename(demand_vector[1].c_str(), demand_vector[2].c_str());
			}
			else if (instruction == "help") {
				if (demand_vector.size() == 2) {
					help_controller(demand_vector[1].c_str());
				}
				else if (demand_vector.size() == 1) printHELP();
				else cout << "help�������﷨����ȷ����ȷ�﷨Ϊ��[help (ָ����)] " << endl;
			}
			else if (instruction == "exit")
				break;
			else {
				cout << "[Error]: �޷�ʶ������� " << instruction << endl;
				cout << "��ʹ�� \'help\' ����鿴ʹ���ֲ�" << endl;
			}
		}
	}
	cout << endl << "�ļ�ϵͳ�Ѱ�ȫ�˳������Թرմ��ڣ�" << endl;
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
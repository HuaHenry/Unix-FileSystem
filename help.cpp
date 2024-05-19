#pragma once

#include "includes.h"

// ��ʽ������
void help_format() {
	cout << "\t[format] ��ʽ���ļ���" << endl;
}

// ��ʾ�ļ�
void help_ls() {
	cout << "\t[ls] ��ʾ��ǰ�ļ�������" << endl;
}

// �����ļ���
void help_mkdir() {
	cout << "\t[mkdir �ļ�����] �������ļ���" << endl;
}

// �������ļ�
void help_create() {
	cout << "\t[create �ļ���] �������ļ�" << endl;
}

// ���ļ�
void help_open() {
	cout << "\t[open �ļ���] ���ļ�" << endl;
}

// �ر��ļ�
void help_close() {
	cout << "\t[close] �ر��ļ�" << endl;
}

// �����ļ�����
void help_fread() {
	cout << "\t[fread ������С ����·����*��ʾ����̨��] ��������������̨�������ļ���" << endl;
}

// д���ļ�����
void help_fwrite() {
	cout << "\t[fwrite д���С д��·����*��ʾ����̨��] �ӿ���̨�������ļ���д����������ǰ���ļ�" << endl;
}

// �ı��ļ�ָ��λ��
void help_seek() {
	cout << "\t[seek ָ����λ��] �ı��ļ�ָ��λ��" << endl;
}

// ɾ���ļ�
void help_delete() {
	cout << "\t[delete �ļ����У���] ɾ���ļ�/�ļ���" << endl;
}

// ��ת·��
void help_cd() {
	cout << "\t[cd ��·��] ��ת����·��" << endl;
}

// �����ⲿ�ļ�
void help_import() {
	cout << "\t[import �ⲿ�ļ�·��] ���ⲿ�ļ������ļ�ϵͳ�ĵ�ǰĿ¼" << endl;
}

// �����ļ����ⲿ
void help_export() {
	cout << "\t[export �ļ�·��] ��ϵͳ���ļ��������ⲿ" << endl;
}

// �����ļ���С
void help_size() {
	cout << "\t[size �ļ���] �����ļ���С" << endl;
}

// �������ļ�
void help_rename() {
	cout << "\t[rename ԭ�ļ��� ���ļ���] �������ļ�" << endl;
}

// ���� help ָ������
void help_controller(string param) {
	if (param == "format") help_format();
	else if (param == "ls") help_ls();
	else if (param == "mkdir") help_mkdir();
	else if (param == "create") help_create();
	else if (param == "open") help_open();
	else if (param == "close") help_close();
	else if (param == "fread") help_fread();
	else if (param == "fwrite") help_fwrite();
	else if (param == "seek") help_seek();
	else if (param == "delete") help_delete();
	else if (param == "cd") help_cd();
	else if (param == "import") help_import();
	else if (param == "export") help_export();
	else if (param == "size") help_size();
	else if (param == "rename") help_rename();
	else {
		cout << "�޿��õ�ָ��˵��" << endl;
	}
}












#pragma once

#include "includes.h"

// 格式化磁盘
void help_format() {
	cout << "\t[format] 格式化文件卷" << endl;
}

// 显示文件
void help_ls() {
	cout << "\t[ls] 显示当前文件夹内容" << endl;
}

// 创建文件夹
void help_mkdir() {
	cout << "\t[mkdir 文件夹名] 创建新文件夹" << endl;
}

// 创建新文件
void help_create() {
	cout << "\t[create 文件名] 创建新文件" << endl;
}

// 打开文件
void help_open() {
	cout << "\t[open 文件名] 打开文件" << endl;
}

// 关闭文件
void help_close() {
	cout << "\t[close] 关闭文件" << endl;
}

// 读出文件内容
void help_fread() {
	cout << "\t[fread 读出大小 读出路径（*表示控制台）] 读出内容至控制台或其他文件中" << endl;
}

// 写入文件内容
void help_fwrite() {
	cout << "\t[fwrite 写入大小 写入路径（*表示控制台）] 从控制台或其他文件中写入内容至当前打开文件" << endl;
}

// 改变文件指针位置
void help_seek() {
	cout << "\t[seek 指针新位置] 改变文件指针位置" << endl;
}

// 删除文件
void help_delete() {
	cout << "\t[delete 文件（夹）名] 删除文件/文件夹" << endl;
}

// 跳转路径
void help_cd() {
	cout << "\t[cd 新路径] 跳转至新路径" << endl;
}

// 导入外部文件
void help_import() {
	cout << "\t[import 外部文件路径] 将外部文件导入文件系统的当前目录" << endl;
}

// 导出文件至外部
void help_export() {
	cout << "\t[export 文件路径] 将系统内文件导出至外部" << endl;
}

// 计算文件大小
void help_size() {
	cout << "\t[size 文件名] 计算文件大小" << endl;
}

// 重命名文件
void help_rename() {
	cout << "\t[rename 原文件名 新文件名] 重命名文件" << endl;
}

// 控制 help 指令的输出
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
		cout << "无可用的指令说明" << endl;
	}
}












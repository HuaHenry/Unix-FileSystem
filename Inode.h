#pragma once
#include "Buffer.h"
#include "includes.h"

// 内存中的索引节点，记录文件的相关信息
class Inode{
public:
	enum INodeFlag      // 状态标志位，记录内存inode状态
	{
		IUPD = 0x2,		// 内存Inode已经修改标志
		IDIR = 0x8,		// 当前Inode为目录标志
		ITEXT = 0x20	// 当前Inode为正文标志
	};
	static const int DISKINODE_SIZE = 64;                                   // Inode大小
	static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);	// 每个索引表项的大小
    // 对于不同大小文件使用不同级别的索引表（设置访问的逻辑块号的最大值）
	static const int SMALL_FILE_BLOCK = 6;	                                // 小型文件：直接索引表最多可寻址的逻辑块号
	static const int LARGE_FILE_BLOCK = 128 * 2 + 6;	                    // 大型文件：经一次间接索引表最多可寻址的逻辑块号
	static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;	        // 巨型文件：经二次间接索引最大可寻址文件逻辑块号
public:
	unsigned int	i_mode;	        // 内存inode是否已经被修改
	int		        i_number;		// 外存inode区中的编号
	int		        i_size;			// 文件大小，字节为单位
	int		        i_addr[10];		// 用于文件逻辑块号和物理块号转换的基本索引表
};

// 外存Inode，和内存Inode
class DiskInode{
public:
	unsigned int d_mode;	        // 状态标志位
	int		d_size;			        // 文件大小（单位为字节）
	int		d_addr[10];		        // 用于文件逻辑块号和物理块号转换的基本索引表
	int		padding[4] = {};        // 填充 -> 64字节
};

class SuperBlock{
public:
	//-------------------- Block-1 -------------------- //
    int		s_isize;		// 外存Inode盘块数
	int		s_fsize;		// 盘块总数
	int		s_nfree;		// 直接管理的空闲盘块数
	int		s_free[100];	// 直接管理的空闲盘块索引表
	int     pad_blk1[25];	// 字节填充

    //-------------------- Block-2 -------------------- //
    int		s_ninode;		// 直接管理的空闲外存Inode数
	int		s_inode[100];	// 直接管理的空闲外存Inode索引表
	int		pad_blk2[27];	// 字节填充
	int		s_fmod;			// 内存中SPB副本被修改标志，即需要更新外存
};

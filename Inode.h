#pragma once
#include "Buffer.h"
#include "includes.h"

// �ڴ��е������ڵ㣬��¼�ļ��������Ϣ
class Inode{
public:
	enum INodeFlag      // ״̬��־λ����¼�ڴ�inode״̬
	{
		IUPD = 0x2,		// �ڴ�Inode�Ѿ��޸ı�־
		IDIR = 0x8,		// ��ǰInodeΪĿ¼��־
		ITEXT = 0x20	// ��ǰInodeΪ���ı�־
	};
	static const int DISKINODE_SIZE = 64;                                   // Inode��С
	static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);	// ÿ����������Ĵ�С
    // ���ڲ�ͬ��С�ļ�ʹ�ò�ͬ��������������÷��ʵ��߼���ŵ����ֵ��
	static const int SMALL_FILE_BLOCK = 6;	                                // С���ļ���ֱ������������Ѱַ���߼����
	static const int LARGE_FILE_BLOCK = 128 * 2 + 6;	                    // �����ļ�����һ�μ������������Ѱַ���߼����
	static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;	        // �����ļ��������μ����������Ѱַ�ļ��߼����
public:
	unsigned int	i_mode;	        // �ڴ�inode�Ƿ��Ѿ����޸�
	int		        i_number;		// ���inode���еı��
	int		        i_size;			// �ļ���С���ֽ�Ϊ��λ
	int		        i_addr[10];		// �����ļ��߼���ź�������ת���Ļ���������
};

// ���Inode�����ڴ�Inode
class DiskInode{
public:
	unsigned int d_mode;	        // ״̬��־λ
	int		d_size;			        // �ļ���С����λΪ�ֽڣ�
	int		d_addr[10];		        // �����ļ��߼���ź�������ת���Ļ���������
	int		padding[4] = {};        // ��� -> 64�ֽ�
};

class SuperBlock{
public:
	//-------------------- Block-1 -------------------- //
    int		s_isize;		// ���Inode�̿���
	int		s_fsize;		// �̿�����
	int		s_nfree;		// ֱ�ӹ���Ŀ����̿���
	int		s_free[100];	// ֱ�ӹ���Ŀ����̿�������
	int     pad_blk1[25];	// �ֽ����

    //-------------------- Block-2 -------------------- //
    int		s_ninode;		// ֱ�ӹ���Ŀ������Inode��
	int		s_inode[100];	// ֱ�ӹ���Ŀ������Inode������
	int		pad_blk2[27];	// �ֽ����
	int		s_fmod;			// �ڴ���SPB�������޸ı�־������Ҫ�������
};

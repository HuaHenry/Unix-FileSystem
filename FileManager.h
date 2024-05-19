#pragma once
#include "includes.h"
#include "Inode.h"
#include "BufferManager.h"
#include "BlockManager.h"

// �ļ�·����أ��ļ���Ϣ����FileManagerά��
class FileManager
{
private:
	Inode pathInode;            // ��ǰ·�����ڴ�Inode���
	Inode inode;	            // ��ǰ���ļ����ڴ�Inode���

	BufferManager* m_BufferManager = nullptr;
	BlockManager* m_BlockManager = nullptr;

	int bufferBlkno;
	uint8_t buffer[BLOCK_SIZE];
	int fileOffset = 0;		    // ��ǰ���ļ���ƫ����
	vector<string> paths;	    // ��¼��ǰ����·��

	static const int FILE_ITEM_BYTES = 32;

private:
	// �ڵ�ǰ�ļ��ĵ�ǰƫ�����Ͻ�����Ϊsize�ֽڵ�����contentд�����
	void write(int size, uint8_t* content, Inode& inode);
	// �ڵ�ǰ�ļ��ĵ�ǰƫ�����ϴӴ��̶�����Ϊsize�ֽڵ����ݴ浽content������ʵ�ʶ�ȡ���ֽ���
	int read(int size, uint8_t* content, Inode& inode);

	// ֱ�Ӵ�i_addr�����̿飬�ǵ�i_addr�������̿��Ϊ0ʱ����
	int allocateBlockFromIADDR(int idx, Inode& inode);
	// ���̿�����̿飬�ǵ������̿�block�������̿��Ϊ0ʱ����
	int allocateBlockFromBlock(int idx);

	int blknoTransform(int logic, Inode& inode); // ִ���߼�����������ŵ�ת��(Bmap)

	Inode readInode(int inodeNo);
	void writeInode(Inode inode);

	// �ڵ�ǰĿ¼�ļ��в��Ҹ��ļ���������������̿���գ�������Inode�������ڷ���false
	bool deleteFile(Inode inode);
	// ɾ����ǰĿ¼�������ļ�����������Ҫ���ǵݹ�
	bool deleteDirectory(Inode inode);

	// ����һ��Ŀ¼�ļ�������name��Ӧ������ظ�������
	// ���nameΪ�մ��������ڲ����Ƿ���֮ǰ��ɾ�������������д���µ�Ŀ¼��
	int findDirItem(string name);
public:
	void init(BufferManager*, BlockManager*);

	// ��paths�������Ϸ���
	string renewPath();

	void setFileOffset(int offset);

	void writeToFile(int size, uint8_t* content);
	int readFromFile(int size, uint8_t* content);

	void flush();
	void destroy();

	// ��inode�Ŷ�Ӧ��Inode���벢����
	bool renewFileInode(int inodeNo);
	
	// ���pathInode���в��������޸�inode������������������򴴽�ʧ�ܣ�����false��
	// ����ʱ���ȵ���searchItemInDirectory�������в���
	bool createItem(string fileName, bool isDir);

	// ����Ŀ¼�ļ���ls������
	vector<string> directoryTraverse();	
	// ��Ŀ¼�ļ��в��ұ�����ض�Ӧ��inode���ţ���û���򷵻�-1
	int searchItemInDirectory(string itemName);	

	// �ж����ļ������������ֱ���������������������ǰĿ¼�¶�Ӧ�ı�����Ҫɾ�������ÿմ���
	bool deleteItem(string name);

	// �����µ�Ŀ¼����һ������һ����
	string changePath(string path);

	// ��ȡ��ǰĿ¼�¸��ļ��Ĵ�С��search���ҵ����ȡ��Ӧinode���鿴i_size��
	int getFileSize(string name);
	// ��������search���ҵ���ͨ��ƫ�����޸�ǰ28�ֽڣ�
	void renameFile(string oldName, string newName);
};

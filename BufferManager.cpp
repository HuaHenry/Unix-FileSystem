#include "BufferManager.h"

BufferManager::BufferManager()
{
	// ������������ƿ����Ӧ����������
	for (int i = 0; i < NBUF; ++i){
		m_Buf[i].b_addr = Buffers[i];
	}

	// �����л����ǰ������ȫ���������ɶ�����
	bufferList.av_back = &m_Buf[0];
	bufferList.av_forw = &m_Buf[NBUF - 1];
	m_Buf[0].av_forw = &bufferList;
	m_Buf[NBUF - 1].av_back = &bufferList;
	for (int i = 1; i < NBUF; ++i) {
		m_Buf[i].av_forw = &m_Buf[i - 1];
	}
	for (int i = 0; i < NBUF - 1; ++i) {
		m_Buf[i].av_back = &m_Buf[i + 1];
	}

	// ����������ȫ����ֵΪ0
	for (int i = 0; i < NBUF; ++i)
		*(Buffers[i]) = {0};
}

void BufferManager::destroy()
{
	// �������������̿�ȫ��д�����
	for (int i = 0; i < NBUF; ++i) {
		// ��busy��־˵����ռ�ã����µ�����д�����
		// ���ӳ�д��־Ҳ��Ҫд�ش���
		if (m_Buf[i].b_flags & (m_Buf[i].B_BUSY | m_Buf[i].B_DELWRI)) {
			Bwrite(&m_Buf[i]);
		}
	}
}

void BufferManager::moveBlk2Rear(Buffer* bp)
{
	// ����Ӷ�����ժ��
	if(bp->av_back)
		bp->av_back->av_forw = bp->av_forw;
	if(bp->av_forw)
		bp->av_forw->av_back = bp->av_back;
	// ���������β
	bp->av_forw = bufferList.av_forw;
	bp->av_back = &bufferList;
	bufferList.av_forw->av_back = bp;
	bufferList.av_forw = bp;
}

Buffer* BufferManager::searchFreeBlk()
{
	// �ڶ����д�ǰ������ң��ҵ��ĵ�һ������B_BUSYλ�ļ����п�
	for (Buffer* buf = bufferList.av_back; buf != &bufferList; buf = buf->av_back) {
		if ((buf->b_flags & buf->B_BUSY) == 0)
			return buf;
	}
	return nullptr; // û���ҵ�����ȫ����ռ�ã��򷵻�nullptr
}

Buffer* BufferManager::GetBlk(int blkno)
{
	Buffer* buf = nullptr;
	// �ж���һ���ַ����Ƿ��Ѿ��ڻ����У��Ƿ�������ã�
	for (int i = 0; i < NBUF; ++i){
		buf = &m_Buf[i];
		if (buf->b_blkno == blkno) {
			// ˵����һ���ַ����Ѿ��ڻ�����
			// ��Ϊֻ��һ�����̣����Բ���Ҫ����B_BUSY��־λ
			moveBlk2Rear(buf);				// ����黺���ŵ���β
			return buf;
		}
	}
	// ��һ���ַ��鲻�ڻ����У���Ҫ����һ�黺��飬����Ӧ�ַ���Ӵ����л���
	// ������һ����еĻ����
	buf = searchFreeBlk();
	if (buf && (buf->b_flags & buf->B_DELWRI)) { // ˵���ҵ�һ����У�������DELWRI��־����ʱ����д�����
		Bwrite(buf);
	}
	else if (buf == nullptr) {
		// ˵��û���ҵ����У������ǵ����̣���ʹ��sleepҲ���ܵȵ�����
		// ��˴˴�ֱ����ռһ����̣�Ҳʹ��LRU�㷨�����Ӷ���ȡ��һ�飩
		buf = bufferList.av_back;
		Bwrite(buf);
	}
	// ˵���ҵ�һ�������û����ʱд��־������ֱ��ʹ��
	buf->b_flags = 0;	// �����ˢ��
	buf->b_blkno = blkno;
	moveBlk2Rear(buf);	// ����黺���ŵ���β
	return buf;
}

Buffer* BufferManager::Bread(int blkno)
{
	Buffer* bp = GetBlk(blkno);
	if (bp->b_flags & bp->B_BUSY) {
		// ��BUSYλ��˵�������Ѿ����룬���������ֱ�ӷ���
		return bp;
	}
	else { // ����BUSYλ��˵���Ǹշ���Ļ���飬��Ҫ�ȶ�ȡһ�´�����Ϣ
		if (!myDisk.readDisk(bp->b_blkno, bp->b_addr)) {
			cout << "ERROR: Read disk failed! Block number: " << bp->b_blkno << endl;
			exit(EXIT_FAILURE);
		}
		bp->b_flags |= bp->B_BUSY;	// ��ȡ��ϣ���BUSYλ
		return bp;
	}
	// ������Bread����Bwrite������Ҫ�ȴӴ����ж�ȡ��Ϣ����˷���ʱ����BUSYλ
}

void BufferManager::Bwrite(Buffer* bp)
{
	bp->b_wcount = BLOCK_SIZE;
	if (!myDisk.writeDisk(bp->b_blkno, bp->b_addr)) {
		cout << "ERROR: Write disk failed! Block number: " << bp->b_blkno << endl;
		exit(EXIT_FAILURE);
	}
	bp->b_flags = 0;	// д������ɺ󣬻����ˢ��
}

bool BufferManager::write(uint8_t* content, int blkno, int offset, int length)
{
	if (offset + length > BLOCK_SIZE) {
		// һ���򵥵Ĳ����ƣ����ܴ���Խ��д���������ȡ��д�룬����false
		return false;
	}
	else {
		Buffer* bp = nullptr;
		if (length == BLOCK_SIZE) {
			// ��Ҫд��ĳ���������������С������Ҫ���룬ֱ�ӷ��伴��
			bp = GetBlk(blkno);
		}
		else {
			// ������Ҫ�ȶ���д
			bp = Bread(blkno);
		}

		if (bp) {
			memcpy(bp->b_addr + offset, content, length);
		}
		else {
			cout << "ERROR: copy to buffer failed!" << endl;
			exit(EXIT_FAILURE);
		}

		if (offset + length == BLOCK_SIZE) {
			// ��ȣ�˵��д���˻����β������ʱ���˿�д�����
			Bwrite(bp);
		}
		else {
			bp->b_flags |= bp->B_DELWRI;	// ���򣬴����ӳ�д��־����������
		}

		return true;
	}
}

int BufferManager::read(uint8_t* holder, int blkno, int offset, int length)
{
	Buffer* bp = Bread(blkno);
	int overflow = offset + length - BLOCK_SIZE;
	int actualLength = overflow > 0 ? overflow : length; // ����ʵ�ʶ�ȡ����

	if (bp) {
		memcpy(holder, bp->b_addr + offset, actualLength);
	}
	else {
		cout << "ERROR: read from buffer failed!" << endl;
		exit(EXIT_FAILURE);
	}

	return actualLength;
}
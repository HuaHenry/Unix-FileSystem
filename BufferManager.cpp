#include "BufferManager.h"

BufferManager::BufferManager()
{
	// 将各个缓存控制块与对应缓冲区相连
	for (int i = 0; i < NBUF; ++i){
		m_Buf[i].b_addr = Buffers[i];
	}

	// 将所有缓冲块前后勾连，全部放至自由队列中
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

	// 缓冲区数组全部赋值为0
	for (int i = 0; i < NBUF; ++i)
		*(Buffers[i]) = {0};
}

void BufferManager::destroy()
{
	// 将缓存中所有盘块全部写入磁盘
	for (int i = 0; i < NBUF; ++i) {
		// 带busy标志说明被占用，将新的内容写入磁盘
		// 带延迟写标志也需要写回磁盘
		if (m_Buf[i].b_flags & (m_Buf[i].B_BUSY | m_Buf[i].B_DELWRI)) {
			Bwrite(&m_Buf[i]);
		}
	}
}

void BufferManager::moveBlk2Rear(Buffer* bp)
{
	// 将其从队列中摘下
	if(bp->av_back)
		bp->av_back->av_forw = bp->av_forw;
	if(bp->av_forw)
		bp->av_forw->av_back = bp->av_back;
	// 将其放至队尾
	bp->av_forw = bufferList.av_forw;
	bp->av_back = &bufferList;
	bufferList.av_forw->av_back = bp;
	bufferList.av_forw = bp;
}

Buffer* BufferManager::searchFreeBlk()
{
	// 在队列中从前往后查找，找到的第一个不带B_BUSY位的即空闲块
	for (Buffer* buf = bufferList.av_back; buf != &bufferList; buf = buf->av_back) {
		if ((buf->b_flags & buf->B_BUSY) == 0)
			return buf;
	}
	return nullptr; // 没有找到，即全部被占用，则返回nullptr
}

Buffer* BufferManager::GetBlk(int blkno)
{
	Buffer* buf = nullptr;
	// 判断这一块字符块是否已经在缓存中（是否可以重用）
	for (int i = 0; i < NBUF; ++i){
		buf = &m_Buf[i];
		if (buf->b_blkno == blkno) {
			// 说明这一块字符块已经在缓存中
			// 因为只有一个进程，所以不需要考虑B_BUSY标志位
			moveBlk2Rear(buf);				// 将这块缓存块放到队尾
			return buf;
		}
	}
	// 这一块字符块不在缓存中，需要申请一块缓存块，将对应字符块从磁盘中换入
	// 首先找一块空闲的缓存块
	buf = searchFreeBlk();
	if (buf && (buf->b_flags & buf->B_DELWRI)) { // 说明找到一块空闲，但带有DELWRI标志，此时将其写入磁盘
		Bwrite(buf);
	}
	else if (buf == nullptr) {
		// 说明没有找到空闲，由于是单进程，即使是sleep也不能等到空闲
		// 因此此处直接抢占一块磁盘（也使用LRU算法，即从队首取下一块）
		buf = bufferList.av_back;
		Bwrite(buf);
	}
	// 说明找到一块空闲且没有延时写标志，可以直接使用
	buf->b_flags = 0;	// 缓存块刷新
	buf->b_blkno = blkno;
	moveBlk2Rear(buf);	// 将这块缓存块放到队尾
	return buf;
}

Buffer* BufferManager::Bread(int blkno)
{
	Buffer* bp = GetBlk(blkno);
	if (bp->b_flags & bp->B_BUSY) {
		// 含BUSY位，说明内容已经换入，无需操作，直接返回
		return bp;
	}
	else { // 不含BUSY位，说明是刚分配的缓存块，需要先读取一下磁盘信息
		if (!myDisk.readDisk(bp->b_blkno, bp->b_addr)) {
			cout << "ERROR: Read disk failed! Block number: " << bp->b_blkno << endl;
			exit(EXIT_FAILURE);
		}
		bp->b_flags |= bp->B_BUSY;	// 读取完毕，置BUSY位
		return bp;
	}
	// 不论是Bread还是Bwrite，都需要先从磁盘中读取信息，因此返回时均有BUSY位
}

void BufferManager::Bwrite(Buffer* bp)
{
	bp->b_wcount = BLOCK_SIZE;
	if (!myDisk.writeDisk(bp->b_blkno, bp->b_addr)) {
		cout << "ERROR: Write disk failed! Block number: " << bp->b_blkno << endl;
		exit(EXIT_FAILURE);
	}
	bp->b_flags = 0;	// 写操作完成后，缓存块刷新
}

bool BufferManager::write(uint8_t* content, int blkno, int offset, int length)
{
	if (offset + length > BLOCK_SIZE) {
		// 一个简单的查错机制：可能存在越界写入的现象，则取消写入，返回false
		return false;
	}
	else {
		Buffer* bp = nullptr;
		if (length == BLOCK_SIZE) {
			// 需要写入的长度是整个缓冲块大小，则不需要读入，直接分配即可
			bp = GetBlk(blkno);
		}
		else {
			// 否则，需要先读后写
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
			// 相等，说明写到了缓存块尾部，此时将此块写入磁盘
			Bwrite(bp);
		}
		else {
			bp->b_flags |= bp->B_DELWRI;	// 否则，打上延迟写标志，不作处理
		}

		return true;
	}
}

int BufferManager::read(uint8_t* holder, int blkno, int offset, int length)
{
	Buffer* bp = Bread(blkno);
	int overflow = offset + length - BLOCK_SIZE;
	int actualLength = overflow > 0 ? overflow : length; // 计算实际读取长度

	if (bp) {
		memcpy(holder, bp->b_addr + offset, actualLength);
	}
	else {
		cout << "ERROR: read from buffer failed!" << endl;
		exit(EXIT_FAILURE);
	}

	return actualLength;
}
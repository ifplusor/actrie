#include <memory.h>
#include <string.h>

#include "acdat.h"


//extern trienode_ptr trie_root;

// Trie 内部接口，仅限 Double-Array Trie 使用
size_t getTrieSize(trie_ptr self);
trienode_ptr getTrieNode(trie_ptr self, size_t index);
#ifdef BINSCH
size_t nextStateByBinary(trie_ptr self, size_t iNode, unsigned char key);
#else
size_t nextState(trie_ptr self, size_t iNode, unsigned char key);
#endif


// Double-Array Trie
// ========================================================

//static datnode_ptr datnodepool[REGIONSIZE],datlead,datroot;
const size_t datrootidx = 255;

static inline datnode_ptr getDatNode(datrie_ptr self, size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
	return &self->_datnodepool[region][position];
}

void allocDatNodepool(datrie_ptr self, int region)
{
	datnode_ptr pnode = (datnode_ptr) malloc(sizeof(datnode)*POOLPOSITIONSIZE);
	if (pnode == NULL) {
		fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
		exit(-1);
	}
	self->_datnodepool[region] = pnode;
	// 节点初始化
	memset(pnode, 0, sizeof(datnode)*POOLPOSITIONSIZE);
	size_t offset = region << REGIONOFFSET;
	for (int i = 0; i < POOLPOSITIONSIZE; ++i) {
		pnode[i].nf.next = offset+i+1;
		pnode[i].lf.last = offset+i-1;
	}
	pnode[POOLPOSITIONSIZE-1].nf.next = 0;

	pnode[0].lf.last = self->_datlead->lf.last;
	getDatNode(self, self->_datlead->lf.last)->nf.next=offset;
	self->_datlead->lf.last = offset+POOLPOSITIONSIZE-1;
}

datnode_ptr getDatNodeWithAlloc(datrie_ptr self, size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
	if (region >= POOLREGIONSIZE) return NULL;
	if (self->_datnodepool[region] == NULL) allocDatNodepool(self, region);
	return &self->_datnodepool[region][position];
}

void initDat(datrie_ptr self)
{
	for (int i = 0; i < POOLREGIONSIZE; ++i) self->_datnodepool[i] = NULL;
	datnode_ptr pnode = (datnode_ptr) malloc(sizeof(datnode)*POOLPOSITIONSIZE);
	if (pnode == NULL) {
		fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
		exit(-1);
	}
	memset(pnode, 0, sizeof(datnode)*POOLPOSITIONSIZE);

	self->_datnodepool[0] = pnode;
	self->_datlead = &self->_datnodepool[0][0];
	self->datroot = &self->_datnodepool[0][datrootidx];

	// 节点初始化
	for (size_t i = 1; i < POOLPOSITIONSIZE; ++i) {
		pnode[i].nf.next = i+1;
		pnode[i].lf.last = i-1;
	}
	pnode[POOLPOSITIONSIZE-1].nf.next = 0;
	pnode[datrootidx+1].lf.last = 0;

	self->_datlead->nf.next = datrootidx+1;
	self->_datlead->lf.last = POOLPOSITIONSIZE-1;
	self->_datlead->check = 1;
}

void closeDat(datrie_ptr self)
{
	for (int i = 0; i < POOLREGIONSIZE; ++i) {
		if (self->_datnodepool[i] != NULL) free(self->_datnodepool[i]);
	}
}

size_t count = 0;
void constructDatByDFS(datrie_ptr self, trie_ptr origin, trienode_ptr pNode, size_t datindex)
{
	datnode_ptr pDatNode = getDatNode(self, datindex);
	pDatNode->lf.f.key = pNode->key;
	pDatNode->lf.f.flag = pNode->flag;
		
	if (pNode->flag & 2) count++;

	if (pNode->child == 0) return;

	unsigned char child[256];
	int len = 0;
	trienode_ptr pChild = getTrieNode(origin, pNode->child);
	while (pChild != origin->trie_root) {
		child[len++] = pChild->key;
		pChild = getTrieNode(origin, pChild->brother);
	}
	size_t pos = self->_datlead->nf.next;
	while (1) {
		if (pos == 0) {
			// 扩容
			int region = 1;
			while (region < POOLREGIONSIZE && self->_datnodepool[region]!=NULL) ++region;
			if (region == POOLREGIONSIZE) {
				fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
				exit(-1);
			}
			allocDatNodepool(self, region);
			pos = (size_t)region<<REGIONOFFSET;
		}
		// 检查: pos容纳第一个子节点
		size_t base = pos - child[0];
		for (int i = 1; i < len; ++i) {
			if (getDatNodeWithAlloc(self, base + child[i])->check != 0) goto checkfailed;
		}
		// base 分配成功
		pDatNode->base = base;
		for (int i = 0; i < len; ++i) {
			// 分配子节点
			datnode_ptr pDatChild = getDatNode(self, base + child[i]);
			pDatChild->check = datindex;
			getDatNode(self, pDatChild->nf.next)->lf.last = pDatChild->lf.last;
			getDatNode(self, pDatChild->lf.last)->nf.next = pDatChild->nf.next;
		}
		break;
checkfailed:
		pos = getDatNode(self, pos)->nf.next;
	}

	// 构建子树
	pChild = getTrieNode(origin, pNode->child);
	while (pChild != origin->trie_root) {
		pChild->pd.datidx = pDatNode->base + pChild->key;
		constructDatByDFS(self, origin, pChild, pChild->pd.datidx);
		pChild = getTrieNode(origin, pChild->brother);
	}
}

void afterDatConstructed(datrie_ptr self)
{
	// 添加占位内存，防止匹配时出现非法访问
	int region = 1;
	while (region < POOLREGIONSIZE && self->_datnodepool[region]!=NULL) ++region;
	if (region >= POOLREGIONSIZE) {
		fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
		exit(-1);
	} else {
		datnode_ptr pnode = (datnode_ptr) malloc(sizeof(datnode) * 256);
		if (pnode == NULL) {
			fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
			exit(-1);
		}
		self->_datnodepool[region] = pnode;
		// 节点初始化
		memset(pnode, 0, sizeof(datnode)*256);
	}
}

void constructDat(datrie_ptr self, trie_ptr origin)
{
	constructDatByDFS(self, origin, origin->trie_root, datrootidx);
	afterDatConstructed(self);
	fprintf(stderr, "pattern: %zu\n", count);
}

int printDatKeywordByRecursion(datrie_ptr self, datnode_ptr pNode)
{
	if (pNode == self->datroot) return 0;

	int pos = printDatKeywordByRecursion(self, getDatNode(self, pNode->check)) + 1;
	putc(pNode->lf.f.key, stdout);
	return pos;
}

void printDatKeyword(datrie_ptr self, datnode_ptr pNode)
{
	int len = printDatKeywordByRecursion(self, pNode);
	putc('\n', stdout);
}

void matchDat(datrie_ptr self, unsigned char *content)
{
	size_t len = strlen(content);
	for (size_t i = 0; i < len; ++i) {
		unsigned char *cur = content + i;
		size_t iCursor = datrootidx;
		datnode_ptr pCursor = self->datroot;
		while (*cur != '\0') {
			size_t iNext = pCursor->base + *cur;
			datnode_ptr pNext = getDatNode(self, iNext);
			if (pNext->check != iCursor) break;
			if (pNext->lf.f.flag & 2) printDatKeyword(self, pNext);
			iCursor = iNext;
			pCursor = pNext;
			cur++;
		}
	}
}


void constructDatAutomation(datrie_ptr self, trie_ptr origin)
{
	trienode_ptr pNode = origin->trie_root;
	size_t iChild = pNode->child;
	while (iChild != 0) {
		trienode_ptr pChild = getTrieNode(origin, iChild);
		// 设置 DAT 的 failed 域
		getDatNode(self, pChild->pd.datidx)->nf.failed = datrootidx;
		iChild = pChild->brother;
	}
	for (size_t index = 1; index < getTrieSize(origin); index++) { // bfs
		pNode = getTrieNode(origin, index);
		iChild = pNode->child;
		while (iChild != 0) {
			trienode_ptr pChild = getTrieNode(origin, iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->failed;
#ifdef BINSCH
			size_t match = nextStateByBinary(origin, iFailed, key);
#else
			size_t match = nextState(origin, iFailed, key);
#endif
			while (iFailed != 0 && match == 0) {
				iFailed = getTrieNode(origin, iFailed)->failed;
#ifdef BINSCH
				match = nextStateByBinary(origin, iFailed, key);
#else
				match = nextState(origin, iFailed, key);
#endif
			}
			pChild->failed = match;
			
			// 设置 DAT 的 failed 域
#ifdef EXTFLAG
			datnode_ptr pDatChild = getDatNode(pChild->pd.datidx);
			if (match != 0) {
				trienode_ptr pFailed = getTrieNode(match);
				pDatChild->nf.failed = pFailed->pd.datidx;
				datnode_ptr pDatFailed = getDatNode(pFailed->pd.datidx);
				if (pDatFailed->lf.f.flag & 6) pDatFailed->lf.f.flag |= 4;
			} else {
				pDatChild->nf.failed = datrootidx;
			}
#else
			getDatNode(self, pChild->pd.datidx)->nf.failed =
				match!=0?getTrieNode(origin, match)->pd.datidx:datrootidx;
#endif
			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void matchAcdat(datrie_ptr self, unsigned char *content)
{
	size_t iCursor = datrootidx;
	datnode_ptr pCursor = self->datroot;
	while(*content != '\0') {
		size_t iNext = pCursor->base + *content;
		datnode_ptr pNext = getDatNode(self, iNext);
		while (pCursor != self->datroot && pNext->check != iCursor) {
			iCursor = pCursor->nf.failed;
			pCursor = getDatNode(self, iCursor);
			iNext = pCursor->base + *content;
			pNext = getDatNode(self, iNext);
		}
		if (pNext->check == iCursor) {
			iCursor = iNext;
			pCursor = pNext;
#ifdef EXTFLAG
			while (pNext->lf.f.flag & 6) {
#else
			while (pNext != self->datroot) {
#endif
				if (pNext->lf.f.flag & 2)
					printDatKeyword(self, pNext);
				pNext = getDatNode(self, pNext->nf.failed);
			}
		}
		content++;
	}
}


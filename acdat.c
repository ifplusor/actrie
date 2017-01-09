#include <memory.h>
#include <string.h>

#include "acdat.h"


extern trienode_ptr trie_root;

// Trie 内部接口，仅限 Double-Array Trie 使用
size_t getTrieSize();
trienode_ptr getTrieNode(size_t index);
#ifdef BINSCH
size_t nextStateByBinary(size_t iNode, unsigned char key);
#else
size_t nextState(size_t iNode, unsigned char key);
#endif


// Double-Array Trie
// ========================================================

static datnode_ptr datnodepool[REGIONSIZE],datlead,datroot;
const size_t datrootidx = 255;

static inline datnode_ptr getDatNode(size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
	return &datnodepool[region][position];
}

void allocDatNodepool(int region)
{
	datnode_ptr pnode = (datnode_ptr) malloc(sizeof(datnode)*POOLPOSITIONSIZE);
	if (pnode == NULL) {
		fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
		exit(-1);
	}
	datnodepool[region] = pnode;
	// 节点初始化
	memset(pnode, 0, sizeof(datnode)*POOLPOSITIONSIZE);
	size_t offset = region << REGIONOFFSET;
	for (int i = 0; i < POOLPOSITIONSIZE; ++i) {
		pnode[i].nf.next = offset+i+1;
		pnode[i].lf.last = offset+i-1;
	}
	pnode[POOLPOSITIONSIZE-1].nf.next = 0;

	pnode[0].lf.last = datlead->lf.last;
	getDatNode(datlead->lf.last)->nf.next=offset;
	datlead->lf.last = offset+POOLPOSITIONSIZE-1;
}

datnode_ptr getDatNodeWithAlloc(size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
	if (region >= POOLREGIONSIZE) return NULL;
	if (datnodepool[region] == NULL) allocDatNodepool(region);
	return &datnodepool[region][position];
}

void initDat()
{
	for (int i = 0; i < POOLREGIONSIZE; ++i) datnodepool[i] = NULL;
	datnode_ptr pnode = (datnode_ptr) malloc(sizeof(datnode)*POOLPOSITIONSIZE);
	if (pnode == NULL) {
		fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
		exit(-1);
	}
	memset(pnode, 0, sizeof(datnode)*POOLPOSITIONSIZE);

	datnodepool[0] = pnode;
	datlead = &datnodepool[0][0];
	datroot = &datnodepool[0][datrootidx];

	// 节点初始化
	for (int i = 1; i < POOLPOSITIONSIZE; ++i) {
		pnode[i].nf.next = i+1;
		pnode[i].lf.last = i-1;
	}
	pnode[POOLPOSITIONSIZE-1].nf.next = 0;
	pnode[datrootidx+1].lf.last = 0;

	datlead->nf.next = datrootidx+1;
	datlead->lf.last = POOLPOSITIONSIZE-1;
	datlead->check = 1;
}

void closeDat()
{
	for (int i = 0; i < POOLREGIONSIZE; ++i) {
		if (datnodepool[i] != NULL) free(datnodepool[i]);
	}
}

size_t count = 0;
void constructDatByDFS(trienode_ptr pNode, size_t datindex)
{
	datnode_ptr pDatNode = getDatNode(datindex);
	pDatNode->lf.f.key = pNode->key;
	pDatNode->lf.f.flag = pNode->flag;
		
	if (pNode->flag & 2) count++;

	if (pNode->child == 0) return;

	unsigned char child[256];
	int len = 0;
	trienode_ptr pChild = getTrieNode(pNode->child);
	while (pChild != trie_root) {
		child[len++] = pChild->key;
		pChild = getTrieNode(pChild->brother);
	}
	size_t pos = datlead->nf.next;
	while (1) {
		if (pos == 0) {
			// 扩容
			int region = 1;
			while (region < POOLREGIONSIZE && datnodepool[region]!=NULL) ++region;
			if (region == POOLREGIONSIZE) {
				fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
				exit(-1);
			}
			allocDatNodepool(region);
			pos = region<<REGIONOFFSET;
		}
		// 检查: pos容纳第一个子节点
		size_t base = pos - child[0];
		for (int i = 1; i < len; ++i) {
			if (getDatNodeWithAlloc(base + child[i])->check != 0) goto checkfailed;
		}
		// base 分配成功
		pDatNode->base = base;
		for (int i = 0; i < len; ++i) {
			// 分配子节点
			datnode_ptr pDatChild = getDatNode(base + child[i]);
			pDatChild->check = datindex;
			getDatNode(pDatChild->nf.next)->lf.last = pDatChild->lf.last;
			getDatNode(pDatChild->lf.last)->nf.next = pDatChild->nf.next;
		}
		break;
checkfailed:
		pos = getDatNode(pos)->nf.next;
	}

	// 构建子树
	pChild = getTrieNode(pNode->child);
	while (pChild != trie_root) {
		pChild->pd.datidx = pDatNode->base + pChild->key;
		constructDatByDFS(pChild, pChild->pd.datidx);
		pChild = getTrieNode(pChild->brother);
	}
}

void afterDatConstructed()
{
	// 添加占位内存，防止匹配时出现非法访问
	int region = 1;
	while (region < POOLREGIONSIZE && datnodepool[region]!=NULL) ++region;
	if (region >= POOLREGIONSIZE) {
		fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
		exit(-1);
	} else {
		datnode_ptr pnode = (datnode_ptr) malloc(sizeof(datnode) * 256);
		if (pnode == NULL) {
			fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
			exit(-1);
		}
		datnodepool[region] = pnode;
		// 节点初始化
		memset(pnode, 0, sizeof(datnode)*256);
	}
}

void constructDat(trienode_ptr pNode, size_t datindex)
{
	constructDatByDFS(trie_root, datrootidx);
	afterDatConstructed();
	fprintf(stderr, "pattern: %d\n", count);
}

int printDatKeywordByRecursion(datnode_ptr pNode)
{
	if (pNode == datroot) return 0;

	int pos = printDatKeywordByRecursion(getDatNode(pNode->check)) + 1;
	putc(pNode->lf.f.key, stdout);
	return pos;
}

void printDatKeyword(datnode_ptr pNode)
{
	int len = printDatKeywordByRecursion(pNode);
	putc('\n', stdout);
}

void matchDat(unsigned char *content)
{
	size_t len = strlen(content);
	for (size_t i = 0; i < len; ++i) {
		unsigned char *cur = content + i;
		size_t iCursor = datrootidx;
		datnode_ptr pCursor = datroot;
		while (*cur != '\0') {
			size_t iNext = pCursor->base + *cur;
			datnode_ptr pNext = getDatNode(iNext);
			if (pNext->check != iCursor) break;
			if (pNext->lf.f.flag & 2) printDatKeyword(pNext);
			iCursor = iNext;
			pCursor = pNext;
			cur++;
		}
	}
}


void constructDatAutomation()
{
	trienode_ptr pNode = trie_root;
	size_t iChild = pNode->child;
	while (iChild != 0) {
		trienode_ptr pChild = getTrieNode(iChild);
		// 设置 DAT 的 failed 域
		getDatNode(pChild->pd.datidx)->nf.failed = datrootidx;
		iChild = pChild->brother;
	}
	for (size_t index = 1; index < getTrieSize(); index++) { // bfs
		pNode = getTrieNode(index);
		iChild = pNode->child;
		while (iChild != 0) {
			trienode_ptr pChild = getTrieNode(iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->failed;
#ifdef BINSCH
			size_t match = nextStateByBinary(iFailed, key);
#else
			size_t match = nextState(iFailed, key);
#endif
			while (iFailed != 0 && match == 0) {
				iFailed = getTrieNode(iFailed)->failed;
#ifdef BINSCH
				match = nextStateByBinary(iFailed, key);
#else
				match = nextState(iFailed, key);
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
			getDatNode(pChild->pd.datidx)->nf.failed = 
				match!=0?getTrieNode(match)->pd.datidx:datrootidx;
#endif
			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void matchAcdat(unsigned char *content)
{
	size_t iCursor = datrootidx;
	datnode_ptr pCursor = datroot;
	while(*content != '\0') {
		size_t iNext = pCursor->base + *content;
		datnode_ptr pNext = getDatNode(iNext);
		while (pCursor != datroot && pNext->check != iCursor) {
			iCursor = pCursor->nf.failed;
			pCursor = getDatNode(iCursor);
			iNext = pCursor->base + *content;
			pNext = getDatNode(iNext);
		}
		if (pNext->check == iCursor) {
			iCursor = iNext;
			pCursor = pNext;
#ifdef EXTFLAG
			while (pNext->lf.f.flag & 6) {
#else
			while (pNext != datroot) {
#endif
				if (pNext->lf.f.flag & 2)
					printDatKeyword(pNext);
				pNext = getDatNode(pNext->nf.failed);
			}
		}
		content++;
	}
}


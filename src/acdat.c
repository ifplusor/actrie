#include <acdat.h>
#include "acdat.h"


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

const size_t datrootidx = 255;

static inline datnode_ptr getDatNode(datrie_ptr self, size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
	return &self->_datnodepool[region][position];
}

void allocDatNodepool(datrie_ptr self, size_t region)
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

	self->datroot->lf.f.depth = 0;
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
			size_t region = 1;
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
			// 对depth的改变要放在last指针改变之后
			pDatChild->lf.f.depth = pDatNode->lf.f.depth + 1;
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

void matchDat(datrie_ptr self, unsigned char content[], size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		size_t iCursor = datrootidx;
		datnode_ptr pCursor = self->datroot;
		for (size_t j = i; j < len; ++j) {
			size_t iNext = pCursor->base + content[i];
			datnode_ptr pNext = getDatNode(self, iNext);
			if (pNext->check != iCursor) break;
			if (pNext->lf.f.flag & 2) printDatKeyword(self, pNext);
			iCursor = iNext;
			pCursor = pNext;
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

void matchAcdat(datrie_ptr self, unsigned char content[], size_t len)
{
	size_t iCursor = datrootidx;
	datnode_ptr pCursor = self->datroot;
	for (size_t i = 0; i < len; ++i) {
		size_t iNext = pCursor->base + content[i];
		datnode_ptr pNext = getDatNode(self, iNext);
		while (pCursor != self->datroot && pNext->check != iCursor) {
			iCursor = pCursor->nf.failed;
			pCursor = getDatNode(self, iCursor);
			iNext = pCursor->base + content[i];
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
	}
}


// Acdat Context
// ===================================================

void initAcdatContext(datcontext_ptr context, datrie_ptr trie, unsigned char content[], size_t len)
{
	context->trie = trie;
	context->content = content;
	context->len = len;

	context->_i = 0;
	context->_iCursor = datrootidx;
	context->_pCursor = context->trie->datroot;
	context->_pMatched = context->trie->datroot;
}

int getMatchedInAcdat(datcontext_ptr context, unsigned char buffer[], size_t size)
{
	int idx = context->_pMatched->lf.f.depth;
	if (idx >= size) return -1;
	buffer[idx--] = '\0';
	for (datnode_ptr pNode = context->_pMatched; idx >= 0; pNode = getDatNode(context->trie, pNode->check)) {
		buffer[idx--] = pNode->lf.f.key;
	}
	return context->_pMatched->lf.f.depth;
}

bool nextWithAcdat(datcontext_ptr context)
{
	// 检查当前匹配点向树根的路径上是否还有匹配的词
#ifdef EXTFLAG
	while (context->_pMatched->lf.f.flag & 6) {
#else
    while (context->_pMatched != context->trie->datroot) {
#endif
		context->_pMatched = getDatNode(context->trie, context->_pMatched->nf.failed);
		if (context->_pMatched->lf.f.flag & 2) {
			return true;
		}
	}

	// 执行匹配
	for (;context->_i < context->len; context->_i++) {
		size_t iNext = context->_pCursor->base + context->content[context->_i];
		datnode_ptr pNext = getDatNode(context->trie, iNext);
		while (context->_pCursor != context->trie->datroot && pNext->check != context->_iCursor) {
			context->_iCursor = context->_pCursor->nf.failed;
			context->_pCursor = getDatNode(context->trie, context->_iCursor);
			iNext = context->_pCursor->base + context->content[context->_i];
			pNext = getDatNode(context->trie, iNext);
		}
		if (pNext->check == context->_iCursor) {
			context->_iCursor = iNext;
			context->_pCursor = pNext;
#ifdef EXTFLAG
			while (pNext->lf.f.flag & 6) {
#else
			while (pNext != context->trie->datroot) {
#endif
				if (pNext->lf.f.flag & 2) {
					context->_pMatched = pNext;
					context->_i++;
					return true;
				}
				pNext = getDatNode(context->trie, pNext->nf.failed);
			}
		}
	}
	return false;
}


#include <string.h>
#include "actrie.h"


// Prime Trie
// ========================================================

size_t getTrieSize(trie_ptr self)
{
	return self->_autoindex;
}

static size_t allocNode(trie_ptr self)
{
	size_t region = self->_autoindex >> REGIONOFFSET;
	size_t position = self->_autoindex & POSITIONMASK;
#ifdef CHECK
	if (region >= POOLREGIONSIZE) return (size_t)-1;
#endif // CHECK
	if (self->_nodepool[region] == NULL) {
		trienode_ptr pnode = (trienode_ptr) malloc(sizeof(trienode)*POOLPOSITIONSIZE);
		if (pnode == NULL) return (size_t)-1;
		self->_nodepool[region] = pnode;
		memset(pnode, 0, sizeof(trienode)*POOLPOSITIONSIZE);
	}
#ifdef CHECK
	if (self->_nodepool[region][position].flag & 1) return (size_t)-1;
#endif // CHECK
	self->_nodepool[region][position].flag |= 1;
	return self->_autoindex++;
}

static inline trienode_ptr getNode(trie_ptr self, size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
#ifdef CHECK
	if (region >= POOLREGIONSIZE || self->_nodepool[region] == NULL
			|| !(self->_nodepool[region][position].flag & 1)) {
		return NULL;
	}
#endif // CHECK
	return &self->_nodepool[region][position];
}

trienode_ptr getTrieNode(trie_ptr self, size_t index)
{
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
#ifdef CHECK
	if (region >= POOLREGIONSIZE || self->_nodepool[region] == NULL
			|| !(self->_nodepool[region][position].flag & 1)) {
		return NULL;
	}
#endif // CHECK
	return &self->_nodepool[region][position];
}

void initTrie(trie_ptr self)
{
	for (int i = 0; i < POOLREGIONSIZE; i++) self->_nodepool[i] = NULL;
	self->_autoindex = 0;
	self->trie_root = getNode(self, allocNode(self));
}

void closeTrie(trie_ptr self)
{
	for (int i = 0; i < POOLREGIONSIZE; i++) {
		if (self->_nodepool[i] != NULL) free(self->_nodepool[i]);
	}
}

bool addKeyword(trie_ptr self, unsigned char keyword[], size_t len)
{
	trienode_ptr pNode = self->trie_root;
	size_t iNode = 0; // iParent保存pNode的index
	for (size_t i = 0; i < len; ++i) {
        // 当创建时，使用插入排序的方法，以保证子节点链接关系有序
		size_t iChild = pNode->child, iBrother = 0; // iBrother跟踪iChild
		trienode_ptr pChild = NULL;
		while (iChild != 0) {
			// 从所有孩子中查找
			pChild = getNode(self, iChild);
#ifdef SORTED
            if (pChild->key == keyword[i]) break;
#else
			if (pChild->key >= keyword[i]) break;
#endif // SORTED
			iBrother = iChild;
			iChild = pChild->brother;
		}

		if (iChild != 0 && pChild->key == keyword[i]) {
			// 找到
			iNode = iChild;
			pNode = pChild;
		} else {
			// 没找到, 创建.
			size_t index = allocNode(self);
			if (index == -1) return false;
			trienode_ptr pc = getNode(self, index);
			if (pc == NULL) return false;
			pc->key = keyword[i];
			if (pChild == NULL) {
				// 没有子节点
#ifdef LOWERUPPER
#ifndef INBFS
				pNode->lower = pc->key;
				pNode->upper = pc->key;
#endif
#endif // LOWERUPPER
				pNode->child = index;
				pc->pd.parent = iNode;
			} else {
#ifdef SORTED
				// 尾插法，无法保证后无向前指针
#ifdef LOWERUPPER
#ifndef INBFS
				pNode->upper = pc->key;
#endif // INBFS
#endif // LOWERUPPER
				pChild->brother = index;
				pc->pd.parent = iBrother;
#else
				if (iBrother == 0) {
					// 插入链表头
#ifdef LOWERUPPER
#ifndef INBFS
					pNode->lower = pc->key;
#endif // INBFS
#endif // LOWERUPPER
                    pc->pd.parent = iNode;
					pc->brother = pNode->child;
                    pNode->child = index;
					pChild->pd.parent = index;
				} else if (pChild->key < keyword[i]) {
					// 插入链表尾
#ifdef LOWERUPPER
#ifndef INBFS
					pNode->upper = pc->key;
#endif // INBFS
#endif // LOWERUPPER
					pc->pd.parent = iBrother;
					pChild->brother = index;
				} else {
					trienode_ptr pBrother = getNode(self, iBrother);
					pc->pd.parent = iBrother;
					pc->brother = iChild;
					pBrother->brother = index;
					pChild->pd.parent = index;
				}
#endif // SORTED
			}
			pNode->len++;
			iNode = index;
			pNode = pc;
		}
	}
	pNode->flag |= 2;
	return true;
}

size_t nextState(trie_ptr self, size_t iNode, unsigned char key)
{
	size_t iChild = getNode(self, iNode)->child;
	while (iChild != 0) {
		trienode_ptr pChild = getNode(self, iChild);
		if (pChild->key == key) break;
		iChild = pChild->brother;
	}
	return iChild;
}

trienode_ptr nextNode(trie_ptr self, trienode_ptr pNode, unsigned char key)
{
	trienode_ptr pChild = getNode(self, pNode->child);
	while (pChild != self->trie_root) {
		if (pChild->key == key) return pChild;
		pChild = getNode(self, pChild->brother);
	}
	return self->trie_root;
}

size_t nextStateByBinary(trie_ptr self, size_t iNode, unsigned char key)
{
	trienode_ptr pNode = getNode(self, iNode);
	if (pNode->len >= 1) {
#ifdef ADVBIN
		size_t right = pNode->child + pNode->len;
		if (key < getNode(pNode->child)->key || getNode(right-1)->key < key)
			return 0;
		size_t left = pNode->child - 1;
		unsigned char tkey, rkey = 0;
		while (left+1 < right) {
			size_t middle = (left+right)>>1;
			tkey = getNode(middle)->key;
			if (tkey < key) {
				left = middle;
			} else {
				rkey = tkey;
				right = middle;
			}
		}
		if (rkey == key) return right;
#else
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		if (key < getNode(self, left)->key || getNode(self, right)->key < key)
			return 0;
		while (left <= right) {
			size_t middle = (left+right)>>1;
			trienode_ptr pMiddle = getNode(self, middle);
			if (pMiddle->key == key) {
				return middle;
			} else if (pMiddle->key > key) {
				right = middle-1;
			} else {
				left = middle+1;
			}
		}
#endif // ADVBIN
	}
	return 0;
}

trienode_ptr nextNodeByBinary(trie_ptr self, trienode_ptr pNode, unsigned char key)
{
#ifndef ADVSCH
	if (pNode->len >= 1) {
#endif // ADVSCH
#ifdef LOWERUPPER
		if (key < pNode->lower || pNode->upper < key) return self->trie_root;
#else
		if (key < getNode(self, pNode->child)->key || getNode(self, pNode->child+pNode->len-1)->key < key)
			return self->trie_root;
#endif // LOWERUPPER
#ifdef ADVBIN // 过度优化?
		size_t left = pNode->child - 1;
		size_t right = pNode->child + pNode->len;
		unsigned char tkey, rkey = 0; // 不会搜索'\0'
		while (left+1 < right) {
			size_t middle = (left+right)>>1;
			tkey = getNode(middle)->key;
			if (tkey < key) {
				left = middle;
			} else {
				rkey = tkey;
				right = middle;
			}
		}
		if (rkey == key) return getNode(right);
#else
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		while (left <= right) {
			size_t middle = (left+right)>>1;
			trienode_ptr pMiddle = getNode(self, middle);
			if (pMiddle->key < key) {
				left = middle+1;
			} else if (pMiddle->key > key) {
				right = middle-1;
			} else {
				return pMiddle;
			}
		}
#endif // ADVBIN
#ifndef ADVSCH
	}
#endif // ADVSCH
	return self->trie_root;
}

#ifdef ADVSCH
trienode_ptr nextNodeByDouble(trie_ptr self, trienode_ptr pNode, unsigned char key)
{
#ifdef LOWERUPPER
	if (pNode->lower == key) return getNode(self, pNode->child);
	if (pNode->upper == key) return getNode(self, pNode->child+1);
#else
	trienode_ptr pChild = getNode(self, pNode->child);
	if (pChild->key == key) return pChild;
	pChild = getNode(self, pChild->brother);
	if (pChild->key == key) return pChild;
#endif
	return self->trie_root;
}

trienode_ptr nextNodeBySingle(trie_ptr self, trienode_ptr pNode, unsigned char key)
{
#ifdef LOWERUPPER
	if (pNode->lower == key) return getNode(self, pNode->child);
#else
	trienode_ptr pChild = getNode(self, pNode->child);
	if (pChild->key == key) return pChild;
#endif
	return self->trie_root;
}

trienode_ptr nextNodeByEmpty(trie_ptr self, trienode_ptr pNode, unsigned char key)
{
	return self->trie_root;
}
#endif // ADVSCH

void swapNodeData(trienode_ptr pa, trienode_ptr pb)
{
	pa->child ^= pb->child;
	pb->child ^= pa->child;
	pa->child ^= pb->child;

	pa->brother ^= pb->brother;
	pb->brother ^= pa->brother;
	pa->brother ^= pb->brother;

	pa->pd.parent ^= pb->pd.parent;
	pb->pd.parent ^= pa->pd.parent;
	pa->pd.parent ^= pb->pd.parent;

	pa->failed ^= pb->failed;
	pb->failed ^= pa->failed;
	pa->failed ^= pb->failed;

	pa->len ^= pb->len;
	pb->len ^= pa->len;
	pa->len ^= pb->len;

	pa->key ^= pb->key;
	pb->key ^= pa->key;
	pa->key ^= pb->key;

	pa->flag ^= pb->flag;
	pb->flag ^= pa->flag;
	pa->flag ^= pb->flag;

#ifdef LOWERUPPER
#ifndef INBFS
	pa->lower ^= pb->lower;
	pb->lower ^= pa->lower;
	pa->lower ^= pb->lower;

	pa->upper ^= pb->upper;
	pb->upper ^= pa->upper;
	pa->upper ^= pb->upper;
#endif
#endif // LOWERUPPER
}

size_t swapNode(trie_ptr self, size_t iChild, size_t iTarget)
{
	trienode_ptr pChild = getNode(self, iChild);
	if (iChild != iTarget) {
		trienode_ptr pTarget = getNode(self, iTarget);

		// 常量
		const size_t ipc = pChild->pd.parent;
		const size_t icc = pChild->child;
		const size_t ibc = pChild->brother;
		const bool bcc = getNode(self, ipc)->child == iChild;
		const size_t ipt = pTarget->pd.parent;
		const size_t ict = pTarget->child;
		const size_t ibt = pTarget->brother;
		const bool bct = getNode(self, ipt)->child == iTarget;

		// 交换节点内容
		swapNodeData(pChild, pTarget);
		trienode_ptr ptmp = pChild;
		pChild = pTarget;
		pTarget = ptmp;

		// 考虑上下级节点交换
		// 调整target的上级，child的下级
		if (ipt == iChild) {
			// target是child的下级，child是target的上级
			pTarget->pd.parent = iTarget;
			if (bct) {
				pChild->child = iChild;
				if (ibc != 0) getNode(self, ibc)->pd.parent = iTarget;
			} else {
				pChild->brother = iChild;
				if (icc != 0) getNode(self, icc)->pd.parent = iTarget;
			}
		} else {
			if (bct)
				getNode(self, ipt)->child = iChild;
			else
				getNode(self, ipt)->brother = iChild;
			if (icc != 0) getNode(self, icc)->pd.parent = iTarget;
			if (ibc != 0) getNode(self, ibc)->pd.parent = iTarget;
		}

		// 调整直接上级指针
		if (bcc)
			getNode(self, ipc)->child = iTarget;
		else
			getNode(self, ipc)->brother = iTarget;

		// 调整下级指针
		if (ict != 0) getNode(self, ict)->pd.parent = iChild;
		if (ibt != 0) getNode(self, ibt)->pd.parent = iChild;
	}
	return pChild->brother;
}

int printTrieKeywordByRecursion(trie_ptr self, trienode_ptr pNode)
{
	if (pNode == self->trie_root) return 0;

	int pos = printTrieKeywordByRecursion(self, getNode(self, pNode->pd.parent)) + 1;
	putc(pNode->key, stdout);
	return pos;
}

void printTrieKeyword(trie_ptr self, trienode_ptr pNode)
{
	int len = printTrieKeywordByRecursion(self, pNode);
	putc('\n', stdout);
}

bool constructTrie(trie_ptr self, FILE *fp)
{
	int count = 0;
	unsigned char keyword[1000];
	while (fscanf(fp, "%s", keyword) != EOF) {
		//printf("%s\n", keyword);
		if (!addKeyword(self, keyword, strlen(keyword))) {
			fprintf(stderr, "fatal: encounter error when add keywords.\n");
			return false;
		}
		++count;
	}
	fprintf(stderr, "pattern: %d, node: %zu\n", count, self->_autoindex);
	fprintf(stderr, "constructTrie succeed!\n");
	return true;
}

void sortTrieToLine(trie_ptr self)
{
	size_t iTarget = 1;
	for (size_t i = 0; i < iTarget; i++) { // 隐式bfs队列
		//printf("sort process: %d\n", i);
		trienode_ptr pNode = getNode(self, i);
		// 将pNode的子节点调整到iTarget位置（加入队列）
		size_t iChild = pNode->child;
		while (iChild != 0) {
			// swap iChild与iTarget
			// 建树时的尾插法担保兄弟节点不会交换，且在重排后是稳定的
			iChild = swapNode(self, iChild, iTarget);
			iTarget++;
		}
#ifdef LOWERUPPER
#ifdef INBFS
		if (pNode->child != 0) {
			pNode->lower = getNode(pNode->child)->key;
			pNode->upper = getNode(pNode->child+pNode->len-1)->key;
		}
#endif
#endif // LOWERUPPER
#ifdef ADVSCH
		// 设置next函数指针
		switch (pNode->len) {
			case 0:
				pNode->next = nextNodeByEmpty;
				break;
			case 1:
				pNode->next = nextNodeBySingle;
				break;
			case 2:
				pNode->next = nextNodeByDouble;
				break;
			default:
				pNode->next = nextNodeByBinary;
				break;
		}
#endif // ADVSCH
	}
	fprintf(stderr, "sort succeed!\n");
}

void setParentByDFS(trie_ptr self, size_t current, size_t parent)
{
	trienode_ptr pNode = getNode(self, current);
	pNode->pd.parent = parent;
	if (pNode->child != 0) setParentByDFS(self, pNode->child, current);
	if (pNode->brother != 0) setParentByDFS(self, pNode->brother, parent);
}

void rebuildTrieParent(trie_ptr self)
{
	if (self->trie_root->child != 0) setParentByDFS(self, self->trie_root->child, 0);
	fprintf(stderr, "rebuild parent succeed!\n");
}

void constructTrieAutomation(trie_ptr self)
{
	for (size_t index = 1; index < self->_autoindex; index++) { // bfs
		trienode_ptr pNode = getNode(self, index);
		size_t iChild = pNode->child;
		while (iChild != 0) {
			trienode_ptr pChild = getNode(self, iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->failed;
#ifdef BINSCH
			size_t match = nextStateByBinary(self, iFailed, key);
#else
			size_t match = nextState(self, iFailed, key);
#endif // BINSCH
			while (iFailed != 0 && match == 0) {
				iFailed = getNode(self, iFailed)->failed;
#ifdef BINSCH
				match = nextStateByBinary(self, iFailed, key);
#else
				match = nextState(self, iFailed, key);
#endif // BINSCH
			}
			pChild->failed = match;

			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void matchActrie(trie_ptr self, unsigned char content[], size_t len)
{
	trienode_ptr pCursor = self->trie_root;
	for (size_t i = 0; i < len; ++i) {
#if defined ADVSCH
		trienode_ptr pNext = pCursor->next(self, pCursor, content[i]);
#elif defined BINSCH
		trienode_ptr pNext = nextNodeByBinary(self, pCursor, content[i]);
#else
		trienode_ptr pNext = nextNode(self, pCursor, *content);
#endif
		while(pCursor != self->trie_root && pNext == self->trie_root) {
			pCursor = getNode(self, pCursor->failed);
#if defined ADVSCH
			pNext = pCursor->next(self, pCursor, content[i]);
#elif defined BINSCH
			pNext = nextNodeByBinary(self, pCursor, content[i]);
#else
			pNext = nextNode(self, pCursor, content[i]);
#endif
		}
		pCursor = pNext;
		while (pNext != self->trie_root) {
			if (pNext->flag & 2) printTrieKeyword(self, pNext);
			pNext = getNode(self, pNext->failed);
		}
	}
}


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <time.h>


#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif


//#define ADVSCH
#ifndef ADVSCH
#define BINSCH
#endif
//#define ADVBIN
//#define LOWERUPPER
#ifdef LOWERUPPER
#define INBFS
#endif


#define REGIONSIZE 2048
#define REGIONOFFSET 18
#define POSITIONMASK 0x0003FFFF

const size_t POOLREGIONSIZE = REGIONSIZE;
const size_t POOLPOSITIONSIZE = POSITIONMASK+1;

typedef struct node *trienode_ptr;
typedef trienode_ptr (*nextnode_func)(trienode_ptr, unsigned char);

typedef struct node {
	size_t child,brother,parent,failed;
#ifdef ADVSCH
	nextnode_func next;
#endif
	unsigned short len;
	unsigned char flag;
	unsigned char key;
#ifdef LOWERUPPER
	unsigned char lower;
	unsigned char upper;
#endif
} trienode;

static trienode_ptr nodepool[REGIONSIZE],root;
static size_t autoindex = 0;


size_t allocNode()
{
	size_t region = autoindex >> REGIONOFFSET;
	size_t position = autoindex & POSITIONMASK;
#ifdef CHECK
	if (region >= REGIONSIZE) {
		return -1;
	}
#endif
	if (nodepool[region] == NULL) {
		//printf("call malloc!\n");
		trienode_ptr pnode = (trienode_ptr) malloc(sizeof(trienode)*POOLPOSITIONSIZE);
		if (pnode != NULL) {
			nodepool[region] = pnode;
		} else {
			return -1;
		}
		memset(pnode, 0, sizeof(trienode)*POOLPOSITIONSIZE);
	}
#ifdef CHECK
	if (nodepool[region][position].flag & 1) {
		return -1;
	} else {
		nodepool[region][position].flag |= 1;
	}
#else
	nodepool[region][position].flag |= 1;
#endif
	return autoindex++;
}

static inline trienode_ptr getNode(size_t index)
{
	/*size_t region = index / POOLPOSITIONSIZE;
	size_t position = index % POOLPOSITIONSIZE;*/
	size_t region = index >> REGIONOFFSET;
	size_t position = index & POSITIONMASK;
#ifdef CHECK
	if (region >= POOLREGIONSIZE || nodepool[region] == NULL
			|| !(nodepool[region][position].flag & 1)) {
		return NULL;
	}
#endif
	return &nodepool[region][position];
}

void initTrie()
{
	for (int i = 0; i < POOLREGIONSIZE; i++) {
		nodepool[i] = NULL;
	}
	autoindex = 0;
	root = getNode(allocNode());
}

void closeTrie()
{
	for (int i = 0; i < POOLREGIONSIZE; i++) {
		if (nodepool[i] != NULL) {
			free(nodepool[i]);
		}
	}
}

BOOL addKeyword(unsigned char *keyword)
{
	trienode_ptr pNode = root;
	size_t iNode = 0; // iParent保存pNode的index
	for (int i = 0; keyword[i] != '\0'; i++) {
		size_t iChild = pNode->child, iBrother; // iBrother跟踪iChild
		trienode_ptr pChild = NULL;
		while (iChild != 0) {
			// 从所有孩子中查找
			pChild = getNode(iChild);
			if (pChild->key == keyword[i]) {
				break;
			}
			iBrother = iChild;
			iChild = pChild->brother;
		}

		if (iChild != 0) {
			// 找到
			iNode = iChild;
			pNode = pChild;
		} else {
			// 没找到，创建
			int index = allocNode();
			if (index == -1) {
				return FALSE;
			}
			trienode_ptr pc = getNode(index);
			if (pc == NULL) {
				return FALSE;
			}
			pc->key = keyword[i];
			if (pChild == NULL) {
				// 没有子节点
#ifdef LOWERUPPER
#ifndef INBFS
				pNode->lower = pc->key;
#endif
#endif
				pNode->child = index;
				pc->parent = iNode;
			} else {
				// 尾插法，无法保证后无向前指针
#ifdef LOWERUPPER
#ifndef INBFS
				pNode->upper = pc->key;
#endif
#endif
				pChild->brother = index;
				pc->parent = iBrother;
			}
			pNode->len++;
			iNode = index;
			pNode = pc;
		}
	}
	pNode->flag |= 2;
	return TRUE;
}

size_t nextState(size_t iNode, unsigned char key)
{
	size_t iChild = getNode(iNode)->child;
	while (iChild != 0) {
		trienode_ptr pChild = getNode(iChild);
		if (pChild->key == key)
			break;
		iChild = pChild->brother;
	}
	return iChild;
}

trienode_ptr nextNode(trienode_ptr pNode, unsigned char key)
{
	trienode_ptr pChild = getNode(pNode->child);
	while (pChild != root) {
		if (pChild->key == key) {
			return pChild;
		}
		pChild = getNode(pChild->brother);
	}
	return root;
}

size_t nextStateByBinary(size_t iNode, unsigned char key)
{
	trienode_ptr pNode = getNode(iNode);
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
		if (rkey == key)
			return right;
#else
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		if (key < getNode(left)->key || getNode(right)->key < key)
			return 0;
		while (left <= right) {
			size_t middle = (left+right)>>1;
			trienode_ptr pMiddle = getNode(middle);
			if (pMiddle->key == key) {
				return middle;
			} else if (pMiddle->key > key) {
				right = middle-1;
			} else {
				left = middle+1;
			}
		}
#endif
	}
	return 0;
}

trienode_ptr nextNodeByBinary(trienode_ptr pNode, unsigned char key)
{
#ifndef ADVSCH
	if (pNode->len >= 1) {
#endif
#ifdef LOWERUPPER
		if (key < pNode->lower || pNode->upper < key)
			return root;
#else
		if (key < getNode(pNode->child)->key || getNode(pNode->child+pNode->len-1)->key < key)
			return root;
#endif
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
		if (rkey == key)
			return getNode(right);
#else
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		while (left <= right) {
			size_t middle = (left+right)>>1;
			trienode_ptr pMiddle = getNode(middle);
			if (pMiddle->key < key) {
				left = middle+1;
			} else if (pMiddle->key > key) {
				right = middle-1;
			} else {
				return pMiddle;
			}
		}
#endif
#ifndef ADVSCH
	}
#endif
	return root;
}

#ifdef ADVSCH
trienode_ptr nextNodeByDouble(trienode_ptr pNode, unsigned char key)
{
#ifdef LOWERUPPER
	if (pNode->lower == key)
		return getNode(pNode->child);
	if (pNode->upper == key)
		return getNode(pNode->child+1);
#else
	trienode_ptr pChild = getNode(pNode->child);
	if (pChild->key == key)
		return pChild;
	pChild = getNode(pChild->brother);
	if (pChild->key == key)
		return pChild;
#endif
	return root;
}

trienode_ptr nextNodeBySingle(trienode_ptr pNode, unsigned char key)
{
#ifdef LOWERUPPER
	if (pNode->lower == key)
		return getNode(pNode->child);
#else
	trienode_ptr pChild = getNode(pNode->child);
	if (pChild->key == key)
		return pChild;
#endif
	return root;
}

trienode_ptr nextNodeByEmpty(trienode_ptr pNode, unsigned char key)
{
	return root;
}
#endif

void swapNodeData(trienode_ptr pa, trienode_ptr pb)
{
	pa->child ^= pb->child;
	pb->child ^= pa->child;
	pa->child ^= pb->child;

	pa->brother ^= pb->brother;
	pb->brother ^= pa->brother;
	pa->brother ^= pb->brother;

	pa->parent ^= pb->parent;
	pb->parent ^= pa->parent;
	pa->parent ^= pb->parent;

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
#endif
}

size_t swapNode(size_t iChild, size_t iTarget)
{
	trienode_ptr pChild = getNode(iChild);
	if (iChild != iTarget) {
		trienode_ptr pTarget = getNode(iTarget);

		// 常量
		const size_t ipc = pChild->parent;
		const size_t icc = pChild->child;
		const size_t ibc = pChild->brother;
		const BOOL bcc = getNode(ipc)->child == iChild;
		const size_t ipt = pTarget->parent;
		const size_t ict = pTarget->child;
		const size_t ibt = pTarget->brother;
		const BOOL bct = getNode(ipt)->child == iTarget;

		// 交换节点内容
		swapNodeData(pChild, pTarget);
		trienode_ptr ptmp = pChild;
		pChild = pTarget;
		pTarget = ptmp;

		// 考虑上下级节点交换
		// 调整target的上级，child的下级
		if (ipt == iChild) {
			// target是child的下级，child是target的上级
			pTarget->parent = iTarget;
			if (bct) {
				pChild->child = iChild;
				if (ibc != 0) {
					getNode(ibc)->parent = iTarget;
				}
			} else {
				pChild->brother = iChild;
				if (icc != 0) {
					getNode(icc)->parent = iTarget;
				}
			}
		} else {
			if (bct) {
				getNode(ipt)->child = iChild;
			} else {
				getNode(ipt)->brother = iChild;
			}
			if (icc != 0) {
				getNode(icc)->parent = iTarget;
			}
			if (ibc != 0) {
				getNode(ibc)->parent = iTarget;
			}
		}

		// 调整直接上级指针
		if (bcc) {
			getNode(ipc)->child = iTarget;
		} else {
			getNode(ipc)->brother = iTarget;
		}

		// 调整下级指针
		if (ict != 0) {
			getNode(ict)->parent = iChild;
		}
		if (ibt != 0) {
			getNode(ibt)->parent = iChild;
		}
	}
	return pChild->brother;
}

void printKeywordByRecursion(trienode_ptr pNode)
{
	if (pNode == root) {
		return;
	}

	printKeywordByRecursion(getNode(pNode->parent));
	putc(pNode->key, stdout);
}

void printKeyword(trienode_ptr pNode)
{
	printKeywordByRecursion(pNode);
	putc('\n', stdout);
}

void constructTrie(FILE *fp)
{
	int i = 0;
	unsigned char keyword[1000];
	while (fscanf(fp, "%s", keyword) != EOF) {
		//printf("%s\n", keyword);
		if (!addKeyword(keyword)) {
			break;
		}
		++i;
	}
	fprintf(stderr, "pattern: %d, node: %d\n", i, autoindex);
	fprintf(stderr, "constructTrie succeed!\n");
}

void sortForBFS()
{
	size_t iTarget = 1;
	for (size_t i = 0; i < iTarget; i++) { // 隐式bfs队列
		//printf("sort process: %d\n", i);
		trienode_ptr pNode = getNode(i);
		// 将pNode的子节点调整到iTarget位置（加入队列）
		size_t iChild = pNode->child;
		while (iChild != 0) {
			// swap iChild与iTarget
			// 建树时的尾插法担保兄弟节点不会交换，且在重排后是稳定的
			iChild = swapNode(iChild, iTarget);
			iTarget++;
		}
#ifdef LOWERUPPER
#ifdef INBFS
		if (pNode->child != 0) {
			pNode->lower = getNode(pNode->child)->key;
			pNode->upper = getNode(pNode->child+pNode->len-1)->key;
		}
#endif
#endif
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
#endif
	}
	fprintf(stderr, "sort succeed!\n");
}

void setParentByDFS(size_t current, size_t parent)
{
	trienode_ptr pNode = getNode(current);
	pNode->parent = parent;
	if (pNode->child != 0) {
		setParentByDFS(pNode->child, current);
	}
	if (pNode->brother != 0) {
		setParentByDFS(pNode->brother, parent);
	}
}

void rebuildParentByDFS()
{
	if (root->child != 0) {
		setParentByDFS(root->child, 0);
	}
	fprintf(stderr, "rebuild parent succeed!\n");
}

void constructAutomation()
{
	for (size_t index = 1; index < autoindex; index++) { // bfs
		trienode_ptr pNode = getNode(index);
		size_t iChild = pNode->child;
		while (iChild != 0) {
			trienode_ptr pChild = getNode(iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->failed;
#ifdef BINSCH
			size_t match = nextStateByBinary(iFailed, key);
#else
			size_t match = nextState(iFailed, key);
#endif
			while (iFailed != 0 && match == 0) {
				iFailed = getNode(iFailed)->failed;
#ifdef BINSCH
				match = nextStateByBinary(iFailed, key);
#else
				match = nextState(iFailed, key);
#endif
			}
			pChild->failed = match;

			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void match(unsigned char *content)
{
	trienode_ptr pCursor = root;
	while(*content != '\0') {
#if defined ADVSCH
		trienode_ptr pNext = pCursor->next(pCursor, *content);
#elif defined BINSCH
		trienode_ptr pNext = nextNodeByBinary(pCursor, *content);
#else
		trienode_ptr pNext = nextNode(pCursor, *content);
#endif
		while(pCursor != root && pNext == root) {
			pCursor = getNode(pCursor->failed);
#if defined ADVSCH
			pNext = pCursor->next(pCursor, *content);
#elif defined BINSCH
			pNext = nextNodeByBinary(pCursor, *content);
#else
			pNext = nextNode(pCursor, *content);
#endif
		}
		pCursor = pNext;
		content++;
		while (pNext != root) {
			if (pNext->flag & 2)
				printKeyword(pNext);
			pNext = getNode(pNext->failed);
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *fpdict, *fpcorpus;
	initTrie();

	time_t s0 = time(NULL);
	// 建立字典树
	fpdict = fopen("sdict", "rb");
	if (fpdict == NULL) return -1;
	constructTrie(fpdict);
	fclose(fpdict);
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %ld\n", s1-s0);

	// 按bfs顺序排序节点
	sortForBFS();
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %ld\n", s2-s1);
	
	// 建立父指针关系
	rebuildParentByDFS();
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %ld\n", s3-s2);
	
	// 构建自动机
	constructAutomation();
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %ld\n", s4-s3);

	fpcorpus = fopen("corpus", "rb");
	unsigned char company[100000];
	while(fscanf(fpcorpus, "%s", company) != EOF) {
		match(company);
	}
	fclose(fpcorpus);
	time_t s5 = time(NULL);
	fprintf(stderr, "s5: %ld\n", s5-s4);

	closeTrie();
	return 0;
}


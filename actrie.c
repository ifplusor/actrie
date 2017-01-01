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

#define BINARY

#define REGIONSIZE 2048
#define REGIONOFFSET 18
#define POSITIONMASK 0x0003FFFF

const size_t POOLREGIONSIZE = REGIONSIZE;
const size_t POOLPOSITIONSIZE = POSITIONMASK+1;

typedef struct node {
	size_t child,brother,parent,failed;
	unsigned char key;
	unsigned char flag;
	short len;
}TRIENODE,*PTRIENODE;

PTRIENODE nodepool[REGIONSIZE],root;
size_t autoindex = 0;


size_t allocNode()
{
	size_t region = autoindex >> REGIONOFFSET;
	size_t position = autoindex & POSITIONMASK;
	if (nodepool[region] == NULL) {
		//printf("call malloc!\n");
		PTRIENODE pnode = (PTRIENODE) malloc(sizeof(TRIENODE)*POOLPOSITIONSIZE);
		if (pnode != NULL) {
			nodepool[region] = pnode;
		} else {
			return -1;
		}
		memset(pnode, 0, sizeof(TRIENODE)*POOLPOSITIONSIZE);
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

PTRIENODE getNode(size_t index)
{
	size_t region = index / POOLPOSITIONSIZE;
	size_t position = index % POOLPOSITIONSIZE;
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
	PTRIENODE pNode = root;
	size_t iNode = 0; // iParent保存pNode的index
	for (int i = 0; keyword[i] != '\0'; i++) {
		size_t iChild = pNode->child, iBrother; // iBrother跟踪iChild
		PTRIENODE pChild = NULL;
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
			PTRIENODE pc = getNode(index);
			if (pc == NULL) {
				return FALSE;
			}
			pc->key = keyword[i];
			if (pChild == NULL) {
				// 没有子节点
				pNode->child = index;
				pc->parent = iNode;
			} else {
				// 尾插法，无法保证后无向前指针
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
		PTRIENODE pChild = getNode(iChild);
		if (pChild->key == key)
			break;
		iChild = pChild->brother;
	}
	return iChild;
}

PTRIENODE nextNode(PTRIENODE pNode, unsigned char key)
{
	size_t iChild = pNode->child;
	while (iChild != 0) {
		PTRIENODE pChild = getNode(iChild);
		if (pChild->key == key)
			return pChild;
		iChild = pChild->brother;
	}
	return root;
}

size_t nextStateByBinary(size_t iNode, unsigned char key)
{
	PTRIENODE pNode = getNode(iNode);
	if (pNode->len > 0) {
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		while (left <= right) {
			size_t middle = (left+right)>>1;
			PTRIENODE pMiddle = getNode(middle);
			if (pMiddle->key == key) {
				return middle;
			} else if (pMiddle->key > key) {
				right = middle-1;
			} else {
				left = middle+1;
			}
		}
	}
	return 0;
}

PTRIENODE nextNodeByBinary(PTRIENODE pNode, unsigned char key)
{
	if (pNode->len > 0) {
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		while (left <= right) {
			size_t middle = (left+right)>>1;
			PTRIENODE pMiddle = getNode(middle);
			if (pMiddle->key == key) {
				return pMiddle;
			} else if (pMiddle->key > key) {
				right = middle-1;
			} else {
				left = middle+1;
			}
		}
	}
	return root;
}

void swapNodeData(PTRIENODE pa, PTRIENODE pb)
{
	char *pva = (char*) pa;
	char *pvb = (char*) pb;
	for (int i = 0; i < sizeof(TRIENODE); i++) {
		pva[i] ^= pvb[i];
		pvb[i] ^= pva[i];
		pva[i] ^= pvb[i];
	}
}

size_t swapNode(size_t iChild, size_t iTarget)
{
	PTRIENODE pChild = getNode(iChild);
	if (iChild != iTarget) {
		PTRIENODE pTarget = getNode(iTarget);

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
		PTRIENODE ptmp = pChild;
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

void printKeywordByRecursion(PTRIENODE pNode)
{
	if (pNode == root) {
		return;
	}

	printKeywordByRecursion(getNode(pNode->parent));
	putc(pNode->key, stdout);
}

void printKeyword(PTRIENODE pNode)
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
	printf("pattern: %d, node: %d\n", i, autoindex);
	printf("constructTrie succeed!\n");
}

void sortForBFS()
{
	size_t iTarget = 1;
	for (size_t i = 0; i < iTarget; i++) { // 隐式bfs队列
		//printf("sort process: %d\n", i);
		PTRIENODE pNode = getNode(i);
		// 将pNode的子节点调整到iTarget位置（加入队列）
		size_t iChild = pNode->child;
		while (iChild != 0) {
			// swap iChild与iTarget
			// 建树时的尾插法担保兄弟节点不会交换，且在重排后是稳定的
			iChild = swapNode(iChild, iTarget);
			iTarget++;
		}
	}
	printf("sort succeed!\n");
}

void setParentByDFS(size_t current, size_t parent)
{
	PTRIENODE pNode = getNode(current);
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
	printf("rebuild parent succeed!\n");
}

void constructAutomation()
{
	for (size_t index = 1; index < autoindex; index++) { // bfs
		PTRIENODE pNode = getNode(index);
		size_t iChild = pNode->child;
		while (iChild != 0) {
			PTRIENODE pChild = getNode(iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->failed;
#ifdef BINARY
			size_t match = nextStateByBinary(iFailed, key);
#else
			size_t match = nextState(iFailed, key);
#endif
			while (iFailed != 0 && match == 0) {
				iFailed = getNode(iFailed)->failed;
#ifdef BINARY
				match = nextStateByBinary(iFailed, key);
#else
				match = nextState(iFailed, key);
#endif
			}
			pChild->failed = match;

			iChild = pChild->brother;
		}
	}
	printf("construct AC automation succeed!\n");
}

void match(unsigned char *content)
{
	PTRIENODE pCursor = root;
	while(*content != '\0') {
#ifdef BINARY
		PTRIENODE pNext = nextNodeByBinary(pCursor, *content);
#else
		PTRIENODE pNext = nextNode(pCursor, *content);
#endif
		while(pCursor != root && pNext == root) {
			pCursor = getNode(pCursor->failed);
#ifdef BINARY
			pNext = nextNodeByBinary(pCursor, *content);
#else
			pNext = nextNode(pCursor, *content);
#endif
		}
		if (pNext->flag & 2) {
			printKeyword(pNext);
		}
		pCursor = pNext;
		content++;
	}
}

int main(int argc, char *argv[])
{
	FILE *fpdict, *fpcorpus;
	initTrie();

	time_t s0 = time(NULL);
	// 建立字典树
	fpdict = fopen("sdict.utf8", "rb");
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
	fprintf(stderr, "s4: %ld\n", s5-s4);

	closeTrie();
	return 0;
}


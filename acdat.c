#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>


#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif


#define REGIONSIZE 0x00001000
#define REGIONOFFSET 18
#define POSITIONMASK 0x0003FFFF

const size_t POOLREGIONSIZE = REGIONSIZE;
const size_t POOLPOSITIONSIZE = POSITIONMASK+1;


long long getSystemTime()
{
	struct timeb t;
	ftime(&t);
	return t.time*1000+t.millitm;
}

// Prime Trie
// ========================================================

typedef struct node {
	size_t child,brother,failed;
	union {
		size_t parent;
		size_t datidx;
	} pd;
	unsigned short len;
	unsigned char flag;
	unsigned char key;
} trienode, *trienode_ptr;

static trienode_ptr nodepool[REGIONSIZE],root;
static size_t autoindex = 0;

size_t allocNode()
{
	size_t region = autoindex >> REGIONOFFSET;
	size_t position = autoindex & POSITIONMASK;
#ifdef CHECK
	if (region >= POOLREGIONSIZE) return -1;
#endif
	if (nodepool[region] == NULL) {
		trienode_ptr pnode = (trienode_ptr) malloc(sizeof(trienode)*POOLPOSITIONSIZE);
		if (pnode == NULL) return -1;
		nodepool[region] = pnode;
		memset(pnode, 0, sizeof(trienode)*POOLPOSITIONSIZE);
	}
#ifdef CHECK
	if (nodepool[region][position].flag & 1) return -1;
#endif
	nodepool[region][position].flag |= 1;
	return autoindex++;
}

static inline trienode_ptr getNode(size_t index)
{
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
	for (int i = 0; i < POOLREGIONSIZE; i++) nodepool[i] = NULL;
	autoindex = 0;
	root = getNode(allocNode());
}

void closeTrie()
{
	for (int i = 0; i < POOLREGIONSIZE; i++) {
		if (nodepool[i] != NULL) free(nodepool[i]);
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
			if (pChild->key == keyword[i]) break;
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
			if (index == -1) return FALSE;
			trienode_ptr pc = getNode(index);
			if (pc == NULL) return FALSE;
			pc->key = keyword[i];
			if (pChild == NULL) {
				// 没有子节点
				pNode->child = index;
				pc->pd.parent = iNode;
			} else {
				// 尾插法，无法保证后无向前指针
				pChild->brother = index;
				pc->pd.parent = iBrother;
			}
			pNode->len++;
			iNode = index;
			pNode = pc;
		}
	}
	pNode->flag |= 2;
	return TRUE;
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
			if (pMiddle->key == key)
				return middle;
			else if (pMiddle->key > key)
				right = middle-1;
			else
				left = middle+1;
		}
#endif
	}
	return 0;
}

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
}

size_t swapNode(size_t iChild, size_t iTarget)
{
	trienode_ptr pChild = getNode(iChild);
	if (iChild != iTarget) {
		trienode_ptr pTarget = getNode(iTarget);

		// 常量
		const size_t ipc = pChild->pd.parent;
		const size_t icc = pChild->child;
		const size_t ibc = pChild->brother;
		const BOOL bcc = getNode(ipc)->child == iChild;
		const size_t ipt = pTarget->pd.parent;
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
			pTarget->pd.parent = iTarget;
			if (bct) {
				pChild->child = iChild;
				if (ibc != 0) getNode(ibc)->pd.parent = iTarget;
			} else {
				pChild->brother = iChild;
				if (icc != 0) getNode(icc)->pd.parent = iTarget;
			}
		} else {
			if (bct)
				getNode(ipt)->child = iChild;
			else
				getNode(ipt)->brother = iChild;
			if (icc != 0) getNode(icc)->pd.parent = iTarget;
			if (ibc != 0) getNode(ibc)->pd.parent = iTarget;
		}

		// 调整直接上级指针
		if (bcc)
			getNode(ipc)->child = iTarget;
		else
			getNode(ipc)->brother = iTarget;

		// 调整下级指针
		if (ict != 0) getNode(ict)->pd.parent = iChild;
		if (ibt != 0) getNode(ibt)->pd.parent = iChild;
	}
	return pChild->brother;
}

void constructTrie(FILE *fp)
{
	int i = 0;
	unsigned char keyword[1000];
	while (fscanf(fp, "%s", keyword) != EOF) {
		//printf("%s\n", keyword);
		if (!addKeyword(keyword)) break;
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
	}
	fprintf(stderr, "sort succeed!\n");
}


// Double-Array Trie
// ========================================================

typedef struct datnode {
	size_t base,check;
	union {
		size_t next;
		size_t failed;
	} nf;
	union {
		size_t last;
		struct {
			unsigned char flag;
			unsigned char key;
		} f;
	} lf;
} datnode, *datnode_ptr;

static datnode_ptr datnodepool[REGIONSIZE],datlead,datroot;
static const size_t datrootidx = 255;

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
void constructDat(trienode_ptr pNode, size_t datindex)
{
	datnode_ptr pDatNode = getDatNode(datindex);
	pDatNode->lf.f.key = pNode->key;
	pDatNode->lf.f.flag = pNode->flag;
		
	if (pNode->flag & 2) count++;

	if (pNode->child == 0) return;

	unsigned char child[256];
	int len = 0;
	trienode_ptr pChild = getNode(pNode->child);
	while (pChild != root) {
		child[len++] = pChild->key;
		pChild = getNode(pChild->brother);
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
	pChild = getNode(pNode->child);
	while (pChild != root) {
		pChild->pd.datidx = pDatNode->base + pChild->key;
		constructDat(pChild, pChild->pd.datidx);
		pChild = getNode(pChild->brother);
	}
}

void afterConstructDat()
{
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

void printKeywordByRecursion(datnode_ptr pNode)
{
	if (pNode == datroot) return;

	printKeywordByRecursion(getDatNode(pNode->check));
	putc(pNode->lf.f.key, stdout);
}

void printKeyword(datnode_ptr pNode)
{
	printKeywordByRecursion(pNode);
	putc('\n', stdout);
}

void datMatch(unsigned char *content)
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
			if (pNext->lf.f.flag & 2) printKeyword(pNext);
			iCursor = iNext;
			pCursor = pNext;
			cur++;
		}
	}
}


void constructAutomation()
{
	trienode_ptr pNode = root;
	size_t iChild = pNode->child;
	while (iChild != 0) {
		trienode_ptr pChild = getNode(iChild);
		// 设置 DAT 的 failed 域
		getDatNode(pChild->pd.datidx)->nf.failed = datrootidx;
		iChild = pChild->brother;
	}
	for (size_t index = 1; index < autoindex; index++) { // bfs
		pNode = getNode(index);
		iChild = pNode->child;
		while (iChild != 0) {
			trienode_ptr pChild = getNode(iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->failed;
			size_t match = nextStateByBinary(iFailed, key);
			while (iFailed != 0 && match == 0) {
				iFailed = getNode(iFailed)->failed;
				match = nextStateByBinary(iFailed, key);
			}
			pChild->failed = match;
			
			// 设置 DAT 的 failed 域
#ifdef EXTFLAG
			datnode_ptr pDatChild = getDatNode(pChild->pd.datidx);
			if (match != 0) {
				trienode_ptr pFailed = getNode(match);
				pDatChild->nf.failed = pFailed->pd.datidx;
				datnode_ptr pDatFailed = getDatNode(pFailed->pd.datidx);
				if (pDatFailed->lf.f.flag & 6) pDatFailed->lf.f.flag |= 4;
			} else {
				pDatChild->nf.failed = datrootidx;
			}
#else
			getDatNode(pChild->pd.datidx)->nf.failed = match!=0?getNode(match)->pd.datidx:datrootidx;
#endif
			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void acdatMatch(unsigned char *content)
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
					printKeyword(pNext);
				pNext = getDatNode(pNext->nf.failed);
			}
		}
		content++;
	}
}

int main(int argc, char *argv[])
{
	FILE *fpdict = fopen("sdict", "rb");
	if (fpdict == NULL) return -1;

	initTrie();
	initDat();

	// 建立字典树
	time_t s0 = time(NULL);
	constructTrie(fpdict);
	time_t s1 = time(NULL);
	fprintf(stderr, "s1: %ld\n", s1-s0);
	fclose(fpdict);
	
	// 排序字典树节点
	sortForBFS();
	time_t s2 = time(NULL);
	fprintf(stderr, "s2: %ld\n", s2 -s1);

	// 建立 Double-Array Trie
	constructDat(root, datrootidx);
	afterConstructDat();
	time_t s3 = time(NULL);
	fprintf(stderr, "s3: %ld, %ld\n", s3-s2, count);
	
	// 建立 AC 自动机
	constructAutomation();
	time_t s4 = time(NULL);
	fprintf(stderr, "s4: %ld\n", s4-s3);
	
	closeTrie();

	// test match
	FILE *fpcorpus = fopen("corpus", "rb");
	if (fpcorpus != NULL) {
		unsigned char company[100000];
		long long s5 = getSystemTime();
		while (fscanf(fpcorpus, "%s", company) != EOF) acdatMatch(company);
		long long s6 = getSystemTime();
		fprintf(stderr, "s5: %lld\n", s6-s5);
		fclose(fpcorpus);
	}

	// 释放字典树
	closeDat();

	return 0;
}

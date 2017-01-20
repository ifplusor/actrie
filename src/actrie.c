#include "actrie.h"


// Prime Trie
// ========================================================

size_t trie_size(trie_ptr self)
{
	return self->_autoindex;
}

static size_t trie_alloc_node(trie_ptr self)
{
	size_t region = self->_autoindex >> REGION_OFFSET;
//	size_t position = self->_autoindex & POSITION_MASK;
#ifdef CHECK
	if (region >= POOL_REGION_SIZE)
		return (size_t)-1;
#endif // CHECK
	if (self->_nodepool[region] == NULL) {
		trie_node_ptr pnode =
				(trie_node_ptr) malloc(sizeof(trie_node) * POOL_POSITION_SIZE);
		if (pnode == NULL) return (size_t) -1;
		self->_nodepool[region] = pnode;
		memset(pnode, 0, sizeof(trie_node) * POOL_POSITION_SIZE);
	}
#ifdef CHECK
	if (self->_autoindex == (size_t)-1)
		return (size_t)-1;
#endif // CHECK
	return self->_autoindex++;
}

static inline trie_node_ptr trie_access_node(trie_ptr self, size_t index)
{
	size_t region = index >> REGION_OFFSET;
	size_t position = index & POSITION_MASK;
#ifdef CHECK
	if (region >= POOL_REGION_SIZE || self->_nodepool[region] == NULL
			|| index >= self->_autoindex) {
		return NULL;
	}
#endif // CHECK
	return &self->_nodepool[region][position];
}

trie_node_ptr trie_access_node_export(trie_ptr self, size_t index)
{
	size_t region = index >> REGION_OFFSET;
	size_t position = index & POSITION_MASK;
#ifdef CHECK
	if (region >= POOL_REGION_SIZE || self->_nodepool[region] == NULL
			|| index >= self->_autoindex) {
		return NULL;
	}
#endif // CHECK
	return &self->_nodepool[region][position];
}

bool trie_add_keyword(trie_ptr self, const unsigned char keyword[],
					  size_t len, const char extra[])
{
	trie_node_ptr pNode = self->root;
	size_t iNode = 0; // iParent保存pNode的index
	for (size_t i = 0; i < len; ++i) {
		// 当创建时，使用插入排序的方法，以保证子节点链接关系有序
		size_t iChild = pNode->child, iBrother = 0; // iBrother跟踪iChild
		trie_node_ptr pChild = NULL;
		while (iChild != 0) {
			// 从所有孩子中查找
			pChild = trie_access_node(self, iChild);
			if (pChild->key >= keyword[i]) break;
			iBrother = iChild;
			iChild = pChild->brother;
		}

		if (iChild != 0 && pChild->key == keyword[i]) {
			// 找到
			iNode = iChild;
			pNode = pChild;
		} else {
			// 没找到, 创建.
			size_t index = trie_alloc_node(self);
			if (index == -1) return false;

			trie_node_ptr pc = trie_access_node(self, index);
			if (pc == NULL) return false;
			pc->key = keyword[i];

			if (pChild == NULL) {
				// 没有子节点
				pNode->child = index;
				pc->trie_parent = iNode;
			} else {
				if (iBrother == 0) {
					// 插入链表头
					pc->trie_parent = iNode;
					pc->brother = pNode->child;
					pNode->child = index;
					pChild->trie_parent = index;
				} else if (pChild->key < keyword[i]) {
					// 插入链表尾
					pc->trie_parent = iBrother;
					pChild->brother = index;
				} else {
					trie_node_ptr pBrother = trie_access_node(self, iBrother);
					pc->trie_parent = iBrother;
					pc->brother = iChild;
					pBrother->brother = index;
					pChild->trie_parent = index;
				}
			}

			pNode->len++;
			iNode = index;
			pNode = pc;
		}
	}
	pNode->trie_keyword = keyword;
	pNode->trie_extra = extra;
	return true;
}

size_t trie_next_state(trie_ptr self, size_t iNode, unsigned char key)
{
	size_t iChild = trie_access_node(self, iNode)->child;
	while (iChild != 0) {
		trie_node_ptr pChild = trie_access_node(self, iChild);
		if (pChild->key == key) break;
		iChild = pChild->brother;
	}
	return iChild;
}

trie_node_ptr trie_next_node(trie_ptr self, trie_node_ptr pNode,
							 unsigned char key)
{
	trie_node_ptr pChild = trie_access_node(self, pNode->child);
	while (pChild != self->root) {
		if (pChild->key == key) return pChild;
		pChild = trie_access_node(self, pChild->brother);
	}
	return self->root;
}

size_t trie_next_state_by_binary(trie_ptr self, size_t iNode, unsigned char key)
{
	trie_node_ptr pNode = trie_access_node(self, iNode);
	if (pNode->len >= 1) {
		size_t left = pNode->child;
		size_t right = left + pNode->len - 1;
		if (key < trie_access_node(self, left)->key ||
			trie_access_node(self, right)->key < key)
			return 0;
		while (left <= right) {
			size_t middle = (left + right) >> 1;
			trie_node_ptr pMiddle = trie_access_node(self, middle);
			if (pMiddle->key == key) {
				return middle;
			} else if (pMiddle->key > key) {
				right = middle - 1;
			} else {
				left = middle + 1;
			}
		}
	}
	return 0;
}

trie_node_ptr trie_next_node_by_binary(trie_ptr self, trie_node_ptr pNode,
									   unsigned char key)
{
	if (key < trie_access_node(self, pNode->child)->key ||
		trie_access_node(self, pNode->child + pNode->len - 1)->key < key)
		return self->root;

	size_t left = pNode->child;
	size_t right = left + pNode->len - 1;
	while (left <= right) {
		size_t middle = (left + right) >> 1;
		trie_node_ptr pMiddle = trie_access_node(self, middle);
		if (pMiddle->key < key) {
			left = middle + 1;
		} else if (pMiddle->key > key) {
			right = middle - 1;
		} else {
			return pMiddle;
		}
	}

	return self->root;
}


void trie_swap_node_data(trie_node_ptr pa, trie_node_ptr pb)
{
	pa->child ^= pb->child;
	pb->child ^= pa->child;
	pa->child ^= pb->child;

	pa->brother ^= pb->brother;
	pb->brother ^= pa->brother;
	pa->brother ^= pb->brother;

	pa->trie_parent ^= pb->trie_parent;
	pb->trie_parent ^= pa->trie_parent;
	pa->trie_parent ^= pb->trie_parent;

	// extra
	pa->trie_failed ^= pb->trie_failed;
	pb->trie_failed ^= pa->trie_failed;
	pa->trie_failed ^= pb->trie_failed;

	// keyword
	pa->trie_p0 ^= pb->trie_p0;
	pb->trie_p0 ^= pa->trie_p0;
	pa->trie_p0 ^= pb->trie_p0;

	pa->len ^= pb->len;
	pb->len ^= pa->len;
	pa->len ^= pb->len;

	pa->key ^= pb->key;
	pb->key ^= pa->key;
	pa->key ^= pb->key;
}

size_t trie_swap_node(trie_ptr self, size_t iChild, size_t iTarget)
{
	trie_node_ptr pChild = trie_access_node(self, iChild);
	if (iChild != iTarget) {
		trie_node_ptr pTarget = trie_access_node(self, iTarget);

		// 常量
		const size_t ipc = pChild->trie_parent;
		const size_t icc = pChild->child;
		const size_t ibc = pChild->brother;
		const bool bcc = trie_access_node(self, ipc)->child == iChild;
		const size_t ipt = pTarget->trie_parent;
		const size_t ict = pTarget->child;
		const size_t ibt = pTarget->brother;
		const bool bct = trie_access_node(self, ipt)->child == iTarget;

		// 交换节点内容
		trie_swap_node_data(pChild, pTarget);
		trie_node_ptr ptmp = pChild;
		pChild = pTarget;
		pTarget = ptmp;

		// 考虑上下级节点交换
		// 调整target的上级，child的下级
		if (ipt == iChild) {
			// target是child的下级，child是target的上级
			pTarget->trie_parent = iTarget;
			if (bct) {
				pChild->child = iChild;
				if (ibc != 0)
					trie_access_node(self, ibc)->trie_parent = iTarget;
			} else {
				pChild->brother = iChild;
				if (icc != 0)
					trie_access_node(self, icc)->trie_parent = iTarget;
			}
		} else {
			if (bct)
				trie_access_node(self, ipt)->child = iChild;
			else
				trie_access_node(self, ipt)->brother = iChild;
			if (icc != 0) trie_access_node(self, icc)->trie_parent = iTarget;
			if (ibc != 0) trie_access_node(self, ibc)->trie_parent = iTarget;
		}

		// 调整直接上级指针
		if (bcc)
			trie_access_node(self, ipc)->child = iTarget;
		else
			trie_access_node(self, ipc)->brother = iTarget;

		// 调整下级指针
		if (ict != 0) trie_access_node(self, ict)->trie_parent = iChild;
		if (ibt != 0) trie_access_node(self, ibt)->trie_parent = iChild;
	}
	return pChild->brother;
}

//static size_t count = 0;

bool trie_fetch_key(const char keyword[], const char extra[], void *argv[])
{
	trie_ptr trie = argv[0];
	if (!trie_add_keyword(trie, keyword, strlen(keyword), extra)) {
		fprintf(stderr, "fatal: encounter error when add keywords.\n");
		return false;
	}
	//++count;
	return true;
}

trie_ptr trie_alloc()
{
	trie_ptr p = (trie_ptr)malloc(sizeof(trie));
	if (p == NULL)
		goto trie_alloc_failed;

	for (int i = 0; i < POOL_REGION_SIZE; i++)
		p->_nodepool[i] = NULL;
	p->_autoindex = 0;

	size_t root = trie_alloc_node(p);
	if (root == (size_t)-1)
		goto trie_alloc_failed;

	p->root = trie_access_node(p, root);
	if (p->root == NULL)
		goto trie_alloc_failed;

	p->_dict = dict_alloc();
	if (p->_dict == NULL)
		goto trie_alloc_failed;

	return p;

trie_alloc_failed:
	trie_release(p);
	return NULL;
}

void trie_release(trie_ptr p)
{
	if (p != NULL) {
		dict_release(p->_dict);
		for (int i = 0; i < POOL_REGION_SIZE; i++) {
			if (p->_nodepool[i] != NULL)
				free(p->_nodepool[i]);
		}
		free(p);
	}
}

trie_ptr trie_construct_by_file(FILE *fp)
{
	trie_ptr p = trie_alloc();
	if (p == NULL)
		return NULL;

	void *argv[] = {p};
	if (!dict_parser_by_file(fp, p->_dict, trie_fetch_key, argv)) {
		trie_release(p);
		return NULL;
	}

	//fprintf(stderr, "pattern: %zu, node: %zu\n", count, self->_autoindex);
	fprintf(stderr, "construct trie succeed!\n");
	return p;
}

trie_ptr trie_construct_by_s(const char *s)
{
	trie_ptr prime_trie = trie_alloc();
	if (prime_trie == NULL) return NULL;

	void *argv[] = {prime_trie};
	if (!dict_parser_by_s(s, prime_trie->_dict, trie_fetch_key, argv)) {
		trie_release(prime_trie);
		return NULL;
	}

	//fprintf(stderr, "pattern: %zu, node: %zu\n", count, self->_autoindex);
	fprintf(stderr, "construct trie succeed!\n");
	return prime_trie;
}

void trie_sort_to_line(trie_ptr self)
{
	size_t iTarget = 1;
	for (size_t i = 0; i < iTarget; i++) { // 隐式bfs队列
		trie_node_ptr pNode = trie_access_node(self, i);
		// 将pNode的子节点调整到iTarget位置（加入队列）
		size_t iChild = pNode->child;
		while (iChild != 0) {
			// swap iChild与iTarget
			// 建树时的尾插法担保兄弟节点不会交换，且在重排后是稳定的
			iChild = trie_swap_node(self, iChild, iTarget);
			iTarget++;
		}
	}
	fprintf(stderr, "sort succeed!\n");
}

void trie_set_parent_by_dfs(trie_ptr self, size_t current, size_t parent)
{
	trie_node_ptr pNode = trie_access_node(self, current);
	pNode->trie_parent = parent;
	if (pNode->child != 0)
		trie_set_parent_by_dfs(self, pNode->child, current);
	if (pNode->brother != 0)
		trie_set_parent_by_dfs(self, pNode->brother, parent);
}

void trie_rebuild_parent_relation(trie_ptr self)
{
	if (self->root->child != 0)
		trie_set_parent_by_dfs(self, self->root->child, 0);
	fprintf(stderr, "rebuild parent succeed!\n");
}

void trie_construct_automation(trie_ptr self)
{
	trie_node_ptr pNode = self->root;
	size_t iChild = pNode->child;
	while (iChild != 0) {
		trie_node_ptr pChild = trie_access_node_export(self, iChild);
		pChild->trie_failed = 0; // 设置 failed 域
		iChild = pChild->brother;
	}

	for (size_t index = 1; index < self->_autoindex; index++) { // bfs
		pNode = trie_access_node(self, index);
		iChild = pNode->child;
		while (iChild != 0) {
			trie_node_ptr pChild = trie_access_node(self, iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->trie_failed;
			size_t match = trie_next_state_by_binary(self, iFailed, key);
			while (iFailed != 0 && match == 0) {
				iFailed = trie_access_node(self, iFailed)->trie_failed;
				match = trie_next_state_by_binary(self, iFailed, key);
			}
			pChild->trie_failed = match;

			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void trie_ac_match(trie_ptr self, unsigned char content[], size_t len)
{
	trie_node_ptr pCursor = self->root;
	for (size_t i = 0; i < len; ++i) {
		trie_node_ptr pNext =
				trie_next_node_by_binary(self, pCursor, content[i]);
		while (pCursor != self->root && pNext == self->root) {
			pCursor = trie_access_node(self, pCursor->trie_failed);
			pNext = trie_next_node_by_binary(self, pCursor, content[i]);
		}
		pCursor = pNext;
		while (pNext != self->root) {
			if (pNext->trie_keyword != NULL)
				printf("%s\n", pNext->trie_keyword);
			pNext = trie_access_node(self, pNext->trie_failed);
		}
	}
}


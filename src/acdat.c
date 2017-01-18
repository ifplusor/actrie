#include "acdat.h"


// Trie 内部接口，仅限 Double-Array Trie 使用
size_t trie_size(trie_ptr self);

trie_node_ptr trie_access_node_export(trie_ptr self, size_t index);

size_t trie_next_state_by_binary(trie_ptr self, size_t iNode,
								 unsigned char key);


// Double-Array Trie
// ========================================================

const size_t DAT_ROOT_IDX = 255;

static inline dat_node_ptr dat_access_node(dat_trie_ptr self, size_t index)
{
	size_t region = index >> REGION_OFFSET;
	size_t position = index & POSITION_MASK;
	return &self->_nodepool[region][position];
}

static void dat_alloc_nodepool(dat_trie_ptr self, size_t region)
{
	dat_node_ptr pnode = (dat_node_ptr) malloc(
			sizeof(dat_node) * POOL_POSITION_SIZE);
	if (pnode == NULL) {
		fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
		exit(-1);
	}
	self->_nodepool[region] = pnode;
	// 节点初始化
	memset(pnode, 0, sizeof(dat_node) * POOL_POSITION_SIZE);
	size_t offset = region << REGION_OFFSET;
	for (int i = 0; i < POOL_POSITION_SIZE; ++i) {
		pnode[i].dat_next = offset + i + 1;
		pnode[i].dat_last = offset + i - 1;
	}
	pnode[POOL_POSITION_SIZE - 1].dat_next = 0;

	pnode[0].dat_last = self->_lead->dat_last;
	dat_access_node(self, self->_lead->dat_last)->dat_next = offset;
	self->_lead->dat_last = offset + POOL_POSITION_SIZE - 1;
}

static dat_node_ptr dat_access_node_with_alloc(dat_trie_ptr self, size_t index)
{
	size_t region = index >> REGION_OFFSET;
	size_t position = index & POSITION_MASK;
	if (region >= POOL_REGION_SIZE) return NULL;
	if (self->_nodepool[region] == NULL) dat_alloc_nodepool(self, region);
	return &self->_nodepool[region][position];
}

void dat_init(dat_trie_ptr self, match_dict_ptr dict)
{
	self->dict = dict;

	for (int i = 0; i < POOL_REGION_SIZE; ++i) self->_nodepool[i] = NULL;
	dat_node_ptr pnode = (dat_node_ptr) malloc(
			sizeof(dat_node) * POOL_POSITION_SIZE);
	if (pnode == NULL) {
		fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
		exit(-1);
	}
	memset(pnode, 0, sizeof(dat_node) * POOL_POSITION_SIZE);

	self->_nodepool[0] = pnode;
	self->_lead = &self->_nodepool[0][0];
	self->root = &self->_nodepool[0][DAT_ROOT_IDX];

	// 节点初始化
	for (size_t i = 1; i < POOL_POSITION_SIZE; ++i) {
		pnode[i].dat_next = i + 1;
		pnode[i].dat_last = i - 1;
	}
	pnode[POOL_POSITION_SIZE - 1].dat_next = 0;
	pnode[DAT_ROOT_IDX + 1].dat_last = 0;

	self->_lead->dat_next = DAT_ROOT_IDX + 1;
	self->_lead->dat_last = POOL_POSITION_SIZE - 1;
	self->_lead->check = 1;

	self->root->dat_depth = 0;
}

void dat_close(dat_trie_ptr self)
{
	for (int i = 0; i < POOL_REGION_SIZE; ++i) {
		if (self->_nodepool[i] != NULL) free(self->_nodepool[i]);
	}

	dict_release(self->dict);
	free(self->dict);
}

//static size_t count = 0;

static void dat_construct_by_dfs(dat_trie_ptr self, trie_ptr origin,
								 trie_node_ptr pNode, size_t datindex)
{
	dat_node_ptr pDatNode = dat_access_node(self, datindex);
	pDatNode->dat_keyword = pNode->trie_keyword;
	pDatNode->dat_extra = pNode->trie_extra;
	pDatNode->dat_key = pNode->key;
	pDatNode->dat_flag = pNode->flag;

	/*if (pDatNode->dat_flag & 2) {
		count++;
	}*/

	if (pNode->child == 0) return;

	unsigned char child[256];
	int len = 0;
	trie_node_ptr pChild = trie_access_node_export(origin, pNode->child);
	while (pChild != origin->root) {
		child[len++] = pChild->key;
		pChild = trie_access_node_export(origin, pChild->brother);
	}
	size_t pos = self->_lead->dat_next;
	while (1) {
		if (pos == 0) {
			// 扩容
			size_t region = 1;
			while (region < POOL_REGION_SIZE &&
				   self->_nodepool[region] != NULL)
				++region;
			if (region == POOL_REGION_SIZE) {
				fprintf(stderr,
						"alloc datnodepool failed: region full.\nexit.\n");
				exit(-1);
			}
			dat_alloc_nodepool(self, region);
			pos = (size_t) region << REGION_OFFSET;
		}
		// 检查: pos容纳第一个子节点
		size_t base = pos - child[0];
		for (int i = 1; i < len; ++i) {
			if (dat_access_node_with_alloc(self, base + child[i])->check != 0)
				goto checkfailed;
		}
		// base 分配成功
		pDatNode->base = base;
		for (int i = 0; i < len; ++i) {
			// 分配子节点
			dat_node_ptr pDatChild = dat_access_node(self, base + child[i]);
			pDatChild->check = datindex;
			dat_access_node(self, pDatChild->dat_next)->dat_last =
					pDatChild->dat_last;
			dat_access_node(self, pDatChild->dat_last)->dat_next =
					pDatChild->dat_next;
			// 对depth的改变要放在last指针改变之后
			pDatChild->dat_depth = pDatNode->dat_depth + 1;
		}
		break;
		checkfailed:
		pos = dat_access_node(self, pos)->dat_next;
	}

	// 构建子树
	pChild = trie_access_node_export(origin, pNode->child);
	while (pChild != origin->root) {
		pChild->trie_datidx = pDatNode->base + pChild->key;
		dat_construct_by_dfs(self, origin, pChild, pChild->trie_datidx);
		pChild = trie_access_node_export(origin, pChild->brother);
	}
}

static void dat_post_construct(dat_trie_ptr self)
{
	// 添加占位内存，防止匹配时出现非法访问
	int region = 1;
	while (region < POOL_REGION_SIZE && self->_nodepool[region] != NULL)
		++region;
	if (region >= POOL_REGION_SIZE) {
		fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
		exit(-1);
	} else {
		dat_node_ptr pnode = (dat_node_ptr) malloc(sizeof(dat_node) * 256);
		if (pnode == NULL) {
			fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
			exit(-1);
		}
		self->_nodepool[region] = pnode;
		// 节点初始化
		memset(pnode, 0, sizeof(dat_node) * 256);
	}
}

void dat_construct(dat_trie_ptr self, trie_ptr origin)
{
	dat_construct_by_dfs(self, origin, origin->root, DAT_ROOT_IDX);
	dat_post_construct(self);
	//fprintf(stderr, "pattern: %zu\n", count);
	fprintf(stderr, "construct double-array trie succeed!\n");
}

int dat_print_keyword_by_recursion(dat_trie_ptr self, dat_node_ptr pNode)
{
	if (pNode == self->root) return 0;

	int pos = dat_print_keyword_by_recursion(
			self, dat_access_node(self, pNode->check)) + 1;
	putc(pNode->dat_key, stdout);
	return pos;
}

void dat_print_keyword(dat_trie_ptr self, dat_node_ptr pNode)
{
	int len = dat_print_keyword_by_recursion(self, pNode);
	putc('\n', stdout);
}

void dat_match(dat_trie_ptr self, unsigned char content[], size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		size_t iCursor = DAT_ROOT_IDX;
		dat_node_ptr pCursor = self->root;
		for (size_t j = i; j < len; ++j) {
			size_t iNext = pCursor->base + content[i];
			dat_node_ptr pNext = dat_access_node(self, iNext);
			if (pNext->check != iCursor) break;
			if (pNext->dat_flag & 2) dat_print_keyword(self, pNext);
			iCursor = iNext;
			pCursor = pNext;
		}
	}
}


void dat_construct_automation(dat_trie_ptr self, trie_ptr origin)
{
	trie_node_ptr pNode = origin->root;
	size_t iChild = pNode->child;
	while (iChild != 0) {
		trie_node_ptr pChild = trie_access_node_export(origin, iChild);

		// 设置 failed 域
		pChild->trie_failed = 0;
		dat_access_node(self, pChild->trie_datidx)->dat_failed = DAT_ROOT_IDX;

		iChild = pChild->brother;
	}

	for (size_t index = 1; index < trie_size(origin); index++) { // bfs
		pNode = trie_access_node_export(origin, index);
		iChild = pNode->child;
		while (iChild != 0) {
			trie_node_ptr pChild = trie_access_node_export(origin, iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->trie_failed;
			size_t match = trie_next_state_by_binary(origin, iFailed, key);
			while (iFailed != 0 && match == 0) {
				iFailed = trie_access_node_export(origin, iFailed)->trie_failed;
				match = trie_next_state_by_binary(origin, iFailed, key);
			}
			pChild->trie_failed = match;

			// 设置 DAT 的 failed 域
			dat_access_node(self, pChild->trie_datidx)->dat_failed =
					match != 0 ?
					trie_access_node_export(origin, match)->trie_datidx :
					DAT_ROOT_IDX;
			iChild = pChild->brother;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}

void dat_ac_match(dat_trie_ptr self, unsigned char content[], size_t len)
{
	size_t iCursor = DAT_ROOT_IDX;
	dat_node_ptr pCursor = self->root;
	for (size_t i = 0; i < len; ++i) {
		size_t iNext = pCursor->base + content[i];
		dat_node_ptr pNext = dat_access_node(self, iNext);
		while (pCursor != self->root && pNext->check != iCursor) {
			iCursor = pCursor->dat_failed;
			pCursor = dat_access_node(self, iCursor);
			iNext = pCursor->base + content[i];
			pNext = dat_access_node(self, iNext);
		}
		if (pNext->check == iCursor) {
			iCursor = iNext;
			pCursor = pNext;
			while (pNext != self->root) {
				if (pNext->dat_flag & 2)
					dat_print_keyword(self, pNext);
				pNext = dat_access_node(self, pNext->dat_failed);
			}
		}
	}
}


// Acdat Context
// ===================================================

void dat_init_context(dat_context_ptr context, dat_trie_ptr trie,
					  unsigned char content[], size_t len)
{
	context->trie = trie;
	context->content = content;
	context->len = len;

	context->_i = 0;
	context->_iCursor = DAT_ROOT_IDX;
	context->_pCursor = context->trie->root;
	context->out_matched = context->trie->root;
}

bool dat_ac_next(dat_context_ptr context)
{
	// 检查当前匹配点向树根的路径上是否还有匹配的词
	while (context->out_matched != context->trie->root) {
		context->out_matched = dat_access_node(context->trie,
											 context->out_matched->dat_failed);
		if (context->out_matched->dat_flag & 2) {
			return true;
		}
	}

	// 执行匹配
	for (; context->_i < context->len; context->_i++) {
		size_t iNext = context->_pCursor->base + context->content[context->_i];
		dat_node_ptr pNext = dat_access_node(context->trie, iNext);
		while (context->_pCursor != context->trie->root &&
			   pNext->check != context->_iCursor) {
			context->_iCursor = context->_pCursor->dat_failed;
			context->_pCursor = dat_access_node(context->trie,
												context->_iCursor);
			iNext = context->_pCursor->base + context->content[context->_i];
			pNext = dat_access_node(context->trie, iNext);
		}
		if (pNext->check == context->_iCursor) {
			context->_iCursor = iNext;
			context->_pCursor = pNext;
			while (pNext != context->trie->root) {
				if (pNext->dat_flag & 2) {
					context->out_matched = pNext;
					context->_i++;
					return true;
				}
				pNext = dat_access_node(context->trie, pNext->dat_failed);
			}
		}
	}
	return false;
}


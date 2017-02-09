#include "acdat.h"


/* Trie 内部接口，仅限 Double-Array Trie 使用 */
size_t trie_size(trie_ptr self);

trie_node_ptr trie_access_node_export(trie_ptr self, size_t index);

size_t trie_next_state_by_binary(trie_ptr self, size_t iNode,
								 unsigned char key);


// Double-Array Trie
// ========================================================

const size_t DAT_ROOT_IDX = 255;

#if defined(_WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

static inline dat_node_ptr dat_access_node(dat_trie_ptr self, size_t index)
{
	size_t region = index >> REGION_OFFSET;
	size_t position = index & POSITION_MASK;
	return &self->_nodepool[region][position];
}

static void dat_alloc_nodepool(dat_trie_ptr self, size_t region)
{
	size_t offset;
	int i;

	dat_node_ptr pnode = (dat_node_ptr) malloc(
			sizeof(dat_node) * POOL_POSITION_SIZE);
	if (pnode == NULL) {
		fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
		exit(-1);
	}
	self->_nodepool[region] = pnode;
	/* 节点初始化 */
	memset(pnode, 0, sizeof(dat_node) * POOL_POSITION_SIZE);
	offset = region << REGION_OFFSET;
	for (i = 0; i < POOL_POSITION_SIZE; ++i) {
		pnode[i].dat_free_next = offset + i + 1;
		pnode[i].dat_free_last = offset + i - 1;
	}
	pnode[POOL_POSITION_SIZE - 1].dat_free_next = 0;

	pnode[0].dat_free_last = self->_lead->dat_free_last;
	dat_access_node(self, self->_lead->dat_free_last)->dat_free_next = offset;
	self->_lead->dat_free_last = offset + POOL_POSITION_SIZE - 1;
}

static dat_node_ptr dat_access_node_with_alloc(dat_trie_ptr self, size_t index)
{
	size_t region = index >> REGION_OFFSET;
	size_t position = index & POSITION_MASK;
	if (region >= POOL_REGION_SIZE) return NULL;
	if (self->_nodepool[region] == NULL) dat_alloc_nodepool(self, region);
	return &self->_nodepool[region][position];
}

//static size_t count = 0;

static void dat_construct_by_dfs(dat_trie_ptr self, trie_ptr origin,
								 trie_node_ptr pNode, size_t datindex)
{
	unsigned char child[256];
	int len = 0;
	trie_node_ptr pChild;
	size_t pos;

	dat_node_ptr pDatNode = dat_access_node(self, datindex);
	pDatNode->dat_dictidx = pNode->trie_dictidx;

	/*if (pDatNode->dat_flag & 2) {
		count++;
	}*/

	if (pNode->trie_child == 0) return;

	pChild = trie_access_node_export(origin, pNode->trie_child);
	while (pChild != origin->root) {
		child[len++] = pChild->key;
		pChild = trie_access_node_export(origin, pChild->trie_brother);
	}
	pos = self->_lead->dat_free_next;
	while (1) {
		int i;
		size_t base;

		if (pos == 0) {
			/* 扩容 */
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
		/* 检查: pos容纳第一个子节点 */
		base = pos - child[0];
		for (i = 1; i < len; ++i) {
			if (dat_access_node_with_alloc(self, base + child[i])->check != 0)
				goto checkfailed;
		}
		/* base 分配成功 */
		pDatNode->base = base;
		for (i = 0; i < len; ++i) {
			/* 分配子节点 */
			dat_node_ptr pDatChild = dat_access_node(self, base + child[i]);
			pDatChild->check = datindex;
			dat_access_node(self, pDatChild->dat_free_next)->dat_free_last =
					pDatChild->dat_free_last;
			dat_access_node(self, pDatChild->dat_free_last)->dat_free_next =
					pDatChild->dat_free_next;
			pDatChild->dat_depth = pDatNode->dat_depth + 1;
		}
		break;
checkfailed:
		pos = dat_access_node(self, pos)->dat_free_next;
	}

	/* 构建子树 */
	pChild = trie_access_node_export(origin, pNode->trie_child);
	while (pChild != origin->root) {
		pChild->trie_datidx = pDatNode->base + pChild->key;
		dat_construct_by_dfs(self, origin, pChild, pChild->trie_datidx);
		pChild = trie_access_node_export(origin, pChild->trie_brother);
	}
}

static void dat_post_construct(dat_trie_ptr self)
{
	/* 添加占位内存，防止匹配时出现非法访问 */
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
		/* 节点初始化 */
		memset(pnode, 0, sizeof(dat_node) * 256);
	}
}

dat_trie_ptr dat_alloc()
{
	dat_node_ptr pnode;
	dat_trie_ptr p;
	size_t i;
	
	pnode = (dat_node_ptr) malloc(sizeof(dat_node) * POOL_POSITION_SIZE);
	if (pnode == NULL) {
		fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
		return NULL;
	}
	memset(pnode, 0, sizeof(dat_node) * POOL_POSITION_SIZE);

	p = (dat_trie_ptr) malloc(sizeof(dat_trie));
	if (p == NULL) {
		return NULL;
	}

	for (i = 0; i < POOL_REGION_SIZE; ++i)
		p->_nodepool[i] = NULL;

	p->_nodepool[0] = pnode;
	p->_lead = &p->_nodepool[0][0];
	p->root = &p->_nodepool[0][DAT_ROOT_IDX];

	/* 节点初始化 */
	for (i = 1; i < POOL_POSITION_SIZE; ++i) {
		pnode[i].dat_free_next = i + 1;
		pnode[i].dat_free_last = i - 1;
	}
	pnode[POOL_POSITION_SIZE - 1].dat_free_next = 0;
	pnode[DAT_ROOT_IDX + 1].dat_free_last = 0;

	p->_lead->dat_free_next = DAT_ROOT_IDX + 1;
	p->_lead->dat_free_last = POOL_POSITION_SIZE - 1;
	p->_lead->check = 1;

	p->root->dat_depth = 0;

	return p;
}

void dat_release(dat_trie_ptr p)
{
	if (p != NULL) {
		int i;
		dict_release(p->_dict);
		for (i = 0; i < POOL_REGION_SIZE; ++i) {
			if (p->_nodepool[i] != NULL)
				free(p->_nodepool[i]);
		}
		free(p);
	}
}

dat_trie_ptr dat_construct(trie_ptr origin)
{
	dat_trie_ptr p = dat_alloc();
	if (p == NULL)
		return NULL;

	p->_dict = dict_assign(origin->_dict);

	dat_construct_by_dfs(p, origin, origin->root, DAT_ROOT_IDX);
	dat_post_construct(p);

	//fprintf(stderr, "pattern: %zu\n", count);
	fprintf(stderr, "construct double-array trie succeed!\n");
	return p;
}

void dat_construct_automation(dat_trie_ptr self, trie_ptr origin)
{
	size_t index;
	trie_node_ptr pNode = origin->root;
	size_t iChild = pNode->trie_child;
	while (iChild != 0) {
		trie_node_ptr pChild = trie_access_node_export(origin, iChild);

		/* 设置 failed 域 */
		iChild = pChild->trie_brother;
		pChild->trie_failed = 0;
		dat_access_node(self, pChild->trie_datidx)->dat_failed = DAT_ROOT_IDX;
	}

	for (index = 1; index < trie_size(origin); index++) { // bfs
		pNode = trie_access_node_export(origin, index);
		iChild = pNode->trie_child;
		while (iChild != 0) {
			trie_node_ptr pChild = trie_access_node_export(origin, iChild);
			unsigned char key = pChild->key;

			size_t iFailed = pNode->trie_failed;
			size_t match = trie_next_state_by_binary(origin, iFailed, key);
			while (iFailed != 0 && match == 0) {
				iFailed = trie_access_node_export(origin,
												  iFailed)->trie_failed;
				match = trie_next_state_by_binary(origin, iFailed, key);
			}
			iChild = pChild->trie_brother;
			pChild->trie_failed = match;

			/* 设置 DAT 的 failed 域 */
			dat_access_node(self, pChild->trie_datidx)->dat_failed =
					match != 0 ?
					trie_access_node_export(origin, match)->trie_datidx :
					DAT_ROOT_IDX;
		}
	}
	fprintf(stderr, "construct AC automation succeed!\n");
}


// dat Context
// ===================================================

void dat_init_context(dat_context_ptr context, dat_trie_ptr trie,
					  unsigned char content[], size_t len)
{
	context->trie = trie;
	context->content = content;
	context->len = len;

	context->out_e = 0;
	context->_i = 0;
	context->_iCursor = DAT_ROOT_IDX;
	context->_pCursor = context->trie->root;
	context->out_matched = context->trie->root;

	context->out_matched_index = NULL;
}

bool dat_next(dat_context_ptr context)
{
	if (context->out_matched_index != NULL) {
		context->out_matched_index = context->out_matched_index->next;
		if (context->out_matched_index != NULL)
			return true;
	}

	for (; context->out_e < context->len; context->out_e++) {
		size_t iNext =
				context->_pCursor->base + context->content[context->out_e];
		dat_node_ptr pNext = dat_access_node(context->trie, iNext);
		if (pNext->check != context->_iCursor)
			break;
		context->_iCursor = iNext;
		context->_pCursor = pNext;
		if (pNext->dat_dictidx != NULL) {
			context->out_matched = pNext;
			context->out_matched_index = context->out_matched->dat_dictidx;
			context->out_e++;
			return true;
		}
	}

	for (context->_i++; context->_i < context->len; context->_i++) {
		context->_iCursor = DAT_ROOT_IDX;
		context->_pCursor = context->trie->root;
		for (context->out_e = context->_i; context->out_e < context->len;
			 context->out_e++) {
			size_t iNext =
					context->_pCursor->base + context->content[context->out_e];
			dat_node_ptr pNext = dat_access_node(context->trie, iNext);
			if (pNext->check != context->_iCursor)
				break;
			context->_iCursor = iNext;
			context->_pCursor = pNext;
			if (pNext->dat_dictidx != NULL) {
				context->out_matched = pNext;
				context->out_matched_index = context->out_matched->dat_dictidx;
				context->out_e++;
				return true;
			}
		}
	}
	return false;
}

bool dat_ac_next_on_node(dat_context_ptr context)
{
	/* 检查当前匹配点向树根的路径上是否还有匹配的词 */
	while (context->out_matched != context->trie->root) {
		context->out_matched =
				dat_access_node(context->trie,
								context->out_matched->dat_failed);
		if (context->out_matched->dat_dictidx != NULL) {
			context->out_matched_index = context->out_matched->dat_dictidx;
			return true;
		}
	}

	/* 执行匹配 */
	for (; context->out_e < context->len; context->out_e++) {
		size_t iNext =
				context->_pCursor->base + context->content[context->out_e];
		dat_node_ptr pNext = dat_access_node(context->trie, iNext);
		while (context->_pCursor != context->trie->root &&
			   pNext->check != context->_iCursor) {
			context->_iCursor = context->_pCursor->dat_failed;
			context->_pCursor = dat_access_node(context->trie,
												context->_iCursor);
			iNext = context->_pCursor->base + context->content[context->out_e];
			pNext = dat_access_node(context->trie, iNext);
		}
		if (pNext->check == context->_iCursor) {
			context->_iCursor = iNext;
			context->_pCursor = pNext;
			while (pNext != context->trie->root) {
				if (pNext->dat_dictidx != NULL) {
					context->out_matched = pNext;
					context->out_matched_index =
							context->out_matched->dat_dictidx;
					context->out_e++;
					return true;
				}
				pNext = dat_access_node(context->trie, pNext->dat_failed);
			}
		}
	}
	return false;
}

bool dat_ac_next_on_index(dat_context_ptr context)
{
	/* 检查 index 列表 */
	if (context->out_matched_index != NULL) {
		context->out_matched_index = context->out_matched_index->next;
		if (context->out_matched_index != NULL)
			return true;
	}

	/* 检查当前匹配点向树根的路径上是否还有匹配的词 */
	while (context->out_matched != context->trie->root) {
		context->out_matched =
				dat_access_node(context->trie,
								context->out_matched->dat_failed);
		if (context->out_matched->dat_dictidx != NULL) {
			context->out_matched_index = context->out_matched->dat_dictidx;
			return true;
		}
	}

	/* 执行匹配 */
	for (; context->out_e < context->len; context->out_e++) {
		size_t iNext =
				context->_pCursor->base + context->content[context->out_e];
		dat_node_ptr pNext = dat_access_node(context->trie, iNext);
		while (context->_pCursor != context->trie->root &&
			   pNext->check != context->_iCursor) {
			context->_iCursor = context->_pCursor->dat_failed;
			context->_pCursor = dat_access_node(context->trie,
												context->_iCursor);
			iNext = context->_pCursor->base + context->content[context->out_e];
			pNext = dat_access_node(context->trie, iNext);
		}
		if (pNext->check == context->_iCursor) {
			context->_iCursor = iNext;
			context->_pCursor = pNext;
			while (pNext != context->trie->root) {
				if (pNext->dat_dictidx != NULL) {
					context->out_matched = pNext;
					context->out_matched_index =
							context->out_matched->dat_dictidx;
					context->out_e++;
					return true;
				}
				pNext = dat_access_node(context->trie, pNext->dat_failed);
			}
		}
	}
	return false;
}


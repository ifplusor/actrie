//
// Created by james on 6/16/17.
//

#include <matcher.h>
#include <actrie.h>
#include <acdat.h>


matcher_t matcher_construct_by_file(matcher_type type, const char *path)
{
	matcher_t matcher = NULL;
	switch (type) {
		case matcher_type_dat:
		case matcher_type_acdat: {
			dat_trie_ptr p = dat_construct_by_file(path);
			if (p != NULL) {
				matcher = &p->header;
				matcher->_type = matcher_type_acdat;
			}
		}
			break;
	}
	return matcher;
}

matcher_t matcher_construct_by_string(matcher_type type, const char *string)
{
	matcher_t matcher = NULL;
	switch (type) {
		case matcher_type_dat:
		case matcher_type_acdat: {
			dat_trie_ptr p = dat_construct_by_string(string);
			if (p != NULL) {
				matcher = &p->header;
				matcher->_type = matcher_type_acdat;
			}
		}
			break;
	}
	return matcher;
}

bool matcher_destruct(matcher_t matcher)
{
	if (matcher == NULL) {
		return false;
	}
	switch (matcher->_type) {
		case matcher_type_dat:
		case matcher_type_acdat:
			dat_destruct(matcher->_self);
			break;
	}
	return true;
}

context_t matcher_alloc_context(matcher_t matcher)
{
	context_t context = NULL;
	if (matcher == NULL) {
		return NULL;
	}
	switch (matcher->_type) {
		case matcher_type_dat:
		case matcher_type_acdat: {
			dat_context_ptr ctx = dat_alloc_context(matcher->_self);
			if (ctx != NULL) {
				context = &ctx->header;
				context->_type = matcher->_type;
			}
		}
			break;
	}
	return context;
}

bool matcher_free_context(context_t context)
{
	if (context == NULL) {
		return false;
	}
	switch (context->_type) {
		case matcher_type_dat:
		case matcher_type_acdat:
			return dat_free_context(context->_self);;
	}
	return false;
}

bool matcher_reset_context(context_t context, char content[], size_t len)
{
	if (context == NULL) {
		return false;
	}
	switch (context->_type) {
		case matcher_type_dat:
		case matcher_type_acdat:
			return dat_reset_context(context->_self, (unsigned char *) content, len);
	}
	return false;
}

bool matcher_next(context_t context)
{
	if (context == NULL) {
		return false;
	}
	switch (context->_type) {
		case matcher_type_dat:
		case matcher_type_acdat:
			return dat_ac_next_on_index(context->_self);
	}
	return false;
}



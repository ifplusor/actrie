//
// Created by james on 6/16/17.
//

#include <matcher.h>
#include <actrie.h>
#include <acdat.h>


const matcher_func *const matcher_func_table[matcher_type_all] = {
	&dat_matcher_func,
	&dat_matcher_func,
};

const context_func *const context_func_table[matcher_type_all] = {
	&dat_context_func,
	&acdat_context_func,
};


matcher_t matcher_construct_by_file(matcher_type type, const char *path)
{
	matcher_t matcher = NULL;
	switch (type) {
		case matcher_type_dat:
			matcher = dat_construct_by_file(path, false);
			break;
		case matcher_type_acdat:
			matcher = dat_construct_by_file(path, true);
			break;
	}
	if (matcher != NULL) {
		matcher->_type = type;
		matcher->_func = *matcher_func_table[type];
	}
	return matcher;
}

matcher_t matcher_construct_by_string(matcher_type type, const char *string)
{
	matcher_t matcher = NULL;
	switch (type) {
		case matcher_type_dat:
			matcher = dat_construct_by_string(string, false);
			break;
		case matcher_type_acdat:
			matcher = dat_construct_by_string(string, true);
			break;
	}
	if (matcher != NULL) {
		matcher->_type = type;
		matcher->_func = *matcher_func_table[type];
	}
	return matcher;
}

bool matcher_destruct(matcher_t matcher)
{
	if (matcher == NULL) {
		return false;
	}
	return matcher->_func.destruct(matcher);
}

context_t matcher_alloc_context(matcher_t matcher)
{
	context_t context = NULL;
	if (matcher == NULL) {
		return NULL;
	}
	context = matcher->_func.alloc_context(matcher);
	if (context != NULL) {
		context->_type = matcher->_type;
		context->_func = *context_func_table[context->_type];
	}
	return context;
}

bool matcher_free_context(context_t context)
{
	if (context == NULL) {
		return false;
	}
	return context->_func.free_context(context);
}

bool matcher_reset_context(context_t context, char content[], size_t len)
{
	if (context == NULL) {
		return false;
	}
	return context->_func.reset_context(context, content, len);
}

bool matcher_next(context_t context)
{
	if (context == NULL) {
		return false;
	}
	return context->_func.next(context);
}


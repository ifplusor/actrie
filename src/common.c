#include <sys/timeb.h>

#include "common.h"


const size_t POOL_REGION_SIZE = REGION_SIZE;
const size_t POOL_POSITION_SIZE = POSITION_MASK + 1;

const char *ALLOCATED_FLAG = "";

long long system_millisecond()
{
	struct timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}

match_dict_ptr dict_alloc()
{
	match_dict_ptr p = (match_dict_ptr) malloc(sizeof(match_dict));
	if (p == NULL) return NULL;

	p->buff = NULL;
	p->_ref_count = 1;
	return p;
}

match_dict_ptr dict_assign(match_dict_ptr dict)
{
	if (dict != NULL) {
		dict->_ref_count++;
	}

	return dict;
}

void dict_release(match_dict_ptr dict)
{
	if (dict != NULL) {
		dict->_ref_count--;
		if (dict->_ref_count == 0) {
			if (dict->buff != NULL)
				free(dict->buff);
			free(dict);
		}
	}
}

bool dict_reset(match_dict_ptr dict, size_t index_count, size_t buffer_size)
{
	if (dict->buff != NULL)
		free(dict->buff);

	dict->count = index_count;
	dict->index = malloc(sizeof(match_dict_index) * dict->count);
	if (dict->index == NULL)
		return false;

	dict->size = buffer_size;
	dict->buff = malloc(sizeof(char) * dict->size);
	if (dict->buff == NULL) {
		free(dict->index);
		return false;
	}

	memset(dict->index, 0, sizeof(match_dict_index) * dict->count);
	memset(dict->buff, 0, sizeof(char) * dict->size);
	dict->_cursor = 1;
	return true;
}

bool dict_add_keyword_and_extra(match_dict_ptr dict, char keyword[],
								char extra[])
{
	if (keyword == NULL || keyword[0] == '\0') {
		dict->_out_key_cur = 0;
		dict->_out_extra_cur = 0;
	} else {
		strcpy(dict->buff + dict->_cursor, keyword);
		dict->_out_key_cur = dict->_cursor;
		dict->_cursor += strlen(keyword) + 1;

		if (extra == NULL || extra[0] == '\0') {
			dict->_out_extra_cur = 0;
		} else {
			strcpy(dict->buff + dict->_cursor, extra);
			dict->_out_extra_cur = dict->_cursor;
			dict->_cursor += strlen(extra) + 1;
		}
	}
	return true;
}

bool dict_parser_by_file(FILE *fp, match_dict_ptr dict,
						 dict_parser_callback callback, void *argv[])
{
	// 静态化以后，不支持多线程
	static char line_buf[MAX_LINE_SIZE];
	static char keyword[MAX_LINE_SIZE];
	static char extra[MAX_LINE_SIZE];

	if (fp == NULL || dict == NULL) {
		return false;
	}

	// 计算词典条目数
	size_t count = 0;
	fseek(fp, 0, SEEK_SET);
	while (fgets(line_buf, MAX_LINE_SIZE, fp) != NULL) {
		count++;
	}

	fseek(fp, 0, SEEK_END);
	if (!dict_reset(dict, count, (size_t)ftell(fp) + 5))
		return false;

	size_t i = 0;
	fseek(fp, 0, SEEK_SET);
	while (fgets(line_buf, MAX_LINE_SIZE, fp) != NULL) {
		int ret;
		if (!i
			&& (unsigned char) line_buf[0] == 0xef
			&& (unsigned char) line_buf[1] == 0xbb
			&& (unsigned char) line_buf[2] == 0xbf) {
			ret = sscanf(line_buf + 3, "%[^\t\r\n]\t%[^\r\n]", keyword, extra);
		} else {
			ret = sscanf(line_buf, "%[^\t\r\n]\t%[^\r\n]", keyword, extra);
		}
		//如果sscanf未读取key 则不用处理 直接continue
		if (ret <= 0) {
			keyword[0] = 0;
			continue;
		}

		//回车换行符此处特殊处理 \r \n \r\n
		if (keyword[0] == '\\') {
			// \r \r\n
			if (keyword[1] == 'r') {
				// \r\n
				if (keyword[2] == '\\' && keyword[3] == 'n' &&
					keyword[4] == '0') {
					keyword[0] = '\r';
					keyword[1] = '\n';
					keyword[2] = 0;
					// \r
				} else if (keyword[2] == 0) {
					keyword[0] = '\r';
					keyword[1] = 0;
				}
				// \n
			} else if (keyword[1] == 'n' && keyword[2] == 0) {
				keyword[0] = '\n';
				keyword[1] = 0;
			}
		}

		//如果不置0 会有上次scanf残留数据
		if (ret <= 1) {
			extra[0] = 0;
		}

		// 将 keyword 和 extra 加入 match_dict
		dict_add_keyword_and_extra(dict, keyword, extra);
		dict->index[i].keyword = dict->buff + dict->_out_key_cur;
		dict->index[i].extra = dict->buff + dict->_out_extra_cur;

		if (!callback(dict->index + i, argv))
			return false;

		i++;
	}

	return true;
}

const char *split = "\n";

bool dict_parser_by_s(const char *s, match_dict_ptr dict,
					  dict_parser_callback callback, void *argv[])
{
	// 静态化以后，不支持多线程
	char *line_buf;
	static char keyword[MAX_LINE_SIZE];
	static char extra[MAX_LINE_SIZE];

	if (s == NULL || dict == NULL) {
		return false;
	}

	// 计算词典条目数
	size_t count = 0;
	char *work_s = strdup(s);
	for(line_buf = strtok(work_s, split); line_buf; line_buf = strtok(NULL, split)) {
		count++;
	}

	if (!dict_reset(dict, count, strlen(s)))
		return false;

	size_t i = 0;
	for(line_buf = strtok(work_s, split); line_buf; line_buf = strtok(NULL, split)) {
		int ret;
		if (!i
			&& (unsigned char) line_buf[0] == 0xef
			&& (unsigned char) line_buf[1] == 0xbb
			&& (unsigned char) line_buf[2] == 0xbf) {
			ret = sscanf(line_buf + 3, "%[^\t\r\n]\t%[^\r\n]", keyword, extra);
		} else {
			ret = sscanf(line_buf, "%[^\t\r\n]\t%[^\r\n]", keyword, extra);
		}
		//如果sscanf未读取key 则不用处理 直接continue
		if (ret <= 0) {
			keyword[0] = 0;
			continue;
		}

		//回车换行符此处特殊处理 \r \n \r\n
		if (keyword[0] == '\\') {
			// \r \r\n
			if (keyword[1] == 'r') {
				// \r\n
				if (keyword[2] == '\\' && keyword[3] == 'n' &&
					keyword[4] == '0') {
					keyword[0] = '\r';
					keyword[1] = '\n';
					keyword[2] = 0;
					// \r
				} else if (keyword[2] == 0) {
					keyword[0] = '\r';
					keyword[1] = 0;
				}
				// \n
			} else if (keyword[1] == 'n' && keyword[2] == 0) {
				keyword[0] = '\n';
				keyword[1] = 0;
			}
		}

		//如果不置0 会有上次scanf残留数据
		if (ret <= 1) {
			extra[0] = 0;
		}

		// 将 keyword 和 extra 加入 match_dict
		dict_add_keyword_and_extra(dict, keyword, extra);
		dict->index[i].keyword = dict->buff + dict->_out_key_cur;
		dict->index[i].extra = dict->buff + dict->_out_extra_cur;

		if (!callback(dict->index + i, argv))
			return false;

		i++;
	}
	free(work_s);

	return true;
}

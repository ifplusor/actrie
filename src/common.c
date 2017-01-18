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

bool dict_reset(match_dict_ptr dict, size_t size)
{
	dict->size = size;
	dict->buff = malloc(sizeof(char) * dict->size);
	if (dict->buff == NULL)
		return false;
	memset(dict->buff, 0, sizeof(char) * dict->size);
	dict->_cursor = 1;
	return true;
}

bool dict_add_keyword_and_extra(match_dict_ptr dict, char keyword[],
								char extra[])
{
	if (keyword == NULL || keyword[0] == '\0') {
		dict->out_key_cur = 0;
		dict->out_extra_cur = 0;
	} else {
		strcpy(dict->buff + dict->_cursor, keyword);
		dict->out_key_cur = dict->_cursor;
		dict->_cursor += strlen(keyword) + 1;

		if (extra == NULL || extra[0] == '\0') {
			dict->out_extra_cur = 0;
		} else {
			strcpy(dict->buff + dict->_cursor, extra);
			dict->out_extra_cur = dict->_cursor;
			dict->_cursor += strlen(extra) + 1;
		}
	}
	return true;
}

void dict_release(match_dict_ptr dict)
{
	if (dict->buff != NULL)
		free(dict->buff);
	dict->buff = NULL;
}

bool dict_parser(FILE *fp, match_dict_ptr dict, dict_parser_callback callback,
				 void *argv[])
{
	// 静态化以后，不支持多线程
	static char line_buf[MAX_LINE_SIZE];
	static char keyword[MAX_LINE_SIZE];
	static char extra[MAX_LINE_SIZE];

	if (fp == NULL || dict == NULL) {
		return false;
	}

	fseek(fp, 0, SEEK_END);
	if (!dict_reset(dict, (size_t)ftell(fp) + 5)) return false;

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
		/*printf("%s : %s\n", keyword, extra);*/

		if (!callback(dict->buff + dict->out_key_cur,
					  dict->buff + dict->out_extra_cur, argv))
			return false;

		i++;
	}

	return true;
}

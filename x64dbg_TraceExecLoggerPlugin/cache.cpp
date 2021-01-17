#include "cache.h"

static std::unordered_map<duint, json> address_json_cache;
static std::list<duint> address_json_fifo;
static std::map<std::pair<duint, duint>, std::string> stack_comment_string_cache;
static std::list<std::pair<duint, duint>> stack_comment_string_fifo;
static std::unordered_map<duint, std::string> gui_asm_string_cache;
static std::list<duint> gui_asm_string_fifo;
static std::unordered_map<duint, std::string> comment_string_cache;
static std::list<duint> comment_string_fifo;


json get_address_json_cache_data(duint key, bool* result)
{
	return get_cache_data_internal<std::unordered_map<duint, json>, duint, json>(address_json_cache, key, result);
}
void set_address_json_cache_data(duint key, json data)
{
	set_cache_data_internal<std::unordered_map<duint, json>, duint, json>(address_json_cache, address_json_fifo, key, data, 10000);
}

json get_stack_comment_string_cache_data(std::pair<duint, duint> key, bool* result)
{
	return get_cache_data_internal<std::map<std::pair<duint, duint>, std::string>, std::pair<duint, duint>, std::string>(stack_comment_string_cache, key, result);
}
void set_stack_comment_string_cache_data(std::pair<duint, duint> key, std::string data)
{
	set_cache_data_internal<std::map<std::pair<duint, duint>, std::string>, std::pair<duint, duint>, std::string>(stack_comment_string_cache, stack_comment_string_fifo, key, data, 1000);
}

json get_gui_asm_string_cache_data(duint key, bool* result)
{
	return get_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(gui_asm_string_cache, key, result);
}
void set_gui_asm_string_cache_data(duint key, std::string data)
{
	set_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(gui_asm_string_cache, gui_asm_string_fifo, key, data, 100);
}

json get_comment_string_cache_data(duint key, bool* result)
{
	return get_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(comment_string_cache, key, result);
}
void set_comment_string_cache_data(duint key, std::string data)
{
	set_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(comment_string_cache, comment_string_fifo, key, data, 100);
}


void clear_cache()
{
	address_json_cache.clear();
	address_json_fifo.clear();
	stack_comment_string_cache.clear();
	stack_comment_string_fifo.clear();
	gui_asm_string_cache.clear();
	gui_asm_string_fifo.clear();
	comment_string_cache.clear();
	comment_string_fifo.clear();
}


void init_cache()
{
	clear_cache();
}


void stop_cache()
{
	clear_cache();
}


void setup_cache()
{
}

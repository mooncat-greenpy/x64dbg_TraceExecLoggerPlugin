#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


template <typename T1, typename T2, typename T3>
T3 get_cache_data_internal(T1& cache, T2 key, bool* result);
template <typename T1, typename T2, typename T3>
void set_cache_data_internal(T1& cache, std::list<T2>& fifo, T2 key, T3 data);

json get_address_json_cache_data(duint key, bool* result);
void set_address_json_cache_data(duint key, json data);
json get_stack_comment_string_cache_data(std::pair<duint, duint> key, bool* result);
void set_stack_comment_string_cache_data(std::pair<duint, duint> key, std::string data);
json get_gui_asm_string_cache_data(duint key, bool* result);
void set_gui_asm_string_cache_data(duint key, std::string data);
json get_comment_string_cache_data(duint key, bool* result);
void set_comment_string_cache_data(duint key, std::string data);

void clear_cache();

void init_cache();
void stop_cache();
void setup_cache();


template <typename T1, typename T2, typename T3>
T3 get_cache_data_internal(T1& cache, T2 key, bool* result)
{
	if (!get_cache_enabled() || cache.count(key) == 0) {
		*result = false;
		return T3();
	}
	*result = true;
	return cache[key];
}


template <typename T1, typename T2, typename T3>
void set_cache_data_internal(T1& cache, std::list<T2>& fifo, T2 key, T3 data, int size)
{
	if (!get_cache_enabled())
	{
		return;
	}
	cache[key] = data;
	fifo.push_back(key);
	if (cache.size() > size)
	{
		cache.erase(*fifo.begin());
		fifo.pop_front();
	}
}

#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json get_cache_data(std::unordered_map<duint, json>& cache, duint key, bool* result);
void set_cache_data(std::unordered_map<duint, json>& cache, std::list<duint>& fifo, duint key, json data);
std::string get_cache_string(std::map<std::pair<duint, duint>, std::string>& cache, duint key1, duint key2, bool* result);
void set_cache_string(std::map<std::pair<duint, duint>, std::string>& cache, std::list<std::pair<duint, duint>>& fifo, duint key1, duint key2, std::string data);
std::string get_cache_string(std::unordered_map<duint, std::string>& cache, duint key, bool* result);
void set_cache_string(std::unordered_map<duint, std::string>& cache, std::list<duint>& fifo, duint key, std::string data);

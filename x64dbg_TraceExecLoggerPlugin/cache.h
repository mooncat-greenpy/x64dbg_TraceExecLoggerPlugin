#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

json get_cache_data(std::unordered_map<duint, json>& cache, duint key);
void set_cache_data(std::unordered_map<duint, json>& cache, std::list<duint>& fifo, duint key, json data);

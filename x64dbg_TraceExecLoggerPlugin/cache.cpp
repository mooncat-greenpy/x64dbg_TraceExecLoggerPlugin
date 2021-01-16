#include "cache.h"


json get_cache_data(std::unordered_map<duint, json>& cache, duint key)
{
	if (cache.count(key) == 0) {
		return NULL;
	}
	return cache[key];
}


void set_cache_data(std::unordered_map<duint, json>& cache, std::list<duint>& fifo, duint key, json data)
{
	cache[key] = data;
	fifo.push_back(key);
	if (cache.size() > 10000)
	{
		cache.erase(*fifo.begin());
		fifo.pop_front();
	}
}

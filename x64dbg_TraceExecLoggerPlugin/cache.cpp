#include "cache.h"


json get_cache_data(std::unordered_map<duint, json>& cache, duint key, bool* result)
{
	if (cache.count(key) == 0) {
		*result = false;
		return json();
	}
	*result = true;
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


std::string get_cache_string(std::map<std::pair<duint, duint>, std::string>& cache, duint key1, duint key2, bool* result)
{
	std::pair<duint, duint> key = std::make_pair(key1, key2);
	if (cache.count(key) == 0) {
		*result = false;
		return std::string();
	}
	*result = true;
	return cache[key];
}


void set_cache_string(std::map<std::pair<duint, duint>, std::string>& cache, std::list<std::pair<duint, duint>>& fifo, duint key1, duint key2, std::string data)
{
	std::pair<duint, duint> key = std::make_pair(key1, key2);
	cache[key] = data;
	fifo.push_back(key);
	if (cache.size() > 1000)
	{
		cache.erase(*fifo.begin());
		fifo.pop_front();
	}
}

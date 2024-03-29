#include "cache.h"

static std::unordered_map<duint, LOG_ADDRESS> address_json_cache;
static std::list<duint> address_json_fifo;
static std::map<std::pair<duint, duint>, std::string> stack_comment_string_cache;
static std::list<std::pair<duint, duint>> stack_comment_string_fifo;
static std::unordered_map<duint, std::string> gui_asm_string_cache;
static std::list<duint> gui_asm_string_fifo;
static std::unordered_map<duint, std::string> comment_string_cache;
static std::list<duint> comment_string_fifo;

static std::list<duint> changed_list;


LOG_ADDRESS get_address_json_cache_data(duint key, bool* result)
{
	return get_cache_data_internal<std::unordered_map<duint, LOG_ADDRESS>, duint, LOG_ADDRESS>(address_json_cache, key, result);
}
void set_address_json_cache_data(duint key, LOG_ADDRESS data)
{
	set_cache_data_internal<std::unordered_map<duint, LOG_ADDRESS>, duint, LOG_ADDRESS>(address_json_cache, address_json_fifo, key, data, 10000);
}

std::string get_stack_comment_string_cache_data(std::pair<duint, duint> key, bool* result)
{
	return get_cache_data_internal<std::map<std::pair<duint, duint>, std::string>, std::pair<duint, duint>, std::string>(stack_comment_string_cache, key, result);
}
void set_stack_comment_string_cache_data(std::pair<duint, duint> key, std::string data)
{
	set_cache_data_internal<std::map<std::pair<duint, duint>, std::string>, std::pair<duint, duint>, std::string>(stack_comment_string_cache, stack_comment_string_fifo, key, data, 1000);
}

std::string get_gui_asm_string_cache_data(duint key, bool* result)
{
	return get_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(gui_asm_string_cache, key, result);
}
void set_gui_asm_string_cache_data(duint key, std::string data)
{
	set_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(gui_asm_string_cache, gui_asm_string_fifo, key, data, 100);
}

std::string get_comment_string_cache_data(duint key, bool* result)
{
	return get_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(comment_string_cache, key, result);
}
void set_comment_string_cache_data(duint key, std::string data)
{
	set_cache_data_internal<std::unordered_map<duint, std::string>, duint, std::string>(comment_string_cache, comment_string_fifo, key, data, 100);
}


void add_changed_memory(duint addr)
{
	changed_list.push_back(addr);
}


void flush_changed_memory()
{
	for (duint i : changed_list)
	{
		address_json_cache.erase(i);
		gui_asm_string_cache.erase(i);
		comment_string_cache.erase(i);
	}
	changed_list.clear();
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


bool cache_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (isCommand(argv[0], "TElogger.cache.help"))
	{
		telogger_logputs("Cache: Help\n"
			"Command:\n"
			"    TElogger.cache.help\n"
			"    TElogger.cache.enable\n"
			"    TElogger.cache.disable");
	}
	else if (isCommand(argv[0], "TElogger.cache.enable"))
	{
		set_cache_enabled(true);
		telogger_logputs("Cache: Enabled");
	}
	else if (isCommand(argv[0], "TElogger.cache.disable"))
	{
		set_cache_enabled(false);
		telogger_logputs("Cache: Disabled");
		clear_cache();
	}

	return true;
}


void init_cache()
{
	_plugin_registercommand(pluginHandle, "TElogger.cache.help", cache_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.cache.enable", cache_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.cache.disable", cache_command_callback, false);

	clear_cache();
}


void stop_cache()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.cache.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.cache.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.cache.disable");

	clear_cache();
}


void setup_cache()
{
}

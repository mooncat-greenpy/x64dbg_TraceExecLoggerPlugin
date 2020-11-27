#include "log_instruction.h"

static bool module_enabled = true;
static bool thread_enabled = true;
static bool memory_enabled = true;


json log_module()
{
	json module_json = json::object();
	if (!module_enabled)
	{
		return module_json;
	}

	BridgeList<Script::Module::ModuleInfo> module_list;
	Script::Module::GetList(&module_list);
	module_json["type"] = "module";
	module_json["list"] = json::array();
	for (int i = 0; i < module_list.Count(); i++)
	{
		json module_entry = json::object();
		Script::Module::ModuleInfo module_info = module_list[i];
		module_entry["name"] = module_info.name;
		module_entry["path"] = module_info.path;
		module_entry["entry_point"] = make_address_json(module_info.entry);
		module_entry["base_address"] = make_address_json(module_info.base);
		module_entry["size"] = module_info.size;
		module_entry["section_count"] = module_info.sectionCount;
		module_json["list"].push_back(module_entry);
	}
	return module_json;
}


json log_thread()
{
	json thread_json = json::object();
	if (!thread_enabled)
	{
		return thread_json;
	}

	THREADLIST thread_list = { 0 };
	DbgGetThreadList(&thread_list);
	thread_json["type"] = "thread";
	thread_json["current"] = thread_list.CurrentThread;
	thread_json["list"] = json::array();
	for (int i = 0; i < thread_list.count; i++)
	{
		json thread_entry = json::object();
		thread_entry["id"] = thread_list.list[i].BasicInfo.ThreadId;
		thread_entry["handle"] = (duint)thread_list.list[i].BasicInfo.Handle;
		thread_entry["name"] = thread_list.list[i].BasicInfo.threadName;
		thread_entry["start_address"] = make_address_json(thread_list.list[i].BasicInfo.ThreadStartAddress);
		thread_entry["local_base"] = make_address_json(thread_list.list[i].BasicInfo.ThreadLocalBase);
		thread_entry["cip"] = make_address_json(thread_list.list[i].ThreadCip);
		thread_entry["suspend_count"] = thread_list.list[i].SuspendCount;
		thread_entry["priority"] = thread_list.list[i].Priority;
		thread_entry["wait_reason"] = thread_list.list[i].WaitReason;
		thread_json["list"].push_back(thread_entry);
	}
	return thread_json;
}


json log_memory()
{
	json memory_json = json::object();
	if (!memory_enabled)
	{
		return memory_json;
	}

	MEMMAP memory_map = { 0 };
	DbgMemMap(&memory_map);
	memory_json["type"] = "memory";
	memory_json["list"] = json::array();
	for (int i = 0; i < memory_map.count; i++)
	{
		json memory_entry = json::object();
		memory_entry["info"] = memory_map.page[i].info;
		memory_entry["base_address"] = make_address_json((duint)memory_map.page[i].mbi.BaseAddress);
		memory_entry["allocation_base"] = make_address_json((duint)memory_map.page[i].mbi.AllocationBase);
		memory_entry["region_size"] = memory_map.page[i].mbi.RegionSize;
		memory_json["list"].push_back(memory_entry);
	}
	return memory_json;
}


bool module_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.proc.module.help\n"
			"    TElogger.proc.module.enable\n"
			"    TElogger.proc.module.disable\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		module_enabled = true;
		_plugin_logputs("Module Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		module_enabled = false;
		_plugin_logputs("Module Log: Disabled");
	}

	return true;
}


bool thread_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.proc.thread.help\n"
			"    TElogger.proc.thread.enable\n"
			"    TElogger.proc.thread.disable\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		thread_enabled = true;
		_plugin_logputs("Thread Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		thread_enabled = false;
		_plugin_logputs("Thread Log: Disabled");
	}

	return true;
}


bool memory_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.proc.memory.help\n"
			"    TElogger.proc.memory.enable\n"
			"    TElogger.proc.memory.disable\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		memory_enabled = true;
		_plugin_logputs("Memory Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		memory_enabled = false;
		_plugin_logputs("Memory Log: Disabled");
	}

	return true;
}


bool proc_info_log_plugin_init(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.proc.module.help", module_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.module.enable", module_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.module.disable", module_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.thread.help", thread_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.thread.enable", thread_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.thread.disable", thread_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.memory.help", memory_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.memory.enable", memory_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.memory.disable", memory_command_callback, false);
	return true;
}


bool proc_info_log_plugin_stop()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.module.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.module.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.module.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.thread.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.thread.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.thread.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.memory.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.memory.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.memory.disable");
	return true;
}


void proc_info_log_plugin_setup()
{
}

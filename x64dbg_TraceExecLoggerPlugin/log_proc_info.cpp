#include "log_instruction.h"


void log_module(LOG_MODULE& module_json)
{
	module_json.list.clear();
	if (!get_module_enabled())
	{
		module_json.enabled = false;
		return;
	}

	module_json.type = "module";
	BridgeList<Script::Module::ModuleInfo> module_list;
	if (!Script::Module::GetList(&module_list))
	{
		telogger_logputs("Module Log: Failed to get module list");
		module_json.enabled = false;
		return;
	}
	module_json.count = module_list.Count();
	for (int i = 0; i < module_list.Count(); i++)
	{
		LOG_MODULE_ENTRY module_entry = LOG_MODULE_ENTRY();
		Script::Module::ModuleInfo module_info = module_list[i];
		module_entry.name = module_info.name;
		module_entry.path = module_info.path;
		make_address_json(module_entry.entry_point, module_info.entry);
		make_address_json(module_entry.base_address, module_info.base);
		module_entry.size = module_info.size;
		module_entry.section_count = module_info.sectionCount;
		module_json.list.push_back(module_entry);
	}

	module_json.enabled = true;
}


void log_thread(LOG_THREAD& thread_json)
{
	thread_json.list.clear();
	if (!get_thread_enabled())
	{
		thread_json.enabled = false;
		return;
	}

	THREADLIST thread_list = {};
	DbgGetThreadList(&thread_list);
	thread_json.type = "thread";
	thread_json.current_thread = thread_list.CurrentThread;
	thread_json.count = thread_list.count;
	for (int i = 0; i < thread_list.count; i++)
	{
		if (thread_list.list == NULL)
		{
			continue;
		}
		LOG_THREAD_ENTRY thread_entry = LOG_THREAD_ENTRY();
		thread_entry.id = thread_list.list[i].BasicInfo.ThreadId;
		thread_entry.handle = thread_list.list[i].BasicInfo.Handle;
		thread_entry.name = thread_list.list[i].BasicInfo.threadName;
		make_address_json(thread_entry.start_address, thread_list.list[i].BasicInfo.ThreadStartAddress);
		make_address_json(thread_entry.local_base, thread_list.list[i].BasicInfo.ThreadLocalBase);
		make_address_json(thread_entry.cip, thread_list.list[i].ThreadCip);
		thread_entry.suspend_count = thread_list.list[i].SuspendCount;
		switch (thread_list.list[i].Priority)
		{
		case _PriorityIdle:
			thread_entry.priority = "Idle";
			break;
		case _PriorityAboveNormal:
			thread_entry.priority = "AboveNormal";
			break;
		case _PriorityBelowNormal:
			thread_entry.priority = "BelowNormal";
			break;
		case _PriorityHighest:
			thread_entry.priority = "Highest";
			break;
		case _PriorityLowest:
			thread_entry.priority = "Lowest";
			break;
		case _PriorityNormal:
			thread_entry.priority = "Normal";
			break;
		case _PriorityTimeCritical:
			thread_entry.priority = "TimeCritical";
			break;
		default:
			thread_entry.priority = "Unknown";
			break;
		}
		switch (thread_list.list[i].WaitReason)
		{
		case _Executive:
			thread_entry.wait_reason = "Executive";
			break;
		case _FreePage:
			thread_entry.wait_reason = "FreePage";
			break;
		case _PageIn:
			thread_entry.wait_reason = "PageIn";
			break;
		case _PoolAllocation:
			thread_entry.wait_reason = "PoolAllocation";
			break;
		case _DelayExecution:
			thread_entry.wait_reason = "DelayExecution";
			break;
		case _Suspended:
			thread_entry.wait_reason = "Suspended";
			break;
		case _UserRequest:
			thread_entry.wait_reason = "UserRequest";
			break;
		case _WrExecutive:
			thread_entry.wait_reason = "WrExecutive";
			break;
		case _WrFreePage:
			thread_entry.wait_reason = "WrFreePage";
			break;
		case _WrPageIn:
			thread_entry.wait_reason = "WrPageIn";
			break;
		case _WrPoolAllocation:
			thread_entry.wait_reason = "WrPoolAllocation";
			break;
		case _WrDelayExecution:
			thread_entry.wait_reason = "WrDelayExecution";
			break;
		case _WrSuspended:
			thread_entry.wait_reason = "WrSuspended";
			break;
		case _WrUserRequest:
			thread_entry.wait_reason = "WrUserRequest";
			break;
		case _WrEventPair:
			thread_entry.wait_reason = "WrEventPair";
			break;
		case _WrQueue:
			thread_entry.wait_reason = "WrQueue";
			break;
		case _WrLpcReceive:
			thread_entry.wait_reason = "WrLpcReceive";
			break;
		case _WrLpcReply:
			thread_entry.wait_reason = "WrLpcReply";
			break;
		case _WrVirtualMemory:
			thread_entry.wait_reason = "WrVirtualMemory";
			break;
		case _WrPageOut:
			thread_entry.wait_reason = "WrPageOut";
			break;
		case _WrRendezvous:
			thread_entry.wait_reason = "WrRendezvous";
			break;
		case _Spare2:
			thread_entry.wait_reason = "Spare2";
			break;
		case _Spare3:
			thread_entry.wait_reason = "Spare3";
			break;
		case _Spare4:
			thread_entry.wait_reason = "Spare4";
			break;
		case _Spare5:
			thread_entry.wait_reason = "Spare5";
			break;
		case _WrCalloutStack:
			thread_entry.wait_reason = "WrCalloutStack";
			break;
		case _WrKernel:
			thread_entry.wait_reason = "WrKernel";
			break;
		case _WrResource:
			thread_entry.wait_reason = "WrResource";
			break;
		case _WrPushLock:
			thread_entry.wait_reason = "WrPushLock";
			break;
		case _WrMutex:
			thread_entry.wait_reason = "WrMutex";
			break;
		case _WrQuantumEnd:
			thread_entry.wait_reason = "WrQuantumEnd";
			break;
		case _WrDispatchInt:
			thread_entry.wait_reason = "WrDispatchInt";
			break;
		case _WrPreempted:
			thread_entry.wait_reason = "WrPreempted";
			break;
		case _WrYieldExecution:
			thread_entry.wait_reason = "WrYieldExecution";
			break;
		case _WrFastMutex:
			thread_entry.wait_reason = "WrFastMutex";
			break;
		case _WrGuardedMutex:
			thread_entry.wait_reason = "WrGuardedMutex";
			break;
		case _WrRundown:
			thread_entry.wait_reason = "WrRundown";
			break;
		default:
			thread_entry.wait_reason = "Unknown";
			break;
		}
		thread_json.list.push_back(thread_entry);
	}

	thread_json.enabled = true;
}


void log_memory(LOG_MEMORY& memory_json)
{
	memory_json.list.clear();
	if (!get_memory_enabled())
	{
		memory_json.enabled = false;
		return;
	}

	MEMMAP memory_map = {};
	memory_json.type = "memory";
	if (!DbgMemMap(&memory_map))
	{
		telogger_logputs("Memory Log: Failed to get memory map");
		memory_json.enabled = false;
		return;
	}
	memory_json.count = memory_map.count;
	for (int i = 0; i < memory_map.count; i++)
	{
		if (memory_map.page == NULL)
		{
			continue;
		}
		LOG_MEMORY_ENTRY memory_entry = LOG_MEMORY_ENTRY();
		memory_entry.info = memory_map.page[i].info;
		make_address_json(memory_entry.base_address, (duint)memory_map.page[i].mbi.BaseAddress);
		make_address_json(memory_entry.allocation_base, (duint)memory_map.page[i].mbi.AllocationBase);
		memory_entry.region_size = memory_map.page[i].mbi.RegionSize;
		char protect[RIGHTS_STRING_SIZE] = { 0 };
		if (!DbgFunctions()->PageRightsToString(memory_map.page[i].mbi.Protect, protect))
		{
			strncpy_s(protect, sizeof(protect), "bad", _TRUNCATE);
		}
		memory_entry.protect = protect;
		char alloc_protect[RIGHTS_STRING_SIZE] = { 0 };
		if (!DbgFunctions()->PageRightsToString(memory_map.page[i].mbi.AllocationProtect, alloc_protect))
		{
			strncpy_s(alloc_protect, sizeof(alloc_protect), "bad", _TRUNCATE);
		}
		memory_entry.allocation_protect = alloc_protect;
		switch (memory_map.page[i].mbi.Type)
		{
		case MEM_IMAGE:
			memory_entry.type = "IMG";
			break;
		case MEM_MAPPED:
			memory_entry.type = "MAP";
			break;
		case MEM_PRIVATE:
			memory_entry.type = "PRV";
			break;
		default:
			memory_entry.type = "N/A";
			break;
		}
		memory_json.list.push_back(memory_entry);
	}

	memory_json.enabled = true;
}


void log_handle(LOG_HANDLE& handle_json)
{
	handle_json.list.clear();
	BridgeList<HANDLEINFO> handles;
	if (!get_handle_enabled() || !DbgFunctions()->EnumHandles(&handles))
	{
		handle_json.enabled = false;
		return;
	}

	int count = handles.Count();
	handle_json.type = "handle";
	handle_json.count = count;

	for (int i = 0; i < count; i++)
	{
		HANDLEINFO handle = handles[i];
		char handle_name[MAX_STRING_SIZE] = { 0 };
		char type_name[MAX_STRING_SIZE] = { 0 };
		if (!DbgFunctions()->GetHandleName(handle.Handle, handle_name, sizeof(handle_name), type_name, sizeof(type_name)))
		{
			strncpy_s(handle_name, sizeof(handle_name), "error", _TRUNCATE);
			strncpy_s(type_name, sizeof(type_name), "error", _TRUNCATE);
		}
		LOG_HANDLE_ENTRY handle_entry = LOG_HANDLE_ENTRY();
		handle_entry.value = handle.Handle;
		handle_entry.name = handle_name;
		handle_entry.type = type_name;
		handle_entry.granted_access = handle.GrantedAccess;
		handle_json.list.push_back(handle_entry);
	}

	handle_json.enabled = true;
}


void log_network(LOG_NETWORK& network_json)
{
	network_json.list.clear();
	BridgeList<TCPCONNECTIONINFO> connections;
	if (!get_network_enabled() || !DbgFunctions()->EnumTcpConnections(&connections))
	{
		network_json.enabled = false;
		return;
	}

	int count = connections.Count();
	network_json.type = "network";
	network_json.count = count;

	for (int i = 0; i < count; i++)
	{
		TCPCONNECTIONINFO connection = connections[i];
		LOG_NETWORK_ENTRY network_entry = LOG_NETWORK_ENTRY();
		network_entry.remote_address = connection.RemoteAddress;
		network_entry.remote_port = connection.RemotePort;
		network_entry.local_address = connection.LocalAddress;
		network_entry.local_port = connection.LocalPort;
		network_entry.state_text = connection.StateText;
		network_entry.state_value = connection.State;
		network_json.list.push_back(network_entry);
	}

	network_json.enabled = true;
}


void log_proc(PROC_LOG& proc_json)
{
	proc_json.type = "proc log";
	log_module(proc_json.module);
	log_thread(proc_json.thread);
	log_memory(proc_json.memory);
	log_handle(proc_json.handle);
	log_network(proc_json.network);
}


bool proc_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Proc Log: Help\n"
			"Command:\n"
			"    TElogger.proc.help\n"
			"    TElogger.proc.log\n"
			"    TElogger.proc.enable\n"
			"    TElogger.proc.disable\n"
			"    TElogger.proc.module.help\n"
			"    TElogger.proc.thread.help\n"
			"    TElogger.proc.memory.help\n"
			"    TElogger.proc.handle.help\n"
			"    TElogger.proc.network.help");
	}
	else if (strstr(argv[0], "log"))
	{
		log_proc_info("Command Log");
		telogger_logputs("Proc Log: Log");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_proc_enabled(true);
		telogger_logputs("Proc Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_proc_enabled(false);
		telogger_logputs("Proc Log: Disabled");
	}

	return true;
}


bool module_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Module Log: Help\n"
			"Command:\n"
			"    TElogger.proc.module.help\n"
			"    TElogger.proc.module.enable\n"
			"    TElogger.proc.module.disable");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_module_enabled(true);
		telogger_logputs("Module Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_module_enabled(false);
		telogger_logputs("Module Log: Disabled");
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
		telogger_logputs("Thread Log: Help\n"
			"Command:\n"
			"    TElogger.proc.thread.help\n"
			"    TElogger.proc.thread.enable\n"
			"    TElogger.proc.thread.disable");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_thread_enabled(true);
		telogger_logputs("Thread Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_thread_enabled(false);
		telogger_logputs("Thread Log: Disabled");
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
		telogger_logputs("Memory Log: Help\n"
			"Command:\n"
			"    TElogger.proc.memory.help\n"
			"    TElogger.proc.memory.enable\n"
			"    TElogger.proc.memory.disable");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_memory_enabled(true);
		telogger_logputs("Memory Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_memory_enabled(false);
		telogger_logputs("Memory Log: Disabled");
	}

	return true;
}


bool handle_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Handle Log: Help\n"
			"Command:\n"
			"    TElogger.proc.handle.help\n"
			"    TElogger.proc.handle.enable\n"
			"    TElogger.proc.handle.disable");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_handle_enabled(true);
		telogger_logputs("Handle Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_handle_enabled(false);
		telogger_logputs("Handle Log: Disabled");
	}

	return true;
}


bool network_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Network Log: Help\n"
			"Command:\n"
			"    TElogger.proc.network.help\n"
			"    TElogger.proc.network.enable\n"
			"    TElogger.proc.network.disable");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_network_enabled(true);
		telogger_logputs("Network Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_network_enabled(false);
		telogger_logputs("Network Log: Disabled");
	}

	return true;
}


bool init_proc_info_log(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.proc.help", proc_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.log", proc_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.enable", proc_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.disable", proc_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.module.help", module_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.module.enable", module_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.module.disable", module_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.thread.help", thread_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.thread.enable", thread_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.thread.disable", thread_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.memory.help", memory_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.memory.enable", memory_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.memory.disable", memory_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.handle.help", handle_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.handle.enable", handle_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.handle.disable", handle_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.network.help", network_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.network.enable", network_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.network.disable", network_command_callback, false);
	return true;
}


bool stop_proc_info_log()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.log");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.module.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.module.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.module.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.thread.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.thread.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.thread.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.memory.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.memory.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.memory.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.handle.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.handle.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.handle.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.network.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.network.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.network.disable");
	return true;
}


void setup_proc_info_log()
{
}

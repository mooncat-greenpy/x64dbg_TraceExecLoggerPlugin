#include "trace_exec_logger.h"


static duint skip_breakpoints_log_addr = 0;


void log_proc_info(const char* msg)
{
	if (msg == NULL || !get_proc_enabled())
	{
		return;
	}

	LOG_CONTAINER entry = {};
	entry.is_proc_log = true;
	entry.proc = PROC_LOG();

	log_proc(entry.proc);
	entry.proc.message = msg;

	add_log(DbgGetThreadId(), &entry);
}


void log_exec(const char* msg, duint cip)
{
	if (msg == NULL || !get_telogger_enabled() || !should_log(cip))
	{
		return;
	}

	LOG_CONTAINER entry = { false, LOG() };

	entry.exec.type = "log";

	StepInfo step_info;

	log_instruction(entry.exec.inst, step_info);
	log_register(entry.exec.reg, step_info);
	log_stack(entry.exec.stack, step_info);
	entry.exec.message = msg;
	add_log(DbgGetThreadId(), &entry);

	flush_changed_memory();
}


bool command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}

	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Log: Help\n"
			"Command:\n"
			"    TElogger.help\n"
			"    TElogger.enable\n"
			"    TElogger.disable\n"
			"    TElogger.threadstop.enable\n"
			"    TElogger.threadstop.disable\n"
			"    TElogger.dllstop.enable\n"
			"    TElogger.dllstop.disable\n"
			"    TElogger.compactlog.enable\n"
			"    TElogger.compactlog.disable\n"
			"    TElogger.inst.help\n"
			"    TElogger.reg.help\n"
			"    TElogger.stack.help\n"
			"    TElogger.proc.help\n"
			"    TElogger.save.help\n"
			"    TElogger.cache.help\n"
			"    TElogger.filt.help\n"
			"    TElogger.auto.help");
	}
	else if (strstr(argv[0], "threadstop.enable"))
	{
		set_thread_stop_enabled(true);
		telogger_logputs("Thread Stop: Enabled");
	}
	else if (strstr(argv[0], "threadstop.disable"))
	{
		set_thread_stop_enabled(false);
		telogger_logputs("Thread Stop: Disabled");
	}
	else if (strstr(argv[0], "dllstop.enable"))
	{
		set_dll_stop_enabled(true);
		telogger_logputs("Dll Stop: Enabled");
	}
	else if (strstr(argv[0], "dllstop.disable"))
	{
		set_dll_stop_enabled(false);
		telogger_logputs("Dll Stop: Disabled");
	}
	else if (strstr(argv[0], "compactlog.enable"))
	{
		set_compact_log_enabled(true);
		telogger_logputs("Compact Log: Enabled");
	}
	else if (strstr(argv[0], "compactlog.disable"))
	{
		set_compact_log_enabled(false);
		telogger_logputs("Compact Log: Disabled");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_telogger_enabled(true);
		telogger_logputs("Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_telogger_enabled(false);
		telogger_logputs("Log: Disabled");
	}

	return true;
}


extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY* info)
{
	menu_callback(info);
}


extern "C" __declspec(dllexport) void CBTRACEEXECUTE(CBTYPE, PLUG_CB_TRACEEXECUTE* info)
{
	log_exec("Trace Execute Log", info->cip);
	skip_breakpoints_log_addr = info->cip;
}


extern "C" __declspec(dllexport) void CBSTEPPED(CBTYPE, PLUG_CB_STEPPED* info)
{
	bool result = false;
	duint cip = DbgEval("cip", &result);
	log_exec("Stepped Log", cip);
	if (result)
	{
		skip_breakpoints_log_addr = cip;
	}
}


extern "C" __declspec(dllexport) void CBBREAKPOINT(CBTYPE, PLUG_CB_BREAKPOINT* info)
{
	if (skip_breakpoints_log_addr == 0 || skip_breakpoints_log_addr != info->breakpoint->addr)
	{
		log_exec("Breakpoint Log", info->breakpoint->addr);
	}
	run_debug();
	skip_breakpoints_log_addr = 0;
}


extern "C" __declspec(dllexport) void CBPAUSEDEBUG(CBTYPE, PLUG_CB_PAUSEDEBUG* info)
{
	run_debug();
}


extern "C" __declspec(dllexport) void CBSYSTEMBREAKPOINT(CBTYPE, PLUG_CB_SYSTEMBREAKPOINT* info)
{
	bool result = false;
	duint cip = DbgEval("cip", &result);
	if (result) {
		skip_system_break_point(cip);
	}
}


extern "C" __declspec(dllexport) void CBINITDEBUG(CBTYPE, PLUG_CB_INITDEBUG* info)
{
	set_auto_run_enabled(false);
	set_file_name(info->szFileName);
	set_log_counter(0);
	clear_cache();
}


extern "C" __declspec(dllexport) void CBSTOPDEBUG(CBTYPE, PLUG_CB_STOPDEBUG* info)
{
	save_all_thread_log();
	delete_all_log();
}


extern "C" __declspec(dllexport) void CBCREATEPROCESS(CBTYPE, PLUG_CB_CREATEPROCESS* info)
{
	telogger_logprintf("CREATEPROCESS ID = %#x\n", info->fdProcessInfo->dwProcessId);
}


extern "C" __declspec(dllexport) void CBCREATETHREAD(CBTYPE, PLUG_CB_CREATETHREAD* info)
{
	telogger_logprintf("CREATETHREAD ID = %#x\n", info->dwThreadId);
	log_proc_info("Create Thread Log");
	if (get_thread_stop_enabled())
	{
		char cmd[DEFAULT_BUF_SIZE] = { 0 };
		_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerStartThread, ss", info->CreateThread->lpStartAddress);
		DbgCmdExecDirect(cmd);
	}
}


extern "C" __declspec(dllexport) void CBLOADDLL(CBTYPE, PLUG_CB_LOADDLL* info)
{
	telogger_logprintf("LOADDLL %s\n", info->modname);
	log_proc_info("Load Dll Log");

	if (!get_dll_stop_enabled())
	{
		return;
	}
	BridgeList<Script::Module::ModuleInfo> module_list;
	if (!Script::Module::GetList(&module_list))
	{
		telogger_logputs("Load DLL: Failed to get module list");
		return;
	}
	for (int i = 0; i < module_list.Count(); i++)
	{
		Script::Module::ModuleInfo module_info = module_list[i];
		if (module_info.base == (duint)info->LoadDll->lpBaseOfDll && module_info.entry != 0)
		{
			char cmd[DEFAULT_BUF_SIZE] = { 0 };
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerLoadDLL_%s, ss", (void*)module_info.entry, info->modname);
			DbgCmdExecDirect(cmd);
			break;
		}
	}
}


bool init_logger_plugin(PLUG_INITSTRUCT* init_struct)
{
	init_menu();
	init_cache();

	init_instruction_log(init_struct);
	init_register_log(init_struct);
	init_stack_log(init_struct);
	init_proc_info_log(init_struct);
	init_filter_log(init_struct);
	init_save(init_struct);
	init_auto_run(init_struct);

	_plugin_registercommand(pluginHandle, "TElogger.help", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.disable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.threadstop.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.threadstop.disable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.dllstop.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.dllstop.disable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.compactlog.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.compactlog.disable", command_callback, false);

	return true;
}


bool stop_logger_plugin()
{
	stop_menu();
	stop_cache();

	stop_instruction_log();
	stop_register_log();
	stop_stack_log();
	stop_proc_info_log();
	stop_filter_log();
	stop_save();
	stop_auto_run();

	_plugin_unregistercommand(pluginHandle, "TElogger.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.threadstop.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.threadstop.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.dllstop.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.dllstop.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.compactlog.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.compactlog.disable");

	return true;
}


void setup_logger_plugin()
{
	setup_menu();
	setup_cache();

	setup_instruction_log();
	setup_register_log();
	setup_stack_log();
	setup_proc_info_log();
	setup_filter_log();
	setup_save();
	setup_auto_run();
}

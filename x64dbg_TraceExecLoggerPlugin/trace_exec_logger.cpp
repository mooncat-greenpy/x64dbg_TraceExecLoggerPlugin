#include "trace_exec_logger.h"


char file_path[MAX_PATH] = { 0 };
static SYSTEMTIME system_time = { 0 };
json log_json = json::array();
static int log_count = 0;


void log_proc_info()
{
	if (!get_proc_enabled())
	{
		return;
	}

	json entry = json::object();
	entry["module"] = log_module();
	entry["thread"] = log_thread();
	entry["memory"] = log_memory();
	log_json.push_back(entry);
}


void log_exec()
{
	if (!get_telogger_enabled())
	{
		return;
	}

	json entry = json::object();
	entry["inst"] = log_instruction();
	entry["reg"] = log_register();
	entry["stack"] = log_stack();
	log_json.push_back(entry);
	log_count++;
	if (log_count % MAX_LOG_COUNT == 0)
	{
		save_json_file(file_path, &system_time, log_count / MAX_LOG_COUNT, log_json.dump().c_str());
		log_json.clear();
	}
}


bool command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "proc.help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.proc.help\n"
			"    TElogger.proc.enable\n"
			"    TElogger.proc.disable\n");
	}
	else if (strstr(argv[0], "proc.log"))
	{
		log_proc_info();
	}
	else if (strstr(argv[0], "proc.enable"))
	{
		set_proc_enabled(true);
		_plugin_logputs(PLUGIN_NAME ": Proc Log Enabled");
	}
	else if (strstr(argv[0], "proc.disable"))
	{
		set_proc_enabled(false);
		_plugin_logputs(PLUGIN_NAME ": Proc Log Disabled");
	}
	else if (strstr(argv[0], "help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.help\n"
			"    TElogger.enable\n"
			"    TElogger.disable\n"
			"    TElogger.proc.help\n"
			"    TElogger.inst.help\n"
			"    TElogger.reg.help\n"
			"    TElogger.stack.help\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_telogger_enabled(true);
		_plugin_logputs(PLUGIN_NAME ": Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_telogger_enabled(false);
		_plugin_logputs(PLUGIN_NAME ": Disabled");
	}

	return true;
}


extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY* info)
{
	menu_callback(info);
}


extern "C" __declspec(dllexport) void CBTRACEEXECUTE(CBTYPE, PLUG_CB_TRACEEXECUTE* info)
{
	log_exec();
}


extern "C" __declspec(dllexport) void CBSTEPPED(CBTYPE, PLUG_CB_STEPPED* info)
{
	log_exec();
}


/*** EIP address is sometimes wrong. ***
extern "C" __declspec(dllexport) void CBDEBUGEVENT(CBTYPE, PLUG_CB_DEBUGEVENT * info)
{
	log_exec();
}
*/


extern "C" __declspec(dllexport) void CBINITDEBUG(CBTYPE, PLUG_CB_INITDEBUG* info)
{
	log_json.clear();
	strncpy_s(file_path, sizeof(file_path), info->szFileName, _TRUNCATE);
	GetLocalTime(&system_time);
}


extern "C" __declspec(dllexport) void CBSTOPDEBUG(CBTYPE, PLUG_CB_STOPDEBUG* info)
{
	save_json_file(file_path, &system_time, log_count / MAX_LOG_COUNT + 1, log_json.dump().c_str());
	memset(file_path, 0, sizeof(file_path));
	log_json.clear();
}


extern "C" __declspec(dllexport) void CBCREATEPROCESS(CBTYPE, PLUG_CB_CREATEPROCESS * info)
{
	_plugin_logprintf("CREATEPROCESS ID = %d\n", info->fdProcessInfo->dwProcessId);
	log_proc_info();
}


extern "C" __declspec(dllexport) void CBCREATETHREAD(CBTYPE, PLUG_CB_CREATETHREAD * info)
{
	_plugin_logprintf("CREATETHREAD ID = %d\n", info->dwThreadId);
	log_proc_info();
	char cmd[MAX_PATH] = { 0 };
	_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerStartThread, ss", info->CreateThread->lpStartAddress);
	DbgCmdExecDirect(cmd);
}


extern "C" __declspec(dllexport) void CBLOADDLL(CBTYPE, PLUG_CB_LOADDLL * info)
{
	_plugin_logprintf("LOADDLL %s\n", info->modname);
	log_proc_info();
}


bool init_logger_plugin(PLUG_INITSTRUCT* init_struct)
{
	init_menu();

	init_instruction_log(init_struct);
	init_register_log(init_struct);
	init_stack_log(init_struct);
	init_proc_info_log(init_struct);

	_plugin_registercommand(pluginHandle, "TElogger.help", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.disable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.help", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.log", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.proc.disable", command_callback, false);

	return true;
}


bool stop_logger_plugin()
{
	stop_instruction_log();
	stop_register_log();
	stop_stack_log();
	stop_proc_info_log();

	_plugin_unregistercommand(pluginHandle, "TElogger.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.log");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.proc.disable");

	return true;
}


void setup_logger_plugin()
{
	setup_menu();
	setup_instruction_log();
	setup_register_log();
	setup_stack_log();
	setup_proc_info_log();
}

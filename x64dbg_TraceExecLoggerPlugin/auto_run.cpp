#include "auto_run.h"

static int current_thread_number = 0;
static int skip_addr = 0;
std::vector<duint> auto_run_breakpoints;
static bool stepover_enabled = false;


void add_breakpoint(duint addr)
{
	auto_run_breakpoints.push_back(addr);
	char cmd[DEFAULT_BUF_SIZE] = { 0 };
	_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoRunBP, ss", (char*)addr);
	DbgCmdExecDirect(cmd);
}


void remove_breakpoint(duint addr)
{
	for (std::vector<duint>::iterator itr = auto_run_breakpoints.begin(); itr != auto_run_breakpoints.end();)
	{
		if (*itr != addr)
		{
			itr++;
			continue;
		}
		itr == auto_run_breakpoints.erase(itr);
	}
}


void run_debug()
{
	if (!get_auto_run_enabled())
	{
		return;
	}
	duint cip = 0;
	bool result_eval = false;
	cip = DbgEval("cip", &result_eval);
	if (result_eval && cip == skip_addr)
	{
		skip_addr = 0;
		DbgCmdExec("run");
		return;
	}

	THREADLIST thread_list = { 0 };
	DbgGetThreadList(&thread_list);
	int thread_id = 0;
	for (int i = 0; i < thread_list.count; i++)
	{
		if (thread_id == 0 && thread_list.list[i].WaitReason != _Suspended)
		{
			thread_id = thread_list.list[i].BasicInfo.ThreadId;
		}

		if (i > current_thread_number && thread_list.list[i].WaitReason != _Suspended)
		{
			thread_id = thread_list.list[i].BasicInfo.ThreadId;
			current_thread_number = i;
			break;
		}
	}

	if (std::find(auto_run_breakpoints.begin(), auto_run_breakpoints.end(), cip) != auto_run_breakpoints.end())
	{
		remove_breakpoint(cip);
		set_auto_run_enabled(false);
		stepover_enabled = false;
		return;
	}

	char cmd[DEFAULT_BUF_SIZE] = { 0 };
	_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "threadswitch %x", thread_id);
	DbgCmdExecDirect(cmd);
	if (stepover_enabled) {
		DbgCmdExec("TraceOverConditional 0, 50");
	}
	else {
		DbgCmdExec("TraceIntoConditional 0, 50");
	}
	telogger_logprintf("Auto Run: ThreadID = %x, Address = %p\n", thread_id, cip);
}


void skip_system_break_point(duint addr)
{
	skip_addr = addr;
}


bool auto_run_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Auto Run Help\n"
			"Command:\n"
			"    TElogger.auto.help\n"
			"    TElogger.auto.enable\n"
			"    TElogger.auto.disable\n"
			"    TElogger.auto.addbp address\n"
			"    TElogger.auto.start address\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_auto_run_enabled(true);
		telogger_logputs("Auto Run Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_auto_run_enabled(false);
		telogger_logputs("Auto Run Log: Disabled");
	}
	else if (strstr(argv[0], "addbp") || strstr(argv[0], "start"))
	{
		if (argc < 2)
		{
			telogger_logprintf("Failed: %s address", argv[0]);
			return false;
		}
		char* end = NULL;
		duint value = (duint)_strtoi64(argv[1], &end, 16);
		if (end == NULL || *end != '\0')
		{
			telogger_logprintf("Failed: %s address", argv[0]);
			return false;
		}

		add_breakpoint(value);
		telogger_logprintf("Add Auto Run BP: %p\n", (char*)value);

		if (strstr(argv[0], "starti"))
		{
			set_auto_run_enabled(true);
			DbgCmdExec("StepInto");
			telogger_logputs("Start Auto Run");
		}
		else if (strstr(argv[0], "starto"))
		{
			set_auto_run_enabled(true);
			stepover_enabled = true;
			DbgCmdExec("StepOver");
			telogger_logputs("Start Auto Run");
		}

	}
	else if (strstr(argv[0], "call"))
	{
		duint cip = 0;
		bool result_eval = false;
		cip = DbgEval("cip", &result_eval);
		if (!result_eval)
		{
			telogger_logputs("Failed: Get cip");
		}
		BASIC_INSTRUCTION_INFO basic_info = { 0 };
		DbgDisasmFastAt(cip, &basic_info);
		if (!basic_info.call)
		{
			telogger_logputs("Failed: Not call");
			return false;
		}

		duint next_cip = cip + basic_info.size;
		add_breakpoint(next_cip);
		telogger_logprintf("Add Auto Run BP: %p\n", (char*)next_cip);

		set_auto_run_enabled(true);
		DbgCmdExec("StepInto");
		telogger_logputs("Start Auto Run");
	}
	return true;
}


bool init_auto_run(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.auto.help", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.enable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.disable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.addbp", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.starti", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.starto", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.call", auto_run_command_callback, false);
	return true;
}


bool stop_auto_run()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.addbp");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starti");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starto");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.call");
	return true;
}


void setup_auto_run()
{
}

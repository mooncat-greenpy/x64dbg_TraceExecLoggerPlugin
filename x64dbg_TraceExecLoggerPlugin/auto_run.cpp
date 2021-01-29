#include "auto_run.h"

static duint skip_addr = 0;
std::vector<duint> auto_run_breakpoints;
static bool stepover_enabled = false;


void add_breakpoint(duint addr)
{
	auto_run_breakpoints.push_back(addr);
	char cmd[DEFAULT_BUF_SIZE] = { 0 };
	_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoRunBP_%p, ss", (char*)addr, (char*)addr);
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
		itr = auto_run_breakpoints.erase(itr);
	}
}


void remove_all_breakpoint()
{
	BPMAP bp_map = { 0 };
	DbgGetBpList(bp_normal, &bp_map);
	if (bp_map.bp == NULL)
	{
		return;
	}
	for (int i = 0; i < bp_map.count; i++)
	{
		if (strstr(bp_map.bp[i].name, "TEloggerAutoRunBP") == NULL)
		{
			continue;
		}
		Script::Debug::DeleteBreakpoint(bp_map.bp[i].addr);
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
	if (!result_eval)
	{
		telogger_logputs("Auto Run: Failed to get cip");
		return;
	}
	if (cip == skip_addr)
	{
		skip_addr = 0;
		DbgCmdExec("run");
		return;
	}

	if (std::find(auto_run_breakpoints.begin(), auto_run_breakpoints.end(), cip) != auto_run_breakpoints.end())
	{
		remove_breakpoint(cip);
		set_auto_run_enabled(false);
		stepover_enabled = false;
		return;
	}

	if (stepover_enabled) {
		DbgCmdExec("TraceOverConditional 0, 50");
	}
	else {
		DbgCmdExec("TraceIntoConditional 0, 50");
	}
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
		telogger_logputs("Auto Run Log: Help\n"
			"Command:\n"
			"    TElogger.auto.help\n"
			"    TElogger.auto.enable\n"
			"    TElogger.auto.disable\n"
			"    TElogger.auto.addbp address\n"
			"    TElogger.auto.starti address\n"
			"    TElogger.auto.starto address");
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
			telogger_logprintf("Auto Run Log: Failed\n"
				"Command:\n"
				"    %s address", argv[0]);
			return false;
		}
		char* end = NULL;
		duint value = (duint)_strtoi64(argv[1], &end, 16);
		if (end == NULL || *end != '\0')
		{
			telogger_logprintf("Auto Run Log: Failed\n"
				"Command:\n"
				"    %s address", argv[0]);
			return false;
		}

		add_breakpoint(value);
		telogger_logprintf("Auto Run Log: BP %p\n", (char*)value);

		if (strstr(argv[0], "starti"))
		{
			set_auto_run_enabled(true);
			stepover_enabled = false;
			DbgCmdExec("StepInto");
			telogger_logputs("Auto Run Log: Start StepInto");
		}
		else if (strstr(argv[0], "starto"))
		{
			set_auto_run_enabled(true);
			stepover_enabled = true;
			DbgCmdExec("StepOver");
			telogger_logputs("Auto Run Log: Start StepOver");
		}
	}
	else if (strstr(argv[0], "rmbp"))
	{
		remove_all_breakpoint();
		telogger_logputs("Auto Run Log: Remove all breakpoints");
	}
	else if (strstr(argv[0], "call"))
	{
		duint cip = 0;
		bool result_eval = false;
		cip = DbgEval("cip", &result_eval);
		if (!result_eval)
		{
			telogger_logputs("Auto Run Log: Failed to get cip");
			return false;
		}
		BASIC_INSTRUCTION_INFO basic_info = { 0 };
		DbgDisasmFastAt(cip, &basic_info);
		if (!basic_info.call)
		{
			telogger_logputs("Auto Run Log: Not call");
			return false;
		}

		duint next_cip = cip + basic_info.size;
		add_breakpoint(next_cip);
		telogger_logprintf("Auto Run Log: BP %p\n", (char*)next_cip);

		set_auto_run_enabled(true);
		stepover_enabled = false;
		DbgCmdExec("StepInto");
		telogger_logputs("Auto Run Log: Start call");
	}
	return true;
}


bool init_auto_run(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.auto.help", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.enable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.disable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.addbp", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.rmbp", auto_run_command_callback, false);
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
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.rmbp");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starti");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starto");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.call");
	return true;
}


void setup_auto_run()
{
}

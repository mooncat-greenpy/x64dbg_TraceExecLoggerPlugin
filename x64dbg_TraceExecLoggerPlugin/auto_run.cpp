#include "auto_run.h"

static duint skip_addr = 0;
std::vector<duint> auto_run_breakpoints;
static AUTO_RUN_TYPE auto_run_state = AUTO_RUN_TYPE::AUTO_STOP;


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


// This function takes a long time to complete
void add_import_log_breakpoint(const char* mod_name = NULL, const char* api_name = NULL)
{
	BridgeList<Script::Module::ModuleInfo> module_list;
	if (!Script::Module::GetList(&module_list))
	{
		telogger_logputs("Auto Run Log: Failed to get module list");
		return;
	}
	for (int i = 0; i < module_list.Count(); i++)
	{
		if (mod_name != NULL && stricmp(module_list[i].name, mod_name) != 0)
		{
			continue;
		}

		BridgeList<Script::Module::ModuleImport> import_list;
		if (!Script::Module::GetImports(&module_list[i], &import_list))
		{
			telogger_logputs("Auto Run Log: Failed to get module import list");
			return;
		}
		for (int j = 0; j < import_list.Count(); j++)
		{
			if (api_name != NULL && stricmp(import_list[j].name, api_name) != 0)
			{
				continue;
			}
			char cmd[DEFAULT_BUF_SIZE] = { 0 };
			if (!DbgMemIsValidReadPtr(import_list[j].iatVa))
			{
				telogger_logprintf("Auto Run Log: Invalid ptr %p\n", import_list[j].iatVa);
				continue;
			}
			char data[sizeof(char*)] = { 0 };
			if (!DbgMemRead(import_list[j].iatVa, data, sizeof(data)))
			{
				telogger_logprintf("Auto Run Log: Failed to read %p\n", import_list[j].iatVa);
				continue;
			}
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoRunLogBP_Import_%s_%s", *(char**)data, module_list[i].name, import_list[j].name);
			DbgCmdExecDirect(cmd);
			if (api_name != NULL)
			{
				break;
			}
		}

		if (mod_name != NULL)
		{
			break;
		}
	}
}


// This function takes a long time to complete
void add_export_log_breakpoint(const char* mod_name = NULL, const char* api_name = NULL)
{
	BridgeList<Script::Module::ModuleInfo> module_list;
	if (!Script::Module::GetList(&module_list))
	{
		telogger_logputs("Auto Run Log: Failed to get module list");
		return;
	}
	for (int i = 0; i < module_list.Count(); i++)
	{
		if (mod_name != NULL && stricmp(module_list[i].name, mod_name) != 0)
		{
			continue;
		}

		BridgeList<Script::Module::ModuleExport> export_list;
		if (!Script::Module::GetExports(&module_list[i], &export_list))
		{
			telogger_logputs("Auto Run Log: Failed to get module export list");
			return;
		}
		for (int j = 0; j < export_list.Count(); j++)
		{
			if (api_name != NULL && stricmp(export_list[j].name, api_name) != 0)
			{
				continue;
			}
			char cmd[DEFAULT_BUF_SIZE] = { 0 };
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoRunLogBP_Export_%s_%s", (char*)export_list[j].va, module_list[i].name, export_list[j].name);
			DbgCmdExecDirect(cmd);
			if (api_name != NULL)
			{
				break;
			}
		}

		if (mod_name != NULL)
		{
			break;
		}
	}
}


void run_debug()
{
	if (!get_auto_run_enabled())
	{
		auto_run_state = AUTO_RUN_TYPE::AUTO_STOP;
		return;
	}
	duint cip = 0;
	bool result_eval = false;
	cip = DbgEval("cip", &result_eval);
	if (!result_eval)
	{
		auto_run_state = AUTO_RUN_TYPE::AUTO_STOP;
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
		auto_run_state = AUTO_RUN_TYPE::AUTO_STOP;
		telogger_logprintf("Auto Run Log: Break at %p\n", cip);
		return;
	}

	if (auto_run_state == AUTO_RUN_TYPE::AUTO_STEP_OVER)
	{
		DbgCmdExec("TraceOverConditional 0, 50");
	}
	else if (auto_run_state == AUTO_RUN_TYPE::AUTO_STEP_INTO)
	{
		DbgCmdExec("TraceIntoConditional 0, 50");
	}
	else if (auto_run_state == AUTO_RUN_TYPE::AUTO_RUN)
	{
		DbgCmdExec("run");
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
			"    TElogger.auto.rmbp address\n"
			"    TElogger.auto.addlogbp.dll.import [dllname]\n"
			"    TElogger.auto.addlogbp.dll.export [dllname]\n"
			"    TElogger.auto.starti address\n"
			"    TElogger.auto.starto address\n"
			"    TElogger.auto.startr address\n"
			"    TElogger.auto.call");
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
			auto_run_state = AUTO_RUN_TYPE::AUTO_STEP_INTO;
			DbgCmdExec("StepInto");
			telogger_logputs("Auto Run Log: Start StepInto");
		}
		else if (strstr(argv[0], "starto"))
		{
			set_auto_run_enabled(true);
			auto_run_state = AUTO_RUN_TYPE::AUTO_STEP_OVER;
			DbgCmdExec("StepOver");
			telogger_logputs("Auto Run Log: Start StepOver");
		}
		else if (strstr(argv[0], "startr"))
		{
			set_auto_run_enabled(true);
			auto_run_state = AUTO_RUN_TYPE::AUTO_RUN;
			DbgCmdExec("run");
			telogger_logputs("Auto Run Log: Start Run");
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
		auto_run_state = AUTO_RUN_TYPE::AUTO_STEP_INTO;
		DbgCmdExec("StepInto");
		telogger_logputs("Auto Run Log: Start call");
	}
	else if (strstr(argv[0], "addlogbp.dll.import"))
	{
		telogger_logputs("Auto Run Log: Start addlogbp.dll.import ...");
		if (argc < 2)
		{
			add_import_log_breakpoint();
		}
		else if (argc < 3)
		{
			add_import_log_breakpoint(argv[1]);
		}
		else
		{
			add_import_log_breakpoint(argv[1], argv[2]);
		}
		telogger_logputs("Auto Run Log: Finish addlogbp.dll.import");
	}
	else if (strstr(argv[0], "addlogbp.dll.export"))
	{
		telogger_logputs("Auto Run Log: Start addlogbp.dll.export ...");
		if (argc < 2)
		{
			add_export_log_breakpoint();
		}
		else if (argc < 3)
		{
			add_export_log_breakpoint(argv[1]);
		}
		else
		{
			add_export_log_breakpoint(argv[1], argv[2]);
		}
		telogger_logputs("Auto Run Log: Finish addlogbp.dll.export");
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
	_plugin_registercommand(pluginHandle, "TElogger.auto.addlogbp.dll.import", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.addlogbp.dll.export", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.starti", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.starto", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.startr", auto_run_command_callback, false);
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
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.addlogbp.dll.import");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.addlogbp.dll.export");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starti");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starto");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.startr");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.call");
	return true;
}


void setup_auto_run()
{
}

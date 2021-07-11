#include "auto_run.h"

static duint skip_addr = 0;
std::vector<duint> auto_run_breakpoints;
static AUTO_RUN_TYPE auto_run_state = AUTO_RUN_TYPE::AUTO_STOP;
static std::map<duint, BP_INFO> log_breakpoints_info;
duint current_address = 0;
duint next_address = 0;
duint jamp_address = 0;


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


void remove_all_breakpoint(const char* name = NULL)
{
	BPMAP bp_map = {};
	DbgGetBpList(bp_normal, &bp_map);
	if (bp_map.bp == NULL)
	{
		return;
	}
	for (int i = 0; i < bp_map.count; i++)
	{
		if (name != NULL && strstr(bp_map.bp[i].name, name) == NULL)
		{
			continue;
		}
		Script::Debug::DeleteBreakpoint(bp_map.bp[i].addr);
		if (log_breakpoints_info.count(bp_map.bp[i].addr))
		{
			log_breakpoints_info.erase(bp_map.bp[i].addr);
		}
	}
}


void enable_all_breakpoint(const char* name = NULL)
{
	BPMAP bp_map = {};
	DbgGetBpList(bp_normal, &bp_map);
	if (bp_map.bp == NULL)
	{
		return;
	}
	for (int i = 0; i < bp_map.count; i++)
	{
		if (bp_map.bp[i].enabled)
		{
			continue;
		}
		if (name != NULL && strstr(bp_map.bp[i].name, name) == NULL)
		{
			continue;
		}
		char cmd[DEFAULT_BUF_SIZE] = {};
		_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "EnableBPX %p", (char*)bp_map.bp[i].addr);
		DbgCmdExecDirect(cmd);
	}
}


void disable_all_breakpoint(const char* name = NULL)
{
	BPMAP bp_map = {};
	DbgGetBpList(bp_normal, &bp_map);
	if (bp_map.bp == NULL)
	{
		return;
	}
	for (int i = 0; i < bp_map.count; i++)
	{
		if (!bp_map.bp[i].enabled)
		{
			continue;
		}
		if (name != NULL && strstr(bp_map.bp[i].name, name) == NULL)
		{
			continue;
		}
		Script::Debug::DisableBreakpoint(bp_map.bp[i].addr);
	}
}


void add_log_breakpoint(duint addr, const char* name, const char* command = NULL)
{
	if (command == NULL)
	{
		log_breakpoints_info[addr] = { addr, name, };
	}
	else
	{
		log_breakpoints_info[addr] = { addr, name, command };
	}
	char cmd[DEFAULT_BUF_SIZE] = { 0 };
	_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoRunLogBP_%s", (char*)addr, name);
	DbgCmdExecDirect(cmd);
}


// This function takes a long time to complete.
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
		if (mod_name != NULL && _stricmp(module_list[i].name, mod_name) != 0)
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
			if (api_name != NULL && _stricmp(import_list[j].name, api_name) != 0)
			{
				continue;
			}
			char name[DEFAULT_BUF_SIZE] = { 0 };
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
			_snprintf_s(name, sizeof(name), _TRUNCATE, "Import_%s_%s", module_list[i].name, import_list[j].name);
			add_log_breakpoint(*(duint*)data, name);
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


// This function takes a long time to complete.
// When mod_name is ntdll.dll, an error sometimes occurs during debugging.
// We need to restart to avoid it.
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
		if (mod_name != NULL && _stricmp(module_list[i].name, mod_name) != 0)
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
			if (api_name != NULL && _stricmp(export_list[j].name, api_name) != 0)
			{
				continue;
			}
			char name[DEFAULT_BUF_SIZE] = { 0 };
			_snprintf_s(name, sizeof(name), _TRUNCATE, "Export_%s_%s", module_list[i].name, export_list[j].name);
			add_log_breakpoint(export_list[j].va, name);
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


void add_export_log_breakpoint_without(std::vector<std::string>& mod_list)
{
	BridgeList<Script::Module::ModuleInfo> module_list;
	if (!Script::Module::GetList(&module_list))
	{
		telogger_logputs("Auto Run Log: Failed to get module list");
		return;
	}
	for (int i = 0; i < module_list.Count(); i++)
	{
		bool skip = false;
		for (auto& mod_name : mod_list)
		{
			if (_stricmp(module_list[i].name, mod_name.c_str()) == 0)
			{
				skip = true;
				break;
			}
		}
		if (skip)
		{
			continue;
		}
		add_export_log_breakpoint(module_list[i].name);
	}
}


void run_debug(StepInfo& step_info)
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
		if (auto_run_state == AUTO_RUN_TYPE::AUTO_MANUAL)
		{
			remove_all_breakpoint("TEloggerAutoManual");
		}
		auto_run_state = AUTO_RUN_TYPE::AUTO_STOP;
		telogger_logprintf("Auto Run Log: Break at %p\n", cip);
		return;
	}

	if (log_breakpoints_info.count(cip))
	{
		DbgCmdExecDirect(log_breakpoints_info[cip].command.c_str());
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
	else if (auto_run_state == AUTO_RUN_TYPE::AUTO_MANUAL)
	{
		// This command renders the GUI screen inoperable.
		char cmd[DEFAULT_BUF_SIZE] = { 0 };
		if (current_address != 0)
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteBPX %p", (char*)current_address);
			DbgCmdExecDirect(cmd);
		}
		if (next_address != 0)
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteBPX %p", (char*)next_address);
			DbgCmdExecDirect(cmd);
		}
		if (jamp_address != 0)
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteBPX %p", (char*)jamp_address);
			DbgCmdExecDirect(cmd);
		}

		DISASM_INSTR* instr = step_info.get_disasm_instr();
		current_address = cip;
		_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoManual_%p", (char*)current_address, (char*)current_address);
		DbgCmdExecDirect(cmd);
		next_address = cip + instr->instr_size;
		_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoManualNext_%p", (char*)next_address, (char*)next_address);
		DbgCmdExecDirect(cmd);
		if (instr->type == instr_branch && instr->argcount > 0)
		{
			jamp_address = instr->arg[0].constant;
			if (instr->arg[0].type == arg_memory)
			{
				jamp_address = instr->arg[0].memvalue;
			}
			if (strncmp(instr->instruction, "call", strlen("call")) != 0 || should_log(jamp_address))
			{
				_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetBPX %p, TEloggerAutoManualJamp_%p", (char*)jamp_address, (char*)jamp_address);
				DbgCmdExecDirect(cmd);
			}
			if (strncmp(instr->instruction, "call", strlen("call")) == 0)
			{
				next_address = 0;
			}
		}
		DbgCmdExec("run");
	}
	else if (auto_run_state == AUTO_RUN_TYPE::AUTO_MANUAL_HARDWARE)
	{
		char cmd[DEFAULT_BUF_SIZE] = { 0 };
		if (current_address != 0 && current_address != cip)
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteHardwareBreakpoint %p", (char*)current_address);
			DbgCmdExecDirect(cmd);
		}
		if (next_address != 0 && next_address != cip)
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteHardwareBreakpoint %p", (char*)next_address);
			DbgCmdExecDirect(cmd);
		}
		if (jamp_address != 0 && jamp_address != cip)
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteHardwareBreakpoint %p", (char*)jamp_address);
			DbgCmdExecDirect(cmd);
		}

		DISASM_INSTR* instr = step_info.get_disasm_instr();
		current_address = cip;
		next_address = 0;
		if (!strstr(instr->instruction, "ret") && !strstr(instr->instruction, "jmp far 0x0033:"))
		{
			next_address = cip + instr->instr_size;
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetHardwareBreakpoint %p, x", (char*)next_address);
			DbgCmdExecDirect(cmd);
		}
		jamp_address = 0;
		if (strstr(instr->instruction, "ret") || strstr(instr->instruction, "jmp far 0x0033:"))
		{
			REGDUMP* reg_dump = step_info.get_reg_dump();
			if (DbgMemIsValidReadPtr(reg_dump->regcontext.csp))
			{
				DbgMemRead(reg_dump->regcontext.csp, &jamp_address, sizeof(jamp_address));
				_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetHardwareBreakpoint %p, x", (char*)jamp_address);
				DbgCmdExecDirect(cmd);
			}
			else
			{
				telogger_logprintf("Auto Run: Invalid ptr %p\n", reg_dump->regcontext.csp);
			}
		}
		else if (instr->type == instr_branch && instr->argcount > 0)
		{
			jamp_address = instr->arg[0].value;
			if (instr->arg[0].type == arg_memory)
			{
				jamp_address = instr->arg[0].memvalue;
			}
			if (strncmp(instr->instruction, "call", strlen("call")) != 0 || should_log(jamp_address))
			{
				_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetHardwareBreakpoint %p, x", (char*)jamp_address);
				DbgCmdExecDirect(cmd);
			}
		}
		DbgCmdExec("run");
	}
	else if (auto_run_state == AUTO_RUN_TYPE::AUTO_MANUAL_MEMORY)
	{
		// Intercepted by seh
		duint mem_size = 0;
		duint base_address = DbgMemFindBaseAddr(current_address, &mem_size);
		char cmd[DEFAULT_BUF_SIZE] = { 0 };
		if (current_address != 0 && (cip < base_address || base_address + mem_size <= cip))
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteMemoryBPX %p", (char*)base_address);
			DbgCmdExecDirect(cmd);
		}
		base_address = DbgMemFindBaseAddr(next_address, &mem_size);
		if (next_address != 0 && (cip < base_address || base_address + mem_size <= cip))
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteMemoryBPX %p", (char*)base_address);
			DbgCmdExecDirect(cmd);
		}
		base_address = DbgMemFindBaseAddr(jamp_address, &mem_size);
		if (jamp_address != 0 && (cip < base_address || base_address + mem_size <= cip))
		{
			_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "DeleteMemoryBPX %p", (char*)base_address);
			DbgCmdExecDirect(cmd);
		}

		DISASM_INSTR* instr = step_info.get_disasm_instr();
		current_address = cip;
		base_address = DbgMemFindBaseAddr(current_address, &mem_size);
		next_address = 0;
		if (!strstr(instr->instruction, "ret") && !strstr(instr->instruction, "jmp far 0x0033:"))
		{
			next_address = cip + instr->instr_size;
			if (next_address < base_address || base_address + mem_size <= next_address)
			{
				_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetMemoryBPX %p, 1, x", (char*)next_address);
				DbgCmdExecDirect(cmd);
			}
		}
		jamp_address = 0;
		if (strstr(instr->instruction, "ret") || strstr(instr->instruction, "jmp far 0x0033:"))
		{
			REGDUMP* reg_dump = step_info.get_reg_dump();
			if (DbgMemIsValidReadPtr(reg_dump->regcontext.csp))
			{
				DbgMemRead(reg_dump->regcontext.csp, &jamp_address, sizeof(jamp_address));
				if (jamp_address < base_address || base_address + mem_size <= jamp_address)
				{
					_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetMemoryBPX %p, 1, x", (char*)jamp_address);
					DbgCmdExecDirect(cmd);
				}
			}
			else
			{
				telogger_logprintf("Auto Run: Invalid ptr %p\n", reg_dump->regcontext.csp);
			}
		}
		else if (instr->type == instr_branch && instr->argcount > 0)
		{
			jamp_address = instr->arg[0].value;
			if (instr->arg[0].type == arg_memory)
			{
				jamp_address = instr->arg[0].memvalue;
			}
			if ((jamp_address < base_address || base_address + mem_size <= jamp_address) &&
				(strncmp(instr->instruction, "call", strlen("call")) != 0 || should_log(jamp_address)))
			{
				_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetMemoryBPX %p, 1, x", (char*)jamp_address);
				DbgCmdExecDirect(cmd);
			}
		}
		DbgCmdExec("run");
	}
	else if(auto_run_state == AUTO_RUN_TYPE::AUTO_MANUAL_STEP)
	{
		DISASM_INSTR* instr = step_info.get_disasm_instr();
		char cmd[DEFAULT_BUF_SIZE] = "StepOver";
		if (instr->type == instr_branch && instr->argcount > 0)
		{
			jamp_address = instr->arg[0].constant;
			if (instr->arg[0].type == arg_memory)
			{
				jamp_address = instr->arg[0].memvalue;
			}
			if (should_log(jamp_address))
			{
				strcpy_s(cmd, _countof(cmd), "StepInto");
			}
		}
		DbgCmdExecDirect(cmd);
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
	if (isCommand(argv[0], "TElogger.auto.help"))
	{
		telogger_logputs("Auto Run Log: Help\n"
			"Command:\n"
			"    TElogger.auto.help\n"
			"    TElogger.auto.enable\n"
			"    TElogger.auto.disable\n"
			"    TElogger.auto.bp.add address\n"
			"    TElogger.auto.bp.rm\n"
			"    TElogger.auto.logbp.add address, name, [command]\n"
			"    TElogger.auto.logbp.add.dll.import dllname, [apiname]\n"
			"    TElogger.auto.logbp.add.dll.export dllname, [apiname]\n"
			"    TElogger.auto.logbp.add.dll.export.without dllname, [...]\n"
			"    TElogger.auto.logbp.rm\n"
			"    TElogger.auto.logbp.enable\n"
			"    TElogger.auto.logbp.disable\n"
			"    TElogger.auto.starti address\n"
			"    TElogger.auto.starto address\n"
			"    TElogger.auto.startr address\n"
			"    TElogger.auto.startm address, [|h|m|s]\n"
			"    TElogger.auto.call");
	}
	else if (isCommand(argv[0], "TElogger.auto.enable"))
	{
		set_auto_run_enabled(true);
		telogger_logputs("Auto Run Log: Enabled");
	}
	else if (isCommand(argv[0], "TElogger.auto.disable"))
	{
		set_auto_run_enabled(false);
		telogger_logputs("Auto Run Log: Disabled");
	}
	else if (isCommand(argv[0], " TElogger.auto.bp.add") || isCommand(argv[0], "TElogger.auto.start"))
	{
		if (argc < 2)
		{
			telogger_logprintf("Auto Run Log: Failed\n"
				"Command:\n"
				"    %s address\n", argv[0]);
			return false;
		}
		bool result_eval = false;
		duint value = DbgEval(argv[1], &result_eval);
		if (!result_eval)
		{
			telogger_logprintf("Auto Run Log: Failed\n"
				"Command:\n"
				"    %s address\n", argv[0]);
			return false;
		}

		add_breakpoint(value);
		telogger_logprintf("Auto Run Log: BP %p\n", (char*)value);

		if (isCommand(argv[0], "TElogger.auto.starti"))
		{
			set_auto_run_enabled(true);
			auto_run_state = AUTO_RUN_TYPE::AUTO_STEP_INTO;
			DbgCmdExec("StepInto");
			telogger_logputs("Auto Run Log: Start StepInto");
		}
		else if (isCommand(argv[0], "TElogger.auto.starto"))
		{
			set_auto_run_enabled(true);
			auto_run_state = AUTO_RUN_TYPE::AUTO_STEP_OVER;
			DbgCmdExec("StepOver");
			telogger_logputs("Auto Run Log: Start StepOver");
		}
		else if (isCommand(argv[0], "TElogger.auto.startr"))
		{
			set_auto_run_enabled(true);
			auto_run_state = AUTO_RUN_TYPE::AUTO_RUN;
			DbgCmdExec("run");
			telogger_logputs("Auto Run Log: Start Run");
		}
		else if (isCommand(argv[0], "TElogger.auto.startm"))
		{
			set_auto_run_enabled(true);
			auto_run_state = AUTO_RUN_TYPE::AUTO_MANUAL;
			current_address = 0;
			next_address = 0;
			jamp_address = 0;
			StepInfo step_info;
			if (argc >= 3)
			{
				if (stricmp(argv[2], "h") == 0)
				{
					auto_run_state = AUTO_RUN_TYPE::AUTO_MANUAL_HARDWARE;
				}
				else if (stricmp(argv[2], "m") == 0)
				{
					auto_run_state = AUTO_RUN_TYPE::AUTO_MANUAL_MEMORY;
					duint cip = 0;
					bool result_eval = false;
					cip = DbgEval("cip", &result_eval);
					if (!result_eval)
					{
						telogger_logputs("Auto Run Log: Failed to get cip");
						return false;
					}
					char cmd[DEFAULT_BUF_SIZE] = { 0 };
					_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "SetMemoryBPX %p, 1, x", (char*)cip);
					DbgCmdExecDirect(cmd);
				}
				else if (stricmp(argv[2], "s") == 0)
				{
					auto_run_state = AUTO_RUN_TYPE::AUTO_MANUAL_STEP;
				}
			}
			run_debug(step_info);
			telogger_logputs("Auto Run Log: Start Manual");
		}
	}
	else if (isCommand(argv[0], "TElogger.auto.bp.rm"))
	{
		remove_all_breakpoint("TEloggerAutoRunBP");
		telogger_logputs("Auto Run Log: Remove all breakpoints");
	}
	else if (isCommand(argv[0], "TElogger.auto.call"))
	{
		duint cip = 0;
		bool result_eval = false;
		cip = DbgEval("cip", &result_eval);
		if (!result_eval)
		{
			telogger_logputs("Auto Run Log: Failed to get cip");
			return false;
		}
		BASIC_INSTRUCTION_INFO basic_info = {};
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
	else if (isCommand(argv[0], "TElogger.auto.logbp.add.dll.import"))
	{
		telogger_logputs("Auto Run Log: Start addlogbp.dll.import ...");
		if (argc < 2)
		{
			telogger_logputs("Auto Run Log: Failed\n"
				"Command:\n"
				"    TElogger.auto.logbp.add.dll.import dllname, [apiname]");
		}
		else if (argc < 3)
		{
			if (!strcmp(argv[1], "all"))
			{
				add_import_log_breakpoint();
			}
			else
			{
				add_import_log_breakpoint(argv[1]);
			}
		}
		else
		{
			add_import_log_breakpoint(argv[1], argv[2]);
		}
		telogger_logputs("Auto Run Log: Finish addlogbp.dll.import");
	}
	else if (isCommand(argv[0], "TElogger.auto.logbp.add.dll.export.without"))
	{
		telogger_logputs("Auto Run Log: Start addlogbp.dll.export.without ...");
		if (argc < 2)
		{
			telogger_logputs("Auto Run Log: Failed\n"
				"Command:\n"
				"    TElogger.auto.logbp.add.dll.export.without dllname, [...]");
		}
		else
		{
			std::vector<std::string> skip_list;
			for (int i = 1; i < argc; i++)
			{
				skip_list.push_back(argv[i]);
			}
			add_export_log_breakpoint_without(skip_list);
		}
		telogger_logputs("Auto Run Log: Finish addlogbp.dll.export.without");
	}
	else if (isCommand(argv[0], "TElogger.auto.logbp.add.dll.export"))
	{
		telogger_logputs("Auto Run Log: Start addlogbp.dll.export ...");
		if (argc < 2)
		{
			telogger_logputs("Auto Run Log: Failed\n"
				"Command:\n"
				"    TElogger.auto.logbp.add.dll.export dllname, [apiname]");
		}
		else if (argc < 3)
		{
			if (!strcmp(argv[1], "all"))
			{
				add_export_log_breakpoint();
			}
			else
			{
				add_export_log_breakpoint(argv[1]);
			}
		}
		else
		{
			add_export_log_breakpoint(argv[1], argv[2]);
		}
		telogger_logputs("Auto Run Log: Finish addlogbp.dll.export");
	}
	else if (isCommand(argv[0], "TElogger.auto.logbp.add"))
	{
		if (argc < 3)
		{
			telogger_logputs("Auto Run Log: Failed\n"
				"Command:\n"
				"    TElogger.auto.logbp.add address, name, [command]");
			return false;
		}
		bool result_eval = false;
		duint value = DbgEval(argv[1], &result_eval);
		if (!result_eval)
		{
			telogger_logputs("Auto Run Log: Failed\n"
				"Command:\n"
				"    TElogger.auto.logbp.add address, name, [command]");
			return false;
		}
		if (argc > 3)
		{
			add_log_breakpoint(value, argv[2], argv[3]);
		}
		else
		{
			add_log_breakpoint(value, argv[2]);
		}
		telogger_logprintf("Auto Run Log: LBP %p\n", (char*)value);
	}
	else if (isCommand(argv[0], "TElogger.auto.logbp.rm"))
	{
		remove_all_breakpoint("TEloggerAutoRunLogBP");
		telogger_logputs("Auto Run Log: Remove all log breakpoints");
	}
	else if (isCommand(argv[0], "TElogger.auto.logbp.enable"))
	{
		enable_all_breakpoint("TEloggerAutoRunLogBP");
		telogger_logputs("Auto Run Log: Enable all log breakpoints");
	}
	else if (isCommand(argv[0], "TElogger.auto.logbp.disable"))
	{
		disable_all_breakpoint("TEloggerAutoRunLogBP");
		telogger_logputs("Auto Run Log: Disable all log breakpoints");
	}

	return true;
}


bool init_auto_run(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.auto.help", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.enable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.disable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.bp.add", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.bp.rm", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.add", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.add.dll.import", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.add.dll.export", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.add.dll.export.without", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.rm", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.enable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.logbp.disable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.starti", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.starto", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.startr", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.startm", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.call", auto_run_command_callback, false);
	return true;
}


bool stop_auto_run()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.bp.add");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.bp.rm");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.add");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.add.dll.import");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.add.dll.export");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.add.dll.export.without");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.rm");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.logbp.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starti");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.starto");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.startr");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.startm");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.call");
	return true;
}


void setup_auto_run()
{
}

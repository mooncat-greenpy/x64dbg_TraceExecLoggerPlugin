#include "log_instruction.h"

static int call_arg_log_count = 5;
static std::unordered_map<duint, std::string> gui_asm_string_cache;
static std::list<duint> gui_asm_string_fifo;
static std::unordered_map<duint, std::string> comment_string_cache;
static std::list<duint> comment_string_fifo;


json make_call_json()
{
	json call_json = json::object();

	bool result = false;
	duint csp = DbgEval("csp", &result);
	if (!result) {
		return call_json;
	}
	char value_name[20] = { 0 };
	call_json["arg"] = json::array();
#ifdef _WIN64
	call_json["arg"].push_back(make_address_json(reg->ccx));
	call_json["arg"][0]["name"] = "ccx";
	call_json["arg"].push_back(make_address_json(reg->cdx));
	call_json["arg"][1]["name"] = "cdx";
	call_json["arg"].push_back(make_address_json(reg->r8));
	call_json["arg"][2]["name"] = "r8";
	call_json["arg"].push_back(make_address_json(reg->r9));
	call_json["arg"][3]["name"] = "r9";
	for (int i = 4; i < call_arg_log_count; i++)
	{
		duint arg_offset = 0x20 + (i - 4) * 8;
		if (!DbgMemIsValidReadPtr(reg->csp + arg_offset))
		{
			continue;
		}
		duint tmp_value = 0;
		DbgMemRead(reg->csp + arg_offset, &tmp_value, sizeof(tmp_value));
		call_json["arg"].push_back(make_address_json(tmp_value));
		_snprintf_s(value_name, sizeof(value_name), _TRUNCATE, "csp + %#x", (int)arg_offset);
		call_json["arg"][i]["name"] = value_name;
	}
#else
	for (int i = 0; i < call_arg_log_count; i++)
	{
		duint arg_offset = i * 4;
		if (!DbgMemIsValidReadPtr(csp + arg_offset))
		{
			continue;
		}
		duint tmp_value = 0;
		DbgMemRead(csp + arg_offset, &tmp_value, sizeof(tmp_value));
		call_json["arg"].push_back(make_address_json(tmp_value));
		_snprintf_s(value_name, sizeof(value_name), _TRUNCATE, "csp + %#x", (int)arg_offset);
		call_json["arg"][i]["name"] = value_name;
	}
#endif
	return call_json;
}


json make_asm_json(duint cip)
{
	json asm_json = json::object();

	DISASM_INSTR instr = { 0 };
	DbgDisasmAt(cip, &instr);
	if (instr.type == instr_normal)
	{
		asm_json["type"] = "normal";
	}
	else if (instr.type == instr_branch)
	{
		asm_json["type"] = "branch";
		if (strncmp(instr.instruction, "call", strlen("call")) == 0)
		{
			asm_json["type"] = "call";
			asm_json["call"] = make_call_json();
		}
	}
	else if (instr.type == instr_stack)
	{
		asm_json["type"] = "stack";
	}
	else
	{
		asm_json["type"] = "other";
	}
	asm_json["instruction"] = instr.instruction;
	asm_json["size"] = instr.instr_size;
	asm_json["argcount"] = instr.argcount;
	asm_json["arg"] = json::array();
	for (int i = 0; i < instr.argcount; i++)
	{
		json arg = json::object();
		if (instr.arg[i].type == arg_normal)
		{
			arg["type"] = "normal";
		}
		else if (instr.arg[i].type == arg_memory)
		{
			arg["type"] = "memory";
		}
		else
		{
			arg["type"] = "other";
		}
		if (instr.arg[i].segment == SEG_DEFAULT)
		{
			arg["segment"] = "default";
		}
		else if (instr.arg[i].segment == SEG_CS)
		{
			arg["segment"] = "cs";
		}
		else if (instr.arg[i].segment == SEG_DS)
		{
			arg["segment"] = "ds";
		}
		else if (instr.arg[i].segment == SEG_ES)
		{
			arg["segment"] = "es";
		}
		else if (instr.arg[i].segment == SEG_FS)
		{
			arg["segment"] = "fs";
		}
		else if (instr.arg[i].segment == SEG_GS)
		{
			arg["segment"] = "gs";
		}
		else if (instr.arg[i].segment == SEG_SS)
		{
			arg["segment"] = "ss";
		}
		else
		{
			arg["segment"] = "other";
		}
		arg["mnemonic"] = instr.arg[i].mnemonic;
		arg["constant"] = instr.arg[i].constant;
		arg["value"] = make_address_json(instr.arg[i].value);
		arg["memvalue"] = make_address_json(instr.arg[i].memvalue);
		asm_json["arg"].push_back(arg);
	}
	return asm_json;
}


json log_instruction()
{
	json inst_json = json::object();
	if (!get_instruction_enabled())
	{
		return inst_json;
	}

	bool result = false;
	duint cip = DbgEval("cip", &result);
	if (!result) {
		return inst_json;
	}

	inst_json["type"] = "instruction";
	inst_json["address"] = make_address_json(cip);

	char asm_string[DEFAULT_BUF_SIZE] = { 0 };
	bool cache_result = false;
	std::string gui_asm_cached = get_gui_asm_string_cache_data(cip, &cache_result);
	if (cache_result)
	{
		inst_json["asm_str"] = gui_asm_cached.c_str();
	}
	else
	{
		GuiGetDisassembly(cip, asm_string);
		inst_json["asm_str"] = asm_string;
		set_gui_asm_string_cache_data(cip, std::string(asm_string));
	}

	inst_json["asm"] = make_asm_json(cip);

	char comment_text[MAX_COMMENT_SIZE] = { 0 };
	cache_result = false;
	std::string comment_cached = get_comment_string_cache_data(cip, &cache_result);
	if (cache_result)
	{
		inst_json["comment"] = comment_cached.c_str();
	}
	else
	{
		if (DbgGetCommentAt(cip, comment_text))
		{
			inst_json["comment"] = comment_text;
		}
		else
		{
			inst_json["comment"] = "";
		}
		set_comment_string_cache_data(cip, std::string(inst_json["comment"]));
	}

	return inst_json;
}


bool instruction_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Instruction Help\n"
			"Command:\n"
			"    TElogger.inst.help\n"
			"    TElogger.inst.enable\n"
			"    TElogger.inst.disable\n"
			"    TElogger.inst.arglogcount [num]\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_instruction_enabled(true);
		telogger_logputs("Instruction Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_instruction_enabled(false);
		telogger_logputs("Instruction Log: Disabled");
	}
	else if (strstr(argv[0], "arglogcount"))
	{
		if (argc < 2)
		{
			telogger_logprintf("Call Argument Log Count: %d\n", call_arg_log_count);
			return true;
		}
		call_arg_log_count = atoi(argv[1]);
		telogger_logprintf("Call Argument Log Count: %d\n", call_arg_log_count);
	}

	return true;
}


bool init_instruction_log(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.inst.help", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.enable", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.disable", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.arglogcount", instruction_command_callback, false);
	return true;
}


bool stop_instruction_log()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.arglogcount");
	return true;
}


void setup_instruction_log()
{
}

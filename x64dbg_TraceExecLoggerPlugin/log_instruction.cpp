#include "log_instruction.h"

static bool instruction_enabled = true;
static int call_arg_log_count = 5;


json make_asm_json(DISASM_INSTR* instr)
{
	json asm_json = json::object();
	if (!instr)
	{
		return asm_json;
	}
	if (instr->type == instr_normal)
	{
		asm_json["type"] = "normal";
	}
	else if (instr->type == instr_branch)
	{
		asm_json["type"] = "branch";
	}
	else if (instr->type == instr_stack)
	{
		asm_json["type"] = "stack";
	}
	else
	{
		asm_json["type"] = "other";
	}
	asm_json["instruction"] = instr->instruction;
	asm_json["size"] = instr->instr_size;
	asm_json["argcount"] = instr->argcount;
	asm_json["arg"] = json::array();
	for (int i = 0; i < instr->argcount; i++)
	{
		json arg = json::object();
		if (instr->arg[i].type == arg_normal)
		{
			arg["type"] = "normal";
		}
		else if (instr->arg[i].type == arg_memory)
		{
			arg["type"] = "memory";
		}
		else
		{
			arg["type"] = "other";
		}
		if (instr->arg[i].segment == SEG_DEFAULT)
		{
			arg["segment"] = "default";
		}
		else if (instr->arg[i].segment == SEG_CS)
		{
			arg["segment"] = "cs";
		}
		else if (instr->arg[i].segment == SEG_DS)
		{
			arg["segment"] = "ds";
		}
		else if (instr->arg[i].segment == SEG_ES)
		{
			arg["segment"] = "es";
		}
		else if (instr->arg[i].segment == SEG_FS)
		{
			arg["segment"] = "fs";
		}
		else if (instr->arg[i].segment == SEG_GS)
		{
			arg["segment"] = "gs";
		}
		else if (instr->arg[i].segment == SEG_SS)
		{
			arg["segment"] = "ss";
		}
		else
		{
			arg["segment"] = "other";
		}
		arg["mnemonic"] = instr->arg[i].mnemonic;
		arg["constant"] = instr->arg[i].constant;
		arg["value"] = make_address_json(instr->arg[i].value);
		arg["memvalue"] = make_address_json(instr->arg[i].memvalue);
		asm_json["arg"].push_back(arg);
	}
	return asm_json;
}


json make_call_json(REGISTERCONTEXT* reg)
{
	json call_json = json::object();
	if (!reg)
	{
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
	for (duint i = 4; i < call_arg_log_count; i++)
	{
		duint arg_offset = 0x20 + (i - 4) * 8;
		if (!DbgMemIsValidReadPtr(reg->csp + arg_offset))
		{
			continue;
		}
		duint tmp_value = 0;
		DbgMemRead(reg->csp + arg_offset, &tmp_value, sizeof(tmp_value));
		call_json["arg"].push_back(make_address_json(tmp_value));
		_snprintf_s(value_name, sizeof(value_name), _TRUNCATE, "csp + 0x%x", (int)arg_offset);
		call_json["arg"][i]["name"] = value_name;
	}
#else
	for (duint i = 0; i < call_arg_log_count; i++)
	{
		duint arg_offset = i * 4;
		if (!DbgMemIsValidReadPtr(reg->csp + arg_offset))
		{
			continue;
		}
		duint tmp_value = 0;
		DbgMemRead(reg->csp + arg_offset, &tmp_value, sizeof(tmp_value));
		call_json["arg"].push_back(make_address_json(tmp_value));
		_snprintf_s(value_name, sizeof(value_name), _TRUNCATE, "csp + 0x%x", (int)arg_offset);
		call_json["arg"][i]["name"] = value_name;
	}
#endif
	return call_json;
}


json log_instruction()
{
	json inst_json = json::object();
	if (!instruction_enabled)
	{
		return inst_json;
	}

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	inst_json["type"] = "instruction";
	inst_json["address"] = reg.cip;

	char asm_string[MAX_PATH] = { 0 };
	GuiGetDisassembly(reg.cip, asm_string);
	inst_json["asm_str"] = asm_string;

	char label_text[MAX_PATH] = { 0 };
	make_address_label_string(reg.cip, label_text, sizeof(label_text));
	inst_json["label"] = label_text;

	DISASM_INSTR instr = { 0 };
	DbgDisasmAt(reg.cip, &instr);
	inst_json["asm"] = make_asm_json(&instr);

	BASIC_INSTRUCTION_INFO basic_info = { 0 };
	DbgDisasmFastAt(reg.cip, &basic_info);
	if (basic_info.call)
	{
		inst_json["call"] = make_call_json(&reg);
	}
	char comment_text[MAX_COMMENT_SIZE] = { 0 };
	if (DbgGetCommentAt(reg.cip, comment_text))
	{
		inst_json["comment"] = comment_text;
	}
	else
	{
		inst_json["comment"] = "";
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
		_plugin_logputs("Command:\n"
			"    TElogger.inst.help\n"
			"    TElogger.inst.enable\n"
			"    TElogger.inst.disable\n"
			"    TElogger.inst.arglogcount [num]\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		instruction_enabled = true;
		_plugin_logputs("Instruction Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		instruction_enabled = false;
		_plugin_logputs("Instruction Log: Disabled");
	}
	else if (strstr(argv[0], "arglogcount"))
	{
		if (argc < 2)
		{
			_plugin_logprintf("Call Argument Log Count: %d\n", call_arg_log_count);
			return true;
		}
		call_arg_log_count = atoi(argv[1]);
		_plugin_logprintf("Call Argument Log Count: %d\n", call_arg_log_count);
	}

	return true;
}


bool instruction_log_plugin_init(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.inst.help", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.enable", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.disable", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.arglogcount", instruction_command_callback, false);
	return true;
}


bool instruction_log_plugin_stop()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.arglogcount");
	return true;
}


void instruction_log_plugin_setup()
{
}
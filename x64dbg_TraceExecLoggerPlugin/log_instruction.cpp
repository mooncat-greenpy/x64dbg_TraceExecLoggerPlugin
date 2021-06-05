#include "log_instruction.h"


void make_asm_json(LOG_ASSEMBLY& asm_json, REGDUMP* reg_dump)
{
	asm_json.arg.clear();
	DISASM_INSTR instr = {};
	DbgDisasmAt(reg_dump->regcontext.cip, &instr);
	if (instr.type == instr_normal)
	{
		asm_json.type = "normal";
	}
	else if (instr.type == instr_branch)
	{
		asm_json.type = "branch";
		if (strncmp(instr.instruction, "call", strlen("call")) == 0)
		{
			asm_json.type = "call";
			add_changed_memory(reg_dump->regcontext.csp - sizeof(duint));
		}
	}
	else if (instr.type == instr_stack)
	{
		asm_json.type = "stack";
	}
	else
	{
		asm_json.type = "other";
	}

	if (strncmp(instr.instruction, "push", strlen("push")) == 0)
	{
		add_changed_memory(reg_dump->regcontext.csp - sizeof(duint));
	}
	if (strstr(instr.instruction, "movs") != NULL || strstr(instr.instruction, "stos") != NULL)
	{
		add_changed_memory(reg_dump->regcontext.cdi);
	}

	asm_json.instruction = instr.instruction;
	asm_json.size = instr.instr_size;
	asm_json.argcount = instr.argcount;
	for (int i = 0; i < instr.argcount; i++)
	{
		LOG_ARGUMENT arg = LOG_ARGUMENT();
		if (instr.arg[i].type == arg_normal)
		{
			arg.type = "normal";
		}
		else if (instr.arg[i].type == arg_memory)
		{
			arg.type = "memory";
			if (i == 0)
			{
				add_changed_memory(instr.arg[i].value);
			}
		}
		else
		{
			arg.type = "other";
		}
		if (instr.arg[i].segment == SEG_DEFAULT)
		{
			arg.segment = "default";
		}
		else if (instr.arg[i].segment == SEG_CS)
		{
			arg.segment = "cs";
		}
		else if (instr.arg[i].segment == SEG_DS)
		{
			arg.segment = "ds";
		}
		else if (instr.arg[i].segment == SEG_ES)
		{
			arg.segment = "es";
		}
		else if (instr.arg[i].segment == SEG_FS)
		{
			arg.segment = "fs";
		}
		else if (instr.arg[i].segment == SEG_GS)
		{
			arg.segment = "gs";
		}
		else if (instr.arg[i].segment == SEG_SS)
		{
			arg.segment = "ss";
		}
		else
		{
			arg.segment = "other";
		}
		arg.mnemonic = instr.arg[i].mnemonic;
		arg.constant = instr.arg[i].constant;
		make_address_json(arg.value, instr.arg[i].value);
		make_address_json(arg.memvalue, instr.arg[i].memvalue);
		asm_json.arg.push_back(arg);
	}
}


void log_instruction(LOG_INSTRUCTION& inst_json, StepInfo& step_info)
{
	if (!get_instruction_enabled())
	{
		inst_json.enabled = false;
		return;
	}

	REGDUMP* reg_dump = step_info.get_reg_dump();
	make_address_json(inst_json.address, reg_dump->regcontext.cip);

	char asm_string[GUI_MAX_DISASSEMBLY_SIZE] = { 0 };
	bool cache_result = false;
	std::string gui_asm_cached = get_gui_asm_string_cache_data(reg_dump->regcontext.cip, &cache_result);
	if (cache_result)
	{
		inst_json.asm_str_cache = true;
		inst_json.asm_str = gui_asm_cached;
	}
	else
	{
		inst_json.asm_str_cache = false;
		if (!GuiGetDisassembly(reg_dump->regcontext.cip, asm_string))
		{
			strncpy_s(asm_string, sizeof(asm_string), "error", _TRUNCATE);
		}
		inst_json.asm_str = asm_string;
		set_gui_asm_string_cache_data(reg_dump->regcontext.cip, std::string(asm_string));
	}

	make_asm_json(inst_json.assembly, reg_dump);

	char comment_text[MAX_COMMENT_SIZE] = { 0 };
	cache_result = false;
	std::string comment_cached = get_comment_string_cache_data(reg_dump->regcontext.cip, &cache_result);
	if (cache_result)
	{
		inst_json.comment_cache = true;
		inst_json.comment = comment_cached;
	}
	else
	{
		inst_json.comment_cache = false;
		if (DbgGetCommentAt(reg_dump->regcontext.cip, comment_text))
		{
			inst_json.comment = comment_text;
		}
		else
		{
			inst_json.comment = "";
		}
		set_comment_string_cache_data(reg_dump->regcontext.cip, inst_json.comment);
	}

	inst_json.enabled = true;
}


bool instruction_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Instruction Log: Help\n"
			"Command:\n"
			"    TElogger.inst.help\n"
			"    TElogger.inst.enable\n"
			"    TElogger.inst.disable");
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

	return true;
}


bool init_instruction_log(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.inst.help", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.enable", instruction_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.inst.disable", instruction_command_callback, false);
	return true;
}


bool stop_instruction_log()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.inst.disable");
	return true;
}


void setup_instruction_log()
{
}

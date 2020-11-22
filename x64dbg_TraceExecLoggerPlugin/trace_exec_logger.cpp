#include "trace_exec_logger.h"


using json = nlohmann::json;
char file_path[MAX_PATH] = { 0 };
json log_json = json::array();


void save_json_file(const char* file_name, const char* buffer)
{
	if (!file_name || !buffer)
	{
		return;
	}

	SYSTEMTIME system_time = { 0 };
	GetLocalTime(&system_time);

	HANDLE log_file_handle = INVALID_HANDLE_VALUE;
	char log_file_name[MAX_PATH] = { 0 };
	_snprintf_s(log_file_name, sizeof(log_file_name), _TRUNCATE, "%s_%d-%d-%d-%d-%d-%d.json", PathFindFileNameA(file_name), system_time.wYear, system_time.wMonth, system_time.wDay, system_time.wHour, system_time.wMinute, system_time.wSecond);
	log_file_handle = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (log_file_handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD written = 0;
	WriteFile(log_file_handle, buffer, strlen(buffer), &written, NULL);

	CloseHandle(log_file_handle);
}


void make_address_label_string(duint addr, char* text, size_t text_size)
{
	if (!text)
	{
		return;
	}

	char module_text[MAX_PATH] = { 0 };
	char label_text[MAX_PATH] = { 0 };
	bool has_module = DbgGetModuleAt(addr, module_text);
	bool has_label = DbgGetLabelAt(addr, SEG_DEFAULT, label_text);
	if (has_module && has_label && strlen(module_text))
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%s.%s", module_text, label_text);
	}
	else if (module_text && strlen(module_text))
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%s.%p", module_text, (char*)addr);
	}
	else if (has_label)
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%p<%s>", (char*)addr, label_text);
	}
	else
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%p", (char*)addr);
	}
}


void make_hex_string(char* data, size_t data_size, char* text, size_t text_size)
{
	if (!data || !text)
	{
		return;
	}

	for (size_t i = 0; i < data_size; i++)
	{
		char tmp[10] = { 0 };
		_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02x ", (unsigned char)data[i]);
		strncat_s(text, text_size, tmp, _TRUNCATE);
	}
}


json make_address_json(duint addr)
{
	json address_json = json::object();
	address_json["value"] = addr;

	char label_text[MAX_PATH] = { 0 };
	make_address_label_string(addr, label_text, sizeof(label_text));
	address_json["label"] = label_text;

	char text[MAX_PATH] = { 0 };
	char string[MAX_PATH] = { 0 };
	char hex_string[MAX_PATH] = { 0 };
	bool has_string = DbgGetStringAt(addr, string);
	bool has_data = false;
	if (DbgMemIsValidReadPtr(addr))
	{
		char data[HEX_SIZE] = { 0 };
		DbgMemRead(addr, data, sizeof(data));
		make_hex_string(data, sizeof(data), hex_string, sizeof(hex_string));
		has_data = true;
	}
	if (has_string)
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "%s", string);
	}
	else if (has_data)
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "%s", hex_string);
	}
	else
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "");
	}
	address_json["data"] = text;

	return address_json;
}


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
	for (duint i = 4; i < 5; i++)
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
	for (duint i = 0; i < 5; i++)
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


json log_stack()
{
	json stack_json = json::object();
	stack_json["type"] = "stack";
	stack_json["data"] = json::array();

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	duint csp = reg_dump.regcontext.csp;

	for (int i = 0; i < 0x10; i++)
	{
		json tmp_json = json::object();
		duint stack_addr = csp + i * sizeof(duint);
		duint stack_value = 0;
		if (!DbgMemIsValidReadPtr(stack_addr))
		{
			tmp_json["address"] = stack_addr;
			tmp_json["value"] = "";
			stack_json["data"].push_back(tmp_json);
			continue;
		}
		DbgMemRead(stack_addr, &stack_value, sizeof(stack_value));

		tmp_json["address"] = stack_addr;
		tmp_json["value"] = make_address_json(stack_value);
		stack_json["data"].push_back(tmp_json);
	}

	return stack_json;
}


json log_registry()
{
	json reg_json = json::object();
	reg_json["type"] = "register";

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	reg_json["cax"] = make_address_json(reg.cax);
	reg_json["cbx"] = make_address_json(reg.cbx);
	reg_json["ccx"] = make_address_json(reg.ccx);
	reg_json["cdx"] = make_address_json(reg.cdx);
	reg_json["csi"] = make_address_json(reg.csi);
	reg_json["cdi"] = make_address_json(reg.cdi);
	reg_json["csp"] = make_address_json(reg.csp);
	reg_json["cbp"] = make_address_json(reg.cbp);
#ifdef _WIN64
	reg_json["r8"] = make_address_json(reg.r8);
	reg_json["r9"] = make_address_json(reg.r9);
	reg_json["r10"] = make_address_json(reg.r10);
	reg_json["r11"] = make_address_json(reg.r11);
	reg_json["r12"] = make_address_json(reg.r12);
	reg_json["r13"] = make_address_json(reg.r13);
	reg_json["r14"] = make_address_json(reg.r14);
	reg_json["r15"] = make_address_json(reg.r15);
#endif
	return reg_json;
}


json log_instruction()
{
	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	json inst_json = json::object();
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

	return inst_json;
}


void log_exec()
{
	if (!regstep_enabled)
		return;

	json entry = json::object();
	entry["inst"] = log_instruction();
	entry["reg"] = log_registry();
	entry["stack"] = log_stack();
	log_json.push_back(entry);
}


extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY* info)
{
	switch (info->hEntry)
	{
	case MENU_ENABLED:
	{
		regstep_enabled = !regstep_enabled;
		BridgeSettingSetUint("trace exec logger", "Enabled", regstep_enabled);
	}
	break;
	}
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
}


extern "C" __declspec(dllexport) void CBSTOPDEBUG(CBTYPE, PLUG_CB_STOPDEBUG* info)
{
	save_json_file(file_path, log_json.dump().c_str());
	memset(file_path, 0, sizeof(file_path));
	log_json.clear();
}


bool logger_plugin_init(PLUG_INITSTRUCT* init_struct)
{
	duint setting = regstep_enabled;
	BridgeSettingGetUint("trace exec logger", "Enabled", &setting);
	regstep_enabled = !!setting;
	return true;
}


bool logger_plugin_stop()
{
	return true;
}


void logger_plugin_setup()
{
	_plugin_menuaddentry(hMenu, MENU_ENABLED, "Enabled");
	_plugin_menuentrysetchecked(pluginHandle, MENU_ENABLED, regstep_enabled);
}


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


void make_address_string(duint addr, char* text, size_t size)
{
	if (!text)
	{
		return;
	}

	char module_text[MAX_PATH] = { 0 };
	char label_text[MAX_PATH] = { 0 };
	char string[MAX_PATH] = { 0 };
	char hex_string[MAX_PATH] = { 0 };
	bool has_module = DbgGetModuleAt(addr, module_text);
	bool has_label = DbgGetLabelAt(addr, SEG_DEFAULT, label_text);
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
		_snprintf_s(text, size, _TRUNCATE, "%p %s", (char*)addr, string);
	}
	else if (has_module && has_label && strlen(module_text))
	{
		_snprintf_s(text, size, _TRUNCATE, "%p %s.%s", (char*)addr, module_text, label_text);
	}
	else if (module_text && strlen(module_text))
	{
		_snprintf_s(text, size, _TRUNCATE, "%p %s.%p", (char*)addr, module_text, (char*)addr);
	}
	else if (has_label)
	{
		_snprintf_s(text, size, _TRUNCATE, "%p <%s>", (char*)addr, label_text);
	}
	else if (has_data)
	{
		_snprintf_s(text, size, _TRUNCATE, "%p %s", (char*)addr, hex_string);
	}
	else
	{
		_snprintf_s(text, size, _TRUNCATE, "%p", (char*)addr);
	}
}


void make_inst_string(duint addr, char* text, size_t size)
{
	if (!text)
	{
		return;
	}

	char asm_string[MAX_PATH] = { 0 };
	GuiGetDisassembly(addr, asm_string);

	char module_text[MAX_PATH] = { 0 };
	char label_text[MAX_PATH] = { 0 };
	bool has_module = DbgGetModuleAt(addr, module_text);
	bool has_label = DbgGetLabelAt(addr, SEG_DEFAULT, label_text);

	if (has_module && has_label)
	{
		_snprintf_s(text, size, _TRUNCATE, "%s.%s %s", module_text, label_text, asm_string);
	}
	else if (module_text)
	{
		_snprintf_s(text, size, _TRUNCATE, "%s.%p %s", module_text, (char*)addr, asm_string);
	}
	else if (has_label)
	{
		_snprintf_s(text, size, _TRUNCATE, "%p <%s> %s", (char*)addr, label_text, asm_string);
	}
	else
	{
		_snprintf_s(text, size, _TRUNCATE, "%p %s", (char*)addr, asm_string);
	}
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
		char addr_string[MAX_PATH] = { 0 };
		duint stack_addr = csp + i * sizeof(duint);
		duint stack_value = 0;
		if (!DbgMemIsValidReadPtr(stack_addr))
		{
			tmp_json["address"] = stack_addr;
			tmp_json["info"] = "";
			stack_json["data"].push_back(tmp_json);
			continue;
		}
		DbgMemRead(stack_addr, &stack_value, sizeof(stack_value));

		make_address_string(stack_value, addr_string, sizeof(addr_string));
		tmp_json["address"] = stack_addr;
		tmp_json["info"] = addr_string;
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

	char reg_string[MAX_PATH] = { 0 };
#ifdef _WIN64
	make_address_string(reg.cax, reg_string, sizeof(reg_string));
	reg_json["cax"] = reg_string;
	make_address_string(reg.cbx, reg_string, sizeof(reg_string));
	reg_json["cbx"] = reg_string;
	make_address_string(reg.ccx, reg_string, sizeof(reg_string));
	reg_json["ccx"] = reg_string;
	make_address_string(reg.cdx, reg_string, sizeof(reg_string));
	reg_json["cdx"] = reg_string;
	make_address_string(reg.csi, reg_string, sizeof(reg_string));
	reg_json["csi"] = reg_string;
	make_address_string(reg.cdi, reg_string, sizeof(reg_string));
	reg_json["cdi"] = reg_string;
	make_address_string(reg.csp, reg_string, sizeof(reg_string));
	reg_json["csp"] = reg_string;
	make_address_string(reg.cbp, reg_string, sizeof(reg_string));
	reg_json["cbp"] = reg_string;
	make_address_string(reg.r8, reg_string, sizeof(reg_string));
	reg_json["r8"] = reg_string;
	make_address_string(reg.r9, reg_string, sizeof(reg_string));
	reg_json["r9"] = reg_string;
	make_address_string(reg.r10, reg_string, sizeof(reg_string));
	reg_json["r10"] = reg_string;
	make_address_string(reg.r11, reg_string, sizeof(reg_string));
	reg_json["r11"] = reg_string;
	make_address_string(reg.r12, reg_string, sizeof(reg_string));
	reg_json["r12"] = reg_string;
	make_address_string(reg.r13, reg_string, sizeof(reg_string));
	reg_json["r13"] = reg_string;
	make_address_string(reg.r14, reg_string, sizeof(reg_string));
	reg_json["r14"] = reg_string;
	make_address_string(reg.r15, reg_string, sizeof(reg_string));
	reg_json["r15"] = reg_string;
#else
	make_address_string(reg.cax, reg_string, sizeof(reg_string));
	reg_json["cax"] = reg_string;
	make_address_string(reg.cbx, reg_string, sizeof(reg_string));
	reg_json["cbx"] = reg_string;
	make_address_string(reg.ccx, reg_string, sizeof(reg_string));
	reg_json["ccx"] = reg_string;
	make_address_string(reg.cdx, reg_string, sizeof(reg_string));
	reg_json["cdx"] = reg_string;
	make_address_string(reg.csi, reg_string, sizeof(reg_string));
	reg_json["csi"] = reg_string;
	make_address_string(reg.cdi, reg_string, sizeof(reg_string));
	reg_json["cdi"] = reg_string;
	make_address_string(reg.csp, reg_string, sizeof(reg_string));
	reg_json["csp"] = reg_string;
	make_address_string(reg.cbp, reg_string, sizeof(reg_string));
	reg_json["cbp"] = reg_string;
#endif
	return reg_json;
}


json log_instruction()
{
	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	char inst_string[MAX_PATH] = { 0 };
	char buffer[MAX_PATH] = { 0 };
	make_inst_string(reg.cip, inst_string, sizeof(inst_string));
	json inst_json = json::object();
	inst_json["type"] = "instruction";
	inst_json["address"] = reg.cip;
	inst_json["asm"] = inst_string;
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


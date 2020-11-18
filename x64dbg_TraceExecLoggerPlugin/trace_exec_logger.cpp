#include "trace_exec_logger.h"


HANDLE log_file_handle = NULL;


void create_file(const char* file_name)
{
	if (!file_name || log_file_handle)
	{
		return;
	}
	SYSTEMTIME system_time = { 0 };
	GetLocalTime(&system_time);
	char log_file_name[MAX_PATH] = { 0 };
	_snprintf_s(log_file_name, sizeof(log_file_name), _TRUNCATE, "%s_%d-%d-%d-%d-%d-%d.txt", PathFindFileNameA(file_name), system_time.wYear, system_time.wMonth, system_time.wDay, system_time.wHour, system_time.wMinute, system_time.wSecond);
	log_file_handle = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}


void close_file()
{
	if (!log_file_handle)
	{
		return;
	}
	CloseHandle(log_file_handle);
	log_file_handle = NULL;
}


void write_file(const char* buffer)
{
	if (!buffer || !log_file_handle)
	{
		return;
	}
	DWORD written = 0;
	WriteFile(log_file_handle, buffer, strlen(buffer), &written, NULL);
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


void log_stack()
{
	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));

	duint csp = reg_dump.regcontext.csp;

	for (int i = 0; i < 0x10; i++)
	{
		char addr_string[MAX_PATH] = { 0 };
		char buffer[MAX_PATH] = { 0 };
		duint stack_addr = csp + i * sizeof(duint);
		duint stack_value = 0;
		if (!DbgMemIsValidReadPtr(stack_addr))
		{
			_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%p\n", (char*)stack_addr);
			write_file(buffer);
			continue;
		}
		DbgMemRead(stack_addr, &stack_value, sizeof(stack_value));

		make_address_string(stack_value, addr_string, sizeof(addr_string));
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%p %s\n", (char*)stack_addr, addr_string);
		write_file(buffer);
	}
}


void log_registry()
{
	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	char buffer[MAX_PATH] = { 0 };
	char reg_string[MAX_PATH] = { 0 };
#ifdef _WIN64
	make_address_string(reg.cax, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rax=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cbx, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rbx=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.ccx, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rcx=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cdx, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rdx=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.csi, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rsi=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cdi, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rdi=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.csp, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rsp=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cbp, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    rbp=%s\n", reg_string);
	write_file(buffer);
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    r8=%p  r9=%p r10=%p r11=%p r12=%p r13=%p r14=%p r15=%p\n",
		(char*)reg.r8, (char*)reg.r9, (char*)reg.r10,
		(char*)reg.r11, (char*)reg.r12, (char*)reg.r13,
		(char*)reg.r14, (char*)reg.r15);
	write_file(buffer);
#else
	make_address_string(reg.cax, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    eax=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cbx, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    ebx=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.ccx, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    ecx=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cdx, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    edx=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.csi, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    esi=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cdi, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    edi=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.csp, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    esp=%s\n", reg_string);
	write_file(buffer);
	make_address_string(reg.cbp, reg_string, sizeof(reg_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "    ebp=%s\n", reg_string);
	write_file(buffer);
#endif
}


void log_instruction()
{
	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	char inst_string[MAX_PATH] = { 0 };
	char buffer[MAX_PATH] = { 0 };
	make_inst_string(reg.cip, inst_string, sizeof(inst_string));
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%s\n", inst_string);
	write_file(buffer);
}


void log_exec()
{
	if (!regstep_enabled)
		return;

	log_instruction();
	log_registry();
	log_stack();
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
	create_file(info->szFileName);
}


extern "C" __declspec(dllexport) void CBSTOPDEBUG(CBTYPE, PLUG_CB_STOPDEBUG* info)
{
	close_file();
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


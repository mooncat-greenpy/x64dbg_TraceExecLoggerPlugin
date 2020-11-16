#include "trace_exec_logger.h"



void make_hex_string(char* data, size_t data_size, char* text, size_t text_size)
{
	if (!data || !text)
	{
		return;
	}

	for (int i = 0; i < data_size; i++)
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


void log_exec()
{
	if (!regstep_enabled)
		return;

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	auto& reg = reg_dump.regcontext;

	char inst_string[MAX_PATH] = { 0 };
	make_inst_string(reg.cip, inst_string, sizeof(inst_string));


#ifdef _WIN64
	_plugin_logprintf("%s\n", inst_point_string);
	char reg_string[MAX_PATH] = { 0 };
	make_address_string(reg.cax, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rax=%s\n", reg_string);
	make_address_string(reg.cbx, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rbx=%s\n", reg_string);
	make_address_string(reg.ccx, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rcx=%s\n", reg_string);
	make_address_string(reg.cdx, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rdx=%s\n", reg_string);
	make_address_string(reg.csi, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rsi=%s\n", reg_string);
	make_address_string(reg.cdi, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rdi=%s\n", reg_string);
	make_address_string(reg.csp, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rsp=%s\n", reg_string);
	make_address_string(reg.cbp, reg_string, sizeof(reg_string));
	_plugin_logprintf("    rbp=%s\n", reg_string);
	_plugin_logprintf("    r8=%p  r9=%p r10=%p r11=%p r12=%p r13=%p r14=%p r15=%p\n",
		reg.r8, reg.r9, reg.r10,
		reg.r11, reg.r12, reg.r13,
		reg.r14, reg.r15);
#else
	_plugin_logprintf("%s\n", inst_string);
	char reg_string[MAX_PATH] = { 0 };
	make_address_string(reg.cax, reg_string, sizeof(reg_string));
	_plugin_logprintf("    eax=%s\n", reg_string);
	make_address_string(reg.cbx, reg_string, sizeof(reg_string));
	_plugin_logprintf("    ebx=%s\n", reg_string);
	make_address_string(reg.ccx, reg_string, sizeof(reg_string));
	_plugin_logprintf("    ecx=%s\n", reg_string);
	make_address_string(reg.cdx, reg_string, sizeof(reg_string));
	_plugin_logprintf("    edx=%s\n", reg_string);
	make_address_string(reg.csi, reg_string, sizeof(reg_string));
	_plugin_logprintf("    esi=%s\n", reg_string);
	make_address_string(reg.cdi, reg_string, sizeof(reg_string));
	_plugin_logprintf("    edi=%s\n", reg_string);
	make_address_string(reg.csp, reg_string, sizeof(reg_string));
	_plugin_logprintf("    esp=%s\n", reg_string);
	make_address_string(reg.cbp, reg_string, sizeof(reg_string));
	_plugin_logprintf("    ebp=%s\n", reg_string);
#endif
}


extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY * info)
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


extern "C" __declspec(dllexport) void CBSTEPPED(CBTYPE, PLUG_CB_STEPPED * info)
{
	log_exec();
}


/*** EIP address is sometimes wrong. ***
extern "C" __declspec(dllexport) void CBDEBUGEVENT(CBTYPE, PLUG_CB_DEBUGEVENT * info)
{
	log_exec();
}
*/


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


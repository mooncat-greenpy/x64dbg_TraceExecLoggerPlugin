#include "trace_exec_logger.h"


void make_inst_string(duint addr, char* text, size_t size)
{
	if (!text)
	{
		return;
	}

	char module_text[MAX_PATH] = { 0 };
	char label_text[MAX_PATH] = { 0 };
	char string[MAX_PATH] = { 0 };
	bool has_module = DbgGetModuleAt(addr, module_text);
	bool has_label = DbgGetLabelAt(addr, SEG_DEFAULT, label_text);
	bool has_string = DbgGetStringAt(addr, string);

	char asm_string[MAX_PATH] = { 0 };
	GuiGetDisassembly(addr, asm_string);

	if (has_string)
	{
		_snprintf_s(text, size, _TRUNCATE, "%p <%s> %s", (char*)addr, string, asm_string);
	}
	else if (has_module && has_label)
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
	_plugin_logprintf("%s rip=%p rax=%p rbx=%p rcx=%p rdx=%p rsi=%p rdi=%p rsp=%p rbp=%p r8=%p  r9=%p r10=%p r11=%p r12=%p r13=%p r14=%p r15=%p\n",
		inst_point_string,
		reg.cax, reg.cbx, reg.ccx,
		reg.cdx, reg.csi, reg.cdi,
		reg.cip, reg.csp, reg.cbp,
		reg.r8, reg.r9, reg.r10,
		reg.r11, reg.r12, reg.r13,
		reg.r14, reg.r15);
#else
	_plugin_logprintf("%s eip=%p eax=%p ebx=%p ecx=%p edx=%p esi=%p edi=%p esp=%p ebp=%p\n",
		inst_string,
		reg.cax, reg.cbx, reg.ccx,
		reg.cdx, reg.csi, reg.cdi,
		reg.cip, reg.csp, reg.cbp);
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


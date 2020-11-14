#include "trace_exec_logger.h"


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
	if (!regstep_enabled)
		return;

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));

	auto& reg = reg_dump.regcontext;
#ifdef _WIN64
	_plugin_logprintf("rax=%p rbx=%p rcx=%p rdx=%p rsi=%p rdi=%p rip=%p rsp=%p rbp=%p r8=%p  r9=%p r10=%p r11=%p r12=%p r13=%p r14=%p r15=%p\n",
		reg.cax, reg.cbx, reg.ccx,
		reg.cdx, reg.csi, reg.cdi,
		reg.cip, reg.csp, reg.cbp,
		reg.r8, reg.r9, reg.r10,
		reg.r11, reg.r12, reg.r13,
		reg.r14, reg.r15);
#else
	_plugin_logprintf("eax=%p ebx=%p ecx=%p edx=%p esi=%p edi=%p eip=%p esp=%p ebp=%p\n",
		reg.cax, reg.cbx, reg.ccx,
		reg.cdx, reg.csi, reg.cdi,
		reg.cip, reg.csp, reg.cbp);
#endif
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


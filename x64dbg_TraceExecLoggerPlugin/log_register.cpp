#include "log_instruction.h"


void log_register(LOG_REGISTER& reg_json, REGDUMP* reg_dump)
{
	if (!get_register_enabled())
	{
		reg_json.enabled = false;
		return;
	}

	REGISTERCONTEXT reg = reg_dump->regcontext;

	make_address_json(reg_json.cax, reg.cax);
	make_address_json(reg_json.cbx, reg.cbx);
	make_address_json(reg_json.ccx, reg.ccx);
	make_address_json(reg_json.cdx, reg.cdx);
	make_address_json(reg_json.csi, reg.csi);
	make_address_json(reg_json.cdi, reg.cdi);
	make_address_json(reg_json.csp, reg.csp);
	make_address_json(reg_json.cbp, reg.cbp);
#ifdef _WIN64
	make_address_json(reg_json.r8, reg.r8);
	make_address_json(reg_json.r9, reg.r9);
	make_address_json(reg_json.r10, reg.r10);
	make_address_json(reg_json.r11, reg.r11);
	make_address_json(reg_json.r12, reg.r12);
	make_address_json(reg_json.r13, reg.r13);
	make_address_json(reg_json.r14, reg.r14);
	make_address_json(reg_json.r15, reg.r15);
#endif

	FLAGS flags = reg_dump->flags;
	reg_json.flags_zf = flags.z;
	reg_json.flags_of = flags.o;
	reg_json.flags_cf = flags.c;
	reg_json.flags_pf = flags.p;
	reg_json.flags_sf = flags.s;
	reg_json.flags_tf = flags.t;
	reg_json.flags_af = flags.a;
	reg_json.flags_df = flags.d;
	reg_json.flags_if = flags.i;

	reg_json.error_name = reg_dump->lastError.name;
	reg_json.error_value = reg_dump->lastError.code;
	reg_json.status_name = reg_dump->lastStatus.name;
	reg_json.status_value = reg_dump->lastStatus.code;

	reg_json.enabled = true;
}


bool register_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Register Log: Help\n"
			"Command:\n"
			"    TElogger.reg.help\n"
			"    TElogger.reg.enable\n"
			"    TElogger.reg.disable");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_register_enabled(true);
		telogger_logputs("Register Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_register_enabled(false);
		telogger_logputs("Register Log: Disabled");
	}

	return true;
}


bool init_register_log(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.reg.help", register_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.reg.enable", register_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.reg.disable", register_command_callback, false);
	return true;
}


bool stop_register_log()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.reg.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.reg.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.reg.disable");
	return true;
}


void setup_register_log()
{
}

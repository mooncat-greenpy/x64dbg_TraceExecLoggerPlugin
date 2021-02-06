#include "log_instruction.h"


LOG_REGISTER log_register(REGDUMP* reg_dump)
{
	LOG_REGISTER reg_json = LOG_REGISTER();
	if (!get_register_enabled())
	{
		return reg_json;
	}

	reg_json.type = "register";

	REGISTERCONTEXT reg = reg_dump->regcontext;

	reg_json.cax = make_address_json(reg.cax);
	reg_json.cbx = make_address_json(reg.cbx);
	reg_json.ccx = make_address_json(reg.ccx);
	reg_json.cdx = make_address_json(reg.cdx);
	reg_json.csi = make_address_json(reg.csi);
	reg_json.cdi = make_address_json(reg.cdi);
	reg_json.csp = make_address_json(reg.csp);
	reg_json.cbp = make_address_json(reg.cbp);
#ifdef _WIN64
	reg_json.r8 = make_address_json(reg.r8);
	reg_json.r9 = make_address_json(reg.r9);
	reg_json.r10 = make_address_json(reg.r10);
	reg_json.r11 = make_address_json(reg.r11);
	reg_json.r12 = make_address_json(reg.r12);
	reg_json.r13 = make_address_json(reg.r13);
	reg_json.r14 = make_address_json(reg.r14);
	reg_json.r15 = make_address_json(reg.r15);
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

	return reg_json;
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

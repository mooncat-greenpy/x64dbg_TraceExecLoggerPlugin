#include "log_instruction.h"


json log_register()
{
	json reg_json = json::object();
	if (!get_register_enabled())
	{
		return reg_json;
	}

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

	FLAGS flags = reg_dump.flags;
	json flags_json = json::object();
	flags_json["zf"] = flags.z;
	flags_json["of"] = flags.o;
	flags_json["cf"] = flags.c;
	flags_json["pf"] = flags.p;
	flags_json["sf"] = flags.s;
	flags_json["tf"] = flags.t;
	flags_json["af"] = flags.a;
	flags_json["df"] = flags.d;
	flags_json["if"] = flags.i;
	reg_json["flags"] = flags_json;

	reg_json["error"] = json::object();
	reg_json["error"]["name"]=reg_dump.lastError.name;
	reg_json["error"]["value"] = reg_dump.lastError.code;
	reg_json["status"] = json::object();
	reg_json["status"]["name"]=reg_dump.lastStatus.name;
	reg_json["status"]["value"] = reg_dump.lastStatus.code;

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
		telogger_logputs("Register Help\n"
			"Command:\n"
			"    TElogger.reg.help\n"
			"    TElogger.reg.enable\n"
			"    TElogger.reg.disable\n");
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

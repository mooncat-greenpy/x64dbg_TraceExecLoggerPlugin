#include "trace_exec_logger.h"


char file_path[MAX_PATH] = { 0 };
json log_json = json::array();
static bool telogger_enabled = true;


void log_exec()
{
	if (!telogger_enabled)
	{
		return;
	}

	json entry = json::object();
	entry["inst"] = log_instruction();
	entry["reg"] = log_register();
	entry["stack"] = log_stack();
	log_json.push_back(entry);
}


bool command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.help\n"
			"    TElogger.enable\n"
			"    TElogger.disable\n"
			"    TElogger.inst.help\n"
			"    TElogger.reg.help\n"
			"    TElogger.stack.help\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		telogger_enabled = true;
		_plugin_menuentrysetchecked(pluginHandle, MENU_ENABLED, telogger_enabled);
		_plugin_logputs(PLUGIN_NAME ": Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		telogger_enabled = false;
		_plugin_menuentrysetchecked(pluginHandle, MENU_ENABLED, telogger_enabled);
		_plugin_logputs(PLUGIN_NAME ": Disabled");
	}

	return true;
}


extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY* info)
{
	switch (info->hEntry)
	{
	case MENU_ENABLED:
	{
		telogger_enabled = !telogger_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, "Enabled", telogger_enabled);
	}
	case MENU_HELP:
	{
		char help_text[] = "[ " PLUGIN_NAME " ]\n"
			"Collect logs when stepping and tracing.\n"
			"Command:\n"
			"    TElogger.help\n"
			"    TElogger.enable\n"
			"    TElogger.disable\n";
		MessageBoxA(NULL, help_text, PLUGIN_NAME, MB_OK);
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
	duint setting = telogger_enabled;
	BridgeSettingGetUint(PLUGIN_NAME, "Enabled", &setting);
	telogger_enabled = !!setting;

	_plugin_registercommand(pluginHandle, "TElogger.help", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.enable", command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.disable", command_callback, false);

	if (!instruction_log_plugin_init(init_struct))
	{
		return false;
	}
	if (!register_log_plugin_init(init_struct))
	{
		return false;
	}
	if (!stack_log_plugin_init(init_struct))
	{
		return false;
	}
	return true;
}


bool logger_plugin_stop()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.disable");

	if (!instruction_log_plugin_stop())
	{
		return false;
	}
	if (!register_log_plugin_stop())
	{
		return false;
	}
	if (!stack_log_plugin_stop())
	{
		return false;
	}
	return true;
}


void logger_plugin_setup()
{
	_plugin_menuaddentry(hMenu, MENU_ENABLED, "Enabled");
	_plugin_menuentrysetchecked(pluginHandle, MENU_ENABLED, telogger_enabled);
	_plugin_menuaddentry(hMenu, MENU_HELP, "Help");

	instruction_log_plugin_setup();
	register_log_plugin_setup();
	stack_log_plugin_setup();
}

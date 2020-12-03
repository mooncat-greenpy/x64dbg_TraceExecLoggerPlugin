#include "log_stack.h"

static int stack_log_count = 0x10;


json log_stack()
{
	json stack_json = json::object();
	if (!get_stack_enabled())
	{
		return stack_json;
	}

	stack_json["type"] = "stack";
	stack_json["data"] = json::array();

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	duint csp = reg_dump.regcontext.csp;

	for (int i = 0; i < stack_log_count; i++)
	{
		json tmp_json = json::object();
		duint stack_addr = csp + i * sizeof(duint);
		duint stack_value = 0;
		STACK_COMMENT comment = { 0 };
		if (!DbgMemIsValidReadPtr(stack_addr))
		{
			tmp_json["address"] = stack_addr;
			tmp_json["value"] = "";
			tmp_json["comment"] = "";
			stack_json["data"].push_back(tmp_json);
			continue;
		}
		DbgMemRead(stack_addr, &stack_value, sizeof(stack_value));

		tmp_json["address"] = stack_addr;
		tmp_json["value"] = make_address_json(stack_value);
		if (DbgStackCommentGet(stack_addr, &comment))
		{
			tmp_json["comment"] = comment.comment;
		}
		else
		{
			tmp_json["comment"] = "";
		}
		stack_json["data"].push_back(tmp_json);
	}

	return stack_json;
}


bool stack_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		_plugin_logputs("Command:\n"
			"    TElogger.stack.help\n"
			"    TElogger.stack.enable\n"
			"    TElogger.stack.disable\n"
			"    TElogger.stack.stacklogcount [num]\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_stack_enabled(true);
		_plugin_logputs("Stack Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_stack_enabled(false);
		_plugin_logputs("Stack Log: Disabled");
	}
	else if (strstr(argv[0], "stacklogcount"))
	{
		if (argc < 2)
		{
			telogger_logprintf("Stack Log Count: %d\n", stack_log_count);
			return true;
		}
		stack_log_count = atoi(argv[1]);
		telogger_logprintf("Stack Log Count: %d\n", stack_log_count);
	}

	return true;
}


bool init_stack_log(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.stack.help", stack_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.stack.enable", stack_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.stack.disable", stack_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.stack.stacklogcount", stack_command_callback, false);
	return true;
}


bool stop_stack_log()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.stacklogcount");
	return true;
}


void setup_stack_log()
{
}

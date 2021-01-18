#include "log_stack.h"

static int stack_log_count = 0x10;
static std::map<std::pair<duint, duint>, std::string> stack_comment_string_cache;
static std::list<std::pair<duint, duint>> stack_comment_string_fifo;


json log_stack()
{
	json stack_json = json::object();
	if (!get_stack_enabled())
	{
		return stack_json;
	}

	stack_json["type"] = "stack";
	stack_json["data"] = json::array();

	bool result = false;
	duint csp = DbgEval("csp", &result);
	if (!result) {
		return stack_json;
	}

	duint* stack_value = stack_value = new duint[stack_log_count];
	DbgMemRead(csp, stack_value, stack_log_count * sizeof(duint));

	for (int i = 0; i < stack_log_count; i++)
	{
		json tmp_json = json::object();
		duint stack_addr = csp + i * sizeof(duint);
		STACK_COMMENT comment = { 0 };
		if (!DbgMemIsValidReadPtr(stack_addr))
		{
			tmp_json["address"] = make_address_json(stack_addr);
			tmp_json["value"] = "";
			tmp_json["comment"] = "";
			stack_json["data"].push_back(tmp_json);
			continue;
		}

		tmp_json["address"] = make_address_json(stack_addr);
		tmp_json["value"] = make_address_json(stack_value[i]);

		bool cache_result = false;
		std::string stack_comment_cached = get_stack_comment_string_cache_data(std::make_pair(stack_addr, stack_value[i]), &cache_result);
		if (cache_result)
		{
			tmp_json["comment_cache"] = true;
			tmp_json["comment"] = stack_comment_cached.c_str();
		}
		else
		{
			tmp_json["comment_cache"] = false;
			if (DbgStackCommentGet(stack_addr, &comment))
			{
				tmp_json["comment"] = comment.comment;
			}
			else
			{
				tmp_json["comment"] = "";
			}
			set_stack_comment_string_cache_data(std::make_pair(stack_addr, stack_value[i]), std::string(tmp_json["comment"]));
		}
		stack_json["data"].push_back(tmp_json);
	}
	delete[] stack_value;

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
		telogger_logputs("Stack Help\n"
			"Command:\n"
			"    TElogger.stack.help\n"
			"    TElogger.stack.enable\n"
			"    TElogger.stack.disable\n"
			"    TElogger.stack.stacklogcount [num]\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_stack_enabled(true);
		telogger_logputs("Stack Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_stack_enabled(false);
		telogger_logputs("Stack Log: Disabled");
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

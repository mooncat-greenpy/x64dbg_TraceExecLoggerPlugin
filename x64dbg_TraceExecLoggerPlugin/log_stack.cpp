#include "log_stack.h"


void log_stack(LOG_STACK& stack_json, StepInfo& step_info)
{
	stack_json.data.clear();
	if (!get_stack_enabled())
	{
		stack_json.enabled = false;
		return;
	}

	REGDUMP* reg_dump = step_info.get_reg_dump();

	// Sometimes fails to read stack
	if (!DbgMemIsValidReadPtr(reg_dump->regcontext.csp))
	{
		telogger_logprintf("Stack Log: Invalid ptr %p\n", reg_dump->regcontext.csp);
		return;
	}

	duint log_count = get_stack_log_count();
	duint* stack_value = new duint[log_count];
	if (!DbgMemRead(reg_dump->regcontext.csp, stack_value, log_count * sizeof(duint)))
	{
		telogger_logprintf("Stack Log: Failed to read stack memory %p\n", reg_dump->regcontext.csp);
		stack_json.enabled = false;
		delete[] stack_value;
		return;
	}

	for (duint i = 0; i < log_count; i++)
	{
		LOG_STACK_ENTRY tmp_json = LOG_STACK_ENTRY();
		duint stack_addr = reg_dump->regcontext.csp + i * sizeof(duint);
		STACK_COMMENT comment = {};

		make_address_json(tmp_json.address, stack_addr);
		make_address_json(tmp_json.value, stack_value[i]);

		bool cache_result = false;
		std::string stack_comment_cached = get_stack_comment_string_cache_data(std::make_pair(stack_addr, stack_value[i]), &cache_result);
		if (cache_result)
		{
			tmp_json.comment_cache = true;
			tmp_json.comment = stack_comment_cached;
		}
		else
		{
			tmp_json.comment_cache = false;
			if (DbgStackCommentGet(stack_addr, &comment))
			{
				tmp_json.comment = comment.comment;
			}
			else
			{
				tmp_json.comment = "";
			}
			set_stack_comment_string_cache_data(std::make_pair(stack_addr, stack_value[i]), tmp_json.comment);
		}
		stack_json.data.push_back(tmp_json);
	}
	delete[] stack_value;

	stack_json.enabled = true;
}


bool stack_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (isCommand(argv[0], "TElogger.stack.help"))
	{
		telogger_logputs("Stack Log: Help\n"
			"Command:\n"
			"    TElogger.stack.help\n"
			"    TElogger.stack.enable\n"
			"    TElogger.stack.disable\n"
			"    TElogger.stack.stack.log.count [count]");
	}
	else if (isCommand(argv[0], "TElogger.stack.enable"))
	{
		set_stack_enabled(true);
		telogger_logputs("Stack Log: Enabled");
	}
	else if (isCommand(argv[0], "TElogger.stack.disable"))
	{
		set_stack_enabled(false);
		telogger_logputs("Stack Log: Disabled");
	}
	else if (isCommand(argv[0], "TElogger.stack.stack.log.count"))
	{
		if (argc < 2)
		{
			telogger_logprintf("Stack Log: Count %#x\n", get_stack_log_count());
			return true;
		}
		bool result_eval = false;
		duint value = DbgEval(argv[1], &result_eval);
		if (!result_eval)
		{
			telogger_logputs("Stack Log: Failed\n"
				"Command:\n"
				"    TElogger.stack.stack.log.count [count]");
			return false;
		}
		set_stack_log_count(value);
		telogger_logprintf("Stack Log: Count %#x\n", get_stack_log_count());
	}

	return true;
}


bool init_stack_log(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.stack.help", stack_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.stack.enable", stack_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.stack.disable", stack_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.stack.stack.log.count", stack_command_callback, false);
	return true;
}


bool stop_stack_log()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.disable");
	_plugin_unregistercommand(pluginHandle, "TElogger.stack.stack.log.count");
	return true;
}


void setup_stack_log()
{
}

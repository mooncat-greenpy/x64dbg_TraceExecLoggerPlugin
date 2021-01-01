#include "auto_run.h"

static int current_thread_number = 0;
static int skip_addr = 0;


void run_debug()
{
	if (!get_auto_run_enabled())
	{
		return;
	}
	duint cip = 0;
	bool result_eval = false;
	cip = DbgEval("cip", &result_eval);
	if (result_eval && cip == skip_addr)
	{
		skip_addr = 0;
		DbgCmdExec("run");
		return;
	}

	THREADLIST thread_list = { 0 };
	DbgGetThreadList(&thread_list);
	int thread_id = 0;
	for (int i = 0; i < thread_list.count; i++)
	{
		if (thread_id == 0 && thread_list.list[i].WaitReason != _Suspended)
		{
			thread_id = thread_list.list[i].BasicInfo.ThreadId;
		}

		if (i > current_thread_number && thread_list.list[i].WaitReason != _Suspended)
		{
			thread_id = thread_list.list[i].BasicInfo.ThreadId;
			current_thread_number = i;
			break;
		}
	}
	char cmd[DEFAULT_BUF_SIZE] = { 0 };
	_snprintf_s(cmd, sizeof(cmd), _TRUNCATE, "threadswitch %x", thread_id);
	DbgCmdExecDirect(cmd);
	DbgCmdExec("TraceIntoConditional 0, 50");
	telogger_logprintf("Auto Run: ThreadID = %x, Address = %p\n", thread_id, cip);
}


void skip_system_break_point(duint addr)
{
	skip_addr = addr;
}


bool auto_run_command_callback(int argc, char* argv[])
{
	if (argc < 1)
	{
		return false;
	}
	if (strstr(argv[0], "help"))
	{
		telogger_logputs("Auto Run Help\n"
			"Command:\n"
			"    TElogger.auto.help\n"
			"    TElogger.auto.enable\n"
			"    TElogger.auto.disable\n");
	}
	else if (strstr(argv[0], "enable"))
	{
		set_auto_run_enabled(true);
		telogger_logputs("Auto Run Log: Enabled");
	}
	else if (strstr(argv[0], "disable"))
	{
		set_auto_run_enabled(false);
		telogger_logputs("Auto Run Log: Disabled");
	}
}


bool init_auto_run(PLUG_INITSTRUCT* init_struct)
{
	_plugin_registercommand(pluginHandle, "TElogger.auto.help", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.enable", auto_run_command_callback, false);
	_plugin_registercommand(pluginHandle, "TElogger.auto.disable", auto_run_command_callback, false);
	return true;
}


bool stop_auto_run()
{
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.help");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.enable");
	_plugin_unregistercommand(pluginHandle, "TElogger.auto.disable");
	return true;
}


void setup_auto_run()
{
}

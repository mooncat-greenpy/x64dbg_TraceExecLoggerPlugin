#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

#include "log_instruction.h"
#include "log_register.h"
#include "log_stack.h"
#include "log_proc_info.h"

enum
{
	MENU_ENABLED,
	MENU_HELP,
	MENU_PROC_ENABLED,
};


bool logger_plugin_init(PLUG_INITSTRUCT* init_struct);
bool logger_plugin_stop();
void logger_plugin_setup();

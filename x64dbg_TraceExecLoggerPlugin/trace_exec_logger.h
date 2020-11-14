#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

enum
{
	MENU_ENABLED,
};

static bool regstep_enabled = true;


bool logger_plugin_init(PLUG_INITSTRUCT* init_struct);


bool logger_plugin_stop();


void logger_plugin_setup();


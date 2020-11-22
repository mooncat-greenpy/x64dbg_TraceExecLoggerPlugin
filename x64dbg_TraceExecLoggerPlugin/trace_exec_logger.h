#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

#define HEX_SIZE 0x30


enum
{
	MENU_ENABLED,
	MENU_HELP,
};


bool logger_plugin_init(PLUG_INITSTRUCT* init_struct);


bool logger_plugin_stop();


void logger_plugin_setup();


#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_stack();

bool stack_log_plugin_init(PLUG_INITSTRUCT* init_struct);
bool stack_log_plugin_stop();
void stack_log_plugin_setup();

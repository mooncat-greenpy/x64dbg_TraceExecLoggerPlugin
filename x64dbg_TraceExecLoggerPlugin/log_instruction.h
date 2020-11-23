#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_instruction();

bool instruction_log_plugin_init(PLUG_INITSTRUCT* init_struct);
bool instruction_log_plugin_stop();
void instruction_log_plugin_setup();

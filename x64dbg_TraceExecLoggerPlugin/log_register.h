#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_register();

bool register_log_plugin_init(PLUG_INITSTRUCT* init_struct);
bool register_log_plugin_stop();
void register_log_plugin_setup();

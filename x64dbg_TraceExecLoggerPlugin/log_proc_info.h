#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_thread();
json log_memory();
json log_module();

bool proc_info_log_plugin_init(PLUG_INITSTRUCT* init_struct);
bool proc_info_log_plugin_stop();
void proc_info_log_plugin_setup();

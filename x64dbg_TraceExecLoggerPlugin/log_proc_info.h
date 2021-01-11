#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_thread();
json log_memory();
json log_module();
json log_handle();

bool init_proc_info_log(PLUG_INITSTRUCT* init_struct);
bool stop_proc_info_log();
void setup_proc_info_log();

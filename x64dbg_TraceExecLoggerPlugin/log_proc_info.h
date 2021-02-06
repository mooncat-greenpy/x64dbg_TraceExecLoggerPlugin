#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


LOG_THREAD log_thread();
LOG_MEMORY log_memory();
LOG_MODULE log_module();
LOG_HANDLE log_handle();
LOG_NETWORK log_network();

bool init_proc_info_log(PLUG_INITSTRUCT* init_struct);
bool stop_proc_info_log();
void setup_proc_info_log();

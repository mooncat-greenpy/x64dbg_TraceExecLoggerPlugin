#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


void log_thread(LOG_THREAD& thread_json);
void log_memory(LOG_MEMORY& memory_json);
void log_module(LOG_MODULE& module_json);
void log_handle(LOG_HANDLE& handle_json);
void log_network(LOG_NETWORK& network_json);
void log_proc(PROC_LOG& proc_json);

bool init_proc_info_log(PLUG_INITSTRUCT* init_struct);
bool stop_proc_info_log();
void setup_proc_info_log();

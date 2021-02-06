#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"
#include <map>


const char* get_file_name();
void set_file_name(const char* name);
void add_log(int thread_id, LOG_CONTAINER* log);
void save_all_thread_log();
void delete_all_log();

bool init_save(PLUG_INITSTRUCT* init_struct);
bool stop_save();
void setup_save();

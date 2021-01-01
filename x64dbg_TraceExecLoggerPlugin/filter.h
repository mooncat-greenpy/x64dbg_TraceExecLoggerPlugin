#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


bool should_log();
void add_pass_module(const char* mod_name);
bool init_filter_log(PLUG_INITSTRUCT* init_struct);
bool stop_filter_log();
void setup_filter_log();

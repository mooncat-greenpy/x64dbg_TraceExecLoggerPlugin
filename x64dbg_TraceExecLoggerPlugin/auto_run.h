#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


void run_debug();
void skip_system_break_point(duint addr);
bool init_auto_run(PLUG_INITSTRUCT* init_struct);
bool stop_auto_run();
void setup_auto_run();

#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_register();

bool init_register_log(PLUG_INITSTRUCT* init_struct);
bool stop_register_log();
void setup_register_log();

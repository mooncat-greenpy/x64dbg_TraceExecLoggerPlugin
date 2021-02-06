#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


LOG_REGISTER log_register(REGDUMP* reg_dump);

bool init_register_log(PLUG_INITSTRUCT* init_struct);
bool stop_register_log();
void setup_register_log();

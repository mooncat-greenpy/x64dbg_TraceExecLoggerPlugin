#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


json log_stack(REGDUMP* reg_dump);

bool init_stack_log(PLUG_INITSTRUCT* init_struct);
bool stop_stack_log();
void setup_stack_log();

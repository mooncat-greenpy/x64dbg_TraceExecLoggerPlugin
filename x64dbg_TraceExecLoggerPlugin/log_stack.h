#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


void log_stack(LOG_STACK& stack_json, REGDUMP* reg_dump);

bool init_stack_log(PLUG_INITSTRUCT* init_struct);
bool stop_stack_log();
void setup_stack_log();

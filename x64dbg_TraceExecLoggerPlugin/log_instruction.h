#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"


void log_instruction(LOG_INSTRUCTION& inst_json, REGDUMP* reg_dump);

bool init_instruction_log(PLUG_INITSTRUCT* init_struct);
bool stop_instruction_log();
void setup_instruction_log();

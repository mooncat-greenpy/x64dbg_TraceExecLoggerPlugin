#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

class StepInfo;

void log_instruction(LOG_INSTRUCTION& inst_json, StepInfo& step_info);

bool init_instruction_log(PLUG_INITSTRUCT* init_struct);
bool stop_instruction_log();
void setup_instruction_log();

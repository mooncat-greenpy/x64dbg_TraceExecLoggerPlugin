#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

class StepInfo;

void log_register(LOG_REGISTER& reg_json, StepInfo& step_info);

bool init_register_log(PLUG_INITSTRUCT* init_struct);
bool stop_register_log();
void setup_register_log();

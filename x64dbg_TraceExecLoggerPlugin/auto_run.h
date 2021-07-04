#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

enum class AUTO_RUN_TYPE
{
	AUTO_STOP,
	AUTO_RUN,
	AUTO_STEP_INTO,
	AUTO_STEP_OVER,
	AUTO_MANUAL,
	AUTO_MANUAL_HARDWARE,
};

struct BP_INFO
{
	duint addr = 0;
	std::string comment;
	std::string command;
};


void run_debug(StepInfo& step_info);
void skip_system_break_point(duint addr);
bool init_auto_run(PLUG_INITSTRUCT* init_struct);
bool stop_auto_run();
void setup_auto_run();

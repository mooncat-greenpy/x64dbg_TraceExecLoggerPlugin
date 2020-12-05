#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

#include "log_instruction.h"
#include "log_register.h"
#include "log_stack.h"
#include "log_proc_info.h"
#include "save.h"
#include "filter.h"

#define DEFAULT_BUF_SIZE 0x500
#define MAX_LOG_COUNT 50000

void log_proc_info();

bool init_logger_plugin(PLUG_INITSTRUCT* init_struct);
bool stop_logger_plugin();
void setup_logger_plugin();

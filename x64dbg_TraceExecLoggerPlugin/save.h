#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"
#include <map>

typedef struct _THREAD_LOG_STATE
{
    char file_name[MAX_PATH] = { 0 };
    json log = json::array();
    int count = 0;
}THREAD_LOG_STATE;


void create_thread_log(int thread_id, const char* file_name);
void save_log(int thread_id);
void add_log(int thread_id, json* log);

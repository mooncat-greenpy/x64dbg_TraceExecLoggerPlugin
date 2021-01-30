#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"
#include <map>

typedef struct _THREAD_LOG_STATE
{
    char file_name[MAX_PATH] = { 0 };
    char cmd_line[MAX_STRING_SIZE] = { 0 };
    json log = json::array();
    int count = 0;
    int save_count = 0;
    int process_id = 0;
    int thread_id = 0;
}THREAD_LOG_STATE;


const char* get_file_name();
void set_file_name(const char* name);
void add_log(int thread_id, json* log);
void save_all_thread_log();
void delete_all_log();

bool init_save(PLUG_INITSTRUCT* init_struct);
bool stop_save();
void setup_save();

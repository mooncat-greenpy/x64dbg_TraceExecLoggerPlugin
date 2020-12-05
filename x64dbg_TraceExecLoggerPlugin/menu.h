#pragma once

#include "trace_exec_logger.h"


#define MENU_LABEL_LOG_ENABLED "Log Enabled"
#define MENU_LABEL_INSTRUCTION_ENABLED "Instruction Enabled"
#define MENU_LABEL_REGISTER_ENABLED "Register Enabled"
#define MENU_LABEL_STACK_ENABLED "Stack Enabled"
#define MENU_LABEL_PROC_LOG_ENABLED "Proc Log Enabled"
#define MENU_LABEL_MODULE_ENABLED "Module Enabled"
#define MENU_LABEL_THREAD_ENABLED "Thread Enabled"
#define MENU_LABEL_MEMORY_ENABLED "Memory Enabled"
#define MENU_LABEL_SAVE_DIR "Save Dir"

enum
{
	MENU_HELP,
	MENU_ENABLED,
	MENU_INSTRUCTION_ENABLED,
	MENU_REGISTER_ENABLED,
	MENU_STACK_ENABLED,
	MENU_PROC_ENABLED,
	MENU_PROC_MODULE_ENABLED,
	MENU_PROC_THREAD_ENABLED,
	MENU_PROC_MEMORY_ENABLED,
};

bool get_telogger_enabled();
void set_telogger_enabled(bool value);
bool get_instruction_enabled();
void set_instruction_enabled(bool value);
bool get_register_enabled();
void set_register_enabled(bool value);
bool get_stack_enabled();
void set_stack_enabled(bool value);
bool get_proc_enabled();
void set_proc_enabled(bool value);
bool get_module_enabled();
void set_module_enabled(bool value);
bool get_thread_enabled();
void set_thread_enabled(bool value);
bool get_memory_enabled();
void set_memory_enabled(bool value);
const char* get_save_dir();
void set_save_dir(const char* dir_name);

void menu_callback(PLUG_CB_MENUENTRY* info);

void init_menu();
void setup_menu();

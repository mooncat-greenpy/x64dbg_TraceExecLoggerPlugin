#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

#define MAX_HEX_SIZE 0x100
#define PLUGIN_NAME_LOG_HEADER "[" PLUGIN_NAME "] "
#define telogger_logprintf(format, ...) _plugin_logprintf(PLUGIN_NAME_LOG_HEADER format, __VA_ARGS__)
#define telogger_logputs(text) _plugin_logputs(PLUGIN_NAME_LOG_HEADER text)
#define logprintf(format, ...) _plugin_logprintf(format, __VA_ARGS__)
#define logputs(text) _plugin_logputs(text)

class StepInfo
{
public:
	bool trace_execute_flag;
	bool stepped_flag;
	bool breakpoint_flag;
	bool pause_debug_flag;

	bool is_disasm_instr_initialized;
	DISASM_INSTR disasm_instr;
	bool is_reg_dump_initialized;
	REGDUMP reg_dump;

	StepInfo();
	void reset();
	int count_flags();
	void init_trace_execute();
	void init_stepped();
	void init_breakpoint_flag();
	void init_pause_debug_flag();
	DISASM_INSTR* get_disasm_instr();
	REGDUMP* get_reg_dump();
};

void make_address_label_string(duint addr, char* text, size_t text_size);
void make_hex_string(char* data, size_t data_size, char* text, size_t text_size);
void make_address_json(LOG_ADDRESS& address_json, duint addr);
bool isCommand(const char* command, const char* str);

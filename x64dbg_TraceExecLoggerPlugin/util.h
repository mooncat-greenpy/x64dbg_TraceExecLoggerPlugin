#pragma once

#include "x64dbg_TraceExecLoggerPlugin.h"

#define HEX_SIZE 0x30


void save_json_file(const char* file_name, const char* buffer);
void make_address_label_string(duint addr, char* text, size_t text_size);
void make_hex_string(char* data, size_t data_size, char* text, size_t text_size);
json make_address_json(duint addr);

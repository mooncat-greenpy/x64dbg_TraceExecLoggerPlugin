#include "util.h"


constexpr char hex_database[][4] = {
	"00 ", "01 ", "02 ", "03 ", "04 ", "05 ", "06 ", "07 ", "08 ", "09 ", "0a ", "0b ", "0c ", "0d ", "0e ", "0f ",
	"10 ", "11 ", "12 ", "13 ", "14 ", "15 ", "16 ", "17 ", "18 ", "19 ", "1a ", "1b ", "1c ", "1d ", "1e ", "1f ",
	"20 ", "21 ", "22 ", "23 ", "24 ", "25 ", "26 ", "27 ", "28 ", "29 ", "2a ", "2b ", "2c ", "2d ", "2e ", "2f ",
	"30 ", "31 ", "32 ", "33 ", "34 ", "35 ", "36 ", "37 ", "38 ", "39 ", "3a ", "3b ", "3c ", "3d ", "3e ", "3f ",
	"40 ", "41 ", "42 ", "43 ", "44 ", "45 ", "46 ", "47 ", "48 ", "49 ", "4a ", "4b ", "4c ", "4d ", "4e ", "4f ",
	"50 ", "51 ", "52 ", "53 ", "54 ", "55 ", "56 ", "57 ", "58 ", "59 ", "5a ", "5b ", "5c ", "5d ", "5e ", "5f ",
	"60 ", "61 ", "62 ", "63 ", "64 ", "65 ", "66 ", "67 ", "68 ", "69 ", "6a ", "6b ", "6c ", "6d ", "6e ", "6f ",
	"70 ", "71 ", "72 ", "73 ", "74 ", "75 ", "76 ", "77 ", "78 ", "79 ", "7a ", "7b ", "7c ", "7d ", "7e ", "7f ",
	"80 ", "81 ", "82 ", "83 ", "84 ", "85 ", "86 ", "87 ", "88 ", "89 ", "8a ", "8b ", "8c ", "8d ", "8e ", "8f ",
	"90 ", "91 ", "92 ", "93 ", "94 ", "95 ", "96 ", "97 ", "98 ", "99 ", "9a ", "9b ", "9c ", "9d ", "9e ", "9f ",
	"a0 ", "a1 ", "a2 ", "a3 ", "a4 ", "a5 ", "a6 ", "a7 ", "a8 ", "a9 ", "aa ", "ab ", "ac ", "ad ", "ae ", "af ",
	"b0 ", "b1 ", "b2 ", "b3 ", "b4 ", "b5 ", "b6 ", "b7 ", "b8 ", "b9 ", "ba ", "bb ", "bc ", "bd ", "be ", "bf ",
	"c0 ", "c1 ", "c2 ", "c3 ", "c4 ", "c5 ", "c6 ", "c7 ", "c8 ", "c9 ", "ca ", "cb ", "cc ", "cd ", "ce ", "cf ",
	"d0 ", "d1 ", "d2 ", "d3 ", "d4 ", "d5 ", "d6 ", "d7 ", "d8 ", "d9 ", "da ", "db ", "dc ", "dd ", "de ", "df ",
	"e0 ", "e1 ", "e2 ", "e3 ", "e4 ", "e5 ", "e6 ", "e7 ", "e8 ", "e9 ", "ea ", "eb ", "ec ", "ed ", "ee ", "ef ",
	"f0 ", "f1 ", "f2 ", "f3 ", "f4 ", "f5 ", "f6 ", "f7 ", "f8 ", "f9 ", "fa ", "fb ", "fc ", "fd ", "fe ", "ff "
};


StepInfo::StepInfo()
{
	reset();
}
void StepInfo::reset()
{
	trace_execute_flag = false;
	stepped_flag = false;
	breakpoint_flag = false;
	pause_debug_flag = false;

	is_disasm_instr_initialized = false;
	disasm_instr = {};
	is_reg_dump_initialized = false;
	reg_dump = {};
}
int StepInfo::count_flags()
{
	int count = 0;
	if (trace_execute_flag)
	{
		count++;
	}
	if (stepped_flag)
	{
		count++;
	}
	if (breakpoint_flag)
	{
		count++;
	}
	if (pause_debug_flag)
	{
		count++;
	}
	return count;
}
void StepInfo::init_trace_execute()
{
	if (trace_execute_flag || count_flags() >= 2 || pause_debug_flag)
	{
		reset();
	}
	trace_execute_flag = true;
}
void StepInfo::init_stepped()
{
	if (stepped_flag || count_flags() >= 2)
	{
		reset();
	}
	stepped_flag = true;
}
void StepInfo::init_breakpoint_flag()
{
	if (breakpoint_flag || count_flags() >= 2)
	{
		reset();
	}
	breakpoint_flag = true;
}
void StepInfo::init_pause_debug_flag()
{
	if (pause_debug_flag || count_flags() >= 2 || trace_execute_flag)
	{
		reset();
	}
	pause_debug_flag = true;
}
DISASM_INSTR* StepInfo::get_disasm_instr()
{
	if (!is_disasm_instr_initialized)
	{
		REGDUMP* tmp_reg_dump = get_reg_dump();
		DbgDisasmAt(tmp_reg_dump->regcontext.cip, &disasm_instr);
		is_disasm_instr_initialized = true;
	}
	return &disasm_instr;
}
REGDUMP* StepInfo::get_reg_dump()
{
	if (!is_reg_dump_initialized)
	{
		DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
		is_reg_dump_initialized = true;
	}
	return &reg_dump;
}


void make_address_label_string(duint addr, char* text, size_t text_size)
{
	if (!text)
	{
		return;
	}

	char module_text[MAX_MODULE_SIZE] = { 0 };
	char label_text[MAX_LABEL_SIZE] = { 0 };
	bool has_module = DbgGetModuleAt(addr, module_text);
	bool has_label = DbgGetLabelAt(addr, SEG_DEFAULT, label_text);
	if (has_module && has_label && strlen(module_text))
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%s.%s", module_text, label_text);
	}
	else if (module_text && strlen(module_text))
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%s.%p", module_text, (char*)addr);
	}
	else if (has_label)
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%p<%s>", (char*)addr, label_text);
	}
	else
	{
		_snprintf_s(text, text_size, _TRUNCATE, "%p", (char*)addr);
	}
}


void make_hex_string(char* data, size_t data_size, char* text, size_t text_size)
{
	if (!data || !text)
	{
		return;
	}

	for (size_t i = 0; i < data_size; i++)
	{
		strncat_s(text, text_size, hex_database[(unsigned char)data[i]], _TRUNCATE);
	}
}


void make_address_json_recursive(LOG_ADDRESS& address_json, duint addr, duint recursive_count = 0)
{
	address_json.xref.clear();
	bool cache_result = false;
	LOG_ADDRESS cache_data = get_address_json_cache_data(addr, &cache_result);
	if (cache_result)
	{
		address_json = cache_data;
		address_json.cache = true;
		return;
	}

	address_json.cache = false;
	address_json.address = addr;

	char label_text[MAX_LABEL_SIZE] = { 0 };
	make_address_label_string(addr, label_text, sizeof(label_text));
	address_json.label = label_text;

	char text[DEFAULT_BUF_SIZE] = { 0 };
	char string_text[MAX_STRING_SIZE] = { 0 };
	char hex_string[DEFAULT_BUF_SIZE] = { 0 };
	bool has_string = DbgGetStringAt(addr, string_text);
	bool has_hex = false;
	bool address_json_cache_enabled = true;
	if (DbgMemIsValidReadPtr(addr))
	{
		char data[MAX_HEX_SIZE] = { 0 };
		duint data_size = get_hex_log_size() < sizeof(data) ? get_hex_log_size() : sizeof(data);
		has_hex = DbgMemRead(addr, data, data_size);
		make_hex_string(data, data_size, hex_string, sizeof(hex_string));
		// Does not support cache
		if (recursive_count > 0)
		{
			duint* child_addresses = (duint*)data;
			for (duint i = 0; i < (data_size / sizeof(duint)); i++)
			{
				LOG_ADDRESS child_address_json;
				make_address_json_recursive(child_address_json, *(child_addresses + i), recursive_count - 1);
				address_json.child.push_back(child_address_json);
			}
		}
	}
	if (has_string)
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "%s", string_text);
	}
	else if (has_hex)
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "%s", hex_string);
	}
	else
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "");
		address_json_cache_enabled = false;
	}
	address_json.data = text;

	XREF_INFO xref_info = {};
	if (DbgXrefGet(addr, &xref_info))
	{
		for (duint i = 0; i < xref_info.refcount; i++)
		{
			LOG_XREF xref_json = LOG_XREF();

			xref_json.address = xref_info.references[i].addr;
			label_text[0] = '\0';
			make_address_label_string(xref_info.references[i].addr, label_text, sizeof(label_text));
			xref_json.label = label_text;

			switch (xref_info.references[i].type)
			{
			case XREF_DATA:
				xref_json.type = "data";
				break;
			case XREF_JMP:
				xref_json.type = "jmp";
				break;
			case XREF_CALL:
				xref_json.type = "call";
				break;
			default:
				xref_json.type = "none";
			}
			address_json.xref.push_back(xref_json);
		}
	}

	if (address_json_cache_enabled)
	{
		set_address_json_cache_data(addr, address_json);
	}
}

void make_address_json(LOG_ADDRESS& address_json, duint addr)
{
	make_address_json_recursive(address_json, addr, get_address_recursive_count());
}


bool isCommand(const char* command, const char* str)
{
	return !_strnicmp(command, str, strlen(str));
}

#include "util.h"


void save_json_file(const char* file_name, SYSTEMTIME* system_time, int number, const char* buffer)
{
	if (!file_name || !buffer)
	{
		return;
	}

	HANDLE log_file_handle = INVALID_HANDLE_VALUE;
	char log_file_name[MAX_PATH] = { 0 };
	_snprintf_s(log_file_name, sizeof(log_file_name), _TRUNCATE, "%s_%d-%d-%d-%d-%d-%d_%d.json", PathFindFileNameA(file_name), system_time->wYear, system_time->wMonth, system_time->wDay, system_time->wHour, system_time->wMinute, system_time->wSecond, number);
	log_file_handle = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (log_file_handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD written = 0;
	WriteFile(log_file_handle, buffer, strlen(buffer), &written, NULL);

	CloseHandle(log_file_handle);
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
		char tmp[10] = { 0 };
		_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02x ", (unsigned char)data[i]);
		strncat_s(text, text_size, tmp, _TRUNCATE);
	}
}


json make_address_json(duint addr)
{
	json address_json = json::object();
	address_json["value"] = addr;

	char label_text[MAX_LABEL_SIZE] = { 0 };
	make_address_label_string(addr, label_text, sizeof(label_text));
	address_json["label"] = label_text;

	char text[DEFAULT_BUF_SIZE] = { 0 };
	char string[MAX_STRING_SIZE] = { 0 };
	char hex_string[DEFAULT_BUF_SIZE] = { 0 };
	bool has_string = DbgGetStringAt(addr, string);
	bool has_data = false;
	if (DbgMemIsValidReadPtr(addr))
	{
		char data[HEX_SIZE] = { 0 };
		DbgMemRead(addr, data, sizeof(data));
		make_hex_string(data, sizeof(data), hex_string, sizeof(hex_string));
		has_data = true;
	}
	if (has_string)
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "%s", string);
	}
	else if (has_data)
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "%s", hex_string);
	}
	else
	{
		_snprintf_s(text, sizeof(text), _TRUNCATE, "");
	}
	address_json["data"] = text;

	address_json["xref"] = json::array();
	XREF_INFO xref_info = { 0 };
	if (DbgXrefGet(addr, &xref_info))
	{
		for (int i = 0; i < xref_info.refcount; i++)
		{
			json xref_json = json::object();

			xref_json["address"] = json::object();
			xref_json["address"]["value"] = xref_info.references[i].addr;
			ZeroMemory(label_text, sizeof(label_text));
			make_address_label_string(xref_info.references[i].addr, label_text, sizeof(label_text));
			xref_json["address"]["label"] = label_text;

			switch (xref_info.references[i].type)
			{
			case XREF_DATA:
				xref_json["type"] = "data";
				break;
			case XREF_JMP:
				xref_json["type"] = "jmp";
				break;
			case XREF_CALL:
				xref_json["type"] = "call";
				break;
			default:
				xref_json["type"] = "none";
			}
			address_json["xref"].push_back(xref_json);
		}
	}

	return address_json;
}

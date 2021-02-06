#include "x64dbg_TraceExecLoggerPlugin.h"


void replace_all(std::string& str, const std::string& old_str, const std::string& new_str)
{
    size_t target = str.find(old_str);
    while (target != std::string::npos)
    {
        str.replace(target, 1, new_str);
        target = str.find(old_str, target + 2);
    }
}

void escape(std::string& str)
{
    replace_all(str, "\\", "\\\\");
    replace_all(str, "\"", "\\\"");
    replace_all(str, "\n", "\\n");
    replace_all(str, "\t", "\\t");
    replace_all(str, "/", "\\/");
    replace_all(str, "\b", "\\b");
    replace_all(str, "\f", "\\f");
    replace_all(str, "\r", "\\r");
    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str.at(i);
        if ((c < 0x20 || 0x7e < c) && c != 0x08 && c != 0x09 && c != 0x0a && c != 0x0c && c != 0x0d)
        {
            char uni[0x10] = { 0 };
            _snprintf_s(uni, sizeof(uni), _TRUNCATE, "\\u%04x", c);
            str.replace(i, 1, uni);
            i = 0;
        }
    }
}

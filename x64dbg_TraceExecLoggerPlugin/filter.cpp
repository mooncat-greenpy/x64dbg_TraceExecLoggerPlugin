#include "filter.h"

std::vector<std::string> pass_mod;
std::vector<IP_RANGE> pass_ip_range;


bool should_log(duint addr)
{
    if (pass_mod.size() == 0 && pass_ip_range.size() == 0)
    {
        return true;
    }

    for (auto i : pass_ip_range)
    {
        if (i.start <= addr && addr < i.end)
        {
            return true;
        }
    }

    if (pass_mod.size() == 0)
    {
        return false;
    }

    char mod_name[MAX_MODULE_SIZE] = { 0 };
    if (!DbgGetModuleAt(addr, mod_name))
    {
        telogger_logputs("Log Filter: Failed to get module name");
        return true;
    }
    char mod_name_lower[MAX_MODULE_SIZE] = { 0 };
    strncpy_s(mod_name_lower, sizeof(mod_name_lower), mod_name, _TRUNCATE);
    _strlwr_s(mod_name_lower, sizeof(mod_name_lower));
    if (std::find(pass_mod.begin(), pass_mod.end(), mod_name_lower) != pass_mod.end())
    {
        return true;
    }
    return false;
}


void add_pass_module(const char* mod_name)
{
    if (mod_name == NULL)
    {
        return;
    }
    char mod_name_lower[MAX_MODULE_SIZE] = { 0 };
    strncpy_s(mod_name_lower, sizeof(mod_name_lower), mod_name, _TRUNCATE);
    _strlwr_s(mod_name_lower, sizeof(mod_name_lower));
    PathRemoveExtensionA(mod_name_lower);
    pass_mod.push_back(mod_name_lower);
}


bool remove_pass_module(const char* mod_name)
{
    if (mod_name == NULL)
    {
        return false;
    }
    char mod_name_lower[MAX_MODULE_SIZE] = { 0 };
    strncpy_s(mod_name_lower, sizeof(mod_name_lower), mod_name, _TRUNCATE);
    _strlwr_s(mod_name_lower, sizeof(mod_name_lower));
    std::vector<std::string>::iterator itr = std::find(pass_mod.begin(), pass_mod.end(), mod_name_lower);
    if (itr == pass_mod.end())
    {
        return false;
    }
    pass_mod.erase(itr);
    return true;
}


void add_pass_ip_range(duint start, duint end, const char* comment)
{
    pass_ip_range.push_back({ start, end, comment });
}


bool remove_pass_ip_range(size_t index)
{
    if (index >= pass_ip_range.size())
    {
        return false;
    }
    pass_ip_range.erase(pass_ip_range.begin() + index);
    return true;
}


bool filter_command_callback(int argc, char* argv[])
{
    if (argc < 1)
    {
        return false;
    }
    if (strstr(argv[0], "help"))
    {
        telogger_logputs("Log Filter: Help\n"
            "Command:\n"
            "    TElogger.filt.help\n"
            "    TElogger.filt.mod.pass [dllname]\n"
            "    TElogger.filt.ip.range.pass [start], [end], [comment]\n"
            "    TElogger.filt.ip.range.pass [remove index]");
    }
    else if (strstr(argv[0], "mod.pass"))
    {
        if (argc < 2)
        {
            telogger_logputs("Log Filter: Pass module list");
            logputs("{");
            for (auto i : pass_mod)
            {
                logprintf("    %s,\n", i.c_str());
            }
            logputs("}");
            return true;
        }
        if (!remove_pass_module(argv[1]))
        {
            add_pass_module(argv[1]);
        }
    }
    else if (strstr(argv[0], "ip.range.pass"))
    {
        if (argc < 2)
        {
            telogger_logputs("Log Filter: IP range list");
            logputs("{");
            for (size_t i = 0; i < pass_ip_range.size(); i++)
            {
                logprintf("    %x: %p-%p    %s,\n", i, pass_ip_range.at(i).start, pass_ip_range.at(i).end, pass_ip_range.at(i).comment.c_str());
            }
            logputs("}");
            return true;
        }
        else if (argc < 3)
        {
            char* str_end = NULL;
            unsigned int index = (unsigned int)_strtoui64(argv[1], &str_end, 16);
            if (str_end == NULL || *str_end != '\0')
            {
                telogger_logprintf("Log Filter: Remove IP range filter\n"
                    "Command:\n"
                    "    TElogger.filt.ip.range.pass index");
                return false;
            }
            remove_pass_ip_range(index);
        }
        else
        {
            char* str1_end = NULL;
            duint start = (duint)_strtoi64(argv[1], &str1_end, 16);
            char* str2_end = NULL;
            duint end = (duint)_strtoi64(argv[2], &str2_end, 16);
            if (str1_end == NULL || *str1_end != '\0' || str2_end == NULL || *str2_end != '\0')
            {
                telogger_logprintf("Log Filter: Add IP range filter\n"
                    "Command:\n"
                    "    TElogger.filt.ip.range.pass start, end, [comment]");
                return false;
            }
            if (argc > 3)
            {
                add_pass_ip_range(start, end, argv[3]);
            }
            else
            {
                add_pass_ip_range(start, end, "");
            }
        }
    }

    return true;
}


bool init_filter_log(PLUG_INITSTRUCT* init_struct)
{
    _plugin_registercommand(pluginHandle, "TElogger.filt.help", filter_command_callback, false);
    _plugin_registercommand(pluginHandle, "TElogger.filt.mod.pass", filter_command_callback, false);
    _plugin_registercommand(pluginHandle, "TElogger.filt.ip.range.pass", filter_command_callback, false);
    return true;
}


bool stop_filter_log()
{
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.help");
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.mod.pass");
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.ip.range.pass");
    return true;
}

void setup_filter_log()
{
}

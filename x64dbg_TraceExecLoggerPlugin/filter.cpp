#include "filter.h"

std::vector<std::string> pass_mod;
std::vector<IP_RANGE> pass_ip_range;
std::vector<std::string> contain_asm;


bool should_log(duint addr)
{
    if (pass_mod.size() == 0 && pass_ip_range.size() == 0 && contain_asm.size() == 0)
    {
        return true;
    }

    for (auto& i : pass_ip_range)
    {
        if (i.start <= addr && addr < i.end)
        {
            return true;
        }
    }

    if (pass_mod.size() != 0)
    {
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
    }

    if (contain_asm.size() != 0)
    {
        char asm_string[GUI_MAX_DISASSEMBLY_SIZE] = { 0 };
        bool cache_result = false;
        std::string gui_asm_cached = get_gui_asm_string_cache_data(addr, &cache_result);
        if (cache_result)
        {
            strncpy_s(asm_string, sizeof(asm_string), gui_asm_cached.c_str(), _TRUNCATE);
        }
        else
        {
            if (!GuiGetDisassembly(addr, asm_string))
            {
                strncpy_s(asm_string, sizeof(asm_string), "error", _TRUNCATE);
            }
            set_gui_asm_string_cache_data(addr, asm_string);
        }
        for (auto& i : contain_asm)
        {
            if (strstr(asm_string, i.c_str()))
            {
                return true;
            }
        }
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


void add_contain_asm(const char* assembly)
{
    contain_asm.push_back(assembly);
}


bool remove_contain_asm(const char* assembly)
{
    if (assembly == NULL)
    {
        return false;
    }

    std::vector<std::string>::iterator itr = std::find(contain_asm.begin(), contain_asm.end(), assembly);
    if (itr == contain_asm.end())
    {
        return false;
    }
    contain_asm.erase(itr);
    return true;
}


bool filter_command_callback(int argc, char* argv[])
{
    if (argc < 1)
    {
        return false;
    }
    if (isCommand(argv[0], "TElogger.filt.help"))
    {
        telogger_logputs("Log Filter: Help\n"
            "Command:\n"
            "    TElogger.filt.help\n"
            "    TElogger.filt.mod.pass [dllname]\n"
            "    TElogger.filt.ip.range.pass [remove index]\n"
            "    TElogger.filt.ip.range.pass start, end, [comment]\n"
            "    TElogger.filt.asm.contain [asm]");
    }
    else if (isCommand(argv[0], "TElogger.filt.mod.pass"))
    {
        if (argc < 2)
        {
            telogger_logputs("Log Filter: Pass module list");
            logputs("{");
            for (auto& i : pass_mod)
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
    else if (isCommand(argv[0], "TElogger.filt.ip.range.pass"))
    {
        if (argc < 2)
        {
            telogger_logputs("Log Filter: IP range list");
            logputs("{");
            for (size_t i = 0; i < pass_ip_range.size(); i++)
            {
                logprintf("    %#x: %p-%p    %s,\n", i, pass_ip_range.at(i).start, pass_ip_range.at(i).end, pass_ip_range.at(i).comment.c_str());
            }
            logputs("}");
            return true;
        }
        else if (argc < 3)
        {
            bool result_eval = false;
            duint index = DbgEval(argv[1], &result_eval);
            if (!result_eval)
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
            bool result_eval1 = false;
            duint start = DbgEval(argv[1], &result_eval1);
            bool result_eval2 = false;
            duint end = DbgEval(argv[2], &result_eval2);
            if (!result_eval1 || !result_eval2)
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
    else if (isCommand(argv[0], "TElogger.filt.asm.contain"))
    {
        if (argc < 2)
        {
            telogger_logputs("Log Filter: Asm list");
            logputs("{");
            for (auto& i : contain_asm)
            {
                logprintf("    %s,\n", i.c_str());
            }
            logputs("}");
            return true;
        }
        if (!remove_contain_asm(argv[1]))
        {
            add_contain_asm(argv[1]);
        }
    }

    return true;
}


bool init_filter_log(PLUG_INITSTRUCT* init_struct)
{
    _plugin_registercommand(pluginHandle, "TElogger.filt.help", filter_command_callback, false);
    _plugin_registercommand(pluginHandle, "TElogger.filt.mod.pass", filter_command_callback, false);
    _plugin_registercommand(pluginHandle, "TElogger.filt.ip.range.pass", filter_command_callback, false);
    _plugin_registercommand(pluginHandle, "TElogger.filt.asm.contain", filter_command_callback, false);
    return true;
}


bool stop_filter_log()
{
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.help");
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.mod.pass");
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.ip.range.pass");
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.asm.contain");
    return true;
}

void setup_filter_log()
{
}

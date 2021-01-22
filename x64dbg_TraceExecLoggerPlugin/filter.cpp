#include "filter.h"

std::vector<std::string> pass_mod;


bool should_log(duint addr)
{
    if (pass_mod.size() == 0)
    {
        return true;
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
    if (std::find(pass_mod.begin(), pass_mod.end(), mod_name_lower) == pass_mod.end())
    {
        return false;
    }
    return true;
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
            "    TElogger.filt.mod.pass dllname");
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

    return true;
}


bool init_filter_log(PLUG_INITSTRUCT* init_struct)
{
    _plugin_registercommand(pluginHandle, "TElogger.filt.help", filter_command_callback, false);
    _plugin_registercommand(pluginHandle, "TElogger.filt.mod.pass", filter_command_callback, false);
    return true;
}


bool stop_filter_log()
{
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.help");
    _plugin_unregistercommand(pluginHandle, "TElogger.filt.mod.pass");
    return true;
}

void setup_filter_log()
{
}

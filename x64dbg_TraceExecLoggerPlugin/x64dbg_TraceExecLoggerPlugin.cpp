#include "x64dbg_TraceExecLoggerPlugin.h"
#include "trace_exec_logger.h"


#define X64DBG_TRACEEXECLOGGER_PLUGIN_INFO "TraceExecLoggerPlugin Usage: TElogger.help\n"


// GLOBAL Plugin SDK variables
int pluginHandle;
HWND hwndDlg;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}


DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = PLUGIN_VERSION;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, PLUGIN_NAME);
    pluginHandle = initStruct->pluginHandle;

    if (!init_logger_plugin(initStruct))
    {
        return false;
    }
    return true;
}


DLL_EXPORT bool plugstop()
{
    _plugin_menuclear(hMenu);

    if (!stop_logger_plugin())
    {
        return false;
    }
    return true;
}


DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    hwndDlg = setupStruct->hwndDlg;
    hMenu = setupStruct->hMenu;
    hMenuDisasm = setupStruct->hMenuDisasm;
    hMenuDump = setupStruct->hMenuDump;
    hMenuStack = setupStruct->hMenuStack;
    
	GuiAddLogMessage, X64DBG_TRACEEXECLOGGER_PLUGIN_INFO;
	
    setup_logger_plugin();
}


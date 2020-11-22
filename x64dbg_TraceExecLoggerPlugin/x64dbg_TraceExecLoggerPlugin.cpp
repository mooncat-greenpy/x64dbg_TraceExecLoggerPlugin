#include "x64dbg_TraceExecLoggerPlugin.h"
#include "trace_exec_logger.h"


// Variables
#define szx64dbg_TraceExecLoggerPluginInfo "TraceExecLoggerPlugin Usage:\n" 



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
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, plugin_name);
    pluginHandle = initStruct->pluginHandle;

    if (!logger_plugin_init(initStruct))
    {
        return false;
    }
    return true;
}


DLL_EXPORT bool plugstop()
{
    _plugin_menuclear(hMenu);

    if (!logger_plugin_stop())
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
    
	GuiAddLogMessage, szx64dbg_TraceExecLoggerPluginInfo;
	
    logger_plugin_setup();
}


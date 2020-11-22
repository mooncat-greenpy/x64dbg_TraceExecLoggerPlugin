#ifndef _PLUGINMAIN_H
#define _PLUGINMAIN_H

// Include files required for plugin SDK
#include "pluginsdk\TitanEngine\TitanEngine.h"
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <Shlwapi.h>
#include "pluginsdk\_plugins.h"
#include "pluginsdk/bridgemain.h"
#include "../include/nlohmann/json.hpp"

#pragma comment(lib, "Shlwapi.lib")

// Plugin SDK required variables
#define plugin_name "TraceExecLogger" // rename to your plugins name
#define plugin_version 1


#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif //DLL_EXPORT

// Superglobal variables
extern int pluginHandle;
extern HWND hwndDlg;
extern int hMenu;
extern int hMenuDisasm;
extern int hMenuDump;
extern int hMenuStack;


#ifdef __cplusplus
extern "C"
{
#endif

// Default plugin exports - required
DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct);
DLL_EXPORT bool plugstop();
DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct);

#ifdef __cplusplus
}
#endif

#endif //_PLUGINMAIN_H

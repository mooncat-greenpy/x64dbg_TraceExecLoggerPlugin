#include "menu.h"

static int proc_info_handle = 0;

static bool telogger_enabled = true;
static bool instruction_enabled = true;
static bool register_enabled = true;
static bool stack_enabled = true;
static bool proc_enabled = true;
static bool module_enabled = true;
static bool thread_enabled = true;
static bool memory_enabled = true;
static bool handle_enabled = true;
static bool network_enabled = true;
static char save_dir[MAX_SETTING_SIZE] = { 0 };
static bool auto_run_enabled = false;

bool get_telogger_enabled()
{
	return telogger_enabled;
}
void set_telogger_enabled(bool value)
{
	telogger_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_ENABLED, telogger_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_LOG_ENABLED, telogger_enabled);
}
bool get_instruction_enabled()
{
	return instruction_enabled;
}
void set_instruction_enabled(bool value)
{
	instruction_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_INSTRUCTION_ENABLED, instruction_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_INSTRUCTION_ENABLED, instruction_enabled);
}
bool get_register_enabled()
{
	return register_enabled;
}
void set_register_enabled(bool value)
{
	register_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_REGISTER_ENABLED, register_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_REGISTER_ENABLED, register_enabled);
}
bool get_stack_enabled()
{
	return stack_enabled;
}
void set_stack_enabled(bool value)
{
	stack_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_STACK_ENABLED, stack_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_STACK_ENABLED, stack_enabled);
}
bool get_proc_enabled()
{
	return proc_enabled;
}
void set_proc_enabled(bool value)
{
	proc_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_ENABLED, proc_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_PROC_LOG_ENABLED, proc_enabled);
}
bool get_module_enabled()
{
	return module_enabled;
}
void set_module_enabled(bool value)
{
	module_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_MODULE_ENABLED, module_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_MODULE_ENABLED, module_enabled);
}
bool get_thread_enabled()
{
	return thread_enabled;
}
void set_thread_enabled(bool value)
{
	thread_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_THREAD_ENABLED, thread_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_THREAD_ENABLED, thread_enabled);
}
bool get_memory_enabled()
{
	return memory_enabled;
}
void set_memory_enabled(bool value)
{
	memory_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_MEMORY_ENABLED, memory_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_MEMORY_ENABLED, memory_enabled);
}
bool get_handle_enabled()
{
	return handle_enabled;
}
void set_handle_enabled(bool value)
{
	handle_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_HANDLE_ENABLED, handle_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_HANDLE_ENABLED, handle_enabled);
}
bool get_network_enabled()
{
	return network_enabled;
}
void set_network_enabled(bool value)
{
	network_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_NETWORK_ENABLED, network_enabled);
	BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_NETWORK_ENABLED, network_enabled);
}
const char* get_save_dir()
{
	return save_dir;
}
void set_save_dir(const char* dir_name)
{
	if (dir_name == NULL)
	{
		return;
	}
	if (!CreateDirectoryA(dir_name, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		return;
	}
	strncpy_s(save_dir, sizeof(save_dir), dir_name, _TRUNCATE);
	BridgeSettingSet(PLUGIN_NAME, MENU_LABEL_SAVE_DIR, save_dir);
}
bool get_auto_run_enabled()
{
	return auto_run_enabled;
}
void set_auto_run_enabled(bool value)
{
	auto_run_enabled = value;
	_plugin_menuentrysetchecked(pluginHandle, MENU_AUTO_RUN_ENABLED, auto_run_enabled);
}


void menu_callback(PLUG_CB_MENUENTRY* info)
{
	switch (info->hEntry)
	{
	case MENU_ENABLED:
		telogger_enabled = !telogger_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_LOG_ENABLED, telogger_enabled);
		break;
	case MENU_INSTRUCTION_ENABLED:
		instruction_enabled = !instruction_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_INSTRUCTION_ENABLED, instruction_enabled);
		break;
	case MENU_REGISTER_ENABLED:
		register_enabled = !register_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_REGISTER_ENABLED, register_enabled);
		break;
	case MENU_STACK_ENABLED:
		stack_enabled = !stack_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_STACK_ENABLED, stack_enabled);
		break;
	case MENU_PROC_ENABLED:
		proc_enabled = !proc_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_PROC_LOG_ENABLED, proc_enabled);
		break;
	case MENU_PROC_MODULE_ENABLED:
		module_enabled = !module_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_MODULE_ENABLED, module_enabled);
		break;
	case MENU_PROC_THREAD_ENABLED:
		thread_enabled = !thread_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_THREAD_ENABLED, thread_enabled);
		break;
	case MENU_PROC_MEMORY_ENABLED:
		memory_enabled = !memory_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_MEMORY_ENABLED, memory_enabled);
		break;
	case MENU_PROC_HANDLE_ENABLED:
		handle_enabled = !handle_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_HANDLE_ENABLED, handle_enabled);
		break;
	case MENU_PROC_NETWORK_ENABLED:
		network_enabled = !network_enabled;
		BridgeSettingSetUint(PLUGIN_NAME, MENU_LABEL_NETWORK_ENABLED, network_enabled);
		break;
	case MENU_AUTO_RUN_ENABLED:
		_plugin_menuentrysetchecked(pluginHandle, MENU_AUTO_RUN_ENABLED, auto_run_enabled);
		break;
	case MENU_HELP:
	default:
		char help_text[] = "[ " PLUGIN_NAME " ]\n"
			"Collect logs when stepping and tracing.\n"
			"Command:\n"
			"    TElogger.help\n"
			"    TElogger.enable\n"
			"    TElogger.disable\n";
		MessageBoxA(NULL, help_text, PLUGIN_NAME, MB_OK);
		break;
	}
}


void init_menu()
{
	duint setting = telogger_enabled;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_LOG_ENABLED, &setting);
	telogger_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_INSTRUCTION_ENABLED, &setting);
	instruction_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_REGISTER_ENABLED, &setting);
	register_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_STACK_ENABLED, &setting);
	stack_enabled = !!setting;

	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_PROC_LOG_ENABLED, &setting);
	proc_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_MODULE_ENABLED, &setting);
	module_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_THREAD_ENABLED, &setting);
	thread_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_MEMORY_ENABLED, &setting);
	memory_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_HANDLE_ENABLED, &setting);
	handle_enabled = !!setting;
	BridgeSettingGetUint(PLUGIN_NAME, MENU_LABEL_NETWORK_ENABLED, &setting);
	network_enabled = !!setting;

	BridgeSettingGet(PLUGIN_NAME, MENU_LABEL_SAVE_DIR, save_dir);
	if (strlen(save_dir) == 0)
	{
		set_save_dir("TElogger");
	}
}


void stop_menu()
{
}


void setup_menu()
{
	_plugin_menuaddentry(hMenu, MENU_HELP, "Help");

	_plugin_menuaddentry(hMenu, MENU_ENABLED, MENU_LABEL_LOG_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_ENABLED, telogger_enabled);
	_plugin_menuaddentry(hMenu, MENU_INSTRUCTION_ENABLED, MENU_LABEL_INSTRUCTION_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_INSTRUCTION_ENABLED, instruction_enabled);
	_plugin_menuaddentry(hMenu, MENU_REGISTER_ENABLED, MENU_LABEL_REGISTER_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_REGISTER_ENABLED, register_enabled);
	_plugin_menuaddentry(hMenu, MENU_STACK_ENABLED, MENU_LABEL_STACK_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_STACK_ENABLED, stack_enabled);
	_plugin_menuaddentry(hMenu, MENU_PROC_ENABLED, MENU_LABEL_PROC_LOG_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_ENABLED, proc_enabled);

	proc_info_handle = _plugin_menuadd(hMenu, "Proc Info");
	_plugin_menuaddentry(proc_info_handle, MENU_PROC_MODULE_ENABLED, MENU_LABEL_MODULE_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_MODULE_ENABLED, module_enabled);
	_plugin_menuaddentry(proc_info_handle, MENU_PROC_THREAD_ENABLED, MENU_LABEL_THREAD_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_THREAD_ENABLED, thread_enabled);
	_plugin_menuaddentry(proc_info_handle, MENU_PROC_MEMORY_ENABLED, MENU_LABEL_MEMORY_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_MEMORY_ENABLED, memory_enabled);
	_plugin_menuaddentry(proc_info_handle, MENU_PROC_HANDLE_ENABLED, MENU_LABEL_HANDLE_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_HANDLE_ENABLED, handle_enabled);
	_plugin_menuaddentry(proc_info_handle, MENU_PROC_NETWORK_ENABLED, MENU_LABEL_NETWORK_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_PROC_NETWORK_ENABLED, network_enabled);

	_plugin_menuaddentry(hMenu, MENU_AUTO_RUN_ENABLED, MENU_LABEL_AUTO_RUN_ENABLED);
	_plugin_menuentrysetchecked(pluginHandle, MENU_AUTO_RUN_ENABLED, auto_run_enabled);
}

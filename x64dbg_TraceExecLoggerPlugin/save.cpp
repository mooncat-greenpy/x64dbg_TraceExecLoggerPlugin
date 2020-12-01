#include "save.h"


static std::map<int, THREAD_LOG_STATE> log_state;


void create_thread_log(int thread_id, const char* file_name)
{
    if (log_state.find(thread_id) != log_state.end())
    {
        log_state[thread_id].log.clear();
        log_state[thread_id].count = 0;
        return;
    }
    else
    {
        THREAD_LOG_STATE thread_log_state = { 0 };
        thread_log_state.log = json::array();
        log_state[thread_id] = thread_log_state;
    }

    SYSTEMTIME system_time = { 0 };
    GetLocalTime(&system_time);
    if (file_name) {
        _snprintf_s(log_state[thread_id].file_name, MAX_PATH, _TRUNCATE, "%s_%d-%d-%d-%d-%d-%d_%x",
            PathFindFileNameA(file_name),
            system_time.wYear, system_time.wMonth, system_time.wDay, system_time.wHour, system_time.wMinute, system_time.wSecond,
            thread_id);
    }
    else
    {
        strncpy_s(log_state[thread_id].file_name, MAX_PATH, "x64dbg_tmp", _TRUNCATE);
    }

    _plugin_logprintf("Create Log: thread id = %x, name = %s", thread_id, log_state[thread_id].file_name);
}


void save_log(int thread_id)
{
    _plugin_logprintf("Save Log: thread id = %x, name = %s", thread_id, log_state[thread_id].file_name);
    int number = log_state[thread_id].count / MAX_LOG_COUNT;

    HANDLE log_file_handle = INVALID_HANDLE_VALUE;
    char log_file_name[MAX_PATH] = { 0 };
    _snprintf_s(log_file_name, sizeof(log_file_name), _TRUNCATE, "%s_%d.json", log_state[thread_id].file_name, number);
    log_file_handle = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (log_file_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD written = 0;
    WriteFile(log_file_handle, log_state[thread_id].log.dump().c_str(), strlen(log_state[thread_id].log.dump().c_str()), &written, NULL);

    CloseHandle(log_file_handle);
}


void add_log(int thread_id, json* log)
{
    if (log == NULL || log_state.find(thread_id) == log_state.end())
    {
        return;
    }

    log_state[thread_id].log.push_back(*log);
    log_state[thread_id].count++;

    if (log_state[thread_id].count % MAX_LOG_COUNT == 0)
    {
        save_log(thread_id);
    }
    _plugin_logprintf("Add Log: thread id = %x, name = %s", thread_id, log_state[thread_id].file_name);
}

#include "save.h"


static std::map<int, THREAD_LOG_STATE> log_state;
static char log_dir_name[MAX_PATH] = { 0 };
static char file_name[MAX_PATH] = { 0 };


const char* get_file_name()
{
    return file_name;
}
void set_file_name(const char* name)
{
    if (name == NULL)
    {
        return;
    }
    strncpy_s(file_name, sizeof(file_name), name, _TRUNCATE);
}


void create_thread_log(int thread_id)
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

    if (strlen(get_file_name())) {
        _snprintf_s(log_state[thread_id].file_name, MAX_PATH, _TRUNCATE, "%s", PathFindFileNameA(get_file_name()));
    }
    else
    {
        strncpy_s(log_state[thread_id].file_name, MAX_PATH, "x64dbgtmp", _TRUNCATE);
    }

    size_t cmd_line_size = MAX_STRING_SIZE;
    if (!DbgFunctions()->GetCmdline(log_state[thread_id].cmd_line, &cmd_line_size))
    {
        strncpy_s(log_state[thread_id].cmd_line, MAX_STRING_SIZE, "error", _TRUNCATE);
    }

    log_state[thread_id].process_id = DbgGetProcessId();
    log_state[thread_id].thread_id = thread_id;

    telogger_logprintf("Create Log: Thread ID = %#x, Name = %s\n", thread_id, log_state[thread_id].file_name);
}


void save_log(int thread_id)
{
    THREAD_LOG_STATE thread_log = log_state[thread_id];
    if (thread_log.log.size() < 1)
    {
        return;
    }

    int count = thread_log.count;
    int number = (count - 1) / MAX_LOG_COUNT;

    json save_info = json::object();
    save_info["process_id"] = thread_log.process_id;
    save_info["thread_id"] = thread_log.thread_id;
    save_info["file_name"] = thread_log.file_name;
    save_info["cmd_line"] = thread_log.cmd_line;
    save_info["count"] = thread_log.log.size();
    save_info["log"] = thread_log.log;

    HANDLE log_file_handle = INVALID_HANDLE_VALUE;
    char log_file_name[MAX_PATH] = { 0 };
    _snprintf_s(log_file_name, sizeof(log_file_name), _TRUNCATE, "%s\\%s_%#x_%#x_%d.json", get_save_dir(), thread_log.file_name, thread_log.process_id, thread_log.thread_id, number);
    log_file_handle = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (log_file_handle == INVALID_HANDLE_VALUE)
    {
        telogger_logprintf("Save Log: Error = %#x\n", GetLastError());
        return;
    }

    DWORD written = 0;
    WriteFile(log_file_handle, save_info.dump().c_str(), strlen(save_info.dump().c_str()), &written, NULL);

    CloseHandle(log_file_handle);
    telogger_logprintf("Save Log: Thread ID = %#x, Name = %s\n", thread_id, thread_log.file_name);

    log_state.erase(thread_id);
    create_thread_log(thread_id);
    log_state[thread_id].count = count;
}


void add_log(int thread_id, json* log)
{
    if (log == NULL)
    {
        return;
    }

    if (log_state.find(thread_id) == log_state.end())
    {
        create_thread_log(thread_id);
    }

    log_state[thread_id].log.push_back(*log);
    log_state[thread_id].count++;

    if (log_state[thread_id].count % MAX_LOG_COUNT == 0)
    {
        save_log(thread_id);
    }
}


void save_all_thread_log()
{
    for (auto i : log_state)
    {
        save_log(i.first);
    }
}


void delete_all_log() {
    for (auto i : log_state)
    {
        log_state.erase(i.first);
    }
}

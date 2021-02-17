#include "save.h"


static std::map<int, THREAD_LOG_STATE> log_state;
static char file_name[MAX_PATH] = { 0 };
static CRITICAL_SECTION save_critical = { 0 };
static unsigned long long log_counter = 0;


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

unsigned long long get_log_counter()
{
    return log_counter;
}
void set_log_counter(unsigned long long value)
{
    log_counter = value;
}


void create_thread_log(int thread_id)
{
    if (log_state.find(thread_id) != log_state.end())
    {
        log_state[thread_id].log.clear();
        log_state.erase(thread_id);
    }

    THREAD_LOG_STATE thread_log_state = THREAD_LOG_STATE();
    log_state[thread_id] = thread_log_state;

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
    const char* dir_name = get_save_dir();
    if (!CreateDirectoryA(dir_name, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        telogger_logputs("Save Log: Failed to create save dir");
        return;
    }

    THREAD_LOG_STATE* thread_log = &log_state[thread_id];
    if (thread_log->log.size() < 1)
    {
        return;
    }

    int count = thread_log->count;
    int save_count = thread_log->save_count;

    HANDLE log_file_handle = INVALID_HANDLE_VALUE;
    char log_file_name[MAX_PATH] = { 0 };
    _snprintf_s(log_file_name, sizeof(log_file_name), _TRUNCATE, "%s\\%s_%#x_%#x_%d.json", dir_name, thread_log->file_name, thread_log->process_id, thread_log->thread_id, save_count);
    log_file_handle = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (log_file_handle == INVALID_HANDLE_VALUE)
    {
        telogger_logprintf("Save Log: Error = %#x\n", GetLastError());
        return;
    }

    thread_log->save_write("", log_file_handle);

    CloseHandle(log_file_handle);
    telogger_logprintf("Save Log: Thread ID = %#x, Name = %s\n", thread_id, log_file_name);

    create_thread_log(thread_id);
    log_state[thread_id].save_count = save_count + 1;
}


void add_log(int thread_id, LOG_CONTAINER* log)
{
    if (log == NULL)
    {
        return;
    }

    EnterCriticalSection(&save_critical);
    if (log_state.find(thread_id) == log_state.end())
    {
        create_thread_log(thread_id);
    }

    log->counter = log_counter++;
    log_state[thread_id].log.push_back(*log);
    log_state[thread_id].count++;

    if (log_state[thread_id].count % (MAX_LOG_COUNT - 1) == 0)
    {
        LOG_CONTAINER proc_entry = { 0 };
        proc_entry.is_proc_log = true;
        proc_entry.proc = PROC_LOG();
        proc_entry.counter = log_counter++;

        log_proc(proc_entry.proc);
        proc_entry.proc.message = "Save Log";
        log_state[thread_id].log.push_back(proc_entry);
        log_state[thread_id].count++;

        save_log(thread_id);
    }
    LeaveCriticalSection(&save_critical);
}


void save_all_thread_log()
{
    EnterCriticalSection(&save_critical);
    std::vector<int> thread_id_list;
    for (auto& i : log_state)
    {
        thread_id_list.push_back(i.first);
    }
    for (auto& i : thread_id_list)
    {
        save_log(i);
    }
    LeaveCriticalSection(&save_critical);
}


void delete_all_log()
{
    EnterCriticalSection(&save_critical);
    log_state.clear();
    LeaveCriticalSection(&save_critical);
}


bool init_save(PLUG_INITSTRUCT* init_struct)
{
    InitializeCriticalSection(&save_critical);

    return true;
}


bool stop_save()
{
    DeleteCriticalSection(&save_critical);

    return true;
}


void setup_save()
{
}

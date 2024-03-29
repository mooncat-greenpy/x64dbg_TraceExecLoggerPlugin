#pragma once

extern bool get_compact_log_enabled();

void escape(std::string& str);

class Log
{
public:
    std::string get_one_indent()
    {
        return get_compact_log_enabled() ? "" : "    ";
    }
    std::string get_end_line()
    {
        return get_compact_log_enabled() ? "" : "\n";
    }

    void add(std::string indent, const std::string& key, std::string value, std::string& write, bool last = false)
    {
        escape(value);
        write += indent + "\"" + key + "\": \"" + value + "\"" + (last ? "" : ",") + get_end_line();
    }

    void add(std::string indent, const std::string& key, const char* value, std::string& write, bool last = false)
    {
        add(indent, key, std::string(value), write, last);
    }

    void add(std::string indent, const std::string& key, bool value, std::string& write, bool last = false)
    {
        write += indent + "\"" + key + "\": " + (value ? "true" : "false") + (last ? "" : ",") + get_end_line();
    }

    template<typename T>
    void add(std::string indent, const std::string& key, const T& value, std::string& write, bool last = false)
    {
        write += indent + "\"" + key + "\": " + std::to_string(value) + (last ? "" : ",") + get_end_line();
    }

    template<typename T>
    void add_list(std::string indent, const std::string& key, std::list<T>& value, std::string& write, bool last = false)
    {
        write += indent + "\"" + key + "\": [" + get_end_line();
        for (auto& i : value)
        {
            write += indent + get_one_indent() + "{" + get_end_line();
            i.save_internal(indent + get_one_indent() + get_one_indent(), write);
            write += indent + get_one_indent() + "}," + get_end_line();
        }
        if (value.size() > 0)
        {
            size_t last_comma = write.rfind(",");
            if (last_comma != std::string::npos && last_comma > write.size() - 3)
            {
                write.replace(last_comma, 1, "");
            }
        }
        write += indent + "]" + (last ? "" : ",") + get_end_line();
    }

    virtual void save_internal(std::string indent, std::string& write) {}

    void save(std::string indent, const std::string& key, std::string& write, bool last = false)
    {
        write += indent + "\"" + key + "\": {" + get_end_line();
        save_internal(indent + get_one_indent(), write);
        write += indent + "}" + (last ? "" : ",") + get_end_line();
    }
};


class LOG_XREF : public Log
{
public:
    std::string type;
    duint address = 0;
    std::string label;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "type", type, write);
        add(indent, "address", address, write);
        add(indent, "label", label, write, true);
    }
};

class LOG_ADDRESS : public Log
{
public:
    bool cache = false;
    duint address = 0;
    std::string label;
    std::string data;
    std::list<LOG_ADDRESS> child;
    std::list<LOG_XREF> xref;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "cache", cache, write);
        add(indent, "address", address, write);
        add(indent, "label", label, write);
        add(indent, "data", data, write);
        add_list(indent, "child", child, write);
        add_list(indent, "xref", xref, write, true);
    }
};


class LOG_CALL_ARUGMENT : public Log
{
public:
    std::string name;
    LOG_ADDRESS data;

    LOG_CALL_ARUGMENT() : name(""), data(LOG_ADDRESS())
    {
    }
    LOG_CALL_ARUGMENT(const std::string& name, const LOG_ADDRESS& data) : name(name), data(data)
    {
    }

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "name", name, write);
        data.save(indent,"data", write, true);
    }
};

class LOG_ARGUMENT : public Log
{
public:
    std::string type;
    std::string segment;
    std::string mnemonic;
    duint constant = 0;
    LOG_ADDRESS value;
    LOG_ADDRESS memvalue;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "type", type, write);
        add(indent, "segment", segment, write);
        add(indent, "mnemonic", mnemonic, write);
        add(indent, "constant", constant, write);
        value.save(indent,"value", write);
        memvalue.save(indent,"memvalue", write, true);
    }
};

class LOG_ASSEMBLY : public Log
{
public:
    std::string type;
    std::string instruction;
    int size = 0;
    int argcount = 0;
    std::list<LOG_ARGUMENT> arg;

    void save_internal(std::string indent, std::string& write)
        {
            add(indent, "type", type, write);
            add(indent, "instruction", instruction, write);
            add(indent, "size", size, write);
            add(indent, "argcount", argcount, write);
            add_list(indent, "arg", arg, write, true);
        }
};


class LOG_INSTRUCTION : public Log
{
public:
    bool enabled = false;
    LOG_ADDRESS address;
    bool asm_str_cache = false;
    std::string asm_str;
    LOG_ASSEMBLY assembly;
    bool comment_cache = false;
    std::string comment;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        address.save(indent,"address", write);
        add(indent, "asm_str_cache", asm_str_cache, write);
        add(indent, "asm_str", asm_str, write);
        assembly.save(indent,"asm", write);
        add(indent, "comment_cache", comment_cache, write);
        add(indent, "comment", comment, write, true);
    }
};


class LOG_REGISTER : public Log
{
public:
    bool enabled = false;
    LOG_ADDRESS cax;
    LOG_ADDRESS cbx;
    LOG_ADDRESS ccx;
    LOG_ADDRESS cdx;
    LOG_ADDRESS csi;
    LOG_ADDRESS cdi;
    LOG_ADDRESS csp;
    LOG_ADDRESS cbp;
    LOG_ADDRESS r8;
    LOG_ADDRESS r9;
    LOG_ADDRESS r10;
    LOG_ADDRESS r11;
    LOG_ADDRESS r12;
    LOG_ADDRESS r13;
    LOG_ADDRESS r14;
    LOG_ADDRESS r15;
    bool flags_zf = false;
    bool flags_of = false;
    bool flags_cf = false;
    bool flags_pf = false;
    bool flags_sf = false;
    bool flags_tf = false;
    bool flags_af = false;
    bool flags_df = false;
    bool flags_if = false;
    std::string error_name;
    DWORD error_value = 0;
    std::string status_name;
    DWORD status_value = 0;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        cax.save(indent, "cax", write);
        cbx.save(indent, "cbx", write);
        ccx.save(indent, "ccx", write);
        cdx.save(indent, "cdx", write);
        csi.save(indent, "csi", write);
        cdi.save(indent, "cdi", write);
        csp.save(indent, "csp", write);
        cbp.save(indent, "cbp", write);
#ifdef _WIN64
        r8.save(indent, "r8", write);
        r9.save(indent, "r9", write);
        r10.save(indent, "r10", write);
        r11.save(indent, "r11", write);
        r12.save(indent, "r12", write);
        r13.save(indent, "r13", write);
        r14.save(indent, "r14", write);
        r15.save(indent, "r15", write);
#endif

        write += indent + "\"flags\": {" + get_end_line();
        add(indent + get_one_indent(), "zf", flags_zf, write);
        add(indent + get_one_indent(), "of", flags_of, write);
        add(indent + get_one_indent(), "cf", flags_cf, write);
        add(indent + get_one_indent(), "pf", flags_pf, write);
        add(indent + get_one_indent(), "sf", flags_sf, write);
        add(indent + get_one_indent(), "tf", flags_tf, write);
        add(indent + get_one_indent(), "af", flags_af, write);
        add(indent + get_one_indent(), "df", flags_df, write);
        add(indent + get_one_indent(), "if", flags_if, write, true);
        write += indent + "}," + get_end_line();

        write += indent + "\"error\": {" + get_end_line();
        add(indent + get_one_indent(), "name", error_name, write);
        add(indent + get_one_indent(), "value", error_value, write, true);
        write += indent + "}," + get_end_line();
        write += indent + "\"status\": {" + get_end_line();
        add(indent + get_one_indent(), "name", status_name, write);
        add(indent + get_one_indent(), "value", status_value, write, true);
        write += indent + "}" + get_end_line();
    }
};


class LOG_STACK_ENTRY : public Log
{
public:
    LOG_ADDRESS address;
    LOG_ADDRESS value;
    bool comment_cache = false;
    std::string comment;

    void save_internal(std::string indent, std::string& write)
    {
        address.save(indent,"address", write);
        value.save(indent,"value", write);
        add(indent, "comment_cache", comment_cache, write);
        add(indent, "comment", comment, write, true);
    }
};

class LOG_STACK : public Log
{
public:
    bool enabled = false;
    std::list<LOG_STACK_ENTRY> data;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        add_list(indent, "data", data, write, true);
    }
};


class LOG : public Log
{
public:
    std::string type;
    std::string message;
    LOG_INSTRUCTION inst;
    LOG_REGISTER reg;
    LOG_STACK stack;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "type", type, write);
        add(indent, "message", message, write);
        inst.save(indent,"inst", write);
        reg.save(indent, "reg", write);
        stack.save(indent, "stack", write, true);
    }
};


class LOG_MODULE_ENTRY : public Log
{
public:
    std::string name;
    std::string path;
    LOG_ADDRESS entry_point;
    LOG_ADDRESS base_address;
    duint size = 0;
    int section_count = 0;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "name", name, write);
        add(indent, "path", path, write);
        entry_point.save(indent, "entry_point", write);
        base_address.save(indent, "base_address", write);
        add(indent, "size", size, write);
        add(indent, "section_count", section_count, write, true);
    }
};

class LOG_MODULE : public Log
{
public:
    bool enabled = false;
    int count = 0;
    std::list<LOG_MODULE_ENTRY> list;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        add(indent, "count", count, write);
        add_list(indent, "list", list, write, true);
    }
};


class LOG_THREAD_ENTRY : public Log
{
public:
    DWORD id = 0;
    HANDLE handle = INVALID_HANDLE_VALUE;
    std::string name;
    LOG_ADDRESS start_address;
    LOG_ADDRESS local_base;
    LOG_ADDRESS cip;
    DWORD suspend_count = 0;
    std::string priority;
    std::string wait_reason;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "id", id, write);
        add(indent, "handle", (duint)handle, write);
        add(indent, "name", name, write);
        start_address.save(indent, "start_address", write);
        local_base.save(indent, "local_base", write);
        cip.save(indent, "cip", write);
        add(indent, "suspend_count", suspend_count, write);
        add(indent, "priority", priority, write);
        add(indent, "wait_reason", wait_reason, write, true);
    }
};

class LOG_THREAD : public Log
{
public:
    bool enabled = false;
    int current_thread = 0;
    int count = 0;
    std::list<LOG_THREAD_ENTRY> list;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        add(indent, "current_thread", current_thread, write);
        add(indent, "count", count, write);
        add_list(indent, "list", list, write, true);
    }
};


class LOG_MEMORY_ENTRY : public Log
{
public:
    std::string info;
    LOG_ADDRESS base_address;
    SIZE_T region_size = 0;
    std::string protect;
    std::string type;
    LOG_ADDRESS allocation_base;
    std::string allocation_protect;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "info", info, write);
        base_address.save(indent, "base_address", write);
        add(indent, "region_size", region_size, write);
        add(indent, "protect", protect, write);
        add(indent, "type", type, write);
        allocation_base.save(indent, "allocation_base", write);
        add(indent, "allocation_protect", allocation_protect, write, true);
    }
};

class LOG_MEMORY : public Log
{
public:
    bool enabled = false;
    int count = 0;
    std::list<LOG_MEMORY_ENTRY> list;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        add(indent, "count", count, write);
        add_list(indent, "list", list, write, true);
    }
};


class LOG_HANDLE_ENTRY : public Log
{
public:
    duint value = 0;
    std::string name;
    std::string type;
    unsigned int granted_access = 0;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "value", value, write);
        add(indent, "name", name, write);
        add(indent, "type", type, write);
        add(indent, "granted_access", granted_access, write, true);
    }
};

class LOG_HANDLE : public Log
{
public:
    bool enabled = false;
    int count = 0;
    std::list<LOG_HANDLE_ENTRY> list;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        add(indent, "count", count, write);
        add_list(indent, "list", list, write, true);
    }
};


class LOG_NETWORK_ENTRY : public Log
{
public:
    std::string remote_address;
    unsigned short remote_port = 0;
    std::string local_address;
    unsigned short local_port = 0;
    std::string state_text;
    unsigned int state_value = 0;

    void save_internal(std::string indent, std::string& write)
    {
        write += indent + "\"remote\": {" + get_end_line();
        add(indent + get_one_indent(), "address", remote_address, write);
        add(indent + get_one_indent(), "port", remote_port, write, true);
        write += indent + "}," + get_end_line();
        write += indent + "\"local\": {" + get_end_line();
        add(indent + get_one_indent(), "address", local_address, write);
        add(indent + get_one_indent(), "port", local_port, write, true);
        write += indent + "}," + get_end_line();
        write += indent + "\"status\": {" + get_end_line();
        add(indent + get_one_indent(), "text", state_text, write);
        add(indent + get_one_indent(), "value", state_value, write, true);
        write += indent + "}" + get_end_line();
    }
};

class LOG_NETWORK : public Log
{
public:
    bool enabled = false;
    int count = 0;
    std::list<LOG_NETWORK_ENTRY> list;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "active", enabled, write, !enabled);
        if (!enabled)
        {
            return;
        }
        add(indent, "count", count, write);
        add_list(indent, "list", list, write, true);
    }
};


class PROC_LOG : public Log
{
public:
    std::string type;
    std::string message;
    LOG_MODULE module;
    LOG_THREAD thread;
    LOG_MEMORY memory;
    LOG_HANDLE handle;
    LOG_NETWORK network;

    void save_internal(std::string indent, std::string& write)
    {
        add(indent, "type", type, write);
        add(indent, "message", message, write);
        module.save(indent, "module", write);
        thread.save(indent, "thread", write);
        memory.save(indent, "memory", write);
        handle.save(indent, "handle", write);
        network.save(indent, "network", write, true);
    }
};


typedef struct _LOG_CONTAINER
{
    bool is_proc_log = false;
    LOG exec;
    PROC_LOG proc;
    unsigned long long counter = 0;
}LOG_CONTAINER;


class THREAD_LOG_STATE : public Log
{
public:
    char file_name[MAX_PATH] = { 0 };
    char cmd_line[MAX_STRING_SIZE] = { 0 };
    int count = 0;
    int save_count = 0;
    int process_id = 0;
    int thread_id = 0;
    std::list<LOG_CONTAINER> log;

    void save_write(std::string indent, HANDLE handle)
    {
        std::string write = indent + "{" + get_end_line();
        indent += get_one_indent();
        add(indent, "file_name", (const char*)file_name, write);
        add(indent, "cmd_line", (const char*)cmd_line, write);
        add(indent, "count", count, write);
        add(indent, "save_count", save_count, write);
        add(indent, "process_id", process_id, write);
        add(indent, "thread_id", thread_id, write);
        write += indent + "\"log\": [\n";
        DWORD written = 0;
        WriteFile(handle, write.c_str(), (DWORD)strlen(write.c_str()), &written, NULL);

        bool first = true;
        for (auto itr = log.begin(); itr != log.end(); itr++)
        {
            write = "";
            if (!first)
            {
                write += ",\n";
            }
            write += indent + get_one_indent() + "{" + get_end_line();
            add(indent + get_one_indent() + get_one_indent(), "counter", itr->counter, write);
            if (itr->is_proc_log)
            {
                itr->proc.save_internal(indent + get_one_indent() + get_one_indent(), write);
            }
            else
            {
                itr->exec.save_internal(indent + get_one_indent() + get_one_indent(), write);
            }
            write += indent + get_one_indent() + "}";
            WriteFile(handle, write.c_str(), (DWORD)strlen(write.c_str()), &written, NULL);
            first = false;
        }

        write = get_end_line() + indent + "]" + get_end_line() + indent + "}" + get_end_line();
        WriteFile(handle, write.c_str(), (DWORD)strlen(write.c_str()), &written, NULL);
    }
};

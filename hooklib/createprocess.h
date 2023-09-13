void createprocess_push_hook_w();
void createprocess_push_hook_a();

struct process_hook_sym_w {
    const wchar_t *name;
    const wchar_t *dll_name;
    const wchar_t *tail;
};

struct process_hook_sym_a {
    const char *name;
    const char *dll_name;
    const char *tail;
};
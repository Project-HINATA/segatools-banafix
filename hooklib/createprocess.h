#pragma once

#include <windows.h>

HRESULT createprocess_push_hook_w(const wchar_t *name, const wchar_t *head, const wchar_t *tail);
HRESULT createprocess_push_hook_a(const char *name, const char *head, const char *tail);

struct process_hook_sym_w {
    const wchar_t *name;
    size_t name_size;
    const wchar_t *head;
    size_t head_size;
    const wchar_t *tail;
    size_t tail_size;
};

struct process_hook_sym_a {
    const char *name;
    size_t name_size;
    const char *head;
    size_t head_size;
    const char *tail;
    size_t tail_size;
};
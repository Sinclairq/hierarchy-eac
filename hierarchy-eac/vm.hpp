#pragma once
#include "stdafx.hpp"

struct context_t
{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rax;
};

namespace vm
{
    extern void *g_is_reloc;
    extern "C" void vm_is_reloc();
    extern "C" void is_reloc( context_t *context );

    void image_callback( PUNICODE_STRING image_name, HANDLE proc_id, PIMAGE_INFO image_info );
} // namespace vm

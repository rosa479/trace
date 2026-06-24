#pragma once
#include <sys/user.h>
#include <sys/ptrace.h>
#include <cstdint>
#include <string>
#include <array>

enum class Reg {
    rax, rbx, rcx, rdx,
    rdi, rsi, rbp, rsp,
    r8,  r9,  r10, r11,
    r12, r13, r14, r15,
    rip, rflags,
    cs, ss, ds, es, fs, gs
};

struct RegDescriptor {
    Reg r;
    std::string name;
    size_t offset;
};

extern const std::array<RegDescriptor, 24> g_reg_descriptors;

uint64_t get_register_value(pid_t pid, Reg r);
void     set_register_value(pid_t pid, Reg r, uint64_t value);
std::string get_register_name(Reg r);

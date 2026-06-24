#include "registers.h"
#include <algorithm>
#include <stdexcept>

const std::array<RegDescriptor, 24> g_reg_descriptors {{
    { Reg::rax,    "rax",    offsetof(user_regs_struct, rax)    },
    { Reg::rbx,    "rbx",    offsetof(user_regs_struct, rbx)    },
    { Reg::rcx,    "rcx",    offsetof(user_regs_struct, rcx)    },
    { Reg::rdx,    "rdx",    offsetof(user_regs_struct, rdx)    },
    { Reg::rdi,    "rdi",    offsetof(user_regs_struct, rdi)    },
    { Reg::rsi,    "rsi",    offsetof(user_regs_struct, rsi)    },
    { Reg::rbp,    "rbp",    offsetof(user_regs_struct, rbp)    },
    { Reg::rsp,    "rsp",    offsetof(user_regs_struct, rsp)    },
    { Reg::r8,     "r8",     offsetof(user_regs_struct, r8)     },
    { Reg::r9,     "r9",     offsetof(user_regs_struct, r9)     },
    { Reg::r10,    "r10",    offsetof(user_regs_struct, r10)    },
    { Reg::r11,    "r11",    offsetof(user_regs_struct, r11)    },
    { Reg::r12,    "r12",    offsetof(user_regs_struct, r12)    },
    { Reg::r13,    "r13",    offsetof(user_regs_struct, r13)    },
    { Reg::r14,    "r14",    offsetof(user_regs_struct, r14)    },
    { Reg::r15,    "r15",    offsetof(user_regs_struct, r15)    },
    { Reg::rip,    "rip",    offsetof(user_regs_struct, rip)    },
    { Reg::rflags, "rflags", offsetof(user_regs_struct, eflags) },
    { Reg::cs,     "cs",     offsetof(user_regs_struct, cs)     },
    { Reg::ss,     "ss",     offsetof(user_regs_struct, ss)     },
    { Reg::ds,     "ds",     offsetof(user_regs_struct, ds)     },
    { Reg::es,     "es",     offsetof(user_regs_struct, es)     },
    { Reg::fs,     "fs",     offsetof(user_regs_struct, fs)     },
    { Reg::gs,     "gs",     offsetof(user_regs_struct, gs)     },
}};

uint64_t get_register_value(pid_t pid, Reg r) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    auto it = std::find_if(g_reg_descriptors.begin(), g_reg_descriptors.end(),
        [r](const RegDescriptor& d) { return d.r == r; });
    return *reinterpret_cast<const uint64_t*>(
        reinterpret_cast<const char*>(&regs) + it->offset);
}

void set_register_value(pid_t pid, Reg r, uint64_t value) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    auto it = std::find_if(g_reg_descriptors.begin(), g_reg_descriptors.end(),
        [r](const RegDescriptor& d) { return d.r == r; });
    *reinterpret_cast<uint64_t*>(
        reinterpret_cast<char*>(&regs) + it->offset) = value;
    ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
}

std::string get_register_name(Reg r) {
    auto it = std::find_if(g_reg_descriptors.begin(), g_reg_descriptors.end(),
        [r](const RegDescriptor& d) { return d.r == r; });
    return it != g_reg_descriptors.end() ? it->name : "unknown";
}

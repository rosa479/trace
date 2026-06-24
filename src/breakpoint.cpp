#include "breakpoint.h"
#include <sys/ptrace.h>

Breakpoint::Breakpoint(pid_t pid, std::intptr_t addr)
    : m_pid(pid), m_addr(addr) {}

void Breakpoint::enable() {
    long data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    m_saved = static_cast<uint8_t>(data & 0xff);
    ptrace(PTRACE_POKEDATA, m_pid, m_addr, (data & ~0xffL) | 0xccL);
    m_enabled = true;
}

void Breakpoint::disable() {
    long data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    ptrace(PTRACE_POKEDATA, m_pid, m_addr, (data & ~0xffL) | m_saved);
    m_enabled = false;
}

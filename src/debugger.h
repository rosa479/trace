#pragma once
#include "breakpoint.h"
#include <string>
#include <unordered_map>
#include <sys/types.h>
#include <cstdint>

class Debugger {
public:
    Debugger(std::string prog, pid_t pid);
    void run();

private:
    void handle_command(const std::string& line);

    void continue_execution();
    void single_step_instruction();
    void step_over_breakpoint();

    void set_breakpoint_at(std::intptr_t addr);
    void remove_breakpoint(std::intptr_t addr);
    void list_breakpoints() const;

    void dump_registers() const;
    void read_memory(uint64_t addr, size_t count) const;
    void write_memory(uint64_t addr, uint64_t value);

    void     wait_for_signal();
    void     handle_sigtrap();
    uint64_t get_pc() const;
    void     set_pc(uint64_t pc);
    void     print_source(uint64_t addr) const;

    std::string  m_prog;
    pid_t        m_pid;
    bool         m_running = true;
    std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;
};

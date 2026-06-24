#include "debugger.h"
#include "registers.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, delim))
        if (!tok.empty()) out.push_back(tok);
    return out;
}

static bool is_prefix(const std::string& s, const std::string& of) {
    return s.size() <= of.size() && of.substr(0, s.size()) == s;
}

Debugger::Debugger(std::string prog, pid_t pid)
    : m_prog(std::move(prog)), m_pid(pid) {}

void Debugger::run() {
    int status;
    waitpid(m_pid, &status, 0);  // consume initial exec stop

    std::string line;
    while (m_running) {
        std::cout << "(trace) ";
        if (!std::getline(std::cin, line)) break;
        if (!line.empty()) handle_command(line);
    }
}

void Debugger::handle_command(const std::string& line) {
    auto args = split(line, ' ');
    if (args.empty()) return;
    const auto& cmd = args[0];

    if (is_prefix(cmd, "continue") || cmd == "c") {
        if (m_running) continue_execution();

    } else if (is_prefix(cmd, "break") || cmd == "b") {
        if (args.size() < 2) { puts("usage: break <hex_addr>"); return; }
        std::intptr_t addr = std::stol(args[1], nullptr, 16);
        set_breakpoint_at(addr);

    } else if (is_prefix(cmd, "delete") || cmd == "d") {
        if (args.size() < 2) { puts("usage: delete <hex_addr>"); return; }
        std::intptr_t addr = std::stol(args[1], nullptr, 16);
        remove_breakpoint(addr);

    } else if (cmd == "info" && args.size() > 1 && args[1] == "break") {
        list_breakpoints();

    } else if (is_prefix(cmd, "step") || cmd == "s") {
        single_step_instruction();

    } else if (is_prefix(cmd, "registers") || cmd == "regs") {
        dump_registers();

    } else if (cmd == "x") {
        if (args.size() < 2) { puts("usage: x <hex_addr> [count]"); return; }
        uint64_t addr  = std::stoull(args[1], nullptr, 16);
        size_t   count = args.size() >= 3 ? std::stoull(args[2]) : 64;
        read_memory(addr, count);

    } else if (cmd == "w") {
        if (args.size() < 3) { puts("usage: w <hex_addr> <hex_val>"); return; }
        uint64_t addr = std::stoull(args[1], nullptr, 16);
        uint64_t val  = std::stoull(args[2], nullptr, 16);
        write_memory(addr, val);

    } else if (is_prefix(cmd, "quit") || cmd == "q") {
        kill(m_pid, SIGKILL);
        m_running = false;

    } else {
        puts("commands: continue | break <addr> | delete <addr> | info break");
        puts("          step | registers | x <addr> [n] | w <addr> <val> | quit");
    }
}

void Debugger::continue_execution() {
    step_over_breakpoint();
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
    wait_for_signal();
}

void Debugger::single_step_instruction() {
    if (m_breakpoints.count(get_pc())) {
        step_over_breakpoint();
    } else {
        ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
        wait_for_signal();
    }
}

void Debugger::step_over_breakpoint() {
    auto pc = get_pc();
    auto it = m_breakpoints.find(pc);
    if (it == m_breakpoints.end() || !it->second.is_enabled()) return;

    it->second.disable();
    ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
    int status;
    waitpid(m_pid, &status, 0);  // wait silently — just stepping past the bp
    it->second.enable();
}

void Debugger::set_breakpoint_at(std::intptr_t addr) {
    if (m_breakpoints.count(addr)) {
        printf("breakpoint already set at 0x%lx\n", addr);
        return;
    }
    Breakpoint bp{m_pid, addr};
    bp.enable();
    m_breakpoints[addr] = bp;
    printf("breakpoint set at 0x%lx\n", addr);
}

void Debugger::remove_breakpoint(std::intptr_t addr) {
    auto it = m_breakpoints.find(addr);
    if (it == m_breakpoints.end()) {
        printf("no breakpoint at 0x%lx\n", addr);
        return;
    }
    if (it->second.is_enabled()) it->second.disable();
    m_breakpoints.erase(it);
    printf("breakpoint removed at 0x%lx\n", addr);
}

void Debugger::list_breakpoints() const {
    if (m_breakpoints.empty()) { puts("no breakpoints"); return; }
    int id = 1;
    for (const auto& [addr, bp] : m_breakpoints)
        printf("  %d  0x%016lx  %s\n", id++, addr,
               bp.is_enabled() ? "enabled" : "disabled");
}

void Debugger::dump_registers() const {
    for (const auto& rd : g_reg_descriptors) {
        auto val = get_register_value(m_pid, rd.r);
        printf("  %-8s  0x%016lx  %lu\n", rd.name.c_str(), val, val);
    }
}

void Debugger::read_memory(uint64_t addr, size_t count) const {
    for (size_t i = 0; i < count; i += 8) {
        long word = ptrace(PTRACE_PEEKDATA, m_pid, addr + i, nullptr);
        printf("0x%016lx: ", addr + i);
        auto* bytes = reinterpret_cast<uint8_t*>(&word);
        for (size_t j = 0; j < 8 && i + j < count; j++)
            printf("%02x ", bytes[j]);
        printf("\n");
    }
}

void Debugger::write_memory(uint64_t addr, uint64_t value) {
    ptrace(PTRACE_POKEDATA, m_pid, addr, value);
}

void Debugger::wait_for_signal() {
    int status;
    waitpid(m_pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("process exited (status %d)\n", WEXITSTATUS(status));
        m_running = false;
        return;
    }
    if (WIFSIGNALED(status)) {
        printf("process killed by signal %d\n", WTERMSIG(status));
        m_running = false;
        return;
    }
    if (WIFSTOPPED(status)) {
        switch (WSTOPSIG(status)) {
        case SIGTRAP: handle_sigtrap(); break;
        case SIGSEGV: printf("segfault at 0x%lx\n", get_pc()); break;
        default:      printf("stopped with signal %d\n", WSTOPSIG(status)); break;
        }
    }
}

void Debugger::handle_sigtrap() {
    siginfo_t info{};
    ptrace(PTRACE_GETSIGINFO, m_pid, nullptr, &info);

    if (info.si_code == TRAP_TRACE) {
        printf("  -> 0x%lx\n", get_pc());
        print_source(get_pc());
        return;
    }

    // TRAP_BRKPT or SI_KERNEL: int3 fired
    if (info.si_code == TRAP_BRKPT || info.si_code == SI_KERNEL) {
        auto pc = get_pc() - 1;
        set_pc(pc);
        if (m_breakpoints.count(pc)) {
            printf("breakpoint hit at 0x%lx\n", pc);
            print_source(pc);
        }
        return;
    }
    // initial exec stop or other — ignore silently
}

uint64_t Debugger::get_pc() const {
    return get_register_value(m_pid, Reg::rip);
}

void Debugger::set_pc(uint64_t pc) {
    set_register_value(m_pid, Reg::rip, pc);
}

void Debugger::print_source(uint64_t addr) const {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "addr2line -e '%s' -f 0x%lx 2>/dev/null",
             m_prog.c_str(), addr);
    FILE* f = popen(cmd, "r");
    if (!f) return;
    char buf[256];
    int lines = 0;
    while (fgets(buf, sizeof(buf), f) && lines < 2) {
        printf("  %s", buf);
        lines++;
    }
    pclose(f);
}

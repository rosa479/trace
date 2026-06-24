#pragma once
#include <sys/types.h>
#include <cstdint>

class Breakpoint {
public:
    Breakpoint() = default;
    Breakpoint(pid_t pid, std::intptr_t addr);

    void enable();
    void disable();

    bool         is_enabled()  const { return m_enabled; }
    std::intptr_t get_address() const { return m_addr;   }

private:
    pid_t         m_pid     = 0;
    std::intptr_t m_addr    = 0;
    bool          m_enabled = false;
    uint8_t       m_saved   = 0;
};

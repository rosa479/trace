# trace

Linux process debugger built on `ptrace`. Supports software breakpoints, single-step execution, register and memory inspection, and source-line mapping.

## Build

```sh
make
make test-targets   # build example targets in test/
```

Requires `g++` (C++17), `gcc`, and `addr2line` (binutils).

## Usage

```sh
./trace <program> [args...]
```

Target binaries should be compiled with `-g -O0 -no-pie` for full breakpoint and source-line support.

```sh
gcc -g -O0 -no-pie -o myprog myprog.c
./trace ./myprog
```

## Commands

| Command | Description |
|---|---|
| `break <hex_addr>` | Set breakpoint at address |
| `delete <hex_addr>` | Remove breakpoint |
| `info break` | List all breakpoints |
| `continue` / `c` | Continue execution |
| `step` / `s` | Single-step one instruction |
| `registers` / `regs` | Dump all registers |
| `x <hex_addr> [n]` | Read n bytes from address |
| `w <hex_addr> <hex_val>` | Write 8 bytes to address |
| `quit` / `q` | Kill process and exit |

## Example session

```sh
$ nm test/hello | grep ' T main'
000000000040113a T main

$ ./trace test/hello
trace: pid 12345
(trace) break 0x40113a
breakpoint set at 0x40113a
(trace) continue
breakpoint hit at 0x40113a
  main
  test/hello.c:7
(trace) step
  -> 0x40113e
  main
  test/hello.c:7
(trace) regs
  rax       0x0000000000000000  0
  rip       0x000000000040113e  4198718
  ...
(trace) quit
```

## How it works

- **Breakpoints** — writes `int3` (0xCC) over the target instruction byte via `PTRACE_POKEDATA`, restores the original on hit and re-patches after stepping past it
- **Single-step** — `PTRACE_SINGLESTEP` sets the trap flag; CPU raises `SIGTRAP` after one instruction
- **Registers** — `PTRACE_GETREGS` / `PTRACE_SETREGS` with `user_regs_struct`
- **Source lines** — `addr2line` maps instruction addresses to file:line

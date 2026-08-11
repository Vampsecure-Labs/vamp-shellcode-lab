<!-- © VampSecure Studios — VampSecure Labs Security Research Division -->
<h1 align="center">vamp-shellcode-lab</h1>
<p align="center">
  <strong>Educational ARM64 shellcode execution lab demonstrating mmap RWX, inline ASM, and direct Linux syscalls</strong><br>
  <em>VampSecure Labs · Security Research Division</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/language-C-blue?style=flat-square&logo=c">
  <img src="https://img.shields.io/badge/architecture-ARM64%20%7C%20AArch64-lightgrey?style=flat-square">
  <img src="https://img.shields.io/badge/platform-linux--arm64-lightgrey?style=flat-square">
  <img src="https://img.shields.io/badge/license-research%20only-red?style=flat-square">
  <img src="https://img.shields.io/badge/VampSecure-Labs-8B0000?style=flat-square">
</p>

---

## Overview

`vamp-shellcode-lab` is an educational shellcode execution laboratory targeting the ARM64 (AArch64) architecture on Linux. It demonstrates the core primitives behind shellcode development and exploitation research through two self-contained demos: inline GNU assembler embedded directly in C, and a pre-calculated shellcode byte array executed from an `mmap`-allocated RWX memory region. Both paths perform direct Linux kernel syscalls without libc, illustrating how exploit payloads communicate with the kernel at the lowest level.

The repository includes `vamp_msg.s` — a pure assembly reference showing the same payload in readable AArch64 assembler syntax, annotated with a quick-reference table of ARM64 Linux syscall numbers and the full AArch64 calling convention.

**Intended audience**: security researchers, reverse engineers, and students studying exploitation techniques, memory protection mechanisms (NX/W^X/PXN), and ARM64 assembly. All content is strictly educational — the shellcode payload does nothing beyond printing a five-byte string and calling `exit(0)`.

## Features

- **Demo 1 — Inline ASM** (`demo_asm_inline`): uses `__asm__ volatile` to execute ARM64 instructions directly within C; the compiler resolves label offsets automatically, eliminating manual offset arithmetic
- **Demo 2 — Shellcode as bytes** (`demo_shellcode_bytes`): copies a pre-calculated byte array to a region obtained via `mmap(PROT_READ | PROT_WRITE | PROT_EXEC)` and invokes it as a function pointer — the canonical shellcode injection technique
- **Annotated shellcode** for `write(1, "VAMP\n", 5)` + `exit(0)` via ARM64 syscalls 64 and 93; each byte in the array is commented against the originating instruction
- **Pure ASM reference** (`vamp_msg.s`): the same payload in readable AArch64 assembler syntax, compilable with `as` + `ld` for independent byte extraction via `objdump`
- **ARM64 syscall table** in `vamp_msg.s`: `write`, `read`, `exit`, `exit_group`, `execve`, `execveat`, `mmap`, `munmap` with argument registers
- **AArch64 calling convention notes**: x0–x7 arguments, x8 syscall number, x19–x28 callee-save, x29 frame pointer, x30 link register, sp stack pointer
- **Defensive context**: in-code notes on how `ptrace`, `seccomp`, `/proc/PID/maps`, `auditd`, `eBPF`, and `checksec` can detect each technique
- **Makefile** with three targets: `make` (main lab), `make asm` (pure ASM reference), `make clean`
- Compatible with Linux ARM64: Raspberry Pi 4/5, AWS Graviton, Oracle Cloud Ampere, and Docker Linux ARM64 on Apple Silicon

## Requirements

- GCC cross-compiler or native ARM64 Linux toolchain
- `binutils` (`as`, `ld`) for the `make asm` target
- Linux ARM64 environment (native or `docker run --platform linux/arm64`)

No Python runtime. No external libraries.

## Installation

```bash
git clone https://github.com/belky-me/vamp-shellcode-lab.git
cd vamp-shellcode-lab
```

**Build the main lab (requires Linux ARM64):**
```bash
make
```

**Build the pure ASM reference binary:**
```bash
make asm
```

**Clean compiled artifacts:**
```bash
make clean
```

### Compilation Details

```bash
gcc -Wall -Wextra -z execstack -o vamp_shell_lab vamp_shell_lab.c
```

The `-z execstack` linker flag disables the NX stack protection that production linkers apply by default. It is required here for Demo 1 (inline ASM references a `.ascii` label in the `.text` section). It must never appear in production builds. Demo 2 uses a heap-allocated `mmap` region and would not strictly require it, but it is included for consistency across GCC/kernel variants.

## Usage

```bash
./vamp_shell_lab
```

Expected output:
```
╔══════════════════════════════════════════════════════╗
║   VampSecure Labs — Shell Lab v2.0 (ARM64)          ║
║   Laboratorio educativo de shellcode Linux ARM64     ║
╚══════════════════════════════════════════════════════╝

Arquitectura objetivo : ARM64 (AArch64) Linux
Protecciones activas  : NX desactivado por -z execstack (SOLO LAB)

[DEMO 1] Ensamblador inline — instrucciones ARM64 directas en C
[*] Ejecutando syscall write via __asm__...
[*] Salida del shellcode: VAMP
[✓] Syscall completada correctamente.

[DEMO 2] Shellcode como array de bytes — técnica base de inyección
[*] Región RWX asignada en: 0x7f... (37 bytes)
[*] Copiando 37 bytes de shellcode...
[*] Salida del shellcode: VAMP
```

### Extracting Byte Values from the ASM Reference

```bash
# Assemble and link
as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o

# Verify output
./vamp_msg

# Extract hex bytes for hardcoding in C
objdump -d vamp_msg.o | grep -A 999 "<.text>"
```

## ARM64 Syscall Reference (from vamp_msg.s)

| Number | Name | Prototype |
|--------|------|-----------|
| 64 | write | `write(fd, buf, count)` → x0, x1, x2 |
| 63 | read | `read(fd, buf, count)` → x0, x1, x2 |
| 93 | exit | `exit(status)` → x0 |
| 94 | exit_group | `exit_group(status)` → x0 |
| 221 | execve | `execve(path, argv, envp)` → x0, x1, x2 |
| 192 | mmap | `mmap(addr, len, prot, flags, fd, off)` → x0–x5 |
| 215 | munmap | `munmap(addr, len)` → x0, x1 |

## Platform Compatibility

| Environment | Status |
|------------|--------|
| Linux ARM64 (native) | Fully supported |
| Docker Linux ARM64 on Apple Silicon | Supported (`--platform linux/arm64`) |
| macOS Apple Silicon (native) | Not supported — different syscall ABI (XNU) |
| Linux x86_64 | Not supported — ARM64 instruction set only |

## Part of VampSecure Labs Toolkit

This tool is part of the **VampSecure Labs Security Toolkit** — a collection of research-grade security tools for authorized penetration testing and red/blue team exercises.

- Full toolkit: [github.com/belky-me](https://github.com/belky-me)
- Orchestrator: [github.com/belky-me/vamp-orchestrator](https://github.com/belky-me/vamp-orchestrator)

---

© VampSecure Studios — VampSecure Labs Security Research Division  
For authorized security testing only.

## Versión
Herramienta de investigación — VampSecure Labs Security Research Division

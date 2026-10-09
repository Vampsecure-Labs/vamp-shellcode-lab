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

> 🇬🇧 [English](#english) · 🇪🇸 [Español](#español)

---

<a name="english"></a>
## 🇬🇧 English

### Overview

`vamp-shellcode-lab` is an educational shellcode execution laboratory targeting the ARM64 (AArch64) architecture on Linux. It demonstrates the core primitives behind shellcode development and exploitation research through two self-contained demos: inline GNU assembler embedded directly in C, and a pre-calculated shellcode byte array executed from an `mmap`-allocated RWX memory region. Both paths perform direct Linux kernel syscalls without libc, illustrating how exploit payloads communicate with the kernel at the lowest level.

The repository includes `vamp_msg.s` — a pure assembly reference showing the same payload in readable AArch64 assembler syntax, annotated with a quick-reference table of ARM64 Linux syscall numbers and the full AArch64 calling convention.

**Intended audience**: security researchers, reverse engineers, and students studying exploitation techniques, memory protection mechanisms (NX/W^X/PXN), and ARM64 assembly. All content is strictly educational — the shellcode payload does nothing beyond printing a five-byte string and calling `exit(0)`.

### Features

- **Demo 1 — Inline ASM** (`demo_asm_inline`): uses `__asm__ volatile` to execute ARM64 instructions directly within C; the compiler resolves label offsets automatically, eliminating manual offset arithmetic
- **Demo 2 — Shellcode as bytes** (`demo_shellcode_bytes`): copies a pre-calculated byte array to a region obtained via `mmap(PROT_READ | PROT_WRITE | PROT_EXEC)` and invokes it as a function pointer — the canonical shellcode injection technique
- **Annotated shellcode** for `write(1, "VAMP\n", 5)` + `exit(0)` via ARM64 syscalls 64 and 93; each byte in the array is commented against the originating instruction
- **Pure ASM reference** (`vamp_msg.s`): the same payload in readable AArch64 assembler syntax, compilable with `as` + `ld` for independent byte extraction via `objdump`
- **ARM64 syscall table** in `vamp_msg.s`: `write`, `read`, `exit`, `exit_group`, `execve`, `execveat`, `mmap`, `munmap` with argument registers
- **AArch64 calling convention notes**: x0–x7 arguments, x8 syscall number, x19–x28 callee-save, x29 frame pointer, x30 link register, sp stack pointer
- **Defensive context**: in-code notes on how `ptrace`, `seccomp`, `/proc/PID/maps`, `auditd`, `eBPF`, and `checksec` can detect each technique
- **Makefile** with three targets: `make` (main lab), `make asm` (pure ASM reference), `make clean`
- Compatible with Linux ARM64: Raspberry Pi 4/5, AWS Graviton, Oracle Cloud Ampere, and Docker Linux ARM64 on Apple Silicon

### Requirements

- GCC cross-compiler or native ARM64 Linux toolchain
- `binutils` (`as`, `ld`) for the `make asm` target
- Linux ARM64 environment (native or `docker run --platform linux/arm64`)

No Python runtime. No external libraries.

### Installation

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

#### Compilation Details

```bash
gcc -Wall -Wextra -z execstack -o vamp_shell_lab vamp_shell_lab.c
```

The `-z execstack` linker flag disables the NX stack protection that production linkers apply by default. It is required here for Demo 1 (inline ASM references a `.ascii` label in the `.text` section). It must never appear in production builds. Demo 2 uses a heap-allocated `mmap` region and would not strictly require it, but it is included for consistency across GCC/kernel variants.

### Usage

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

#### Extracting Byte Values from the ASM Reference

```bash
# Assemble and link
as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o

# Verify output
./vamp_msg

# Extract hex bytes for hardcoding in C
objdump -d vamp_msg.o | grep -A 999 "<.text>"
```

### ARM64 Syscall Reference (from vamp_msg.s)

| Number | Name | Prototype |
|--------|------|-----------|
| 64 | write | `write(fd, buf, count)` → x0, x1, x2 |
| 63 | read | `read(fd, buf, count)` → x0, x1, x2 |
| 93 | exit | `exit(status)` → x0 |
| 94 | exit_group | `exit_group(status)` → x0 |
| 221 | execve | `execve(path, argv, envp)` → x0, x1, x2 |
| 192 | mmap | `mmap(addr, len, prot, flags, fd, off)` → x0–x5 |
| 215 | munmap | `munmap(addr, len)` → x0, x1 |

### Platform Compatibility

| Environment | Status |
|------------|--------|
| Linux ARM64 (native) | Fully supported |
| Docker Linux ARM64 on Apple Silicon | Supported (`--platform linux/arm64`) |
| macOS Apple Silicon (native) | Not supported — different syscall ABI (XNU) |
| Linux x86_64 | Not supported — ARM64 instruction set only |

### Sample Output

```
$ make && ./vamp_shell_lab
gcc -Wall -Wextra -z execstack -o vamp_shell_lab vamp_shell_lab.c

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
[*] Región RWX asignada en: 0x7f8a3c0000 (37 bytes)
[*] Copiando 37 bytes de shellcode...
[*] Saltando a la región RWX como función...
[*] Salida del shellcode: VAMP
[✓] Demo 2 completada. Región liberada con munmap.
```

```
$ make asm && ./vamp_msg
as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o
VAMP

$ objdump -d vamp_msg.o | grep -A 20 "<.text>"
vamp_msg.o:     file format elf64-littleaarch64
Disassembly of section .text:
0000000000000000 <.text>:
   0: d28000a0  mov  x0, #0x5       // fd = stdout
   4: 10000061  adr  x1, 10 <msg>  // buf = &msg
   8: d2800042  mov  x2, #0x5      // count = 5 ("VAMP\n")
   c: d2800808  mov  x8, #0x40     // NR_write = 64
  10: d4000001  svc  #0x0
  14: d2800000  mov  x0, #0x0      // status = 0
  18: d2800ba8  mov  x8, #0x5d     // NR_exit = 93
  1c: d4000001  svc  #0x0
```

### Why vamp-shellcode-lab vs. pwndbg tutorials · shellcode databases · ARM64 exploit dev guides

| Capability | vamp-shellcode-lab | pwndbg tutorials | Shellcode databases | ARM64 exploit guides |
|------------|--------------------|------------------|---------------------|----------------------|
| Self-contained compilable code (C + Makefile) | ✅ | ❌ Docs only | ❌ Hex bytes only | ❌ Snippets, no build |
| Inline ASM + byte array side by side | ✅ Two demos compared | ❌ | ❌ | ❌ |
| Annotated .s reference (objdump-ready) | ✅ `vamp_msg.s` | ❌ | ❌ Partial | ✅ Varies |
| Syscall table embedded in source | ✅ 7 syscalls with register layout | ❌ | ✅ External tables | ✅ |
| Defensive detection notes (ptrace / seccomp / eBPF) | ✅ Inline comments | ✅ pwndbg-specific | ❌ | ❌ |
| Docker ARM64 on Apple Silicon (no host toolchain) | ✅ `--platform linux/arm64` | ❌ | ❌ | ❌ |
| No Python / no framework dependency | ✅ C + gcc + binutils only | ❌ needs pwndbg | ❌ | ❌ |

- **Two execution paths** — side-by-side comparison of inline ASM and shellcode-as-bytes in the same run makes the conceptual leap explicit: one is code the compiler embeds, the other is data the program treats as code.
- **Byte-level traceability** — every byte in the array is commented back to its ARM64 instruction; `objdump` on `vamp_msg.o` lets you verify the extraction yourself without taking anything on faith.
- **Detection context baked in** — inline notes on `ptrace`, `seccomp`, `auditd`, and `checksec` show which host-side countermeasure catches each technique, turning the lab into a red/blue bridge.
- **Zero-dependency ARM64 Docker path** — `docker run --platform linux/arm64` on any Apple Silicon Mac; no host toolchain setup required, no emulation quirks.

### Educational Coverage

| Technique | What it demonstrates |
|-----------|----------------------|
| AArch64 calling convention | x0–x7 argument registers, x8 syscall number, x29 FP, x30 LR, callee-save x19–x28 |
| Direct Linux syscall (write + exit) | Syscall numbers 64 and 93; bypassing libc / glibc entirely with `svc #0` |
| `__asm__ volatile` inline assembler | Embedding ARM64 instructions directly in C without a separate .s translation unit |
| `mmap(PROT_READ\|PROT_WRITE\|PROT_EXEC)` | Allocating a writable and executable heap region — the canonical RWX shellcode staging technique |
| Function pointer cast to shellcode | Casting `void *` to `void (*)(void)` and jumping into a byte array — the core injection model |
| NX / W^X bypass (conceptual) | `-z execstack` disabling GNU stack protection; lab notes explain PXN and why this flag must never appear in production |
| `objdump` byte extraction pipeline | `as` → `ld` → `objdump -d` to derive the hardcoded byte array from readable assembly |
| Syscall argument layout (ARM64) | Full register-to-argument mapping for write, read, exit, exit_group, execve, mmap, munmap |
| Defensive detection surface | `ptrace`, `seccomp` filters, `/proc/PID/maps` inspection, `auditd`, `eBPF` tracepoints, `checksec` output |
| Cross-compilation / Docker ARM64 | Native build on Graviton / Raspberry Pi or `--platform linux/arm64` container on Apple Silicon |

### Part of VampSecure Labs Toolkit

This tool is part of the **VampSecure Labs Security Toolkit** — a collection of research-grade security tools for authorized penetration testing and red/blue team exercises.

- Full toolkit: [github.com/belky-me](https://github.com/belky-me)
- Orchestrator: [github.com/belky-me/vamp-orchestrator](https://github.com/belky-me/vamp-orchestrator)

### Version History

| Version | Main changes |
|---------|-------------|
| v1.1 | Bilingual README (EN/ES) |
| v1.0 | Initial educational ARM64 shellcode lab |

---

© VampSecure Studios — VampSecure Labs Security Research Division  
For authorized security testing only.

---
---

<a name="español"></a>
## 🇪🇸 Español

### Descripción general

`vamp-shellcode-lab` es un laboratorio educativo de ejecución de shellcode orientado a la arquitectura ARM64 (AArch64) sobre Linux. Demuestra los primitivos básicos del desarrollo de shellcode y la investigación de exploits mediante dos demos autocontenidas: ensamblador GNU inline embebido directamente en C, y un array de bytes de shellcode precalculado ejecutado desde una región de memoria RWX asignada con `mmap`. Ambas rutas realizan syscalls directas al kernel de Linux sin libc, ilustrando cómo los payloads de exploit se comunican con el kernel al nivel más bajo.

El repositorio incluye `vamp_msg.s` — una referencia en ensamblador puro que muestra el mismo payload en sintaxis AArch64 legible, anotada con una tabla de consulta rápida de números de syscall ARM64 de Linux y la convención de llamada AArch64 completa.

**Público objetivo**: investigadores de seguridad, ingenieros de reversing y estudiantes que estudian técnicas de explotación, mecanismos de protección de memoria (NX/W^X/PXN) y ensamblador ARM64. Todo el contenido es estrictamente educativo — el payload del shellcode no hace nada más allá de imprimir una cadena de cinco bytes y llamar a `exit(0)`.

### Características

- **Demo 1 — ASM inline** (`demo_asm_inline`): usa `__asm__ volatile` para ejecutar instrucciones ARM64 directamente en C; el compilador resuelve los offsets de etiquetas automáticamente, eliminando la aritmética manual de offsets
- **Demo 2 — Shellcode como bytes** (`demo_shellcode_bytes`): copia un array de bytes precalculado a una región obtenida mediante `mmap(PROT_READ | PROT_WRITE | PROT_EXEC)` y la invoca como puntero a función — la técnica canónica de inyección de shellcode
- **Shellcode anotado** para `write(1, "VAMP\n", 5)` + `exit(0)` mediante syscalls ARM64 64 y 93; cada byte del array está comentado con la instrucción ARM64 de origen
- **Referencia en ASM puro** (`vamp_msg.s`): el mismo payload en sintaxis AArch64 legible, compilable con `as` + `ld` para extracción independiente de bytes mediante `objdump`
- **Tabla de syscalls ARM64** en `vamp_msg.s`: `write`, `read`, `exit`, `exit_group`, `execve`, `execveat`, `mmap`, `munmap` con registros de argumento
- **Notas de convención de llamada AArch64**: argumentos x0–x7, número de syscall x8, callee-save x19–x28, frame pointer x29, link register x30, stack pointer sp
- **Contexto defensivo**: notas en el código sobre cómo `ptrace`, `seccomp`, `/proc/PID/maps`, `auditd`, `eBPF` y `checksec` pueden detectar cada técnica
- **Makefile** con tres objetivos: `make` (lab principal), `make asm` (referencia ASM puro), `make clean`
- Compatible con Linux ARM64: Raspberry Pi 4/5, AWS Graviton, Oracle Cloud Ampere y Docker Linux ARM64 en Apple Silicon

### Requisitos

- Compilador cruzado GCC o toolchain nativo Linux ARM64
- `binutils` (`as`, `ld`) para el objetivo `make asm`
- Entorno Linux ARM64 (nativo o `docker run --platform linux/arm64`)

Sin runtime Python. Sin librerías externas.

### Instalación

```bash
git clone https://github.com/belky-me/vamp-shellcode-lab.git
cd vamp-shellcode-lab
```

**Compilar el lab principal (requiere Linux ARM64):**
```bash
make
```

**Compilar el binario de referencia ASM puro:**
```bash
make asm
```

**Limpiar artefactos compilados:**
```bash
make clean
```

#### Detalles de compilación

```bash
gcc -Wall -Wextra -z execstack -o vamp_shell_lab vamp_shell_lab.c
```

El flag de enlazador `-z execstack` desactiva la protección NX de pila que los enlazadores de producción aplican por defecto. Es necesario aquí para la Demo 1 (el ASM inline referencia una etiqueta `.ascii` en la sección `.text`). No debe aparecer nunca en builds de producción. La Demo 2 usa una región `mmap` en el heap y no lo requeriría estrictamente, pero se incluye por consistencia entre variantes de GCC/kernel.

### Uso

```bash
./vamp_shell_lab
```

Salida esperada:
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

#### Extracción de valores de bytes desde la referencia ASM

```bash
# Ensamblar y enlazar
as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o

# Verificar salida
./vamp_msg

# Extraer bytes hex para hardcodear en C
objdump -d vamp_msg.o | grep -A 999 "<.text>"
```

### Referencia de syscalls ARM64 (de vamp_msg.s)

| Número | Nombre | Prototipo |
|--------|--------|-----------|
| 64 | write | `write(fd, buf, count)` → x0, x1, x2 |
| 63 | read | `read(fd, buf, count)` → x0, x1, x2 |
| 93 | exit | `exit(status)` → x0 |
| 94 | exit_group | `exit_group(status)` → x0 |
| 221 | execve | `execve(path, argv, envp)` → x0, x1, x2 |
| 192 | mmap | `mmap(addr, len, prot, flags, fd, off)` → x0–x5 |
| 215 | munmap | `munmap(addr, len)` → x0, x1 |

### Compatibilidad de plataformas

| Entorno | Estado |
|---------|--------|
| Linux ARM64 (nativo) | Totalmente soportado |
| Docker Linux ARM64 en Apple Silicon | Soportado (`--platform linux/arm64`) |
| macOS Apple Silicon (nativo) | No soportado — ABI de syscall diferente (XNU) |
| Linux x86_64 | No soportado — solo conjunto de instrucciones ARM64 |

### Salida de ejemplo

```
$ make && ./vamp_shell_lab
gcc -Wall -Wextra -z execstack -o vamp_shell_lab vamp_shell_lab.c

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
[*] Región RWX asignada en: 0x7f8a3c0000 (37 bytes)
[*] Copiando 37 bytes de shellcode...
[*] Saltando a la región RWX como función...
[*] Salida del shellcode: VAMP
[✓] Demo 2 completada. Región liberada con munmap.
```

```
$ make asm && ./vamp_msg
as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o
VAMP

$ objdump -d vamp_msg.o | grep -A 20 "<.text>"
vamp_msg.o:     file format elf64-littleaarch64
Disassembly of section .text:
0000000000000000 <.text>:
   0: d28000a0  mov  x0, #0x5       // fd = stdout
   4: 10000061  adr  x1, 10 <msg>  // buf = &msg
   8: d2800042  mov  x2, #0x5      // count = 5 ("VAMP\n")
   c: d2800808  mov  x8, #0x40     // NR_write = 64
  10: d4000001  svc  #0x0
  14: d2800000  mov  x0, #0x0      // status = 0
  18: d2800ba8  mov  x8, #0x5d     // NR_exit = 93
  1c: d4000001  svc  #0x0
```

### Por qué vamp-shellcode-lab vs. tutoriales pwndbg · bases de datos de shellcode · guías de exploit ARM64

| Capacidad | vamp-shellcode-lab | Tutoriales pwndbg | Bases de datos shellcode | Guías exploit ARM64 |
|-----------|--------------------|--------------------|--------------------------|----------------------|
| Código compilable autocontenido (C + Makefile) | ✅ | ❌ Solo docs | ❌ Solo bytes hex | ❌ Fragmentos sin build |
| ASM inline + array de bytes juntos | ✅ Dos demos comparadas | ❌ | ❌ | ❌ |
| Referencia .s anotada (lista para objdump) | ✅ `vamp_msg.s` | ❌ | ❌ Parcial | ✅ Varía |
| Tabla de syscalls embebida en el código | ✅ 7 syscalls con layout de registros | ❌ | ✅ Tablas externas | ✅ |
| Notas de detección defensiva (ptrace / seccomp / eBPF) | ✅ Comentarios inline | ✅ Específico pwndbg | ❌ | ❌ |
| Docker ARM64 en Apple Silicon (sin toolchain en host) | ✅ `--platform linux/arm64` | ❌ | ❌ | ❌ |
| Sin Python / sin dependencia de framework | ✅ Solo C + gcc + binutils | ❌ requiere pwndbg | ❌ | ❌ |

- **Dos rutas de ejecución** — la comparación lado a lado de ASM inline y shellcode-como-bytes en la misma ejecución hace explícito el salto conceptual: una es código que el compilador embebe, la otra son datos que el programa trata como código.
- **Trazabilidad al nivel de byte** — cada byte del array está comentado de vuelta a su instrucción ARM64; `objdump` sobre `vamp_msg.o` permite verificar la extracción sin aceptar nada a ciegas.
- **Contexto de detección incluido** — notas inline sobre `ptrace`, `seccomp`, `auditd` y `checksec` muestran qué contramedida del host detecta cada técnica, convirtiendo el lab en un puente rojo/azul.
- **Ruta Docker ARM64 sin dependencias** — `docker run --platform linux/arm64` en cualquier Mac Apple Silicon; no se requiere configuración de toolchain en el host, sin peculiaridades de emulación.

### Cobertura educativa

| Técnica | Qué demuestra |
|---------|---------------|
| Convención de llamada AArch64 | Registros de argumento x0–x7, número de syscall x8, FP x29, LR x30, callee-save x19–x28 |
| Syscall Linux directa (write + exit) | Números de syscall 64 y 93; saltando libc / glibc por completo con `svc #0` |
| Ensamblador inline `__asm__ volatile` | Embeber instrucciones ARM64 directamente en C sin una unidad de traducción .s separada |
| `mmap(PROT_READ\|PROT_WRITE\|PROT_EXEC)` | Asignar una región del heap escribible y ejecutable — la técnica canónica de staging de shellcode RWX |
| Cast de puntero a función a shellcode | Hacer cast de `void *` a `void (*)(void)` y saltar a un array de bytes — el modelo de inyección central |
| Bypass NX / W^X (conceptual) | `-z execstack` desactivando la protección de pila GNU; las notas del lab explican PXN y por qué este flag nunca debe aparecer en producción |
| Pipeline de extracción de bytes con `objdump` | `as` → `ld` → `objdump -d` para derivar el array de bytes hardcodeado a partir de ensamblador legible |
| Layout de argumentos de syscall (ARM64) | Mapeo completo registro-argumento para write, read, exit, exit_group, execve, mmap, munmap |
| Superficie de detección defensiva | Filtros `ptrace`, `seccomp`, inspección de `/proc/PID/maps`, `auditd`, tracepoints `eBPF`, salida `checksec` |
| Compilación cruzada / Docker ARM64 | Build nativo en Graviton / Raspberry Pi o contenedor `--platform linux/arm64` en Apple Silicon |

### Parte del toolkit de VampSecure Labs

Esta herramienta forma parte del **Toolkit de Seguridad de VampSecure Labs** — una colección de herramientas de seguridad de grado investigación para pruebas de penetración autorizadas y ejercicios de equipo rojo/azul.

- Toolkit completo: [github.com/belky-me](https://github.com/belky-me)
- Orquestador: [github.com/belky-me/vamp-orchestrator](https://github.com/belky-me/vamp-orchestrator)

### Historial de versiones

| Versión | Cambios principales |
|---------|---------------------|
| v1.1 | README bilingüe (EN/ES) |
| v1.0 | Laboratorio educativo de shellcode ARM64 inicial |

---

© VampSecure Studios — VampSecure Labs Security Research Division  
Uso exclusivo en pruebas de seguridad autorizadas.

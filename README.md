# vamp-shellcode-lab

**VampSecure Labs — Security Research Division**  
Laboratorio de shellcode ARM64: mmap RWX, syscalls directas y ejecución de código nativo.

---

## Descripción

Laboratorio educativo de bajo nivel para entornos Linux ARM64 (AArch64). Demuestra los
conceptos fundamentales sobre los que se construyen técnicas de explotación de memoria:

1. **mmap con permisos RWX** — región de memoria que se puede leer, escribir Y ejecutar
2. **Ensamblador inline en C** — instrucciones ARM64 directas mediante `__asm__`
3. **Syscalls directas al kernel** — sin libc, acceso directo a la ABI de Linux ARM64
4. **Shellcode como bytes** — array C que se copia a memoria ejecutable y se invoca

No es una herramienta ofensiva: el shellcode de demostración únicamente escribe "VAMP" en
stdout y termina. El objetivo es entender los mecanismos, no proporcionar exploits listos.

## Plataformas compatibles

| Plataforma | Compatible |
|------------|------------|
| Linux ARM64 (Raspberry Pi 4/5) | ✓ |
| Linux ARM64 (AWS Graviton, Oracle Ampere) | ✓ |
| macOS Apple Silicon (Docker Linux ARM64) | ✓ |
| macOS Apple Silicon (nativo) | ✗ — diferente ABI y kernel |
| Linux x86/x86_64 | ✗ — instrucciones ARM64 no ejecutables |

## Contenido

| Fichero | Descripción |
|---------|-------------|
| `vamp_shell_lab.c` | Programa principal con dos demos (inline ASM + shellcode bytes) |
| `vamp_msg.s` | Referencia del shellcode en ensamblador puro (para estudio) |
| `Makefile` | Compilación simplificada |

## Compilación

```bash
# En Linux ARM64
make

# Compilar también la referencia ASM pura
make asm

# Limpiar binarios
make clean
```

Equivalente manual:
```bash
# Laboratorio principal
gcc -o vamp_shell_lab vamp_shell_lab.c -z execstack

# Referencia ASM (ensamblador + linker)
as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o
```

## Ejecución

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
─────────────────────────────────────────────────────────────────
[*] Ejecutando syscall write via __asm__...
[*] Salida del shellcode: VAMP
[✓] Syscall completada correctamente.

[DEMO 2] Shellcode como array de bytes — técnica base de inyección
─────────────────────────────────────────────────────────────────
[*] Región RWX asignada en: 0x7f8a3c0000 (37 bytes)
[*] Copiando 37 bytes de shellcode...
[*] Salida del shellcode: VAMP
```

## Conceptos explicados

### mmap RWX

```c
void *mem = mmap(NULL, 1024,
    PROT_READ | PROT_WRITE | PROT_EXEC,
    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
```

`PROT_EXEC` permite ejecutar el contenido de la región como código máquina. En sistemas
con W^X (Write XOR Execute) habilitado a nivel hardware (ARM: PXN/UXN, x86: NX/XD),
esta combinación de RWX es bloqueada por la CPU en modo usuario.

### Syscalls ARM64 directas

```asm
mov x0, #1       // fd = stdout
adr x1, msg      // puntero al string
mov x2, #5       // longitud
mov x8, #64      // número de syscall write en Linux ARM64
svc #0           // supervisor call → trampa al kernel
```

`svc #0` es la instrucción de trampa que transfiere el control al kernel con la syscall
identificada por el número en `x8`. No interviene libc ni ninguna capa de abstracción.

### Tabla de syscalls ARM64 de referencia

| Número | Nombre | Argumentos |
|--------|--------|------------|
| 64 | write | fd, buf, count |
| 63 | read | fd, buf, count |
| 93 | exit | status |
| 94 | exit_group | status |
| 221 | execve | path, argv, envp |
| 192 | mmap | addr, len, prot, flags, fd, offset |

### Por qué -z execstack

El linker GNU aplica por defecto protección `NX` (No-eXecute) a la pila. La opción
`-z execstack` la desactiva, lo que es necesario cuando el código ensamblador inline
queda en la sección de texto pero hace referencia a datos en la pila. En la Demo 2 no
sería estrictamente necesaria porque el shellcode está en heap (mmap), pero se incluye
para que los dos ejemplos funcionen correctamente en todas las variantes de GCC/kernel.

## Contexto de defensa: cómo detectar esta técnica

| Técnica | Detección |
|---------|-----------|
| mmap RWX | `ptrace`, `seccomp`, `/proc/PID/maps` con `rwxp` |
| Syscalls directas | auditoría de syscalls con `auditd` o `eBPF` |
| Segmentos ejecutables | `checksec --file=binario` (detecta `NX disabled`) |
| Shellcode en heap | análisis dinámico con Frida, GDB, ASAN |

## Aviso legal

**Uso exclusivo en entornos de laboratorio propios o con autorización escrita.**  
Los conceptos demostrados son la base de técnicas de explotación reales. Su aplicación
en sistemas sin autorización constituye un delito. VampSecure Studios no se responsabiliza
del uso indebido de este laboratorio.

---

© VampSecure Studios — VampSecure Labs Security Research Division  
Licencia: MIT

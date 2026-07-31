/*
 * vamp_msg.s — Referencia de shellcode ARM64 en ensamblador puro
 *
 * VampSecure Labs — Security Research Division
 * © VampSecure Studios — VampSecure Labs Security Research Division
 *
 * Fichero de REFERENCIA: muestra el shellcode en ensamblador legible
 * antes de convertirlo a bytes hexadecimales para vamp_shell_lab.c
 *
 * COMPILAR (solo para verificar):
 *   as -o vamp_msg.o vamp_msg.s && ld -o vamp_msg vamp_msg.o
 *   ./vamp_msg   (en Linux ARM64)
 *
 * EXTRAER BYTES (para hardcodear en C):
 *   objdump -d vamp_msg.o | grep -A 999 "<.text>"
 */

.section .text
.global _start

_start:
    /* syscall write(1, msg, 5) — escribe "VAMP\n" en stdout */
    mov x0, #1          /* fd = 1 (stdout) */
    adr x1, msg         /* dirección del string */
    mov x2, #5          /* longitud = 5 bytes ("VAMP\n") */
    mov x8, #64         /* número de syscall: write = 64 en Linux ARM64 */
    svc #0              /* trampa al kernel (supervisor call) */

    /* syscall exit(0) — terminar el proceso limpiamente */
    mov x0, #0          /* código de salida = 0 (éxito) */
    mov x8, #93         /* número de syscall: exit = 93 en Linux ARM64 */
    svc #0              /* trampa al kernel */

/* Datos del mensaje (5 bytes: V A M P \n) */
msg:
    .ascii "VAMP\n"

/*
 * TABLA DE SYSCALLS ARM64 LINUX (referencia rápida de laboratorio)
 * ─────────────────────────────────────────────────────────────────
 * Número   Nombre          Prototipo C                    Registros
 * ───────  ──────────      ──────────────────────────     ─────────
 *  64      write           write(fd, buf, count)          x0, x1, x2
 *  63      read            read(fd, buf, count)           x0, x1, x2
 *  93      exit            exit(status)                   x0
 *  94      exit_group      exit_group(status)             x0
 *  221     execve          execve(path, argv, envp)       x0, x1, x2
 *  222     execveat        execveat(dirfd, path, ...)     x0-x4
 *  192     mmap            mmap(addr, len, prot, ...)     x0-x5
 *  215     munmap          munmap(addr, len)              x0, x1
 *
 * CONVENCIÓN DE LLAMADA ARM64 (AArch64):
 *   - x0-x7  : argumentos (y valor de retorno en x0)
 *   - x8     : número de syscall
 *   - x9-x15 : registros temporales (corruptibles)
 *   - x19-x28: registros guardados (callee-save)
 *   - x29    : frame pointer (fp)
 *   - x30    : link register (lr) — dirección de retorno
 *   - sp     : stack pointer
 */

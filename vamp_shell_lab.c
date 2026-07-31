/*
 * vamp_shell_lab.c — Laboratorio de shellcode ARM64
 *
 * VampSecure Labs — Security Research Division
 * © VampSecure Studios — VampSecure Labs Security Research Division
 *
 * Versión: 2.0
 *
 * DESCRIPCIÓN
 * -----------
 * Laboratorio educativo que demuestra los conceptos fundamentales de
 * ejecución de shellcode en arquitectura ARM64 (AArch64) bajo Linux:
 *
 *   1. mmap con permisos RWX (lectura + escritura + ejecución simultáneos)
 *   2. Ensamblador inline en C mediante __asm__
 *   3. Syscalls directas al kernel Linux ARM64 (sin libc)
 *   4. Ejecución de shellcode precalculado como array de bytes
 *
 * PLATAFORMAS COMPATIBLES
 * -----------------------
 * - Linux ARM64 (AArch64): Raspberry Pi 4/5, servidores AWS Graviton,
 *   Oracle Cloud Ampere, etc.
 * - macOS Apple Silicon con Docker Linux ARM64 (no macOS nativo)
 *
 * COMPILACIÓN
 * -----------
 *   gcc -o vamp_shell_lab vamp_shell_lab.c -z execstack
 *
 * La opción -z execstack desactiva la protección NX/XD del linker,
 * que en entornos de producción impide la ejecución de pila. Solo para lab.
 *
 * USO EXCLUSIVO EN ENTORNOS DE LABORATORIO PROPIOS O AUTORIZADOS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define VERSION "2.0"

/* Shellcode ARM64 precalculado: syscall write("VAMP\n") + syscall exit(0)
 *
 * Equivale al siguiente ensamblador ARM64:
 *
 *   mov x0, #1           ; fd = stdout (1)
 *   adr x1, msg          ; dirección del string "VAMP\n"
 *   mov x2, #5           ; longitud = 5 bytes
 *   mov x8, #64          ; syscall write (número 64 en Linux ARM64)
 *   svc #0               ; trap al kernel
 *   mov x0, #0           ; código de salida = 0
 *   mov x8, #93          ; syscall exit (número 93 en Linux ARM64)
 *   svc #0               ; trap al kernel
 *   .ascii "VAMP\n"      ; datos del string (5 bytes)
 *
 * Nota: Los números de syscall varían entre arquitecturas y kernels.
 * En ARM64 Linux: write=64, exit=93, exit_group=94.
 */
static unsigned char shellcode[] = {
    /* mov x0, #1          */ 0x20, 0x00, 0x80, 0xD2,
    /* adr x1, msg         */ 0x41, 0x00, 0x00, 0x10,  /* offset calculado para 8 instrucciones adelante */
    /* mov x2, #5          */ 0xA2, 0x00, 0x80, 0xD2,
    /* mov x8, #64         */ 0x08, 0x08, 0x80, 0xD2,
    /* svc #0              */ 0x01, 0x00, 0x00, 0xD4,
    /* mov x0, #0          */ 0x00, 0x00, 0x80, 0xD2,
    /* mov x8, #93         */ 0xA8, 0x0B, 0x80, 0xD2,
    /* svc #0              */ 0x01, 0x00, 0x00, 0xD4,
    /* "VAMP\n"            */ 0x56, 0x41, 0x4D, 0x50, 0x0A
};

static void imprimir_banner(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║   VampSecure Labs — Shell Lab v%s (ARM64)          ║\n", VERSION);
    printf("║   Laboratorio educativo de shellcode Linux ARM64     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}

/*
 * demo_asm_inline — Demostración con ensamblador inline C (__asm__)
 *
 * Utiliza la extensión GNU __asm__ para incluir instrucciones ARM64
 * directamente en el binario C. El compilador gestiona la alineación
 * y el offset de la etiqueta 'message' automáticamente.
 *
 * Ventaja: el compilador calcula los offsets correctamente.
 * Limitación: el código ensamblador queda incrustado en el texto del binario,
 * no en memoria asignada dinámicamente.
 */
static void demo_asm_inline(void) {
    printf("[DEMO 1] Ensamblador inline — instrucciones ARM64 directas en C\n");
    printf("─────────────────────────────────────────────────────────────────\n");
    printf("[*] Ejecutando syscall write via __asm__...\n");
    printf("[*] Salida del shellcode: ");
    fflush(stdout);

    __asm__ volatile (
        "mov x0, #1          \n"  /* fd = stdout */
        "adr x1, .Lmessage   \n"  /* puntero al string (offset calculado por el compilador) */
        "mov x2, #5          \n"  /* longitud de "VAMP\n" */
        "mov x8, #64         \n"  /* syscall write (Linux ARM64) */
        "svc #0              \n"  /* trampa al kernel */
        "b .Lend             \n"  /* saltar los datos para no intentar ejecutarlos */
        ".Lmessage: .ascii \"VAMP\\n\" \n"
        ".Lend:              \n"
        ::: "x0", "x1", "x2", "x8", "memory"
    );

    printf("[✓] Syscall completada correctamente.\n\n");
}

/*
 * demo_shellcode_bytes — Demostración con shellcode como array de bytes
 *
 * Copia el shellcode precalculado (array de bytes) a una región de memoria
 * con permisos RWX obtenida mediante mmap(), y ejecuta esa región como
 * si fuera una función.
 *
 * Esta es la técnica base de la inyección de shellcode:
 *   1. Obtener región RWX (mmap o VirtualAlloc en Windows)
 *   2. Copiar el shellcode a esa región
 *   3. Saltar a la dirección de inicio del shellcode
 *
 * En producción, las protecciones W^X (Write XOR Execute) del sistema
 * operativo impiden esta técnica a nivel de hardware (NX/XD/PXN).
 */
static void demo_shellcode_bytes(void) {
    printf("[DEMO 2] Shellcode como array de bytes — técnica base de inyección\n");
    printf("─────────────────────────────────────────────────────────────────\n");

    /* Reservar región de memoria con permisos R+W+X */
    void *mem = mmap(
        NULL,                           /* sin dirección preferida, el kernel elige */
        sizeof(shellcode),              /* tamaño: solo lo necesario */
        PROT_READ | PROT_WRITE | PROT_EXEC,  /* RWX: leer, escribir Y ejecutar */
        MAP_ANONYMOUS | MAP_PRIVATE,    /* sin fichero subyacente, privado */
        -1,                             /* sin descriptor de fichero */
        0                               /* sin offset */
    );

    if (mem == MAP_FAILED) {
        /* mmap puede fallar si el kernel tiene políticas de memoria estrictas */
        perror("[-] mmap falló");
        printf("[-] Probable causa: CONFIG_STRICT_DEVMEM o seccomp activo.\n");
        return;
    }

    printf("[*] Región RWX asignada en: %p (%zu bytes)\n", mem, sizeof(shellcode));
    printf("[*] Copiando %zu bytes de shellcode...\n", sizeof(shellcode));

    /* Copiar el shellcode a la región ejecutable */
    memcpy(mem, shellcode, sizeof(shellcode));

    printf("[*] Salida del shellcode: ");
    fflush(stdout);

    /* Ejecutar el shellcode: cast a puntero de función sin argumentos ni retorno.
     * NOTA: El shellcode llama a syscall exit(0) → el proceso termina aquí.
     * En un lab real se usaría una rutina que retorne al código C.
     */
    void (*fn)(void) = (void (*)(void))mem;
    fn();

    /* Si el shellcode no llama a exit(), llegamos aquí */
    printf("[✓] Shellcode ejecutado y control devuelto.\n");

    /* Liberar la región mapeada */
    munmap(mem, sizeof(shellcode));
}

int main(void) {
    imprimir_banner();

    printf("Arquitectura objetivo : ARM64 (AArch64) Linux\n");
    printf("Protecciones activas  : NX desactivado por -z execstack (SOLO LAB)\n\n");

    /* Demostración 1: ensamblador inline en C */
    demo_asm_inline();

    /* Demostración 2: shellcode como bytes en región RWX
     * (el shellcode llama a exit(0), por lo que el proceso termina aquí) */
    demo_shellcode_bytes();

    return 0;
}

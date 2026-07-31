# Makefile — vamp-shellcode-lab
# VampSecure Labs — Security Research Division
#
# USO (en Linux ARM64):
#   make            -> compila el laboratorio principal
#   make asm        -> compila la referencia ASM pura
#   make clean      -> elimina binarios compilados

CC      = gcc
AS      = as
LD      = ld
CFLAGS  = -Wall -Wextra -z execstack
TARGET  = vamp_shell_lab
ASM_SRC = vamp_msg.s
ASM_OBJ = vamp_msg.o
ASM_BIN = vamp_msg

.PHONY: all asm clean

all: $(TARGET)

$(TARGET): vamp_shell_lab.c
	@echo "[*] Compilando laboratorio principal..."
	$(CC) $(CFLAGS) -o $(TARGET) vamp_shell_lab.c
	@echo "[✓] Compilado: ./$(TARGET)"
	@echo "[!] Ejecutar en Linux ARM64 o Docker ARM64. No funciona en x86."

asm: $(ASM_BIN)

$(ASM_BIN): $(ASM_OBJ)
	$(LD) -o $(ASM_BIN) $(ASM_OBJ)
	@echo "[✓] Compilado: ./$(ASM_BIN)"

$(ASM_OBJ): $(ASM_SRC)
	$(AS) -o $(ASM_OBJ) $(ASM_SRC)

clean:
	rm -f $(TARGET) $(ASM_BIN) $(ASM_OBJ)
	@echo "[✓] Binarios eliminados."

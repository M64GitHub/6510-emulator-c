#include "6510.h"
#include <stdio.h>

void cpu_print_registers(CPU *cpu)
{
    printf("[CPU] [%04X] [OP:%02X] [CYC:%1X] A: %02X, X: %02X, Y: %02X, "
           "CYC-TT: %d\n",
           cpu->PC, cpu->opcode_last, cpu->cycles_last_step, cpu->A, cpu->X,
           cpu->Y, cpu->cycles_executed);
}

int main(int argc, char **argv)
{
    CPU cpu1;
    CPU *cpu_ptr = &cpu1;

    printf("Initializing CPU:\n");
    CPU_Init(cpu_ptr, 0x0800);
    cpu_print_registers(cpu_ptr);

    printf("Writing Program ...\n");
    CPU_WriteByte(cpu_ptr, 0xa9, 0x0800); //         LDA
    CPU_WriteByte(cpu_ptr, 0x0a, 0x0801); //             #0A     ; 10
    CPU_WriteByte(cpu_ptr, 0xaa, 0x0802); //         TAX
    CPU_WriteByte(cpu_ptr, 0xe8, 0x0803); // LOOP:   INX
    CPU_WriteByte(cpu_ptr, 0xe0, 0x0804); //         CPX
    CPU_WriteByte(cpu_ptr, 0x14, 0x0805); //             #$14    ; 20
    CPU_WriteByte(cpu_ptr, 0xd0, 0x0806); //         BNE
    CPU_WriteByte(cpu_ptr, 0xfb, 0x0807); //             LOOP
    CPU_WriteByte(cpu_ptr, 0x60, 0x0808); //         RTS
    cpu_print_registers(cpu_ptr);

    printf("Running CPU:\n");

    while (CPU_Run_Step(cpu_ptr)) {
        cpu_print_registers(cpu_ptr);
    }

    return 0;
}

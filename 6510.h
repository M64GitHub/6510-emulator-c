// 6510.h 2025-03 M64
#ifndef M64_6510_EMU_H
#define M64_6510_EMU_H

typedef struct S_CPUFlags {
    unsigned char C;      // 0: Carry Flag
    unsigned char Z;      // 1: Zero Flag
    unsigned char I;      // 2: Interrupt disable
    unsigned char D;      // 3: Decimal mode
    unsigned char B;      // 4: Break
    unsigned char Unused; // 5: Unused
    unsigned char V;      // 6: Overflow
    unsigned char N;      // 7: Negative
} CPUFlags_t;

typedef struct S_Mem64k {
    unsigned char Data[0x10000]; // 64K RAM
} MEM64K_t;

void MEM_Init(MEM64K_t *mem);

typedef struct S_CPU {
    unsigned short PC; // program counter
    unsigned char SP;  // stack pointer

    unsigned char A, X, Y; // registers

    char Status;
    CPUFlags_t Flags;

    MEM64K_t memdata;
    MEM64K_t *mem;

    unsigned int cycles_executed;
    unsigned int cycles_last_step;
    unsigned char opcode_last;
} CPU;

// general cpu functions
void CPU_Reset(CPU *cpu);
void CPU_Init(CPU *cpu, unsigned short PC_init);
void CPU_FlagsToPS(CPU *cpu);
void CPU_PSToFlags(CPU *cpu);
char CPU_FetchByte(CPU *cpu);
unsigned char CPU_FetchUByte(CPU *cpu);
unsigned short CPU_FetchWord(CPU *cpu);
unsigned char CPU_ReadByte(CPU *cpu, unsigned short Address);
unsigned short CPU_ReadWord(CPU *cpu, unsigned short Address);
void CPU_WriteByte(CPU *cpu, unsigned char Value, unsigned short Address);
void CPU_WriteWord(CPU *cpu, unsigned short Value, unsigned short Address);
unsigned short CPU_SPToAddress(CPU *cpu);
void CPU_PushWordToStack(CPU *cpu, unsigned short Value);
void CPU_PushPCToStack(CPU *cpu);
void CPU_PushByteOntoStack(CPU *cpu, unsigned char Value);
unsigned char CPU_PopByteFromStack(CPU *cpu);
unsigned short CPU_PopWordFromStack(CPU *cpu);
void CPU_UpdateFlags(CPU *cpu, unsigned char Register);
unsigned short CPU_LoadPrg(CPU *cpu, const char *Program,
                           unsigned int NumBytes);

// cpu execution related functions

void CPU_LoadRegister(CPU *cpu, unsigned short Address,
                      unsigned char *Register);
void CPU_And(CPU *cpu, unsigned short Address);
void CPU_Ora(CPU *cpu, unsigned short Address);
void CPU_Xor(CPU *cpu, unsigned short Address);
void CPU_Branch(CPU *cpu, char Test, char Expected);
void CPU_ADC(CPU *cpu, char Operand);
void CPU_SBC(CPU *cpu, char Operand);
char CPU_ASL(CPU *cpu, char Operand);
char CPU_LSR(CPU *cpu, char Operand);
char CPU_ROL(CPU *cpu, char Operand);
char CPU_ROR(CPU *cpu, char Operand);
void CPU_PushPSToStack(CPU *cpu);
void CPU_PopPSFromStack(CPU *cpu);

char CPU_Run_Step(CPU *cpu);

// cpu addressing mode functions

// return the number of cycles that were used
signed int CPU_Execute(CPU *cpu);

unsigned short CPU_AddrZeroPage(CPU *cpu);
unsigned short CPU_AddrZeroPageX(CPU *cpu);
unsigned short CPU_AddrZeroPageY(CPU *cpu);
unsigned short CPU_AddrAbsolute(CPU *cpu);
unsigned short CPU_AddrAbsoluteX(CPU *cpu);
unsigned short CPU_AddrAbsoluteX_5(CPU *cpu);
unsigned short CPU_AddrAbsoluteY(CPU *cpu);
unsigned short CPU_AddrAbsoluteY_5(CPU *cpu);
unsigned short CPU_AddrIndirectX(CPU *cpu);
unsigned short CPU_AddrIndirectY(CPU *cpu);
unsigned short CPU_AddrIndirectY_6(CPU *cpu);

// OPCODES

#define INSN_LDA_IM ((unsigned char)0xA9)
#define INSN_LDA_ZP ((unsigned char)0xA5)
#define INSN_LDA_ZPX ((unsigned char)0xB5)
#define INSN_LDA_ABS ((unsigned char)0xAD)
#define INSN_LDA_ABSX ((unsigned char)0xBD)
#define INSN_LDA_ABSY ((unsigned char)0xB9)
#define INSN_LDA_INDX ((unsigned char)0xA1)
#define INSN_LDA_INDY ((unsigned char)0xB1)

#define INSN_LDX_IM ((unsigned char)0xA2)
#define INSN_LDX_ZP ((unsigned char)0xA6)
#define INSN_LDX_ZPY ((unsigned char)0xB6)
#define INSN_LDX_ABS ((unsigned char)0xAE)
#define INSN_LDX_ABSY ((unsigned char)0xBE)

#define INSN_LDY_IM ((unsigned char)0xA0)
#define INSN_LDY_ZP ((unsigned char)0xA4)
#define INSN_LDY_ZPX ((unsigned char)0xB4)
#define INSN_LDY_ABS ((unsigned char)0xAC)
#define INSN_LDY_ABSX ((unsigned char)0xBC)

#define INSN_STA_ZP ((unsigned char)0x85)
#define INSN_STA_ZPX ((unsigned char)0x95)
#define INSN_STA_ABS ((unsigned char)0x8D)
#define INSN_STA_ABSX ((unsigned char)0x9D)
#define INSN_STA_ABSY ((unsigned char)0x99)
#define INSN_STA_INDX ((unsigned char)0x81)
#define INSN_STA_INDY ((unsigned char)0x91)

#define INSN_STX_ZP ((unsigned char)0x86)
#define INSN_STX_ZPY ((unsigned char)0x96)
#define INSN_STX_ABS ((unsigned char)0x8E)
#define INSN_STY_ZP ((unsigned char)0x84)
#define INSN_STY_ZPX ((unsigned char)0x94)
#define INSN_STY_ABS ((unsigned char)0x8C)

#define INSN_TSX ((unsigned char)0xBA)
#define INSN_TXS ((unsigned char)0x9A)
#define INSN_PHA ((unsigned char)0x48)
#define INSN_PLA ((unsigned char)0x68)

#define INSN_PHP ((unsigned char)0x08)
#define INSN_PLP ((unsigned char)0x28)

#define INSN_JMP_ABS ((unsigned char)0x4C)
#define INSN_JMP_IND ((unsigned char)0x6C)
#define INSN_JSR ((unsigned char)0x20)
#define INSN_RTS ((unsigned char)0x60)

#define INSN_AND_IM ((unsigned char)0x29)
#define INSN_AND_ZP ((unsigned char)0x25)
#define INSN_AND_ZPX ((unsigned char)0x35)
#define INSN_AND_ABS ((unsigned char)0x2D)
#define INSN_AND_ABSX ((unsigned char)0x3D)
#define INSN_AND_ABSY ((unsigned char)0x39)
#define INSN_AND_INDX ((unsigned char)0x21)
#define INSN_AND_INDY ((unsigned char)0x31)

#define INSN_ORA_IM ((unsigned char)0x09)
#define INSN_ORA_ZP ((unsigned char)0x05)
#define INSN_ORA_ZPX ((unsigned char)0x15)
#define INSN_ORA_ABS ((unsigned char)0x0D)
#define INSN_ORA_ABSX ((unsigned char)0x1D)
#define INSN_ORA_ABSY ((unsigned char)0x19)
#define INSN_ORA_INDX ((unsigned char)0x01)
#define INSN_ORA_INDY ((unsigned char)0x11)

#define INSN_XOR_IM ((unsigned char)0x49)
#define INSN_XOR_ZP ((unsigned char)0x45)
#define INSN_XOR_ZPX ((unsigned char)0x55)
#define INSN_XOR_ABS ((unsigned char)0x4D)
#define INSN_XOR_ABSX ((unsigned char)0x5D)
#define INSN_XOR_ABSY ((unsigned char)0x59)
#define INSN_XOR_INDX ((unsigned char)0x41)
#define INSN_XOR_INDY ((unsigned char)0x51)

#define INSN_BIT_ZP ((unsigned char)0x24)
#define INSN_BIT_ABS ((unsigned char)0x2C)

#define INSN_TAX ((unsigned char)0xAA)
#define INSN_TAY ((unsigned char)0xA8)
#define INSN_TXA ((unsigned char)0x8A)
#define INSN_TYA ((unsigned char)0x98)

#define INSN_INX ((unsigned char)0xE8)
#define INSN_INY ((unsigned char)0xC8)
#define INSN_DEY ((unsigned char)0x88)
#define INSN_DEX ((unsigned char)0xCA)
#define INSN_DEC_ZP ((unsigned char)0xC6)
#define INSN_DEC_ZPX ((unsigned char)0xD6)
#define INSN_DEC_ABS ((unsigned char)0xCE)
#define INSN_DEC_ABSX ((unsigned char)0xDE)
#define INSN_INC_ZP ((unsigned char)0xE6)
#define INSN_INC_ZPX ((unsigned char)0xF6)
#define INSN_INC_ABS ((unsigned char)0xEE)
#define INSN_INC_ABSX ((unsigned char)0xFE)

#define INSN_BEQ ((unsigned char)0xF0)
#define INSN_BNE ((unsigned char)0xD0)
#define INSN_BCS ((unsigned char)0xB0)
#define INSN_BCC ((unsigned char)0x90)
#define INSN_BMI ((unsigned char)0x30)
#define INSN_BPL ((unsigned char)0x10)
#define INSN_BVC ((unsigned char)0x50)
#define INSN_BVS ((unsigned char)0x70)

#define INSN_CLC ((unsigned char)0x18)
#define INSN_SEC ((unsigned char)0x38)
#define INSN_CLD ((unsigned char)0xD8)
#define INSN_SED ((unsigned char)0xF8)
#define INSN_CLI ((unsigned char)0x58)
#define INSN_SEI ((unsigned char)0x78)
#define INSN_CLV ((unsigned char)0xB8)

#define INSN_ADC ((unsigned char)0x69)
#define INSN_ADC_ZP ((unsigned char)0x65)
#define INSN_ADC_ZPX ((unsigned char)0x75)
#define INSN_ADC_ABS ((unsigned char)0x6D)
#define INSN_ADC_ABSX ((unsigned char)0x7D)
#define INSN_ADC_ABSY ((unsigned char)0x79)
#define INSN_ADC_INDX ((unsigned char)0x61)
#define INSN_ADC_INDY ((unsigned char)0x71)

#define INSN_SBC ((unsigned char)0xE9)
#define INSN_SBC_ABS ((unsigned char)0xED)
#define INSN_SBC_ZP ((unsigned char)0xE5)
#define INSN_SBC_ZPX ((unsigned char)0xF5)
#define INSN_SBC_ABSX ((unsigned char)0xFD)
#define INSN_SBC_ABSY ((unsigned char)0xF9)
#define INSN_SBC_INDX ((unsigned char)0xE1)
#define INSN_SBC_INDY ((unsigned char)0xF1)

#define INSN_CMP ((unsigned char)0xC9)
#define INSN_CMP_ZP ((unsigned char)0xC5)
#define INSN_CMP_ZPX ((unsigned char)0xD5)
#define INSN_CMP_ABS ((unsigned char)0xCD)
#define INSN_CMP_ABSX ((unsigned char)0xDD)
#define INSN_CMP_ABSY ((unsigned char)0xD9)
#define INSN_CMP_INDX ((unsigned char)0xC1)
#define INSN_CMP_INDY ((unsigned char)0xD1)
#define INSN_CPX ((unsigned char)0xE0)
#define INSN_CPY ((unsigned char)0xC0)
#define INSN_CPX_ZP ((unsigned char)0xE4)
#define INSN_CPY_ZP ((unsigned char)0xC4)

#define INSN_CPX_ABS ((unsigned char)0xEC)
#define INSN_CPY_ABS ((unsigned char)0xCC)

#define INSN_ASL ((unsigned char)0x0A)
#define INSN_ASL_ZP ((unsigned char)0x06)
#define INSN_ASL_ZPX ((unsigned char)0x16)
#define INSN_ASL_ABS ((unsigned char)0x0E)
#define INSN_ASL_ABSX ((unsigned char)0x1E)
#define INSN_LSR ((unsigned char)0x4A)
#define INSN_LSR_ZP ((unsigned char)0x46)
#define INSN_LSR_ZPX ((unsigned char)0x56)
#define INSN_LSR_ABS ((unsigned char)0x4E)
#define INSN_LSR_ABSX ((unsigned char)0x5E)
#define INSN_ROL ((unsigned char)0x2A)
#define INSN_ROL_ZP ((unsigned char)0x26)
#define INSN_ROL_ZPX ((unsigned char)0x36)
#define INSN_ROL_ABS ((unsigned char)0x2E)
#define INSN_ROL_ABSX ((unsigned char)0x3E)
#define INSN_ROR ((unsigned char)0x6A)
#define INSN_ROR_ZP ((unsigned char)0x66)
#define INSN_ROR_ZPX ((unsigned char)0x76)
#define INSN_ROR_ABS ((unsigned char)0x6E)
#define INSN_ROR_ABSX ((unsigned char)0x7E)

#define INSN_NOP ((unsigned char)0xEA)
#define INSN_BRK ((unsigned char)0x00)
#define INSN_RTI ((unsigned char)0x40)

// Status bits
#define FB_Negative ((unsigned char)0b10000000)
#define FB_Overflow ((unsigned char)0b01000000)
#define FB_Unused ((unsigned char)0b000100000)
#define FB_Break ((unsigned char)0b000010000)
#define FB_Decimal ((unsigned char)0b000001000)
#define FB_InterruptDisable ((unsigned char)0b000000100)
#define FB_Zero ((unsigned char)0b000000010)
#define FB_Carry ((unsigned char)0b000000001)

#endif

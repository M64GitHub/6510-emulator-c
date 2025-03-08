// 6510.c 2025-03 M64
#include "6510.h"

// -- MEM

void MEM_Init(MEM64K_t *mem)
{
    for (int i = 0; i < 0x10000; i++) {
        mem->Data[i] = 0;
    }
}

// -- CPU

void CPU_Init(CPU *cpu, unsigned short PC_init)
{
    cpu->PC = PC_init;
    cpu->SP = 0xFF;
    cpu->Flags.C = cpu->Flags.Z = cpu->Flags.I = cpu->Flags.D = cpu->Flags.B =
        cpu->Flags.V = cpu->Flags.N = 0;
    cpu->A = cpu->X = cpu->Y = 0;
    cpu->cycles_executed = 0;
    cpu->cycles_last_step = 0;
    cpu->opcode_last = 0x00;
    cpu->mem = &cpu->memdata;
    MEM_Init(cpu->mem);
}

void CPU_Reset(CPU *cpu)
{
    CPU_Init(cpu, 0xFFFC);
}

unsigned char CPU_FetchUByte(CPU *cpu)
{
    unsigned char Data = cpu->mem->Data[cpu->PC];
    cpu->PC++;
    cpu->cycles_executed++;
    return Data;
}

char CPU_FetchByte(CPU *cpu)
{
    return CPU_FetchUByte(cpu);
}

unsigned short CPU_FetchWord(CPU *cpu)
{
    // little endian
    unsigned short Data = cpu->mem->Data[cpu->PC];
    cpu->PC++;

    Data |= (cpu->mem->Data[cpu->PC] << 8);
    cpu->PC++;

    cpu->cycles_executed += 2;
    return Data;
}

unsigned char CPU_ReadByte(CPU *cpu, unsigned short Address)
{
    unsigned char Data = cpu->mem->Data[Address];
    cpu->cycles_executed++;
    return Data;
}

unsigned short CPU_ReadWord(CPU *cpu, unsigned short Address)
{
    unsigned char LoByte = CPU_ReadByte(cpu, Address);
    unsigned char HiByte = CPU_ReadByte(cpu, Address + 1);
    return LoByte | (HiByte << 8);
}

void CPU_WriteByte(CPU *cpu, unsigned char Value, unsigned short Address)
{
    cpu->mem->Data[Address] = Value;
    cpu->cycles_executed++;
}

void CPU_WriteWord(CPU *cpu, unsigned short Value, unsigned short Address)
{
    cpu->mem->Data[Address] = Value & 0xFF;
    cpu->mem->Data[Address + 1] = (Value >> 8);
    cpu->cycles_executed += 2;
}

unsigned short CPU_SPToAddress(CPU *cpu)
{
    return 0x100 | cpu->SP;
}

void CPU_PushWordToStack(CPU *cpu, unsigned short Value)
{
    CPU_WriteByte(cpu, Value >> 8, CPU_SPToAddress(cpu));
    cpu->SP--;
    CPU_WriteByte(cpu, Value & 0xFF, CPU_SPToAddress(cpu));
    cpu->SP--;
}

void CPU_PushPCToStack(CPU *cpu)
{
    CPU_PushWordToStack(cpu, cpu->PC);
}

void CPU_PushByteOntoStack(CPU *cpu, unsigned char Value)
{
    const unsigned short SPWord = CPU_SPToAddress(cpu);
    cpu->mem->Data[SPWord] = Value;
    cpu->cycles_executed++;
    cpu->SP--;
    cpu->cycles_executed++;
}

unsigned char CPU_PopByteFromStack(CPU *cpu)
{
    cpu->SP++;
    cpu->cycles_executed++;
    const unsigned short SPWord = CPU_SPToAddress(cpu);
    unsigned char Value = cpu->mem->Data[SPWord];
    cpu->cycles_executed++;
    return Value;
}

unsigned short CPU_PopWordFromStack(CPU *cpu)
{
    unsigned short ValueFromStack =
        CPU_ReadWord(cpu, CPU_SPToAddress(cpu) + 1);
    cpu->SP += 2;
    cpu->cycles_executed++;
    return ValueFromStack;
}

void CPU_UpdateFlags(CPU *cpu, unsigned char Register)
{
    cpu->Flags.Z = (Register == 0);
    cpu->Flags.N = (Register & FB_Negative) > 0;
}

unsigned short CPU_LoadPrg(CPU *cpu, const char *Program,
                           unsigned int NumBytes)
{
    unsigned short LoadAddress = 0;
    if (Program && (NumBytes > 2)) {
        unsigned int At = 0;
        const unsigned short Lo = Program[At++];
        const unsigned short Hi = Program[At++] << 8;
        LoadAddress = Lo | Hi;
        for (unsigned short i = LoadAddress; i < LoadAddress + NumBytes - 2;
             i++) {
            cpu->mem->Data[i] = Program[At++];
        }
    }

    return LoadAddress;
}

unsigned short CPU_AddrZeroPage(CPU *cpu)
{
    char ZeroPageAddr = CPU_FetchUByte(cpu);
    return ZeroPageAddr;
}

unsigned short CPU_AddrZeroPageX(CPU *cpu)
{
    char ZeroPageAddr = CPU_FetchUByte(cpu);
    ZeroPageAddr += cpu->X;
    cpu->cycles_executed++;
    return ZeroPageAddr;
}

unsigned short CPU_AddrZeroPageY(CPU *cpu)
{
    char ZeroPageAddr = CPU_FetchUByte(cpu);
    ZeroPageAddr += cpu->Y;
    cpu->cycles_executed++;
    return ZeroPageAddr;
}

unsigned short CPU_AddrAbsolute(CPU *cpu)
{
    unsigned short AbsAddress = CPU_FetchWord(cpu);
    return AbsAddress;
}

unsigned short CPU_AddrAbsoluteX(CPU *cpu)
{
    unsigned short AbsAddress = CPU_FetchWord(cpu);
    unsigned short AbsAddressX = AbsAddress + cpu->X;
    char CrossedPageBoundary = (AbsAddress ^ AbsAddressX) >> 8;
    if (CrossedPageBoundary) {
        cpu->cycles_executed++;
    }

    return AbsAddressX;
}

unsigned short CPU_AddrAbsoluteX_5(CPU *cpu)
{
    unsigned short AbsAddress = CPU_FetchWord(cpu);
    unsigned short AbsAddressX = AbsAddress + cpu->X;
    cpu->cycles_executed++;
    return AbsAddressX;
}

unsigned short CPU_AddrAbsoluteY(CPU *cpu)
{
    unsigned short AbsAddress = CPU_FetchWord(cpu);
    unsigned short AbsAddressY = AbsAddress + cpu->Y;
    char CrossedPageBoundary = (AbsAddress ^ AbsAddressY) >> 8;
    if (CrossedPageBoundary) {
        cpu->cycles_executed++;
    }

    return AbsAddressY;
}

unsigned short CPU_AddrAbsoluteY_5(CPU *cpu)
{
    unsigned short AbsAddress = CPU_FetchWord(cpu);
    unsigned short AbsAddressY = AbsAddress + cpu->Y;
    cpu->cycles_executed++;
    return AbsAddressY;
}

unsigned short CPU_AddrIndirectX(CPU *cpu)
{
    char ZPAddress = CPU_FetchUByte(cpu);
    ZPAddress += cpu->X;
    cpu->cycles_executed++;
    unsigned short EffectiveAddr = CPU_ReadWord(cpu, ZPAddress);
    return EffectiveAddr;
}

unsigned short CPU_AddrIndirectY(CPU *cpu)
{
    char ZPAddress = CPU_FetchUByte(cpu);
    unsigned short EffectiveAddr = CPU_ReadWord(cpu, ZPAddress);
    unsigned short EffectiveAddrY = EffectiveAddr + cpu->Y;
    char CrossedPageBoundary = (EffectiveAddr ^ EffectiveAddrY) >> 8;
    if (CrossedPageBoundary) {
        cpu->cycles_executed++;
    }
    return EffectiveAddrY;
}

unsigned short CPU_AddrIndirectY_6(CPU *cpu)
{
    char ZPAddress = CPU_FetchUByte(cpu);
    unsigned short EffectiveAddr = CPU_ReadWord(cpu, ZPAddress);
    unsigned short EffectiveAddrY = EffectiveAddr + cpu->Y;
    cpu->cycles_executed++;
    return EffectiveAddrY;
}

void CPU_LoadRegister(CPU *cpu, unsigned short Address,
                      unsigned char *Register)
{
    *Register = CPU_ReadByte(cpu, Address);
    CPU_UpdateFlags(cpu, *Register);
};

void CPU_And(CPU *cpu, unsigned short Address)
{
    cpu->A &= CPU_ReadByte(cpu, Address);
    CPU_UpdateFlags(cpu, cpu->A);
};

void CPU_Ora(CPU *cpu, unsigned short Address)
{
    cpu->A |= CPU_ReadByte(cpu, Address);
    CPU_UpdateFlags(cpu, cpu->A);
};

void CPU_Xor(CPU *cpu, unsigned short Address)
{
    cpu->A ^= CPU_ReadByte(cpu, Address);
    CPU_UpdateFlags(cpu, cpu->A);
};

void CPU_Branch(CPU *cpu, char Test, char Expected)
{
    char Offset = CPU_FetchByte(cpu);
    if (Test == Expected) {
        const unsigned short PCOld = cpu->PC;
        cpu->PC += Offset;
        cpu->cycles_executed++;

        char PageChanged = (cpu->PC >> 8) != (PCOld >> 8);
        if (PageChanged) {
            cpu->cycles_executed++;
        }
    }
};

void CPU_ADC(CPU *cpu, char Operand)
{
    char AreSignBitsTheSame = !((cpu->A ^ Operand) & FB_Negative);
    unsigned short Sum = cpu->A;
    Sum += Operand;
    Sum += cpu->Flags.C;
    cpu->A = (Sum & 0xFF);
    CPU_UpdateFlags(cpu, cpu->A);
    cpu->Flags.C = Sum > 0xFF;
    cpu->Flags.V = AreSignBitsTheSame && ((cpu->A ^ Operand) & FB_Negative);
};

void CPU_SBC(CPU *cpu, char Operand)
{
    CPU_ADC(cpu, ~Operand);
};

void CPU_RegisterCompare(CPU *cpu, char Operand, char RegisterValue)
{
    char Temp = RegisterValue - Operand;
    cpu->Flags.N = (Temp & FB_Negative) > 0;
    cpu->Flags.Z = RegisterValue == Operand;
    cpu->Flags.C = RegisterValue >= Operand;
};

char CPU_ASL(CPU *cpu, char Operand)
{
    cpu->Flags.C = (Operand & FB_Negative) > 0;
    char Result = Operand << 1;
    CPU_UpdateFlags(cpu, Result);
    cpu->cycles_executed++;
    return Result;
};

char CPU_LSR(CPU *cpu, char Operand)
{
    cpu->Flags.C = (Operand & FB_Zero) > 0;
    char Result = Operand >> 1;
    CPU_UpdateFlags(cpu, Result);
    cpu->cycles_executed++;
    return Result;
};

char CPU_ROL(CPU *cpu, char Operand)
{
    char NewBit0 = cpu->Flags.C ? FB_Zero : 0;
    cpu->Flags.C = (Operand & FB_Negative) > 0;
    Operand = Operand << 1;
    Operand |= NewBit0;
    CPU_UpdateFlags(cpu, Operand);
    cpu->cycles_executed++;
    return Operand;
};

char CPU_ROR(CPU *cpu, char Operand)
{
    char OldBit0 = (Operand & FB_Zero) > 0;
    Operand = Operand >> 1;
    if (cpu->Flags.C) {
        Operand |= FB_Negative;
    }
    cpu->cycles_executed++;
    cpu->Flags.C = OldBit0;
    CPU_UpdateFlags(cpu, Operand);
    return Operand;
};

void CPU_PushPSToStack(CPU *cpu)
{
    CPU_FlagsToPS(cpu);
    char PSStack = cpu->Status | FB_Break | FB_Unused;
    CPU_PushByteOntoStack(cpu, PSStack);
};

void CPU_PopPSFromStack(CPU *cpu)
{
    cpu->Status = CPU_PopByteFromStack(cpu);
    CPU_PSToFlags(cpu);
    cpu->Flags.B = 0;
    cpu->Flags.Unused = 0;
};

void CPU_PSToFlags(CPU *cpu)
{
    cpu->Flags.Unused = (cpu->Status & FB_Unused) != 0;
    cpu->Flags.C = (cpu->Status & FB_Carry) != 0;
    cpu->Flags.Z = (cpu->Status & FB_Zero) != 0;
    cpu->Flags.I = (cpu->Status & FB_InterruptDisable) != 0;
    cpu->Flags.D = (cpu->Status & FB_Decimal) != 0;
    cpu->Flags.B = (cpu->Status & FB_Break) != 0;
    cpu->Flags.V = (cpu->Status & FB_Overflow) != 0;
    cpu->Flags.N = (cpu->Status & FB_Negative) != 0;
}

void CPU_FlagsToPS(CPU *cpu)
{
    char ps = 0x00;
    if (cpu->Flags.Unused) ps |= FB_Unused;
    if (cpu->Flags.C) ps |= FB_Unused;
    if (cpu->Flags.Z) ps |= FB_Zero;
    if (cpu->Flags.I) ps |= FB_InterruptDisable;
    if (cpu->Flags.D) ps |= FB_Decimal;
    if (cpu->Flags.B) ps |= FB_Break;
    if (cpu->Flags.V) ps |= FB_Overflow;
    if (cpu->Flags.N) ps |= FB_Negative;

    cpu->Status = ps;
}

// -- -------------------------------------------------------------------- --

char CPU_Run_Step(CPU *cpu)
{
    unsigned int cycles_now = cpu->cycles_executed;
    unsigned char opcode = CPU_FetchUByte(cpu);
    cpu->opcode_last = opcode;

    switch (opcode) {
    case INSN_AND_IM:
        cpu->A &= CPU_FetchUByte(cpu);
        CPU_UpdateFlags(cpu, cpu->A);
        break;
    case INSN_ORA_IM:
        cpu->A |= CPU_FetchUByte(cpu);
        CPU_UpdateFlags(cpu, cpu->A);
        break;
    case INSN_XOR_IM:
        cpu->A ^= CPU_FetchUByte(cpu);
        CPU_UpdateFlags(cpu, cpu->A);
        break;
    case INSN_AND_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_Xor(cpu, Address);
    } break;
    case INSN_AND_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_Xor(cpu, Address);
    } break;
    case INSN_AND_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_Xor(cpu, Address);
    } break;
    case INSN_AND_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        CPU_Xor(cpu, Address);
    } break;
    case INSN_AND_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        CPU_Xor(cpu, Address);
    } break;
    case INSN_AND_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        CPU_Xor(cpu, Address);
    } break;
    case INSN_AND_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        CPU_And(cpu, Address);
    } break;
    case INSN_ORA_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        CPU_Ora(cpu, Address);
    } break;
    case INSN_XOR_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        CPU_Xor(cpu, Address);
    } break;

    case INSN_BIT_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        cpu->Flags.Z = !(cpu->A & Value);
        cpu->Flags.N = (Value & FB_Negative) != 0;
        cpu->Flags.V = (Value & FB_Overflow) != 0;
    } break;
    case INSN_BIT_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        cpu->Flags.Z = !(cpu->A & Value);
        cpu->Flags.N = (Value & FB_Negative) != 0;
        cpu->Flags.V = (Value & FB_Overflow) != 0;
    } break;
    case INSN_LDA_IM: {
        cpu->A = CPU_FetchUByte(cpu);
        CPU_UpdateFlags(cpu, cpu->A);
    } break;
    case INSN_LDX_IM: {
        cpu->X = CPU_FetchUByte(cpu);
        CPU_UpdateFlags(cpu, cpu->X);
    } break;
    case INSN_LDY_IM: {
        cpu->Y = CPU_FetchUByte(cpu);
        CPU_UpdateFlags(cpu, cpu->Y);
    } break;
    case INSN_LDA_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_LDX_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->X);
    } break;
    case INSN_LDX_ZPY: {
        unsigned short Address = CPU_AddrZeroPageY(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->X);
    } break;
    case INSN_LDY_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->Y);
    } break;
    case INSN_LDA_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_LDY_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->Y);
    } break;
    case INSN_LDA_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_LDX_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->X);
    } break;
    case INSN_LDY_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->Y);
    } break;
    case INSN_LDA_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_LDY_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->Y);
    } break;
    case INSN_LDA_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_LDX_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->X);
    } break;
    case INSN_LDA_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_STA_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_LDA_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        CPU_LoadRegister(cpu, Address, &cpu->A);
    } break;
    case INSN_STA_INDY: {
        unsigned short Address = CPU_AddrIndirectY_6(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_STA_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_STX_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_WriteByte(cpu, cpu->X, Address);
    } break;
    case INSN_STX_ZPY: {
        unsigned short Address = CPU_AddrZeroPageY(cpu);
        CPU_WriteByte(cpu, cpu->X, Address);
    } break;
    case INSN_STY_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        CPU_WriteByte(cpu, cpu->Y, Address);
    } break;
    case INSN_STA_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_STX_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_WriteByte(cpu, cpu->X, Address);
    } break;
    case INSN_STY_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        CPU_WriteByte(cpu, cpu->Y, Address);
    } break;
    case INSN_STA_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_STY_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        CPU_WriteByte(cpu, cpu->Y, Address);
    } break;
    case INSN_STA_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_STA_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY_5(cpu);
        CPU_WriteByte(cpu, cpu->A, Address);
    } break;
    case INSN_JSR: {
        unsigned short SubAddr = CPU_FetchWord(cpu);
        CPU_PushWordToStack(cpu, cpu->PC - 1);
        cpu->PC = SubAddr;
        cpu->cycles_executed++;
    } break;
    case INSN_RTS: {
        unsigned short ReturnAddress = CPU_PopWordFromStack(cpu);
        cpu->PC = ReturnAddress + 1;
        cpu->cycles_executed += 2;
    } break;
    case INSN_JMP_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        cpu->PC = Address;
    } break;
    case INSN_JMP_IND: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        Address = CPU_ReadWord(cpu, Address);
        cpu->PC = Address;
    } break;
    case INSN_TSX: {
        cpu->X = cpu->SP;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->X);
    } break;
    case INSN_TXS: {
        cpu->SP = cpu->X;
        cpu->cycles_executed++;
    } break;
    case INSN_PHA: {
        CPU_PushByteOntoStack(cpu, cpu->A);
    } break;
    case INSN_PLA: {
        cpu->A = CPU_PopByteFromStack(cpu);
        CPU_UpdateFlags(cpu, cpu->A);
        cpu->cycles_executed++;
    } break;
    case INSN_PHP: {
        CPU_PushPSToStack(cpu);
    } break;
    case INSN_PLP: {
        CPU_PopPSFromStack(cpu);
        cpu->cycles_executed++;
    } break;
    case INSN_TAX: {
        cpu->X = cpu->A;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->X);
    } break;
    case INSN_TAY: {
        cpu->Y = cpu->A;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->Y);
    } break;
    case INSN_TXA: {
        cpu->A = cpu->X;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->A);
    } break;
    case INSN_TYA: {
        cpu->A = cpu->Y;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->A);
    } break;
    case INSN_INX: {
        cpu->X++;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->X);
    } break;
    case INSN_INY: {
        cpu->Y++;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->Y);
    } break;
    case INSN_DEX: {
        cpu->X--;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->X);
    } break;
    case INSN_DEY: {
        cpu->Y--;
        cpu->cycles_executed++;
        CPU_UpdateFlags(cpu, cpu->Y);
    } break;
    case INSN_DEC_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value--;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_DEC_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value--;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_DEC_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value--;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_DEC_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value--;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_INC_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value++;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_INC_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value++;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_INC_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value++;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_INC_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        char Value = CPU_ReadByte(cpu, Address);
        Value++;
        cpu->cycles_executed++;
        CPU_WriteByte(cpu, Value, Address);
        CPU_UpdateFlags(cpu, Value);
    } break;
    case INSN_BEQ: {
        CPU_Branch(cpu, cpu->Flags.Z, 1);
    } break;
    case INSN_BNE: {
        CPU_Branch(cpu, cpu->Flags.Z, 0);
    } break;
    case INSN_BCS: {
        CPU_Branch(cpu, cpu->Flags.C, 1);
    } break;
    case INSN_BCC: {
        CPU_Branch(cpu, cpu->Flags.C, 0);
    } break;
    case INSN_BMI: {
        CPU_Branch(cpu, cpu->Flags.N, 1);
    } break;
    case INSN_BPL: {
        CPU_Branch(cpu, cpu->Flags.N, 0);
    } break;
    case INSN_BVC: {
        CPU_Branch(cpu, cpu->Flags.V, 0);
    } break;
    case INSN_BVS: {
        CPU_Branch(cpu, cpu->Flags.V, 1);
    } break;
    case INSN_CLC: {
        cpu->Flags.C = 0;
        cpu->cycles_executed++;
    } break;
    case INSN_SEC: {
        cpu->Flags.C = 1;
        cpu->cycles_executed++;
    } break;
    case INSN_CLD: {
        cpu->Flags.D = 0;
        cpu->cycles_executed++;
    } break;
    case INSN_SED: {
        cpu->Flags.D = 1;
        cpu->cycles_executed++;
    } break;
    case INSN_CLI: {
        cpu->Flags.I = 0;
        cpu->cycles_executed++;
    } break;
    case INSN_SEI: {
        cpu->Flags.I = 1;
        cpu->cycles_executed++;
    } break;
    case INSN_CLV: {
        cpu->Flags.V = 0;
        cpu->cycles_executed++;
    } break;
    case INSN_NOP: {
        cpu->cycles_executed++;
    } break;
    case INSN_ADC_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_ADC: {
        char Operand = CPU_FetchUByte(cpu);
        CPU_ADC(cpu, Operand);
    } break;
    case INSN_SBC: {
        char Operand = CPU_FetchUByte(cpu);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_SBC_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_SBC(cpu, Operand);
    } break;
    case INSN_CPX: {
        char Operand = CPU_FetchUByte(cpu);
        CPU_RegisterCompare(cpu, Operand, cpu->X);
    } break;
    case INSN_CPY: {
        char Operand = CPU_FetchUByte(cpu);
        CPU_RegisterCompare(cpu, Operand, cpu->Y);
    } break;
    case INSN_CPX_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->X);
    } break;
    case INSN_CPY_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->Y);
    } break;
    case INSN_CPX_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->X);
    } break;
    case INSN_CPY_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->Y);
    } break;
    case INSN_CMP: {
        char Operand = CPU_FetchUByte(cpu);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_ABSY: {
        unsigned short Address = CPU_AddrAbsoluteY(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_INDX: {
        unsigned short Address = CPU_AddrIndirectX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_CMP_INDY: {
        unsigned short Address = CPU_AddrIndirectY(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        CPU_RegisterCompare(cpu, Operand, cpu->A);
    } break;
    case INSN_ASL: {
        cpu->A = CPU_ASL(cpu, cpu->A);
    } break;
    case INSN_ASL_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ASL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ASL_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ASL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ASL_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ASL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ASL_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ASL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_LSR: {
        cpu->A = CPU_LSR(cpu, cpu->A);
    } break;
    case INSN_LSR_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_LSR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_LSR_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_LSR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_LSR_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_LSR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_LSR_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_LSR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROL: {
        cpu->A = CPU_ROL(cpu, cpu->A);
    } break;
    case INSN_ROL_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROL_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROL_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROL_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROL(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROR: {
        cpu->A = CPU_ROR(cpu, cpu->A);
    } break;
    case INSN_ROR_ZP: {
        unsigned short Address = CPU_AddrZeroPage(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROR_ZPX: {
        unsigned short Address = CPU_AddrZeroPageX(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROR_ABS: {
        unsigned short Address = CPU_AddrAbsolute(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_ROR_ABSX: {
        unsigned short Address = CPU_AddrAbsoluteX_5(cpu);
        char Operand = CPU_ReadByte(cpu, Address);
        char Result = CPU_ROR(cpu, Operand);
        CPU_WriteByte(cpu, Result, Address);
    } break;
    case INSN_BRK: {
        CPU_PushWordToStack(cpu, cpu->PC + 1);
        CPU_PushPSToStack(cpu);
        cpu->PC = CPU_ReadWord(cpu, 0xFFFE);
        cpu->Flags.B = 1;
        cpu->Flags.I = 1;
        return 0;
    } break;
    case INSN_RTI: {
        CPU_PopPSFromStack(cpu);
        cpu->PC = CPU_PopWordFromStack(cpu);
    } break;
    default:
        // unhandled instruction -> stop
        return 0;
    }

    cpu->cycles_last_step = cpu->cycles_executed - cycles_now;

    return cpu->cycles_last_step;
}

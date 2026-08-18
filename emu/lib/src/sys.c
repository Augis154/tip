#include "sys.h"
#include "spec.h"
#include "formats.h"

int decode_sys(struct cpu *cpu, struct Instruction *instr) {
    union Operands *ops = &instr->operands;
    switch(instr->fn) {
        case SYS_SRET:
            cpu->pc = cpu->system_register[SR_STPC] - 4;
            uint32_t *sts = &cpu->system_register[SR_STS];
            *sts &= ~((1 << POS_STS_SIE) | (1 << POS_STS_SMODE));
            *sts |= (((*sts >> POS_STS_PIE) << POS_STS_SIE) & (1 << POS_STS_SIE));
            *sts |= (((*sts >> POS_STS_PMODE) << POS_STS_SMODE) & (1 << POS_STS_SMODE));
            break;
        case SYS_SCALL:
            trap(cpu, STC_SCALL, instr);
            break;
        case SYS_SBREAK:
            trap(cpu, STC_SBREAK, instr);
            break;
        case SYS_SRW:
            decode_instr_operands(instr, I);
            write_register(cpu, ops->iType.rd, cpu->system_register[ops->iType.imm13]);
            cpu->system_register[ops->iType.imm13] = get_register(cpu, ops->iType.r1);
            break;
        case SYS_SRS:
            decode_instr_operands(instr, I);
            write_register(cpu, ops->iType.rd, cpu->system_register[ops->iType.imm13]);
            cpu->system_register[ops->iType.imm13] |= get_register(cpu, ops->iType.r1);
            break;
        case SYS_SRC:
            decode_instr_operands(instr, I);
            write_register(cpu, ops->iType.rd, cpu->system_register[ops->iType.imm13]);
            cpu->system_register[ops->iType.imm13] &= ~get_register(cpu, ops->iType.r1);
            break;
        default:
            fprintf(stderr, "Invalid SYS instruction");
            return ILL_INSTR;
    }
    return OK;
}

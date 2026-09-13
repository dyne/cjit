/* ARM64 inline assembly error tests.
   Verify that invalid assembly produces proper error messages.
   Run with -dt; skipped on non-arm64 architectures. */

#include <stdio.h>

#if defined test_unknown_instruction
int main(void)
{
    __asm__("fubar x0, x1, x2");
    return 0;
}

#elif defined test_shift_imm_range_32
int main(void)
{
    /* LSL by 32 is out of range for 32-bit operand */
    __asm__("lsl w0, w1, #32");
    return 0;
}

#elif defined test_shift_imm_range_64
int main(void)
{
    /* LSL by 64 is out of range for 64-bit operand */
    __asm__("lsl x0, x1, #64");
    return 0;
}

#elif defined test_invalid_sysreg
int main(void)
{
    /* Bogus system register name */
    __asm__("mrs x0, bogusreg");
    return 0;
}

#elif defined test_invalid_barrier_option
int main(void)
{
    /* Invalid barrier scope name */
    __asm__("dmb xyz");
    return 0;
}

#elif defined test_missing_third_operand
int main(void)
{
    __asm__("add x0, x1");
    return 0;
}

#elif defined test_movz_imm_range
int main(void)
{
    __asm__("movz x0, #0x10000");
    return 0;
}

#elif defined test_movz_shift_range
int main(void)
{
    __asm__("movz x0, #1, lsl #8");
    return 0;
}

#elif defined test_invalid_muls
int main(void)
{
    __asm__("muls x0, x1, x2");
    return 0;
}

#elif defined test_extended_inline_asm
int main(void)
{
    int x = 1;
    /* Invalid operand reference in extended inline asm */
    __asm__("add %0, %1, #1" : "=r"(x) : "2"(x));
    return 0;
}

#elif defined test_extended_inline_clobber
int main(void)
{
    /* Invalid clobber register name */
    __asm__ volatile ("nop" : : : "bogus");
    return 0;
}

#endif

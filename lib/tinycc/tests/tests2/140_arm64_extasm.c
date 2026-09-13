/*
 * ARM64 Extended Inline Assembly Tests
 * Tests for GCC-style extended inline assembly with operands, constraints, and clobbers
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

struct pair64 {
    uint64_t a;
    uint64_t b;
};

static int arm64_symbol_target(void)
{
    return 7;
}

static void test_symbolic_address_constraint_compile_only(void)
{
    asm volatile("" : : "S"(arm64_symbol_target));
}

/* Test 1: Basic output operand */
void test_basic_output(void)
{
    int x;
    asm("mov %0, #42" : "=r"(x));
    assert(x == 42);
    printf("Test 1 (basic output): PASSED\n");
}

/* Test 2: Input operand */
void test_input_operand(void)
{
    int x = 10;
    int y;
    asm("add %0, %1, #5" : "=r"(y) : "r"(x));
    assert(y == 15);
    printf("Test 2 (input operand): PASSED\n");
}

/* Test 3: Read-write operand */
void test_read_write_operand(void)
{
    int x = 10;
    asm("add %0, %0, #1" : "+r"(x));
    assert(x == 11);
    printf("Test 3 (read-write operand): PASSED\n");
}

/* Test 4: Memory operand - load */
void test_memory_load(void)
{
    int x = 42;
    int y;
    asm("ldr %w0, [%1]" : "=r"(y) : "r"(&x));
    assert(y == 42);
    printf("Test 4 (memory load): PASSED\n");
}

/* Test 5: Memory operand - store */
void test_memory_store(void)
{
    int x;
    int val = 123;
    asm("str %w1, [%0]" : : "r"(&x), "r"(val));
    assert(x == 123);
    printf("Test 5 (memory store): PASSED\n");
}

/* Test 5a: Stack memory operand with frame-relative offset */
void test_stack_memory_operand(void)
{
    long x = 0;
    long val = 321;

    asm volatile("str %0, %1" : : "r"(val), "m"(x));
    assert(x == 321);
    printf("Test 5a (stack memory operand): PASSED\n");
}

/* Test 5b: Symbol memory operand */
static long arm64_symbol_mem;

void test_symbol_memory_operand(void)
{
    long val = 654;

    arm64_symbol_mem = 0;
    asm volatile("str %0, %1" : : "r"(val), "m"(arm64_symbol_mem));
    assert(arm64_symbol_mem == 654);
    printf("Test 5b (symbol memory operand): PASSED\n");
}

/* Test 6: Clobber list */
void test_clobber_list(void)
{
    int x = 10;
    int y = 20;
    int result;
    asm("add %0, %1, %2" 
        : "=r"(result)
        : "r"(x), "r"(y)
        : "cc");
    assert(result == 30);
    printf("Test 6 (clobber list): PASSED\n");
}

/* Test 7: Multiple outputs */
void test_multiple_outputs(void)
{
    int a, b;
    asm("mov %0, #1; mov %1, #2" 
        : "=r"(a), "=r"(b));
    assert(a == 1 && b == 2);
    printf("Test 7 (multiple outputs): PASSED\n");
}

/* Test 8: Constraint reference */
void test_constraint_reference(void)
{
    int x = 10;
    int y;
    asm("add %0, %1, #5" : "=r"(y) : "0"(x));
    assert(y == 15);
    printf("Test 8 (constraint reference): PASSED\n");
}

/* Test 9: Early clobber */
void test_early_clobber(void)
{
    int x = 10;
    int y;
    asm("mov %0, #42" : "=&r"(y) : "r"(x));
    assert(y == 42);
    printf("Test 9 (early clobber): PASSED\n");
}

/* Test 10: 32-bit register modifier */
void test_w_register(void)
{
    uint32_t x = 100;
    uint32_t y;
    asm("add %w0, %w1, #50" : "=r"(y) : "r"(x));
    assert(y == 150);
    printf("Test 10 (w register modifier): PASSED\n");
}

/* Test 11: Immediate constraint 'I' (12-bit immediate) */
void test_immediate_i_constraint(void)
{
    int x = 100;
    int y;
    asm("add %0, %1, #200" : "=r"(y) : "r"(x), "I"(200));
    assert(y == 300);
    printf("Test 11 (immediate I constraint): PASSED\n");
}

/* Test 12: Register constraint */
void test_general_operand_constraint(void)
{
    int x = 50;
    int y;
    asm("add %0, %1, #10" : "=r"(y) : "r"(x));
    assert(y == 60);
    printf("Test 12 (general operand constraint): PASSED\n");
}

/* Test 13: Multiple inputs and outputs */
void test_multiple_io(void)
{
    int a = 10, b = 20;
    int sum, diff;
    asm("add %0, %1, %2" : "=r"(sum) : "r"(a), "r"(b));
    asm("sub %0, %1, %2" : "=r"(diff) : "r"(a), "r"(b));
    assert(sum == 30 && diff == -10);
    printf("Test 13 (multiple IO): PASSED\n");
}

/* Test 14: Register variable preservation */
void test_regvar_preservation(void)
{
    register uint64_t keep asm("x19") = 0x123456789abcdef0ULL;
    uint64_t out;

    asm volatile("mov x19, #7; add %0, x19, #1"
        : "=r"(out)
        :
        : "x19");
    assert(keep == 0x123456789abcdef0ULL);
    assert(out == 8);
    printf("Test 14 (regvar preservation): PASSED\n");
}

/* Test 15: Complex arithmetic */
void test_complex_arithmetic(void)
{
    int a = 100, b = 50, c = 25;
    int result;
    asm("add %0, %1, %2; sub %0, %0, %3"
        : "=&r"(result)
        : "r"(a), "r"(b), "r"(c));
    assert(result == 125);
    printf("Test 15 (complex arithmetic): PASSED\n");
}

/* Test 16: Named operand (GCC extension) */
void test_named_operand(void)
{
    int input = 10;
    int output;
    asm("add %[out], %[in], #5"
        : [out] "=r"(output)
        : [in] "r"(input));
    assert(output == 15);
    printf("Test 16 (named operand): PASSED\n");
}

/* Test 17: Memory clobber */
void test_memory_clobber(void)
{
    int x = 10;
    asm volatile("" : : : "memory");
    assert(x == 10);
    printf("Test 17 (memory clobber): PASSED\n");
}

/* Test 18: Condition flags clobber */
void test_cc_clobber(void)
{
    int x = 100;
    int y;
    asm("adds %0, %1, #0" : "=r"(y) : "r"(x) : "cc");
    assert(y == 100);
    printf("Test 18 (cc clobber): PASSED\n");
}

/* Test 19: Large immediate with movz/movk */
void test_large_immediate(void)
{
    uint64_t val;
    asm("movz %0, #0x1234, lsl #16; movk %0, #0x5678" : "=r"(val));
    assert(val == 0x12345678ULL);
    printf("Test 19 (large immediate): PASSED\n");
}

/* Test 20: Bitwise operations */
void test_bitwise_ops(void)
{
    uint64_t a = 0xf0f0f0f00f0f0f0fULL;
    uint64_t b = 0x3333ffff0000ccccULL;
    uint64_t andv;
    uint64_t orv;
    uint64_t xorv;
    uint64_t imm_and;

    asm("and %0, %1, %2" : "=r"(andv) : "r"(a), "r"(b));
    asm("orr %0, %1, %2" : "=r"(orv) : "r"(a), "r"(b));
    asm("eor %0, %1, %2" : "=r"(xorv) : "r"(a), "r"(b));
    asm("and %0, %1, %2" : "=r"(imm_and)
        : "r"(~0ULL), "L"(0xff00ff00ff00ff00ULL));

    assert(andv == (a & b));
    assert(orv == (a | b));
    assert(xorv == (a ^ b));
    assert(imm_and == 0xff00ff00ff00ff00ULL);
    printf("Test 20 (bitwise ops): PASSED\n");
}

/* Test 21: Register shift operands */
void test_register_shift_operands(void)
{
    uint64_t val = 3;
    uint64_t amount = 4;
    uint64_t shifted;
    uint64_t rotated;

    asm("lsl %0, %1, %2" : "=r"(shifted) : "r"(val), "r"(amount));
    asm("ror %0, %1, %2" : "=r"(rotated)
        : "r"(0x0123456789abcdefULL), "r"(8ULL));
    assert(shifted == 48);
    assert(rotated == 0xef0123456789abcdULL);
    printf("Test 21 (register shifts): PASSED\n");
}

/* Test 22: ROR immediate alias of EXTR */
void test_ror_immediate(void)
{
    uint64_t rotated;

    asm("ror %0, %1, #8" : "=r"(rotated) : "r"(0x0123456789abcdefULL));
    assert(rotated == 0xef0123456789abcdULL);
    printf("Test 22 (ror immediate): PASSED\n");
}

/* Test 23: FP/SIMD register constraint and modifier */
void test_fp_register_operand(void)
{
    double x = 3.5;
    double y;

    asm("ldr %d0, [%1]" : "=w"(y) : "r"(&x));
    assert(y == x);
    printf("Test 23 (fp register operand): PASSED\n");
}

/* Test 24: Zero constraint */
void test_zero_constraint(void)
{
    uint64_t x = 41;
    uint64_t y;

    asm("add %0, %1, %x2" : "=r"(y) : "r"(x), "Z"(0));
    assert(y == x);
    printf("Test 24 (Z constraint): PASSED\n");
}

/* Test 25: 32-bit logical immediate constraint */
void test_logical_imm32_constraint(void)
{
    uint32_t y;

    asm("orr %w0, wzr, %1" : "=r"(y) : "K"(0xff));
    assert(y == 0xff);
    printf("Test 25 (K constraint): PASSED\n");
}

/* Test 26: 64-bit logical immediate constraint */
void test_logical_imm64_constraint(void)
{
    uint64_t y;

    asm("orr %0, xzr, %1" : "=r"(y) : "L"(0xff00ff00ff00ff00ULL));
    assert(y == 0xff00ff00ff00ff00ULL);
    printf("Test 26 (L constraint): PASSED\n");
}

/* Test 27: 32-bit MOV pseudo immediate constraint */
void test_mov_imm32_constraint(void)
{
    uint32_t y;

    asm("mov %w0, %1" : "=r"(y) : "M"(0x1234));
    assert(y == 0x1234);
    printf("Test 27 (M constraint): PASSED\n");
}

/* Test 28: 64-bit MOV pseudo immediate constraint */
void test_mov_imm64_constraint(void)
{
    uint64_t y;

    asm("mov %0, %1" : "=r"(y) : "N"(0x12340000ULL));
    assert(y == 0x12340000ULL);
    printf("Test 28 (N constraint): PASSED\n");
}

/* Test 29: x FP/SIMD register constraint */
void test_x_constraint_fp(void)
{
    double x = 6.25;
    double y;

    asm("ldr %d0, [%1]" : "=x"(y) : "r"(&x));
    assert(y == x);
    printf("Test 29 (x constraint): PASSED\n");
}

/* Test 30: y FP/SIMD register constraint */
void test_y_constraint_fp(void)
{
    double x = 7.25;
    double y;

    asm("ldr %d0, [%1]" : "=y"(y) : "r"(&x));
    assert(y == x);
    printf("Test 30 (y constraint): PASSED\n");
}

/* Test 31: Q memory constraint */
static int arm64_q_load(int *ptr)
{
    int y;

    asm("ldr %w0, %1" : "=r"(y) : "Q"(*ptr));
    return y;
}

void test_q_memory_constraint(void)
{
    int x = 91;
    assert(arm64_q_load(&x) == 91);
    printf("Test 31 (Q constraint): PASSED\n");
}

/* Test 32: Ump pair-memory constraint */
void test_ump_memory_constraint(void)
{
    struct pair64 pair = { 0x1122334455667788ULL, 0x99aabbccddeeff00ULL };
    uint64_t a;
    uint64_t b;

    asm("ldp %0, %1, %2" : "=r"(a), "=r"(b) : "Ump"(pair));
    assert(a == pair.a);
    assert(b == pair.b);
    printf("Test 32 (Ump constraint): PASSED\n");
}

int main(void)
{
    printf("ARM64 Extended Inline Assembly Tests\n");
    test_basic_output();
    test_input_operand();
    test_read_write_operand();
    test_memory_load();
    test_memory_store();
    test_stack_memory_operand();
    test_symbol_memory_operand();
    test_clobber_list();
    test_multiple_outputs();
    test_constraint_reference();
    test_early_clobber();
    test_w_register();
    test_immediate_i_constraint();
    test_general_operand_constraint();
    test_multiple_io();
    test_regvar_preservation();
    test_complex_arithmetic();
    test_named_operand();
    test_memory_clobber();
    test_cc_clobber();
    test_large_immediate();
    test_bitwise_ops();
    test_register_shift_operands();
    test_ror_immediate();
    test_fp_register_operand();
    test_zero_constraint();
    test_logical_imm32_constraint();
    test_logical_imm64_constraint();
    test_mov_imm32_constraint();
    test_mov_imm64_constraint();
    test_x_constraint_fp();
    test_y_constraint_fp();
    test_q_memory_constraint();
    test_ump_memory_constraint();
    test_symbolic_address_constraint_compile_only();
    printf("ARM64 Extended Inline Assembly Tests PASSED!\n");
    return 0;
}

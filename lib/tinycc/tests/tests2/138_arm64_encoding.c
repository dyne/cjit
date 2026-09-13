/* ARM64 instruction encoding verification test.
   Exercises shift-by-immediate, load/store addressing modes,
   conditional branches, and system register access. */

#include <stdio.h>
#include <stdint.h>

/* ---- shift-by-immediate helpers ---- */

static uint32_t lsl32(uint32_t x, int n) { return x << n; }
static uint32_t lsr32(uint32_t x, int n) { return x >> n; }
static int32_t  asr32(int32_t  x, int n) { return x >> n; }
static uint64_t lsl64(uint64_t x, int n) { return x << n; }
static uint64_t lsr64(uint64_t x, int n) { return x >> n; }
static int64_t  asr64(int64_t  x, int n) { return x >> n; }

static void test_shifts(void)
{
    printf("shift-imm:\n");
    printf("%x\n", lsl32(1, 0));
    printf("%x\n", lsl32(1, 15));
    printf("%x\n", lsl32(1, 31));
    printf("%x\n", lsr32(0x80000000U, 31));
    printf("%x\n", lsr32(0x80000000U, 16));
    printf("%x\n", asr32(-1, 1));
    printf("%x\n", asr32(-256, 4));
    printf("%llx\n", (unsigned long long)lsl64(1ULL, 0));
    printf("%llx\n", (unsigned long long)lsl64(1ULL, 32));
    printf("%llx\n", (unsigned long long)lsl64(1ULL, 63));
    printf("%llx\n", (unsigned long long)lsr64(0x8000000000000000ULL, 63));
    printf("%llx\n", (unsigned long long)asr64(-1LL, 1));
    printf("%llx\n", (unsigned long long)asr64(-256LL, 4));
}

/* ---- load/store with various addressing modes ---- */

static void test_ldr_str(void)
{
    int64_t buf[4] = { 0x1122334455667788LL, 0x99AABBCCDDEEFF00LL,
                       0x0F1E2D3C4B5A6978LL, 0x0000000000000001LL };
    int64_t val;
    int32_t wval;
    int64_t *p;

    printf("ldr-str:\n");

    /* LDR X, [Xn, #offset] */
    val = buf[0];
    printf("%llx\n", (unsigned long long)val);
    val = buf[1];
    printf("%llx\n", (unsigned long long)val);

    /* LDR W, [Xn, #offset] (32-bit load) */
    wval = *(int32_t*)&buf[0];
    printf("%x\n", (unsigned)wval);

    /* Pre-indexed: store pointer, modify */
    p = &buf[0];
    val = *(p + 2);
    printf("%llx\n", (unsigned long long)val);

    /* Post-indexed simulation via pointer arithmetic */
    p = &buf[0];
    val = *p;
    p++;
    printf("%llx %llx\n", (unsigned long long)val,
           (unsigned long long)*p);
}

/* ---- STP/LDP (store/load pair) ---- */

static void test_ldp_stp(void)
{
    int64_t src[2] = { 0xAAAABBBBCCCCDDDDLL, 0x1111222233334444LL };
    int64_t dst[2];

    /* The compiler should use stp/ldp for this */
    dst[0] = src[0];
    dst[1] = src[1];

    printf("ldp-stp:\n");
    printf("%llx %llx\n",
           (unsigned long long)dst[0],
           (unsigned long long)dst[1]);
}

/* ---- CBZ / CBNZ via C patterns ---- */

static const char *cbz_test(int x)
{
    if (x == 0)
        return "zero";
    return "nonzero";
}

static const char *cbnz_test(int x)
{
    if (x != 0)
        return "nonzero";
    return "zero";
}

static void test_cond_branches(void)
{
    printf("cbz-cbnz:\n");
    printf("%s\n", cbz_test(0));
    printf("%s\n", cbz_test(42));
    printf("%s\n", cbnz_test(0));
    printf("%s\n", cbnz_test(42));
}

/* ---- conditional compare patterns (CCMP/CCMN) ---- */

static int cond_select(int a, int b)
{
    /* TCC should generate csel or equivalent */
    return a > b ? a : b;
}

static void test_cond_select(void)
{
    printf("csel:\n");
    printf("%d\n", cond_select(5, 10));
    printf("%d\n", cond_select(10, 5));
    printf("%d\n", cond_select(0, 0));
    printf("%d\n", cond_select(-1, 1));
}

/* ---- MRS/MSR system registers (FPCR/FPSR) ---- */

static void test_sysregs(void)
{
    unsigned int fpcr, fpsr;

    printf("sysregs:\n");

    /* Read FPCR */
    __asm__ volatile ("mrs %0, fpcr" : "=r"(fpcr));
    printf("%u\n", fpcr & 0x0F);

    /* Read FPSR */
    __asm__ volatile ("mrs %0, fpsr" : "=r"(fpsr));
    printf("%u\n", fpsr);

    /* Write/restore FPCR (should be same value) */
    __asm__ volatile ("msr fpcr, %0" : : "r"(fpcr));

    /* Read back and verify */
    {
        unsigned int check;
        __asm__ volatile ("mrs %0, fpcr" : "=r"(check));
        printf("%s\n", check == fpcr ? "ok" : "fail");
    }
}

/* ---- NOP encoding (should not crash) ---- */

static void test_nop(void)
{
    printf("nop:\n");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    printf("ok\n");
}

/* ---- barrier instructions ---- */

static void test_barriers(void)
{
    printf("barriers:\n");
    __asm__ volatile ("dmb sy");
    __asm__ volatile ("dsb sy");
    __asm__ volatile ("isb");
    printf("ok\n");
}

/* ---- MOV (register) patterns ---- */

static int64_t mov_x0_x1(int64_t x)
{
    register int64_t r __asm__("x0") = x;
    __asm__ volatile ("" : "=r"(r) : "0"(r));
    return r;
}

static void test_mov_reg(void)
{
    printf("mov-reg:\n");
    printf("%lld\n", (long long)mov_x0_x1(42));
    printf("%lld\n", (long long)mov_x0_x1(-1));
}

/* ---- large struct passing (reference semantics) ---- */

struct large { int64_t a, b, c, d; };

static int64_t sum_large(struct large s)
{
    return s.a + s.b + s.c + s.d;
}

static struct large make_large(int64_t a, int64_t b, int64_t c, int64_t d)
{
    struct large s = { a, b, c, d };
    return s;
}

static void test_large_structs(void)
{
    struct large s = { 1, 2, 3, 4 };
    struct large t;

    printf("large-struct:\n");
    printf("%lld\n", (long long)sum_large(s));

    t = make_large(10, 20, 30, 40);
    printf("%lld %lld %lld %lld\n",
           (long long)t.a, (long long)t.b,
           (long long)t.c, (long long)t.d);
}

/* ---- boundary-sized structs ---- */

struct s18 { char x[18]; };
struct s24 { char x[24]; };
struct s32 { char x[32]; };

static void print_s18(struct s18 s) { printf("%.18s\n", s.x); }
static void print_s24(struct s24 s) { printf("%.24s\n", s.x); }
static void print_s32(struct s32 s) { printf("%.32s\n", s.x); }

static struct s18 ret_s18(void)
{
    struct s18 s = { "123456789012345678" };
    return s;
}

static struct s24 ret_s24(void)
{
    struct s24 s = { "123456789012345678901234" };
    return s;
}

static struct s32 ret_s32(void)
{
    struct s32 s = { "12345678901234567890123456789012" };
    return s;
}

static void test_boundary_structs(void)
{
    struct s18 a;
    struct s24 b;
    struct s32 c;

    printf("boundary-structs:\n");
    a = ret_s18();
    printf("%.18s\n", a.x);
    b = ret_s24();
    printf("%.24s\n", b.x);
    c = ret_s32();
    printf("%.32s\n", c.x);
}

int main(void)
{
    test_shifts();
    test_ldr_str();
    test_ldp_stp();
    test_cond_branches();
    test_cond_select();
    test_sysregs();
    test_nop();
    test_barriers();
    test_mov_reg();
    test_large_structs();
    test_boundary_structs();
    return 0;
}

/* ---------------------------------------------- */
/* alloca.S */

#ifdef __leading_underscore
# define _(s) _##s
#else
# define _(s) s
#endif

/* ---------------------------------------------- */
#if defined __i386__

.globl _(alloca), _(__alloca)
_(alloca):
_(__alloca):
    pop     %edx
    pop     %eax
    add     $3,%eax
    and     $-4,%eax
    jz      p3

#ifdef _WIN32
p1:
    cmp     $4096,%eax
    jb      p2
    test    %eax,-4096(%esp)
    sub     $4096,%esp
    sub     $4096,%eax
    jmp     p1
p2:
#endif
    sub     %eax,%esp
    mov     %esp,%eax
p3:
    push    %edx
    push    %edx
    ret

/* ---------------------------------------------- */
#elif defined __x86_64__

.globl _(alloca)
_(alloca):
    pop     %rdx
#ifdef _WIN32
    mov     %rcx,%rax
#else
    mov     %rdi,%rax
#endif
    add     $15,%rax
    and     $-16,%rax
    jz      p3

#ifdef _WIN32
p1:
    cmp     $4096,%rax
    jb      p2
    test    %rax,-4096(%rsp)
    sub     $4096,%rsp
    sub     $4096,%rax
    jmp p1
p2:
#endif
    sub     %rax,%rsp
    mov     %rsp,%rax
p3:
    push    %rdx
    ret

/* ---------------------------------------------- */
#elif defined __arm__

.globl _(alloca)
_(alloca):
    rsb sp, r0, sp
    bic sp, sp, #7
    mov r0, sp
    mov pc, lr

/* ---------------------------------------------- */
#elif defined __aarch64__ || defined __arm64__

.globl _(alloca)
_(alloca):
#ifdef __TINYC__
    .int 0x91003c00
    .int 0x927cec00
#ifdef _WIN32
    .int 0xb4000160
    .int 0xd2820001
    .int 0xeb01001f
    .int 0x540000c3
    .int 0xcb2163e2
    .int 0xf940005f
    .int 0xcb2163ff
    .int 0xcb010000
    .int 0x17fffffa
    .int 0xb4000040
#endif
    .int 0xcb2063ff
    .int 0x910003e0
    .int 0xd65f03c0
#else
    add x0, x0, #15     // Round up to 16-byte boundary
    and x0, x0, #-16    // Ensure 16-byte alignment
#ifdef _WIN32
    cbz x0, p100        // If size is 0, skip to return
    // Windows requires page-wise allocation with stack probing
    mov x1, #4096       // Page size = 4096 bytes

p101:
    cmp x0, x1          // Compare remaining size with page size
    b.lo    p102        // If less than page, jump to remainder

    // Probe first, then allocate
    sub x2, sp, x1      // Calculate guard page address (sp - 4096)
    ldr xzr, [x2]       // Touch guard page FIRST
    sub sp, sp, x1      // THEN allocate the page

    sub x0, x0, x1      // Decrement remaining size
    b   p101            // Continue loop

p102:
    // Allocate remaining bytes (less than one page)
    cbz x0, p100        // If no remaining bytes, skip
    sub sp, sp, x0      // Allocate remaining space
#else
    // Non-Windows: simple one-time allocation
    sub sp, sp, x0      // Allocate space on stack
#endif

p100:
    mov x0, sp          // Return allocated address
    ret                 // Return to caller
#endif

/* ---------------------------------------------- */
#elif defined __riscv

.globl _(alloca)
_(alloca):
    sub    sp, sp, a0
    addi   sp, sp, -15
    andi   sp, sp, -16
    add    a0, sp, zero
    ret

#endif

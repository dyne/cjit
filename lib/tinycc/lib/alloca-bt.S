/* ---------------------------------------------- */
/* alloca-bt.S */

#ifdef __leading_underscore
# define _(s) _##s
#else
# define _(s) s
#endif

/* ---------------------------------------------- */
#if defined __i386__

.globl _(__bound_alloca)
_(__bound_alloca):
    pop     %edx
    pop     %eax
    mov     %eax, %ecx
    test    %eax,%eax
    jz      p6
    add     $3 + 1,%eax
    and     $-4,%eax

#ifdef _WIN32
p4:
    cmp     $4096,%eax
    jb      p5
    test    %eax,-4096(%esp)
    sub     $4096,%esp
    sub     $4096,%eax
    jmp p4

p5:
#endif

    sub     %eax,%esp
    mov     %esp,%eax

    push    %edx
    push    %eax
    push    %ecx
    push    %eax
    call    _(__bound_new_region)
    add     $8, %esp
    pop     %eax
    pop     %edx

p6:
    push    %edx
    push    %edx
    ret

/* ---------------------------------------------- */
#elif defined __x86_64__

.globl _(__bound_alloca)
_(__bound_alloca):
#ifdef _WIN32
    inc %rcx            # add one extra to separate regions
    jmp _(alloca)
.globl _(__bound_alloca_nr)
_(__bound_alloca_nr):
    dec     %rcx
    push    %rax
    mov     %rcx,%rdx
    mov     %rax,%rcx
    sub     $32,%rsp
    call    _(__bound_new_region)
    add     $32,%rsp
    pop     %rax
    ret
#else
    pop     %rdx
    mov     %rdi,%rax
    and     %eax,%eax
    jz      p3
    mov     %rax,%rsi	# size, a second parm to the __bound_new_region
    add     $15 + 1,%rax  # add one extra to separate regions
    and     $-16,%rax

    sub     %rax,%rsp
    mov     %rsp,%rdi	# pointer, a first parm to the __bound_new_region
    mov     %rsp,%rax

    push    %rdx
    push    %rax
    call    _(__bound_new_region)
    pop     %rax
    pop     %rdx

p3:
    push    %rdx
    ret
#endif

/* ---------------------------------------------- */
#elif defined __arm__

.globl _(__bound_alloca)
_(__bound_alloca):
    mov r1, r0
    add r0, r0, #1
    rsb sp, r0, sp
    bic sp, sp, #7
    mov r0, sp
    push { lr }
    bl _(__bound_new_region)
    pop { lr }
    mov r0, sp
    mov pc, lr

/* ---------------------------------------------- */
#elif defined __aarch64__ || defined __arm64__

.globl _(__bound_alloca)
_(__bound_alloca):
#ifdef __TINYC__
    .int 0xaa0003e1
    .int 0x91004000
    .int 0x927cec00
#ifdef _WIN32
    .int 0xb4000160
    .int 0xd2820002
    .int 0xeb02001f
    .int 0x540000c3
    .int 0xcb2263e3
    .int 0xf940007f
    .int 0xcb2263ff
    .int 0xcb020000
    .int 0x17fffffa
    .int 0xb4000040
#endif
    .int 0xcb2063ff
    .int 0x910003e0
    .int 0xa9bf7bfd
    .reloc ., R_AARCH64_CALL26,  _(__bound_new_region)
    .int 0x94000000
    .int 0xa8c17bfd
    .int 0x910003e0
    .int 0xd65f03c0
#else
    mov x1, x0
    add x0, x0, #16     // Round up to 16-byte boundary
    and x0, x0, #-16    // Ensure 16-byte alignment
#ifdef _WIN32
    cbz x0, p100        // If size is 0, skip to return
    // Windows requires page-wise allocation with stack probing
    mov x2, #4096       // Page size = 4096 bytes

p101:
    cmp x0, x2          // Compare remaining size with page size
    b.lo    p102        // If less than page, jump to remainder

    // Probe first, then allocate
    sub x3, sp, x2      // Calculate guard page address (sp - 4096)
    ldr xzr, [x3]       // Touch guard page FIRST
    sub sp, sp, x2      // THEN allocate the page

    sub x0, x0, x2      // Decrement remaining size
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
    stp x29, x30, [sp, #-16]!
    bl _(__bound_new_region)
    ldp x29, x30, [sp], #16
    mov x0, sp          // Return allocated address
    ret                 // Return to caller
#endif

/* ---------------------------------------------- */
#elif defined __riscv

.globl _(__bound_alloca)
_(__bound_alloca):
    mv     a1, a0
    sub    sp, sp, a0
    addi   sp, sp, -16
    andi   sp, sp, -16
    add    a0, sp, zero
    addi   sp,sp,-16
    sd     s0,0(sp)
    sd     ra,8(sp)
    jal    _(__bound_new_region)
    ld     s0,0(sp)
    ld     ra,8(sp)
    addi   sp,sp,16
    add    a0, sp, zero
    ret

/* ---------------------------------------------- */
#endif

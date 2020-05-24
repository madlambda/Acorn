.globl _start

_start:
    # RegReg

    mov %al, %al
    mov %al, %cl
    mov %al, %dl
    mov %al, %bl
    mov %al, %ah
    mov %al, %ch
    mov %al, %dh
    mov %al, %bh

    mov %cl, %al
    mov %cl, %cl
    mov %cl, %dl
    mov %cl, %bl
    mov %cl, %ah
    mov %cl, %ch
    mov %cl, %dh
    mov %cl, %bh

    mov %dl, %al
    mov %dl, %cl
    mov %dl, %dl
    mov %dl, %bl
    mov %dl, %ah
    mov %dl, %ch
    mov %dl, %dh
    mov %dl, %bh

    mov %bl, %al
    mov %bl, %cl
    mov %bl, %dl
    mov %bl, %bl
    mov %bl, %ah
    mov %bl, %ch
    mov %bl, %dh
    mov %bl, %bh

    mov %ah, %al
    mov %ah, %cl
    mov %ah, %dl
    mov %ah, %bl
    mov %ah, %ah
    mov %ah, %ch
    mov %ah, %dh
    mov %ah, %bh

    mov %ch, %al
    mov %ch, %cl
    mov %ch, %dl
    mov %ch, %bl
    mov %ch, %ah
    mov %ch, %ch
    mov %ch, %dh
    mov %ch, %bh

    mov %dh, %al
    mov %dh, %cl
    mov %dh, %dl
    mov %dh, %bl
    mov %dh, %ah
    mov %dh, %ch
    mov %dh, %dh
    mov %dh, %bh

    mov %bh, %al
    mov %bh, %cl
    mov %bh, %dl
    mov %bh, %bl
    mov %bh, %ah
    mov %bh, %ch
    mov %bh, %dh
    mov %bh, %bh

    mov %ax, %ax
    mov %ax, %cx
    mov %ax, %dx
    mov %ax, %bx
    mov %ax, %sp
    mov %ax, %bp
    mov %ax, %si
    mov %ax, %di

    mov %cx, %ax
    mov %cx, %cx
    mov %cx, %dx
    mov %cx, %bx
    mov %cx, %sp
    mov %cx, %bp
    mov %cx, %si
    mov %cx, %di

    mov %dx, %ax
    mov %dx, %cx
    mov %dx, %dx
    mov %dx, %bx
    mov %dx, %sp
    mov %dx, %bp
    mov %dx, %si
    mov %dx, %di

    mov %bx, %ax
    mov %bx, %cx
    mov %bx, %dx
    mov %bx, %bx
    mov %bx, %sp
    mov %bx, %bp
    mov %bx, %si
    mov %bx, %di

    mov %sp, %ax
    mov %sp, %cx
    mov %sp, %dx
    mov %sp, %bx
    mov %sp, %sp
    mov %sp, %bp
    mov %sp, %si
    mov %sp, %di

    mov %bp, %ax
    mov %bp, %cx
    mov %bp, %dx
    mov %bp, %bx
    mov %bp, %sp
    mov %bp, %bp
    mov %bp, %si
    mov %bp, %di

    mov %si, %ax
    mov %si, %cx
    mov %si, %dx
    mov %si, %bx
    mov %si, %sp
    mov %si, %bp
    mov %si, %si
    mov %si, %di

    mov %di, %ax
    mov %di, %cx
    mov %di, %dx
    mov %di, %bx
    mov %di, %sp
    mov %di, %bp
    mov %di, %si
    mov %di, %di

    mov %eax, %eax
    mov %eax, %ecx
    mov %eax, %edx
    mov %eax, %ebx
    mov %eax, %esp
    mov %eax, %ebp
    mov %eax, %esi
    mov %eax, %edi

    mov %ecx, %eax
    mov %ecx, %ecx
    mov %ecx, %edx
    mov %ecx, %ebx
    mov %ecx, %esp
    mov %ecx, %ebp
    mov %ecx, %esi
    mov %ecx, %edi

    mov %edx, %eax
    mov %edx, %ecx
    mov %edx, %edx
    mov %edx, %ebx
    mov %edx, %esp
    mov %edx, %ebp
    mov %edx, %esi
    mov %edx, %edi

    mov %ebx, %eax
    mov %ebx, %ecx
    mov %ebx, %edx
    mov %ebx, %ebx
    mov %ebx, %esp
    mov %ebx, %ebp
    mov %ebx, %esi
    mov %ebx, %edi

    mov %esp, %eax
    mov %esp, %ecx
    mov %esp, %edx
    mov %esp, %ebx
    mov %esp, %esp
    mov %esp, %ebp
    mov %esp, %esi
    mov %esp, %edi

    mov %ebp, %eax
    mov %ebp, %ecx
    mov %ebp, %edx
    mov %ebp, %ebx
    mov %ebp, %esp
    mov %ebp, %ebp
    mov %ebp, %esi
    mov %ebp, %edi

    mov %esi, %eax
    mov %esi, %ecx
    mov %esi, %edx
    mov %esi, %ebx
    mov %esi, %esp
    mov %esi, %ebp
    mov %esi, %esi
    mov %esi, %edi

    mov %edi, %eax
    mov %edi, %ecx
    mov %edi, %edx
    mov %edi, %ebx
    mov %edi, %esp
    mov %edi, %ebp
    mov %edi, %esi
    mov %edi, %edi

    mov %rax, %rax
    mov %rax, %rcx
    mov %rax, %rdx
    mov %rax, %rbx
    mov %rax, %rsp
    mov %rax, %rbp
    mov %rax, %rsi
    mov %rax, %rdi

    mov %rcx, %rax
    mov %rcx, %rcx
    mov %rcx, %rdx
    mov %rcx, %rbx
    mov %rcx, %rsp
    mov %rcx, %rbp
    mov %rcx, %rsi
    mov %rcx, %rdi

    mov %rdx, %rax
    mov %rdx, %rcx
    mov %rdx, %rdx
    mov %rdx, %rbx
    mov %rdx, %rsp
    mov %rdx, %rbp
    mov %rdx, %rsi
    mov %rdx, %rdi

    mov %rbx, %rax
    mov %rbx, %rcx
    mov %rbx, %rdx
    mov %rbx, %rbx
    mov %rbx, %rsp
    mov %rbx, %rbp
    mov %rbx, %rsi
    mov %rbx, %rdi

    mov %rsp, %rax
    mov %rsp, %rcx
    mov %rsp, %rdx
    mov %rsp, %rbx
    mov %rsp, %rsp
    mov %rsp, %rbp
    mov %rsp, %rsi
    mov %rsp, %rdi

    mov %rbp, %rax
    mov %rbp, %rcx
    mov %rbp, %rdx
    mov %rbp, %rbx
    mov %rbp, %rsp
    mov %rbp, %rbp
    mov %rbp, %rsi
    mov %rbp, %rdi

    mov %rsi, %rax
    mov %rsi, %rcx
    mov %rsi, %rdx
    mov %rsi, %rbx
    mov %rsi, %rsp
    mov %rsi, %rbp
    mov %rsi, %rsi
    mov %rsi, %rdi

    mov %rdi, %rax
    mov %rdi, %rcx
    mov %rdi, %rdx
    mov %rdi, %rbx
    mov %rdi, %rsp
    mov %rdi, %rbp
    mov %rdi, %rsi
    mov %rdi, %rdi

    mov %r8b, %r8b
    mov %r8b, %r9b
    mov %r8b, %r10b
    mov %r8b, %r11b
    mov %r8b, %r12b
    mov %r8b, %r13b
    mov %r8b, %r14b
    mov %r8b, %r15b

    mov %r9b, %r8b
    mov %r9b, %r9b
    mov %r9b, %r10b
    mov %r9b, %r11b
    mov %r9b, %r12b
    mov %r9b, %r13b
    mov %r9b, %r14b
    mov %r9b, %r15b

    mov %r10b, %r8b
    mov %r10b, %r9b
    mov %r10b, %r10b
    mov %r10b, %r11b
    mov %r10b, %r12b
    mov %r10b, %r13b
    mov %r10b, %r14b
    mov %r10b, %r15b

    mov %r11b, %r8b
    mov %r11b, %r9b
    mov %r11b, %r10b
    mov %r11b, %r11b
    mov %r11b, %r12b
    mov %r11b, %r13b
    mov %r11b, %r14b
    mov %r11b, %r15b

    mov %r12b, %r8b
    mov %r12b, %r9b
    mov %r12b, %r10b
    mov %r12b, %r11b
    mov %r12b, %r12b
    mov %r12b, %r13b
    mov %r12b, %r14b
    mov %r12b, %r15b

    mov %r13b, %r8b
    mov %r13b, %r9b
    mov %r13b, %r10b
    mov %r13b, %r11b
    mov %r13b, %r12b
    mov %r13b, %r13b
    mov %r13b, %r14b
    mov %r13b, %r15b

    mov %r14b, %r8b
    mov %r14b, %r9b
    mov %r14b, %r10b
    mov %r14b, %r11b
    mov %r14b, %r12b
    mov %r14b, %r13b
    mov %r14b, %r14b
    mov %r14b, %r15b

    mov %r15b, %r8b
    mov %r15b, %r9b
    mov %r15b, %r10b
    mov %r15b, %r11b
    mov %r15b, %r12b
    mov %r15b, %r13b
    mov %r15b, %r14b
    mov %r15b, %r15b

    mov %r8w, %r8w
    mov %r8w, %r9w
    mov %r8w, %r10w
    mov %r8w, %r11w
    mov %r8w, %r12w
    mov %r8w, %r13w
    mov %r8w, %r14w
    mov %r8w, %r15w

    mov %r9w, %r8w
    mov %r9w, %r9w
    mov %r9w, %r10w
    mov %r9w, %r11w
    mov %r9w, %r12w
    mov %r9w, %r13w
    mov %r9w, %r14w
    mov %r9w, %r15w

    mov %r10w, %r8w
    mov %r10w, %r9w
    mov %r10w, %r10w
    mov %r10w, %r11w
    mov %r10w, %r12w
    mov %r10w, %r13w
    mov %r10w, %r14w
    mov %r10w, %r15w

    mov %r11w, %r8w
    mov %r11w, %r9w
    mov %r11w, %r10w
    mov %r11w, %r11w
    mov %r11w, %r12w
    mov %r11w, %r13w
    mov %r11w, %r14w
    mov %r11w, %r15w

    mov %r12w, %r8w
    mov %r12w, %r9w
    mov %r12w, %r10w
    mov %r12w, %r11w
    mov %r12w, %r12w
    mov %r12w, %r13w
    mov %r12w, %r14w
    mov %r12w, %r15w

    mov %r13w, %r8w
    mov %r13w, %r9w
    mov %r13w, %r10w
    mov %r13w, %r11w
    mov %r13w, %r12w
    mov %r13w, %r13w
    mov %r13w, %r14w
    mov %r13w, %r15w

    mov %r14w, %r8w
    mov %r14w, %r9w
    mov %r14w, %r10w
    mov %r14w, %r11w
    mov %r14w, %r12w
    mov %r14w, %r13w
    mov %r14w, %r14w
    mov %r14w, %r15w

    mov %r15w, %r8w
    mov %r15w, %r9w
    mov %r15w, %r10w
    mov %r15w, %r11w
    mov %r15w, %r12w
    mov %r15w, %r13w
    mov %r15w, %r14w
    mov %r15w, %r15w

    mov %r8d, %r8d
    mov %r8d, %r9d
    mov %r8d, %r10d
    mov %r8d, %r11d
    mov %r8d, %r12d
    mov %r8d, %r13d
    mov %r8d, %r14d
    mov %r8d, %r15d

    mov %r9d, %r8d
    mov %r9d, %r9d
    mov %r9d, %r10d
    mov %r9d, %r11d
    mov %r9d, %r12d
    mov %r9d, %r13d
    mov %r9d, %r14d
    mov %r9d, %r15d

    mov %r10d, %r8d
    mov %r10d, %r9d
    mov %r10d, %r10d
    mov %r10d, %r11d
    mov %r10d, %r12d
    mov %r10d, %r13d
    mov %r10d, %r14d
    mov %r10d, %r15d

    mov %r11d, %r8d
    mov %r11d, %r9d
    mov %r11d, %r10d
    mov %r11d, %r11d
    mov %r11d, %r12d
    mov %r11d, %r13d
    mov %r11d, %r14d
    mov %r11d, %r15d

    mov %r12d, %r8d
    mov %r12d, %r9d
    mov %r12d, %r10d
    mov %r12d, %r11d
    mov %r12d, %r12d
    mov %r12d, %r13d
    mov %r12d, %r14d
    mov %r12d, %r15d

    mov %r13d, %r8d
    mov %r13d, %r9d
    mov %r13d, %r10d
    mov %r13d, %r11d
    mov %r13d, %r12d
    mov %r13d, %r13d
    mov %r13d, %r14d
    mov %r13d, %r15d

    mov %r14d, %r8d
    mov %r14d, %r9d
    mov %r14d, %r10d
    mov %r14d, %r11d
    mov %r14d, %r12d
    mov %r14d, %r13d
    mov %r14d, %r14d
    mov %r14d, %r15d

    mov %r15d, %r8d
    mov %r15d, %r9d
    mov %r15d, %r10d
    mov %r15d, %r11d
    mov %r15d, %r12d
    mov %r15d, %r13d
    mov %r15d, %r14d
    mov %r15d, %r15d

    mov %r8, %r8
    mov %r8, %r9
    mov %r8, %r10
    mov %r8, %r11
    mov %r8, %r12
    mov %r8, %r13
    mov %r8, %r14
    mov %r8, %r15

    mov %r9, %r8
    mov %r9, %r9
    mov %r9, %r10
    mov %r9, %r11
    mov %r9, %r12
    mov %r9, %r13
    mov %r9, %r14
    mov %r9, %r15

    mov %r10, %r8
    mov %r10, %r9
    mov %r10, %r10
    mov %r10, %r11
    mov %r10, %r12
    mov %r10, %r13
    mov %r10, %r14
    mov %r10, %r15

    mov %r11, %r8
    mov %r11, %r9
    mov %r11, %r10
    mov %r11, %r11
    mov %r11, %r12
    mov %r11, %r13
    mov %r11, %r14
    mov %r11, %r15

    mov %r12, %r8
    mov %r12, %r9
    mov %r12, %r10
    mov %r12, %r11
    mov %r12, %r12
    mov %r12, %r13
    mov %r12, %r14
    mov %r12, %r15

    mov %r13, %r8
    mov %r13, %r9
    mov %r13, %r10
    mov %r13, %r11
    mov %r13, %r12
    mov %r13, %r13
    mov %r13, %r14
    mov %r13, %r15

    mov %r14, %r8
    mov %r14, %r9
    mov %r14, %r10
    mov %r14, %r11
    mov %r14, %r12
    mov %r14, %r13
    mov %r14, %r14
    mov %r14, %r15

    mov %r15, %r8
    mov %r15, %r9
    mov %r15, %r10
    mov %r15, %r11
    mov %r15, %r12
    mov %r15, %r13
    mov %r15, %r14
    mov %r15, %r15

    # ImmReg

    mov $127, %al
    mov $-127, %al
    mov $-128, %al

    mov $0x69, %al
    mov $0x69, %cl
    mov $0x69, %dl
    mov $0x69, %bl
    mov $0x69, %ah
    mov $0x69, %ch
    mov $0x69, %dh
    mov $0x69, %bh

    mov $0x69, %ax
    mov $0x69, %cx
    mov $0x69, %dx
    mov $0x69, %bx
    mov $0x69, %sp
    mov $0x69, %bp
    mov $0x69, %si
    mov $0x69, %di

    mov $0x69, %eax
    mov $0x69, %ecx
    mov $0x69, %edx
    mov $0x69, %ebx
    mov $0x69, %esp
    mov $0x69, %ebp
    mov $0x69, %esi
    mov $0x69, %edi

    # movabsq $0x69, %rax
    # movabsq $0x69, %rcx
    # ...
.byte 0x48
.byte 0xb8
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xb9
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xba
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xbb
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xbc
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xbd
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xbe
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x48
.byte 0xbf
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0


    mov $0x69, %r8b
    mov $0x69, %r9b
    mov $0x69, %r10b
    mov $0x69, %r11b
    mov $0x69, %r12b
    mov $0x69, %r13b
    mov $0x69, %r14b
    mov $0x69, %r15b

    mov $0x69, %r8w
    mov $0x69, %r9w
    mov $0x69, %r10w
    mov $0x69, %r11w
    mov $0x69, %r12w
    mov $0x69, %r13w
    mov $0x69, %r14w
    mov $0x69, %r15w

    mov $0x69, %r8d
    mov $0x69, %r9d
    mov $0x69, %r10d
    mov $0x69, %r11d
    mov $0x69, %r12d
    mov $0x69, %r13d
    mov $0x69, %r14d
    mov $0x69, %r15d

.byte 0x49
.byte 0xb8
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xb9
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xba
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xbb
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xbc
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xbd
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xbe
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

.byte 0x49
.byte 0xbf
.byte 0x69
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0
.byte 0x0

    movw $32767, %ax
    mov $32767, %eax
    mov $0x7fffffff, %eax
    mov $0x80000000, %eax
    movabs $-0x8000000000000000, %rax
    movabs $0x7fffffffffffffff, %rax
    movabs $-0x8000000000000000, %r8
    movabs $0x7fffffffffffffff, %r8

    # MemReg
    mov 0x1, %al
    mov 0x1, %cl
    mov 0x1, %dl
    mov 0x1, %bl
    mov 0x1, %ah
    mov 0x1, %ch
    mov 0x1, %dh
    mov 0x1, %bh

    mov 0x1, %ax
    mov 0x1, %cx
    mov 0x1, %dx
    mov 0x1, %bx
    mov 0x1, %sp
    mov 0x1, %bp
    mov 0x1, %si
    mov 0x1, %di

    mov 0x1, %eax
    mov 0x1, %ecx
    mov 0x1, %edx
    mov 0x1, %ebx
    mov 0x1, %esp
    mov 0x1, %ebp
    mov 0x1, %esi
    mov 0x1, %edi

    mov 0x1, %rax
    mov 0x1, %rcx
    mov 0x1, %rdx
    mov 0x1, %rbx
    mov 0x1, %rsp
    mov 0x1, %rbp
    mov 0x1, %rsi
    mov 0x1, %rdi

    mov 0x1, %r8b
    mov 0x1, %r9b
    mov 0x1, %r10b
    mov 0x1, %r11b
    mov 0x1, %r12b
    mov 0x1, %r13b
    mov 0x1, %r14b
    mov 0x1, %r15b

    mov 0x1, %r8w
    mov 0x1, %r9w
    mov 0x1, %r10w
    mov 0x1, %r11w
    mov 0x1, %r12w
    mov 0x1, %r13w
    mov 0x1, %r14w
    mov 0x1, %r15w

    mov 0x1, %r8d
    mov 0x1, %r9d
    mov 0x1, %r10d
    mov 0x1, %r11d
    mov 0x1, %r12d
    mov 0x1, %r13d
    mov 0x1, %r14d
    mov 0x1, %r15d

    mov 0x1, %r8
    mov 0x1, %r9
    mov 0x1, %r10
    mov 0x1, %r11
    mov 0x1, %r12
    mov 0x1, %r13
    mov 0x1, %r14
    mov 0x1, %r15

    # Binutils Gas assembler encodes the instruction below as movabsl (sign
    # extending the value) while LLVM assembler encodes it as movl.
    # We choose the smaller encoding.
    # mov 0xdeadbeef, %eax
    .byte 0x8b
    .byte 0x04
    .byte 0x25
    .byte 0xef
    .byte 0xbe
    .byte 0xad
    .byte 0xde

    #movabsq 0xdeadbeef, %rax
    .byte 0x48
    .byte 0x8b
    .byte 0x04
    .byte 0x25
    .byte 0xef
    .byte 0xbe
    .byte 0xad
    .byte 0xde

    mov 0x7fffffff, %eax

    mov 0x1000, %rax

    mov 0x7fffffff, %r10
    mov 0x1000, %r10

    movw 0xffff, %r10w

    mov (%eax), %eax
    mov (%ecx), %eax
    mov (%edx), %eax
    mov (%ebx), %eax
    mov (%esp), %eax
    mov (%ebp), %eax
    mov (%esi), %eax
    mov (%edi), %eax

    mov (%eax), %ecx
    mov (%ecx), %ecx
    mov (%edx), %ecx
    mov (%ebx), %ecx
    mov (%esp), %ecx
    mov (%ebp), %ecx
    mov (%esi), %ecx
    mov (%edi), %ecx

    mov (%eax), %edx
    mov (%ecx), %edx
    mov (%edx), %edx
    mov (%ebx), %edx
    mov (%esp), %edx
    mov (%ebp), %edx
    mov (%esi), %edx
    mov (%edi), %edx

    mov (%eax), %ebx
    mov (%ecx), %ebx
    mov (%edx), %ebx
    mov (%ebx), %ebx
    mov (%esp), %ebx
    mov (%ebp), %ebx
    mov (%esi), %ebx
    mov (%edi), %ebx

    mov (%eax), %esp
    mov (%ecx), %esp
    mov (%edx), %esp
    mov (%ebx), %esp
    mov (%esp), %esp
    mov (%ebp), %esp
    mov (%esi), %esp
    mov (%edi), %esp

    mov (%eax), %ebp
    mov (%ecx), %ebp
    mov (%edx), %ebp
    mov (%ebx), %ebp
    mov (%esp), %ebp
    mov (%ebp), %ebp
    mov (%esi), %ebp
    mov (%edi), %ebp

    mov (%eax), %esi
    mov (%ecx), %esi
    mov (%edx), %esi
    mov (%ebx), %esi
    mov (%esp), %esi
    mov (%ebp), %esi
    mov (%esi), %esi
    mov (%edi), %esi

    mov (%eax), %edi
    mov (%ecx), %edi
    mov (%edx), %edi
    mov (%ebx), %edi
    mov (%esp), %edi
    mov (%ebp), %edi
    mov (%esi), %edi
    mov (%edi), %edi

    mov (%rax), %rax
    mov (%rcx), %rax
    mov (%rdx), %rax
    mov (%rbx), %rax
    mov (%rsp), %rax
    mov (%rbp), %rax
    mov (%rsi), %rax
    mov (%rdi), %rax

    mov (%rax), %rcx
    mov (%rcx), %rcx
    mov (%rdx), %rcx
    mov (%rbx), %rcx
    mov (%rsp), %rcx
    mov (%rbp), %rcx
    mov (%rsi), %rcx
    mov (%rdi), %rcx

    mov (%rax), %rdx
    mov (%rcx), %rdx
    mov (%rdx), %rdx
    mov (%rbx), %rdx
    mov (%rsp), %rdx
    mov (%rbp), %rdx
    mov (%rsi), %rdx
    mov (%rdi), %rdx

    mov (%rax), %rbx
    mov (%rcx), %rbx
    mov (%rdx), %rbx
    mov (%rbx), %rbx
    mov (%rsp), %rbx
    mov (%rbp), %rbx
    mov (%rsi), %rbx
    mov (%rdi), %rbx

    mov (%rax), %rsp
    mov (%rcx), %rsp
    mov (%rdx), %rsp
    mov (%rbx), %rsp
    mov (%rsp), %rsp
    mov (%rbp), %rsp
    mov (%rsi), %rsp
    mov (%rdi), %rsp

    mov (%rax), %rbp
    mov (%rcx), %rbp
    mov (%rdx), %rbp
    mov (%rbx), %rbp
    mov (%rsp), %rbp
    mov (%rbp), %rbp
    mov (%rsi), %rbp
    mov (%rdi), %rbp

    mov (%rax), %rsi
    mov (%rcx), %rsi
    mov (%rdx), %rsi
    mov (%rbx), %rsi
    mov (%rsp), %rsi
    mov (%rbp), %rsi
    mov (%rsi), %rsi
    mov (%rdi), %rsi

    mov (%rax), %rdi
    mov (%rcx), %rdi
    mov (%rdx), %rdi
    mov (%rbx), %rdi
    mov (%rsp), %rdi
    mov (%rbp), %rdi
    mov (%rsi), %rdi
    mov (%rdi), %rdi

    mov (%eax), %ax
    mov (%eax), %cx
    mov (%eax), %dx
    mov (%eax), %bx
    mov (%eax), %sp
    mov (%eax), %bp
    mov (%eax), %si
    mov (%eax), %di

    mov (%eax), %eax
    mov (%eax), %ecx
    mov (%eax), %edx
    mov (%eax), %ebx
    mov (%eax), %esp
    mov (%eax), %ebp
    mov (%eax), %esi
    mov (%eax), %edi

    mov (%eax), %rax
    mov (%eax), %rcx
    mov (%eax), %rdx
    mov (%eax), %rbx
    mov (%eax), %rsp
    mov (%eax), %rbp
    mov (%eax), %rsi
    mov (%eax), %rdi

    mov (%eax), %r8b
    mov (%eax), %r9b
    mov (%eax), %r10b
    mov (%eax), %r11b
    mov (%eax), %r12b
    mov (%eax), %r13b
    mov (%eax), %r14b
    mov (%eax), %r15b

    mov (%eax), %r8w
    mov (%eax), %r9w
    mov (%eax), %r10w
    mov (%eax), %r11w
    mov (%eax), %r12w
    mov (%eax), %r13w
    mov (%eax), %r14w
    mov (%eax), %r15w

    mov (%eax), %r8d
    mov (%eax), %r9d
    mov (%eax), %r10d
    mov (%eax), %r11d
    mov (%eax), %r12d
    mov (%eax), %r13d
    mov (%eax), %r14d
    mov (%eax), %r15d

    mov (%eax), %r8
    mov (%eax), %r9
    mov (%eax), %r10
    mov (%eax), %r11
    mov (%eax), %r12
    mov (%eax), %r13
    mov (%eax), %r14
    mov (%eax), %r15

    mov 8(%eax), %ax
    mov 8(%eax), %cx
    mov 8(%eax), %dx
    mov 8(%eax), %bx
    mov 8(%eax), %sp
    mov 8(%eax), %bp
    mov 8(%eax), %si
    mov 8(%eax), %di

    mov 8(%eax), %eax
    mov 8(%eax), %ecx
    mov 8(%eax), %edx
    mov 8(%eax), %ebx
    mov 8(%eax), %esp
    mov 8(%eax), %ebp
    mov 8(%eax), %esi
    mov 8(%eax), %edi

    mov 8(%eax), %rax
    mov 8(%eax), %rcx
    mov 8(%eax), %rdx
    mov 8(%eax), %rbx
    mov 8(%eax), %rsp
    mov 8(%eax), %rbp
    mov 8(%eax), %rsi
    mov 8(%eax), %rdi

    mov 8(%eax), %r8b
    mov 8(%eax), %r9b
    mov 8(%eax), %r10b
    mov 8(%eax), %r11b
    mov 8(%eax), %r12b
    mov 8(%eax), %r13b
    mov 8(%eax), %r14b
    mov 8(%eax), %r15b

    mov 8(%eax), %r8w
    mov 8(%eax), %r9w
    mov 8(%eax), %r10w
    mov 8(%eax), %r11w
    mov 8(%eax), %r12w
    mov 8(%eax), %r13w
    mov 8(%eax), %r14w
    mov 8(%eax), %r15w

    mov 8(%eax), %r8d
    mov 8(%eax), %r9d
    mov 8(%eax), %r10d
    mov 8(%eax), %r11d
    mov 8(%eax), %r12d
    mov 8(%eax), %r13d
    mov 8(%eax), %r14d
    mov 8(%eax), %r15d

    mov 8(%eax), %r8
    mov 8(%eax), %r9
    mov 8(%eax), %r10
    mov 8(%eax), %r11
    mov 8(%eax), %r12
    mov 8(%eax), %r13
    mov 8(%eax), %r14
    mov 8(%eax), %r15

    mov 16(%eax), %ax
    mov 16(%eax), %cx
    mov 16(%eax), %dx
    mov 16(%eax), %bx
    mov 16(%eax), %sp
    mov 16(%eax), %bp
    mov 16(%eax), %si
    mov 16(%eax), %di

    mov 16(%eax), %eax
    mov 16(%eax), %ecx
    mov 16(%eax), %edx
    mov 16(%eax), %ebx
    mov 16(%eax), %esp
    mov 16(%eax), %ebp
    mov 16(%eax), %esi
    mov 16(%eax), %edi

    mov 16(%eax), %rax
    mov 16(%eax), %rcx
    mov 16(%eax), %rdx
    mov 16(%eax), %rbx
    mov 16(%eax), %rsp
    mov 16(%eax), %rbp
    mov 16(%eax), %rsi
    mov 16(%eax), %rdi

    mov 16(%eax), %r8b
    mov 16(%eax), %r9b
    mov 16(%eax), %r10b
    mov 16(%eax), %r11b
    mov 16(%eax), %r12b
    mov 16(%eax), %r13b
    mov 16(%eax), %r14b
    mov 16(%eax), %r15b

    mov 16(%eax), %r8w
    mov 16(%eax), %r9w
    mov 16(%eax), %r10w
    mov 16(%eax), %r11w
    mov 16(%eax), %r12w
    mov 16(%eax), %r13w
    mov 16(%eax), %r14w
    mov 16(%eax), %r15w

    mov 16(%eax), %r8d
    mov 16(%eax), %r9d
    mov 16(%eax), %r10d
    mov 16(%eax), %r11d
    mov 16(%eax), %r12d
    mov 16(%eax), %r13d
    mov 16(%eax), %r14d
    mov 16(%eax), %r15d

    mov 16(%eax), %r8
    mov 16(%eax), %r9
    mov 16(%eax), %r10
    mov 16(%eax), %r11
    mov 16(%eax), %r12
    mov 16(%eax), %r13
    mov 16(%eax), %r14
    mov 16(%eax), %r15

    mov 2000(%eax), %ax
    mov 2000(%eax), %cx
    mov 2000(%eax), %dx
    mov 2000(%eax), %bx
    mov 2000(%eax), %sp
    mov 2000(%eax), %bp
    mov 2000(%eax), %si
    mov 2000(%eax), %di

    mov 2000(%eax), %eax
    mov 2000(%eax), %ecx
    mov 2000(%eax), %edx
    mov 2000(%eax), %ebx
    mov 2000(%eax), %esp
    mov 2000(%eax), %ebp
    mov 2000(%eax), %esi
    mov 2000(%eax), %edi

    mov 2000(%eax), %rax
    mov 2000(%eax), %rcx
    mov 2000(%eax), %rdx
    mov 2000(%eax), %rbx
    mov 2000(%eax), %rsp
    mov 2000(%eax), %rbp
    mov 2000(%eax), %rsi
    mov 2000(%eax), %rdi

    mov 2000(%eax), %r8b
    mov 2000(%eax), %r9b
    mov 2000(%eax), %r10b
    mov 2000(%eax), %r11b
    mov 2000(%eax), %r12b
    mov 2000(%eax), %r13b
    mov 2000(%eax), %r14b
    mov 2000(%eax), %r15b

    mov 2000(%eax), %r8w
    mov 2000(%eax), %r9w
    mov 2000(%eax), %r10w
    mov 2000(%eax), %r11w
    mov 2000(%eax), %r12w
    mov 2000(%eax), %r13w
    mov 2000(%eax), %r14w
    mov 2000(%eax), %r15w

    mov 2000(%eax), %r8d
    mov 2000(%eax), %r9d
    mov 2000(%eax), %r10d
    mov 2000(%eax), %r11d
    mov 2000(%eax), %r12d
    mov 2000(%eax), %r13d
    mov 2000(%eax), %r14d
    mov 2000(%eax), %r15d

    mov 2000(%eax), %r8
    mov 2000(%eax), %r9
    mov 2000(%eax), %r10
    mov 2000(%eax), %r11
    mov 2000(%eax), %r12
    mov 2000(%eax), %r13
    mov 2000(%eax), %r14
    mov 2000(%eax), %r15

    mov 0x10(%eax), %ax
    mov 0x10(%ecx), %ax
    mov 0x10(%edx), %ax
    mov 0x10(%ebx), %ax
    mov 0x10(%esp), %ax
    mov 0x10(%ebp), %ax
    mov 0x10(%esi), %ax
    mov 0x10(%edi), %ax

    mov 0x10(%rax), %ax
    mov 0x10(%rcx), %ax
    mov 0x10(%rdx), %ax
    mov 0x10(%rbx), %ax
    mov 0x10(%rsp), %ax
    mov 0x10(%rbp), %ax
    mov 0x10(%rsi), %ax
    mov 0x10(%rdi), %ax

    mov 0x10(%r8d), %ax
    mov 0x10(%r9d), %ax
    mov 0x10(%r10d), %ax
    mov 0x10(%r11d), %ax
    mov 0x10(%r12d), %ax
    mov 0x10(%r13d), %ax
    mov 0x10(%r14d), %ax
    mov 0x10(%r15d), %ax

    mov 0x10(%r8), %ax
    mov 0x10(%r9), %ax
    mov 0x10(%r10), %ax
    mov 0x10(%r11), %ax
    mov 0x10(%r12), %ax
    mov 0x10(%r13), %ax
    mov 0x10(%r14), %ax
    mov 0x10(%r15), %ax

    mov 0x10(%eax), %cx
    mov 0x10(%ecx), %cx
    mov 0x10(%edx), %cx
    mov 0x10(%ebx), %cx
    mov 0x10(%esp), %cx
    mov 0x10(%ebp), %cx
    mov 0x10(%esi), %cx
    mov 0x10(%edi), %cx

    mov 0x10(%rax), %cx
    mov 0x10(%rcx), %cx
    mov 0x10(%rdx), %cx
    mov 0x10(%rbx), %cx
    mov 0x10(%rsp), %cx
    mov 0x10(%rbp), %cx
    mov 0x10(%rsi), %cx
    mov 0x10(%rdi), %cx

    mov 0x10(%r8d), %cx
    mov 0x10(%r9d), %cx
    mov 0x10(%r10d), %cx
    mov 0x10(%r11d), %cx
    mov 0x10(%r12d), %cx
    mov 0x10(%r13d), %cx
    mov 0x10(%r14d), %cx
    mov 0x10(%r15d), %cx

    mov 0x10(%r8), %cx
    mov 0x10(%r9), %cx
    mov 0x10(%r10), %cx
    mov 0x10(%r11), %cx
    mov 0x10(%r12), %cx
    mov 0x10(%r13), %cx
    mov 0x10(%r14), %cx
    mov 0x10(%r15), %cx

    mov (%ebp), %cx
    mov (%esp), %cx
    mov (%esp), %ax

    mov (%rsp), %ax
    mov (%rbp), %ax

    mov (%r8), %ax
    mov (%r9), %ax
    mov (%r10), %ax
    mov (%r11), %ax
    mov (%r12), %ax
    mov (%r13), %ax
    mov (%r14), %ax
    mov (%r15), %ax
    mov (%r8), %eax

    mov 8(%rsp), %rax
    mov 8(%rsp), %rcx
    mov 16(%rsp), %rcx
    mov 1000(%esp), %eax
    mov 8(%rax), %rcx
    mov 16(%rax), %rax
    mov 16(%rcx), %rsp
    mov 127(%rax), %rax
    mov 128(%rax), %rax
    mov 1000(%rax), %rax
    mov -100(%rax), %rax
    mov -1000(%rax), %rax
    mov 8(%rbp), %rax
    mov 9(%rbp), %rax
    mov -8(%rbp), %rax

    mov %al, (%rax)
    mov %cl, (%rax)
    mov %dl, (%rax)
    mov %bl, (%rax)
    mov %ah, (%rax)
    mov %ch, (%rax)
    mov %dh, (%rax)
    mov %bh, (%rax)

    mov %ax, (%rax)
    mov %cx, (%rax)
    mov %dx, (%rax)
    mov %bx, (%rax)
    mov %sp, (%rax)
    mov %bp, (%rax)
    mov %si, (%rax)
    mov %di, (%rax)

    mov %eax, (%rax)
    mov %ecx, (%rax)
    mov %edx, (%rax)
    mov %ebx, (%rax)
    mov %esp, (%rax)
    mov %ebp, (%rax)
    mov %esi, (%rax)
    mov %edi, (%rax)

    mov %rax, (%rax)
    mov %rcx, (%rax)
    mov %rdx, (%rax)
    mov %rbx, (%rax)
    mov %rsp, (%rax)
    mov %rbp, (%rax)
    mov %rsi, (%rax)
    mov %rdi, (%rax)

    mov %r8b, (%rax)
    mov %r9b, (%rax)
    mov %r10b, (%rax)
    mov %r11b, (%rax)
    mov %r12b, (%rax)
    mov %r13b, (%rax)
    mov %r14b, (%rax)
    mov %r15b, (%rax)

    mov %r8w, (%rax)
    mov %r9w, (%rax)
    mov %r10w, (%rax)
    mov %r11w, (%rax)
    mov %r12w, (%rax)
    mov %r13w, (%rax)
    mov %r14w, (%rax)
    mov %r15w, (%rax)

    mov %r8d, (%rax)
    mov %r9d, (%rax)
    mov %r10d, (%rax)
    mov %r11d, (%rax)
    mov %r12d, (%rax)
    mov %r13d, (%rax)
    mov %r14d, (%rax)
    mov %r15d, (%rax)

    mov %r8, (%rax)
    mov %r9, (%rax)
    mov %r10, (%rax)
    mov %r11, (%rax)
    mov %r12, (%rax)
    mov %r13, (%rax)
    mov %r14, (%rax)
    mov %r15, (%rax)

    mov %al, 8(%rax)
    mov %cl, 8(%rax)
    mov %dl, 8(%rax)
    mov %bl, 8(%rax)
    mov %ah, 8(%rax)
    mov %ch, 8(%rax)
    mov %dh, 8(%rax)
    mov %bh, 8(%rax)

    mov %ax, 8(%rax)
    mov %cx, 8(%rax)
    mov %dx, 8(%rax)
    mov %bx, 8(%rax)
    mov %sp, 8(%rax)
    mov %bp, 8(%rax)
    mov %si, 8(%rax)
    mov %di, 8(%rax)

    mov %eax, 8(%rax)
    mov %ecx, 8(%rax)
    mov %edx, 8(%rax)
    mov %ebx, 8(%rax)
    mov %esp, 8(%rax)
    mov %ebp, 8(%rax)
    mov %esi, 8(%rax)
    mov %edi, 8(%rax)

    mov %rax, 8(%rax)
    mov %rcx, 8(%rax)
    mov %rdx, 8(%rax)
    mov %rbx, 8(%rax)
    mov %rsp, 8(%rax)
    mov %rbp, 8(%rax)
    mov %rsi, 8(%rax)
    mov %rdi, 8(%rax)

    mov %r8b, 8(%rax)
    mov %r9b, 8(%rax)
    mov %r10b, 8(%rax)
    mov %r11b, 8(%rax)
    mov %r12b, 8(%rax)
    mov %r13b, 8(%rax)
    mov %r14b, 8(%rax)
    mov %r15b, 8(%rax)

    mov %r8w, 8(%rax)
    mov %r9w, 8(%rax)
    mov %r10w, 8(%rax)
    mov %r11w, 8(%rax)
    mov %r12w, 8(%rax)
    mov %r13w, 8(%rax)
    mov %r14w, 8(%rax)
    mov %r15w, 8(%rax)

    mov %r8d, 8(%rax)
    mov %r9d, 8(%rax)
    mov %r10d, 8(%rax)
    mov %r11d, 8(%rax)
    mov %r12d, 8(%rax)
    mov %r13d, 8(%rax)
    mov %r14d, 8(%rax)
    mov %r15d, 8(%rax)

    mov %r8, 8(%rax)
    mov %r9, 8(%rax)
    mov %r10, 8(%rax)
    mov %r11, 8(%rax)
    mov %r12, 8(%rax)
    mov %r13, 8(%rax)
    mov %r14, 8(%rax)
    mov %r15, 8(%rax)

    mov %al, -8(%rax)
    mov %cl, -8(%rax)
    mov %dl, -8(%rax)
    mov %bl, -8(%rax)
    mov %ah, -8(%rax)
    mov %ch, -8(%rax)
    mov %dh, -8(%rax)
    mov %bh, -8(%rax)

    mov %ax, -8(%rax)
    mov %cx, -8(%rax)
    mov %dx, -8(%rax)
    mov %bx, -8(%rax)
    mov %sp, -8(%rax)
    mov %bp, -8(%rax)
    mov %si, -8(%rax)
    mov %di, -8(%rax)

    mov %eax, -8(%rax)
    mov %ecx, -8(%rax)
    mov %edx, -8(%rax)
    mov %ebx, -8(%rax)
    mov %esp, -8(%rax)
    mov %ebp, -8(%rax)
    mov %esi, -8(%rax)
    mov %edi, -8(%rax)

    mov %rax, -8(%rax)
    mov %rcx, -8(%rax)
    mov %rdx, -8(%rax)
    mov %rbx, -8(%rax)
    mov %rsp, -8(%rax)
    mov %rbp, -8(%rax)
    mov %rsi, -8(%rax)
    mov %rdi, -8(%rax)

    mov %r8b, -8(%rax)
    mov %r9b, -8(%rax)
    mov %r10b, -8(%rax)
    mov %r11b, -8(%rax)
    mov %r12b, -8(%rax)
    mov %r13b, -8(%rax)
    mov %r14b, -8(%rax)
    mov %r15b, -8(%rax)

    mov %r8w, -8(%rax)
    mov %r9w, -8(%rax)
    mov %r10w, -8(%rax)
    mov %r11w, -8(%rax)
    mov %r12w, -8(%rax)
    mov %r13w, -8(%rax)
    mov %r14w, -8(%rax)
    mov %r15w, -8(%rax)

    mov %r8d, -8(%rax)
    mov %r9d, -8(%rax)
    mov %r10d, -8(%rax)
    mov %r11d, -8(%rax)
    mov %r12d, -8(%rax)
    mov %r13d, -8(%rax)
    mov %r14d, -8(%rax)
    mov %r15d, -8(%rax)

    mov %r8, -8(%rax)
    mov %r9, -8(%rax)
    mov %r10, -8(%rax)
    mov %r11, -8(%rax)
    mov %r12, -8(%rax)
    mov %r13, -8(%rax)
    mov %r14, -8(%rax)
    mov %r15, -8(%rax)

    mov %al, 1000(%rax)
    mov %cl, 1000(%rax)
    mov %dl, 1000(%rax)
    mov %bl, 1000(%rax)
    mov %ah, 1000(%rax)
    mov %ch, 1000(%rax)
    mov %dh, 1000(%rax)
    mov %bh, 1000(%rax)

    mov %ax, 1000(%rax)
    mov %cx, 1000(%rax)
    mov %dx, 1000(%rax)
    mov %bx, 1000(%rax)
    mov %sp, 1000(%rax)
    mov %bp, 1000(%rax)
    mov %si, 1000(%rax)
    mov %di, 1000(%rax)

    mov %eax, 1000(%rax)
    mov %ecx, 1000(%rax)
    mov %edx, 1000(%rax)
    mov %ebx, 1000(%rax)
    mov %esp, 1000(%rax)
    mov %ebp, 1000(%rax)
    mov %esi, 1000(%rax)
    mov %edi, 1000(%rax)

    mov %rax, 1000(%rax)
    mov %rcx, 1000(%rax)
    mov %rdx, 1000(%rax)
    mov %rbx, 1000(%rax)
    mov %rsp, 1000(%rax)
    mov %rbp, 1000(%rax)
    mov %rsi, 1000(%rax)
    mov %rdi, 1000(%rax)

    mov %r8b, 1000(%rax)
    mov %r9b, 1000(%rax)
    mov %r10b, 1000(%rax)
    mov %r11b, 1000(%rax)
    mov %r12b, 1000(%rax)
    mov %r13b, 1000(%rax)
    mov %r14b, 1000(%rax)
    mov %r15b, 1000(%rax)

    mov %r8w, 1000(%rax)
    mov %r9w, 1000(%rax)
    mov %r10w, 1000(%rax)
    mov %r11w, 1000(%rax)
    mov %r12w, 1000(%rax)
    mov %r13w, 1000(%rax)
    mov %r14w, 1000(%rax)
    mov %r15w, 1000(%rax)

    mov %r8d, 1000(%rax)
    mov %r9d, 1000(%rax)
    mov %r10d, 1000(%rax)
    mov %r11d, 1000(%rax)
    mov %r12d, 1000(%rax)
    mov %r13d, 1000(%rax)
    mov %r14d, 1000(%rax)
    mov %r15d, 1000(%rax)

    mov %r8, 1000(%rax)
    mov %r9, 1000(%rax)
    mov %r10, 1000(%rax)
    mov %r11, 1000(%rax)
    mov %r12, 1000(%rax)
    mov %r13, 1000(%rax)
    mov %r14, 1000(%rax)
    mov %r15, 1000(%rax)

    mov %al, 1000(%rbp)
    mov %cl, 1000(%rbp)
    mov %dl, 1000(%rbp)
    mov %bl, 1000(%rbp)
    mov %ah, 1000(%rbp)
    mov %ch, 1000(%rbp)
    mov %dh, 1000(%rbp)
    mov %bh, 1000(%rbp)

    mov %ax, 1000(%rbp)
    mov %cx, 1000(%rbp)
    mov %dx, 1000(%rbp)
    mov %bx, 1000(%rbp)
    mov %sp, 1000(%rbp)
    mov %bp, 1000(%rbp)
    mov %si, 1000(%rbp)
    mov %di, 1000(%rbp)

    mov %eax, 1000(%rbp)
    mov %ecx, 1000(%rbp)
    mov %edx, 1000(%rbp)
    mov %ebx, 1000(%rbp)
    mov %esp, 1000(%rbp)
    mov %ebp, 1000(%rbp)
    mov %esi, 1000(%rbp)
    mov %edi, 1000(%rbp)

    mov %rax, 1000(%rbp)
    mov %rcx, 1000(%rbp)
    mov %rdx, 1000(%rbp)
    mov %rbx, 1000(%rbp)
    mov %rsp, 1000(%rbp)
    mov %rbp, 1000(%rbp)
    mov %rsi, 1000(%rbp)
    mov %rdi, 1000(%rbp)

    mov %r8b, 1000(%rbp)
    mov %r9b, 1000(%rbp)
    mov %r10b, 1000(%rbp)
    mov %r11b, 1000(%rbp)
    mov %r12b, 1000(%rbp)
    mov %r13b, 1000(%rbp)
    mov %r14b, 1000(%rbp)
    mov %r15b, 1000(%rbp)

    mov %r8w, 1000(%rbp)
    mov %r9w, 1000(%rbp)
    mov %r10w, 1000(%rbp)
    mov %r11w, 1000(%rbp)
    mov %r12w, 1000(%rbp)
    mov %r13w, 1000(%rbp)
    mov %r14w, 1000(%rbp)
    mov %r15w, 1000(%rbp)

    mov %r8d, 1000(%rbp)
    mov %r9d, 1000(%rbp)
    mov %r10d, 1000(%rbp)
    mov %r11d, 1000(%rbp)
    mov %r12d, 1000(%rbp)
    mov %r13d, 1000(%rbp)
    mov %r14d, 1000(%rbp)
    mov %r15d, 1000(%rbp)

    mov %r8, 1000(%rbp)
    mov %r9, 1000(%rbp)
    mov %r10, 1000(%rbp)
    mov %r11, 1000(%rbp)
    mov %r12, 1000(%rbp)
    mov %r13, 1000(%rbp)
    mov %r14, 1000(%rbp)
    mov %r15, 1000(%rbp)
    mov %al, 1000(%rsp)
    mov %cl, 1000(%rsp)
    mov %dl, 1000(%rsp)
    mov %bl, 1000(%rsp)
    mov %ah, 1000(%rsp)
    mov %ch, 1000(%rsp)
    mov %dh, 1000(%rsp)
    mov %bh, 1000(%rsp)

    mov %ax, 1000(%rsp)
    mov %cx, 1000(%rsp)
    mov %dx, 1000(%rsp)
    mov %bx, 1000(%rsp)
    mov %sp, 1000(%rsp)
    mov %bp, 1000(%rsp)
    mov %si, 1000(%rsp)
    mov %di, 1000(%rsp)

    mov %eax, 1000(%rsp)
    mov %ecx, 1000(%rsp)
    mov %edx, 1000(%rsp)
    mov %ebx, 1000(%rsp)
    mov %esp, 1000(%rsp)
    mov %ebp, 1000(%rsp)
    mov %esi, 1000(%rsp)
    mov %edi, 1000(%rsp)

    mov %rax, 1000(%rsp)
    mov %rcx, 1000(%rsp)
    mov %rdx, 1000(%rsp)
    mov %rbx, 1000(%rsp)
    mov %rsp, 1000(%rsp)
    mov %rbp, 1000(%rsp)
    mov %rsi, 1000(%rsp)
    mov %rdi, 1000(%rsp)

    mov %r8b, 1000(%rsp)
    mov %r9b, 1000(%rsp)
    mov %r10b, 1000(%rsp)
    mov %r11b, 1000(%rsp)
    mov %r12b, 1000(%rsp)
    mov %r13b, 1000(%rsp)
    mov %r14b, 1000(%rsp)
    mov %r15b, 1000(%rsp)

    mov %r8w, 1000(%rsp)
    mov %r9w, 1000(%rsp)
    mov %r10w, 1000(%rsp)
    mov %r11w, 1000(%rsp)
    mov %r12w, 1000(%rsp)
    mov %r13w, 1000(%rsp)
    mov %r14w, 1000(%rsp)
    mov %r15w, 1000(%rsp)

    mov %r8d, 1000(%rsp)
    mov %r9d, 1000(%rsp)
    mov %r10d, 1000(%rsp)
    mov %r11d, 1000(%rsp)
    mov %r12d, 1000(%rsp)
    mov %r13d, 1000(%rsp)
    mov %r14d, 1000(%rsp)
    mov %r15d, 1000(%rsp)

    mov %r8, 1000(%rsp)
    mov %r9, 1000(%rsp)
    mov %r10, 1000(%rsp)
    mov %r11, 1000(%rsp)
    mov %r12, 1000(%rsp)
    mov %r13, 1000(%rsp)
    mov %r14, 1000(%rsp)
    mov %r15, 1000(%rsp)

    # RegMem
    mov %al, 0x69
    mov %cl, 0x69
    mov %dl, 0x69
    mov %bl, 0x69
    mov %ah, 0x69
    mov %ch, 0x69
    mov %dh, 0x69
    mov %bh, 0x69

    mov %ax, 0x69
    mov %cx, 0x69
    mov %dx, 0x69
    mov %bx, 0x69
    mov %sp, 0x69
    mov %bp, 0x69
    mov %si, 0x69
    mov %di, 0x69

    mov %eax, 0x69
    mov %ecx, 0x69
    mov %edx, 0x69
    mov %ebx, 0x69
    mov %esp, 0x69
    mov %ebp, 0x69
    mov %esi, 0x69
    mov %edi, 0x69

    mov %rax, 0x69
    mov %rcx, 0x69
    mov %rdx, 0x69
    mov %rbx, 0x69
    mov %rsp, 0x69
    mov %rbp, 0x69
    mov %rsi, 0x69
    mov %rdi, 0x69

    mov %r8b, 0x69
    mov %r9b, 0x69
    mov %r10b, 0x69
    mov %r11b, 0x69
    mov %r12b, 0x69
    mov %r13b, 0x69
    mov %r14b, 0x69
    mov %r15b, 0x69

    mov %r8w, 0x69
    mov %r9w, 0x69
    mov %r10w, 0x69
    mov %r11w, 0x69
    mov %r12w, 0x69
    mov %r13w, 0x69
    mov %r14w, 0x69
    mov %r15w, 0x69

    mov %r8d, 0x69
    mov %r9d, 0x69
    mov %r10d, 0x69
    mov %r11d, 0x69
    mov %r12d, 0x69
    mov %r13d, 0x69
    mov %r14d, 0x69
    mov %r15d, 0x69

    mov %r8, 0x69
    mov %r9, 0x69
    mov %r10, 0x69
    mov %r11, 0x69
    mov %r12, 0x69
    mov %r13, 0x69
    mov %r14, 0x69
    mov %r15, 0x69

    mov %al, 0x7fffffff
    mov %cl, 0x7fffffff
    mov %dl, 0x7fffffff
    mov %bl, 0x7fffffff
    mov %ah, 0x7fffffff
    mov %ch, 0x7fffffff
    mov %dh, 0x7fffffff
    mov %bh, 0x7fffffff

    mov %ax, 0x7fffffff
    mov %cx, 0x7fffffff
    mov %dx, 0x7fffffff
    mov %bx, 0x7fffffff
    mov %sp, 0x7fffffff
    mov %bp, 0x7fffffff
    mov %si, 0x7fffffff
    mov %di, 0x7fffffff

    mov %eax, 0x7fffffff
    mov %ecx, 0x7fffffff
    mov %edx, 0x7fffffff
    mov %ebx, 0x7fffffff
    mov %esp, 0x7fffffff
    mov %ebp, 0x7fffffff
    mov %esi, 0x7fffffff
    mov %edi, 0x7fffffff

    mov %rax, 0x7fffffff
    mov %rcx, 0x7fffffff
    mov %rdx, 0x7fffffff
    mov %rbx, 0x7fffffff
    mov %rsp, 0x7fffffff
    mov %rbp, 0x7fffffff
    mov %rsi, 0x7fffffff
    mov %rdi, 0x7fffffff

    mov %r8b, 0x7fffffff
    mov %r9b, 0x7fffffff
    mov %r10b, 0x7fffffff
    mov %r11b, 0x7fffffff
    mov %r12b, 0x7fffffff
    mov %r13b, 0x7fffffff
    mov %r14b, 0x7fffffff
    mov %r15b, 0x7fffffff

    mov %r8w, 0x7fffffff
    mov %r9w, 0x7fffffff
    mov %r10w, 0x7fffffff
    mov %r11w, 0x7fffffff
    mov %r12w, 0x7fffffff
    mov %r13w, 0x7fffffff
    mov %r14w, 0x7fffffff
    mov %r15w, 0x7fffffff

    mov %r8d, 0x7fffffff
    mov %r9d, 0x7fffffff
    mov %r10d, 0x7fffffff
    mov %r11d, 0x7fffffff
    mov %r12d, 0x7fffffff
    mov %r13d, 0x7fffffff
    mov %r14d, 0x7fffffff
    mov %r15d, 0x7fffffff

    mov %r8, 0x7fffffff
    mov %r9, 0x7fffffff
    mov %r10, 0x7fffffff
    mov %r11, 0x7fffffff
    mov %r12, 0x7fffffff
    mov %r13, 0x7fffffff
    mov %r14, 0x7fffffff
    mov %r15, 0x7fffffff

    # ImmInd

    movb $1, (%eax)
    movb $1, (%ecx)
    movb $1, (%edx)
    movb $1, (%ebx)
    movb $1, (%esp)
    movb $1, (%ebp)
    movb $1, (%esi)
    movb $1, (%edi)

    movb $1, (%rax)
    movb $1, (%rcx)
    movb $1, (%rdx)
    movb $1, (%rbx)
    movb $1, (%rsp)
    movb $1, (%rbp)
    movb $1, (%rsi)
    movb $1, (%rdi)

    movb $1, (%r8d)
    movb $1, (%r9d)
    movb $1, (%r10d)
    movb $1, (%r11d)
    movb $1, (%r12d)
    movb $1, (%r13d)
    movb $1, (%r14d)
    movb $1, (%r15d)

    movb $1, (%r8)
    movb $1, (%r9)
    movb $1, (%r10)
    movb $1, (%r11)
    movb $1, (%r12)
    movb $1, (%r13)
    movb $1, (%r14)
    movb $1, (%r15)

    movb $1, 8(%eax)
    movb $1, 8(%ecx)
    movb $1, 8(%edx)
    movb $1, 8(%ebx)
    movb $1, 8(%esp)
    movb $1, 8(%ebp)
    movb $1, 8(%esi)
    movb $1, 8(%edi)

    movb $1, 8(%rax)
    movb $1, 8(%rcx)
    movb $1, 8(%rdx)
    movb $1, 8(%rbx)
    movb $1, 8(%rsp)
    movb $1, 8(%rbp)
    movb $1, 8(%rsi)
    movb $1, 8(%rdi)

    movb $1, 8(%r8d)
    movb $1, 8(%r9d)
    movb $1, 8(%r10d)
    movb $1, 8(%r11d)
    movb $1, 8(%r12d)
    movb $1, 8(%r13d)
    movb $1, 8(%r14d)
    movb $1, 8(%r15d)

    movb $1, 8(%r8)
    movb $1, 8(%r9)
    movb $1, 8(%r10)
    movb $1, 8(%r11)
    movb $1, 8(%r12)
    movb $1, 8(%r13)
    movb $1, 8(%r14)
    movb $1, 8(%r15)

    movb $1, 16(%eax)
    movb $1, 16(%ecx)
    movb $1, 16(%edx)
    movb $1, 16(%ebx)
    movb $1, 16(%esp)
    movb $1, 16(%ebp)
    movb $1, 16(%esi)
    movb $1, 16(%edi)

    movb $1, 16(%rax)
    movb $1, 16(%rcx)
    movb $1, 16(%rdx)
    movb $1, 16(%rbx)
    movb $1, 16(%rsp)
    movb $1, 16(%rbp)
    movb $1, 16(%rsi)
    movb $1, 16(%rdi)

    movb $1, 16(%r8d)
    movb $1, 16(%r9d)
    movb $1, 16(%r10d)
    movb $1, 16(%r11d)
    movb $1, 16(%r12d)
    movb $1, 16(%r13d)
    movb $1, 16(%r14d)
    movb $1, 16(%r15d)

    movb $1, 16(%r8)
    movb $1, 16(%r9)
    movb $1, 16(%r10)
    movb $1, 16(%r11)
    movb $1, 16(%r12)
    movb $1, 16(%r13)
    movb $1, 16(%r14)
    movb $1, 16(%r15)

    movb $1, 1000(%eax)
    movb $1, 1000(%ecx)
    movb $1, 1000(%edx)
    movb $1, 1000(%ebx)
    movb $1, 1000(%esp)
    movb $1, 1000(%ebp)
    movb $1, 1000(%esi)
    movb $1, 1000(%edi)

    movb $1, 1000(%rax)
    movb $1, 1000(%rcx)
    movb $1, 1000(%rdx)
    movb $1, 1000(%rbx)
    movb $1, 1000(%rsp)
    movb $1, 1000(%rbp)
    movb $1, 1000(%rsi)
    movb $1, 1000(%rdi)

    movb $1, 1000(%r8d)
    movb $1, 1000(%r9d)
    movb $1, 1000(%r10d)
    movb $1, 1000(%r11d)
    movb $1, 1000(%r12d)
    movb $1, 1000(%r13d)
    movb $1, 1000(%r14d)
    movb $1, 1000(%r15d)

    movb $1, 1000(%r8)
    movb $1, 1000(%r9)
    movb $1, 1000(%r10)
    movb $1, 1000(%r11)
    movb $1, 1000(%r12)
    movb $1, 1000(%r13)
    movb $1, 1000(%r14)
    movb $1, 1000(%r15)

    movl $0xffffff, 1000(%eax)
    movl $0xffffff, 1000(%ecx)
    movl $0xffffff, 1000(%edx)
    movl $0xffffff, 1000(%ebx)
    movl $0xffffff, 1000(%esp)
    movl $0xffffff, 1000(%ebp)
    movl $0xffffff, 1000(%esi)
    movl $0xffffff, 1000(%edi)

    movl $0xffffff, 1000(%rax)
    movl $0xffffff, 1000(%rcx)
    movl $0xffffff, 1000(%rdx)
    movl $0xffffff, 1000(%rbx)
    movl $0xffffff, 1000(%rsp)
    movl $0xffffff, 1000(%rbp)
    movl $0xffffff, 1000(%rsi)
    movl $0xffffff, 1000(%rdi)

    movl $0xffffff, 1000(%r8d)
    movl $0xffffff, 1000(%r9d)
    movl $0xffffff, 1000(%r10d)
    movl $0xffffff, 1000(%r11d)
    movl $0xffffff, 1000(%r12d)
    movl $0xffffff, 1000(%r13d)
    movl $0xffffff, 1000(%r14d)
    movl $0xffffff, 1000(%r15d)

    movl $0xffffff, 1000(%r8)
    movl $0xffffff, 1000(%r9)
    movl $0xffffff, 1000(%r10)
    movl $0xffffff, 1000(%r11)
    movl $0xffffff, 1000(%r12)
    movl $0xffffff, 1000(%r13)
    movl $0xffffff, 1000(%r14)
    movl $0xffffff, 1000(%r15)

    movq $0xffffff, 1000(%eax)
    movq $0xffffff, 1000(%ecx)
    movq $0xffffff, 1000(%edx)
    movq $0xffffff, 1000(%ebx)
    movq $0xffffff, 1000(%esp)
    movq $0xffffff, 1000(%ebp)
    movq $0xffffff, 1000(%esi)
    movq $0xffffff, 1000(%edi)

    movq $0xffffff, 1000(%rax)
    movq $0xffffff, 1000(%rcx)
    movq $0xffffff, 1000(%rdx)
    movq $0xffffff, 1000(%rbx)
    movq $0xffffff, 1000(%rsp)
    movq $0xffffff, 1000(%rbp)
    movq $0xffffff, 1000(%rsi)
    movq $0xffffff, 1000(%rdi)

    movq $0xffffff, 1000(%r8d)
    movq $0xffffff, 1000(%r9d)
    movq $0xffffff, 1000(%r10d)
    movq $0xffffff, 1000(%r11d)
    movq $0xffffff, 1000(%r12d)
    movq $0xffffff, 1000(%r13d)
    movq $0xffffff, 1000(%r14d)
    movq $0xffffff, 1000(%r15d)

    movq $0xffffff, 1000(%r8)
    movq $0xffffff, 1000(%r9)
    movq $0xffffff, 1000(%r10)
    movq $0xffffff, 1000(%r11)
    movq $0xffffff, 1000(%r12)
    movq $0xffffff, 1000(%r13)
    movq $0xffffff, 1000(%r14)
    movq $0xffffff, 1000(%r15)

    movb $1, (%rcx)
    movl %edx, 4(%rcx)
    movq 0x10(%rcx), %rcx
    movl $1, (%rcx)
    movb $1, 0(%rcx)
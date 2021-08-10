.globl _start

_start:
    # RegReg

    add %al, %al
    add %al, %cl
    add %al, %dl
    add %al, %bl
    add %al, %ah
    add %al, %ch
    add %al, %dh
    add %al, %bh

    add %cl, %al
    add %cl, %cl
    add %cl, %dl
    add %cl, %bl
    add %cl, %ah
    add %cl, %ch
    add %cl, %dh
    add %cl, %bh

    add %dl, %al
    add %dl, %cl
    add %dl, %dl
    add %dl, %bl
    add %dl, %ah
    add %dl, %ch
    add %dl, %dh
    add %dl, %bh

    add %bl, %al
    add %bl, %cl
    add %bl, %dl
    add %bl, %bl
    add %bl, %ah
    add %bl, %ch
    add %bl, %dh
    add %bl, %bh

    add %ah, %al
    add %ah, %cl
    add %ah, %dl
    add %ah, %bl
    add %ah, %ah
    add %ah, %ch
    add %ah, %dh
    add %ah, %bh

    add %ch, %al
    add %ch, %cl
    add %ch, %dl
    add %ch, %bl
    add %ch, %ah
    add %ch, %ch
    add %ch, %dh
    add %ch, %bh

    add %dh, %al
    add %dh, %cl
    add %dh, %dl
    add %dh, %bl
    add %dh, %ah
    add %dh, %ch
    add %dh, %dh
    add %dh, %bh

    add %bh, %al
    add %bh, %cl
    add %bh, %dl
    add %bh, %bl
    add %bh, %ah
    add %bh, %ch
    add %bh, %dh
    add %bh, %bh

    add %ax, %ax
    add %ax, %cx
    add %ax, %dx
    add %ax, %bx
    add %ax, %sp
    add %ax, %bp
    add %ax, %si
    add %ax, %di

    add %cx, %ax
    add %cx, %cx
    add %cx, %dx
    add %cx, %bx
    add %cx, %sp
    add %cx, %bp
    add %cx, %si
    add %cx, %di

    add %dx, %ax
    add %dx, %cx
    add %dx, %dx
    add %dx, %bx
    add %dx, %sp
    add %dx, %bp
    add %dx, %si
    add %dx, %di

    add %bx, %ax
    add %bx, %cx
    add %bx, %dx
    add %bx, %bx
    add %bx, %sp
    add %bx, %bp
    add %bx, %si
    add %bx, %di

    add %sp, %ax
    add %sp, %cx
    add %sp, %dx
    add %sp, %bx
    add %sp, %sp
    add %sp, %bp
    add %sp, %si
    add %sp, %di

    add %bp, %ax
    add %bp, %cx
    add %bp, %dx
    add %bp, %bx
    add %bp, %sp
    add %bp, %bp
    add %bp, %si
    add %bp, %di

    add %si, %ax
    add %si, %cx
    add %si, %dx
    add %si, %bx
    add %si, %sp
    add %si, %bp
    add %si, %si
    add %si, %di

    add %di, %ax
    add %di, %cx
    add %di, %dx
    add %di, %bx
    add %di, %sp
    add %di, %bp
    add %di, %si
    add %di, %di

    add %eax, %eax
    add %eax, %ecx
    add %eax, %edx
    add %eax, %ebx
    add %eax, %esp
    add %eax, %ebp
    add %eax, %esi
    add %eax, %edi

    add %ecx, %eax
    add %ecx, %ecx
    add %ecx, %edx
    add %ecx, %ebx
    add %ecx, %esp
    add %ecx, %ebp
    add %ecx, %esi
    add %ecx, %edi

    add %edx, %eax
    add %edx, %ecx
    add %edx, %edx
    add %edx, %ebx
    add %edx, %esp
    add %edx, %ebp
    add %edx, %esi
    add %edx, %edi

    add %ebx, %eax
    add %ebx, %ecx
    add %ebx, %edx
    add %ebx, %ebx
    add %ebx, %esp
    add %ebx, %ebp
    add %ebx, %esi
    add %ebx, %edi

    add %esp, %eax
    add %esp, %ecx
    add %esp, %edx
    add %esp, %ebx
    add %esp, %esp
    add %esp, %ebp
    add %esp, %esi
    add %esp, %edi

    add %ebp, %eax
    add %ebp, %ecx
    add %ebp, %edx
    add %ebp, %ebx
    add %ebp, %esp
    add %ebp, %ebp
    add %ebp, %esi
    add %ebp, %edi

    add %esi, %eax
    add %esi, %ecx
    add %esi, %edx
    add %esi, %ebx
    add %esi, %esp
    add %esi, %ebp
    add %esi, %esi
    add %esi, %edi

    add %edi, %eax
    add %edi, %ecx
    add %edi, %edx
    add %edi, %ebx
    add %edi, %esp
    add %edi, %ebp
    add %edi, %esi
    add %edi, %edi

    add %rax, %rax
    add %rax, %rcx
    add %rax, %rdx
    add %rax, %rbx
    add %rax, %rsp
    add %rax, %rbp
    add %rax, %rsi
    add %rax, %rdi

    add %rcx, %rax
    add %rcx, %rcx
    add %rcx, %rdx
    add %rcx, %rbx
    add %rcx, %rsp
    add %rcx, %rbp
    add %rcx, %rsi
    add %rcx, %rdi

    add %rdx, %rax
    add %rdx, %rcx
    add %rdx, %rdx
    add %rdx, %rbx
    add %rdx, %rsp
    add %rdx, %rbp
    add %rdx, %rsi
    add %rdx, %rdi

    add %rbx, %rax
    add %rbx, %rcx
    add %rbx, %rdx
    add %rbx, %rbx
    add %rbx, %rsp
    add %rbx, %rbp
    add %rbx, %rsi
    add %rbx, %rdi

    add %rsp, %rax
    add %rsp, %rcx
    add %rsp, %rdx
    add %rsp, %rbx
    add %rsp, %rsp
    add %rsp, %rbp
    add %rsp, %rsi
    add %rsp, %rdi

    add %rbp, %rax
    add %rbp, %rcx
    add %rbp, %rdx
    add %rbp, %rbx
    add %rbp, %rsp
    add %rbp, %rbp
    add %rbp, %rsi
    add %rbp, %rdi

    add %rsi, %rax
    add %rsi, %rcx
    add %rsi, %rdx
    add %rsi, %rbx
    add %rsi, %rsp
    add %rsi, %rbp
    add %rsi, %rsi
    add %rsi, %rdi

    add %rdi, %rax
    add %rdi, %rcx
    add %rdi, %rdx
    add %rdi, %rbx
    add %rdi, %rsp
    add %rdi, %rbp
    add %rdi, %rsi
    add %rdi, %rdi

    add %r8, %r8
    add %r8, %r9
    add %r8, %r10
    add %r8, %r11
    add %r8, %r12
    add %r8, %r13
    add %r8, %r14
    add %r8, %r15

    add %r9, %r8
    add %r9, %r9
    add %r9, %r10
    add %r9, %r11
    add %r9, %r12
    add %r9, %r13
    add %r9, %r14
    add %r9, %r15

    add %r10, %r8
    add %r10, %r9
    add %r10, %r10
    add %r10, %r11
    add %r10, %r12
    add %r10, %r13
    add %r10, %r14
    add %r10, %r15

    add %r11, %r8
    add %r11, %r9
    add %r11, %r10
    add %r11, %r11
    add %r11, %r12
    add %r11, %r13
    add %r11, %r14
    add %r11, %r15

    add %r12, %r8
    add %r12, %r9
    add %r12, %r10
    add %r12, %r11
    add %r12, %r12
    add %r12, %r13
    add %r12, %r14
    add %r12, %r15

    add %r13, %r8
    add %r13, %r9
    add %r13, %r10
    add %r13, %r11
    add %r13, %r12
    add %r13, %r13
    add %r13, %r14
    add %r13, %r15

    add %r14, %r8
    add %r14, %r9
    add %r14, %r10
    add %r14, %r11
    add %r14, %r12
    add %r14, %r13
    add %r14, %r14
    add %r14, %r15

    add %r15, %r8
    add %r15, %r9
    add %r15, %r10
    add %r15, %r11
    add %r15, %r12
    add %r15, %r13
    add %r15, %r14
    add %r15, %r15

    add %r8w, %r8w
    add %r8w, %r9w
    add %r8w, %r10w
    add %r8w, %r11w
    add %r8w, %r12w
    add %r8w, %r13w
    add %r8w, %r14w
    add %r8w, %r15w

    add %r9w, %r8w
    add %r9w, %r9w
    add %r9w, %r10w
    add %r9w, %r11w
    add %r9w, %r12w
    add %r9w, %r13w
    add %r9w, %r14w
    add %r9w, %r15w

    add %r10w, %r8w
    add %r10w, %r9w
    add %r10w, %r10w
    add %r10w, %r11w
    add %r10w, %r12w
    add %r10w, %r13w
    add %r10w, %r14w
    add %r10w, %r15w

    add %r11w, %r8w
    add %r11w, %r9w
    add %r11w, %r10w
    add %r11w, %r11w
    add %r11w, %r12w
    add %r11w, %r13w
    add %r11w, %r14w
    add %r11w, %r15w

    add %r12w, %r8w
    add %r12w, %r9w
    add %r12w, %r10w
    add %r12w, %r11w
    add %r12w, %r12w
    add %r12w, %r13w
    add %r12w, %r14w
    add %r12w, %r15w

    add %r13w, %r8w
    add %r13w, %r9w
    add %r13w, %r10w
    add %r13w, %r11w
    add %r13w, %r12w
    add %r13w, %r13w
    add %r13w, %r14w
    add %r13w, %r15w

    add %r14w, %r8w
    add %r14w, %r9w
    add %r14w, %r10w
    add %r14w, %r11w
    add %r14w, %r12w
    add %r14w, %r13w
    add %r14w, %r14w
    add %r14w, %r15w

    add %r15w, %r8w
    add %r15w, %r9w
    add %r15w, %r10w
    add %r15w, %r11w
    add %r15w, %r12w
    add %r15w, %r13w
    add %r15w, %r14w
    add %r15w, %r15w

    add %r8, %rax
    add %r8, %rcx
    add %r8, %rdx
    add %r8, %rbx
    add %r8, %rsp
    add %r8, %rbp
    add %r8, %rsi
    add %r8, %rdi

    add %r9, %rax
    add %r9, %rcx
    add %r9, %rdx
    add %r9, %rbx
    add %r9, %rsp
    add %r9, %rbp
    add %r9, %rsi
    add %r9, %rdi

    add %r10, %rax
    add %r10, %rcx
    add %r10, %rdx
    add %r10, %rbx
    add %r10, %rsp
    add %r10, %rbp
    add %r10, %rsi
    add %r10, %rdi

    add %r11, %rax
    add %r11, %rcx
    add %r11, %rdx
    add %r11, %rbx
    add %r11, %rsp
    add %r11, %rbp
    add %r11, %rsi
    add %r11, %rdi

    add %r12, %rax
    add %r12, %rcx
    add %r12, %rdx
    add %r12, %rbx
    add %r12, %rsp
    add %r12, %rbp
    add %r12, %rsi
    add %r12, %rdi

    add %r13, %rax
    add %r13, %rcx
    add %r13, %rdx
    add %r13, %rbx
    add %r13, %rsp
    add %r13, %rbp
    add %r13, %rsi
    add %r13, %rdi

    add %r14, %rax
    add %r14, %rcx
    add %r14, %rdx
    add %r14, %rbx
    add %r14, %rsp
    add %r14, %rbp
    add %r14, %rsi
    add %r14, %rdi

    add %r15, %rax
    add %r15, %rcx
    add %r15, %rdx
    add %r15, %rbx
    add %r15, %rsp
    add %r15, %rbp
    add %r15, %rsi
    add %r15, %rdi

    add %ax, %r8w
    add %ax, %r9w
    add %ax, %r10w
    add %ax, %r11w
    add %ax, %r12w
    add %ax, %r13w
    add %ax, %r14w
    add %ax, %r15w

    add %cx, %r8w
    add %cx, %r9w
    add %cx, %r10w
    add %cx, %r11w
    add %cx, %r12w
    add %cx, %r13w
    add %cx, %r14w
    add %cx, %r15w

    add %dx, %r8w
    add %dx, %r9w
    add %dx, %r10w
    add %dx, %r11w
    add %dx, %r12w
    add %dx, %r13w
    add %dx, %r14w
    add %dx, %r15w

    add %bx, %r8w
    add %bx, %r9w
    add %bx, %r10w
    add %bx, %r11w
    add %bx, %r12w
    add %bx, %r13w
    add %bx, %r14w
    add %bx, %r15w

    add %sp, %r8w
    add %sp, %r9w
    add %sp, %r10w
    add %sp, %r11w
    add %sp, %r12w
    add %sp, %r13w
    add %sp, %r14w
    add %sp, %r15w

    add %bp, %r8w
    add %bp, %r9w
    add %bp, %r10w
    add %bp, %r11w
    add %bp, %r12w
    add %bp, %r13w
    add %bp, %r14w
    add %bp, %r15w

    add %si, %r8w
    add %si, %r9w
    add %si, %r10w
    add %si, %r11w
    add %si, %r12w
    add %si, %r13w
    add %si, %r14w
    add %si, %r15w

    add %di, %r8w
    add %di, %r9w
    add %di, %r10w
    add %di, %r11w
    add %di, %r12w
    add %di, %r13w
    add %di, %r14w
    add %di, %r15w

    add %r8w, %ax
    add %r8w, %cx
    add %r8w, %dx
    add %r8w, %bx
    add %r8w, %sp
    add %r8w, %bp
    add %r8w, %si
    add %r8w, %di

    add %r9w, %ax
    add %r9w, %cx
    add %r9w, %dx
    add %r9w, %bx
    add %r9w, %sp
    add %r9w, %bp
    add %r9w, %si
    add %r9w, %di

    add %r10w, %ax
    add %r10w, %cx
    add %r10w, %dx
    add %r10w, %bx
    add %r10w, %sp
    add %r10w, %bp
    add %r10w, %si
    add %r10w, %di

    add %r11w, %ax
    add %r11w, %cx
    add %r11w, %dx
    add %r11w, %bx
    add %r11w, %sp
    add %r11w, %bp
    add %r11w, %si
    add %r11w, %di

    add %r12w, %ax
    add %r12w, %cx
    add %r12w, %dx
    add %r12w, %bx
    add %r12w, %sp
    add %r12w, %bp
    add %r12w, %si
    add %r12w, %di

    add %r13w, %ax
    add %r13w, %cx
    add %r13w, %dx
    add %r13w, %bx
    add %r13w, %sp
    add %r13w, %bp
    add %r13w, %si
    add %r13w, %di

    add %r14w, %ax
    add %r14w, %cx
    add %r14w, %dx
    add %r14w, %bx
    add %r14w, %sp
    add %r14w, %bp
    add %r14w, %si
    add %r14w, %di

    add %r15w, %ax
    add %r15w, %cx
    add %r15w, %dx
    add %r15w, %bx
    add %r15w, %sp
    add %r15w, %bp
    add %r15w, %si
    add %r15w, %di

    add 0x01, %al
    add 0x01, %cl
    add 0x01, %dl
    add 0x01, %bl
    add 0x01, %ah
    add 0x01, %ch
    add 0x01, %dh
    add 0x01, %bh

    add 0x01, %ax
    add 0x01, %cx
    add 0x01, %dx
    add 0x01, %bx
    add 0x01, %sp
    add 0x01, %bp
    add 0x01, %si
    add 0x01, %di

    add 0x01, %eax
    add 0x01, %ecx
    add 0x01, %edx
    add 0x01, %ebx
    add 0x01, %esp
    add 0x01, %ebp
    add 0x01, %esi
    add 0x01, %edi

    add 0x01, %rax
    add 0x01, %rcx
    add 0x01, %rdx
    add 0x01, %rbx
    add 0x01, %rsp
    add 0x01, %rbp
    add 0x01, %rsi
    add 0x01, %rdi

    add 0x01, %r8b
    add 0x01, %r9b
    add 0x01, %r10b
    add 0x01, %r11b
    add 0x01, %r12b
    add 0x01, %r13b
    add 0x01, %r14b
    add 0x01, %r15b

    add 0x01, %r8w
    add 0x01, %r9w
    add 0x01, %r10w
    add 0x01, %r11w
    add 0x01, %r12w
    add 0x01, %r13w
    add 0x01, %r14w
    add 0x01, %r15w

    add 0x01, %r8d
    add 0x01, %r9d
    add 0x01, %r10d
    add 0x01, %r11d
    add 0x01, %r12d
    add 0x01, %r13d
    add 0x01, %r14d
    add 0x01, %r15d

    add 0x01, %r8
    add 0x01, %r9
    add 0x01, %r10
    add 0x01, %r11
    add 0x01, %r12
    add 0x01, %r13
    add 0x01, %r14
    add 0x01, %r15

    add 0x1337, %al
    add 0x1337, %cl
    add 0x1337, %dl
    add 0x1337, %bl
    add 0x1337, %ah
    add 0x1337, %ch
    add 0x1337, %dh
    add 0x1337, %bh

    add 0x1337, %ax
    add 0x1337, %cx
    add 0x1337, %dx
    add 0x1337, %bx
    add 0x1337, %sp
    add 0x1337, %bp
    add 0x1337, %si
    add 0x1337, %di

    add 0x1337, %eax
    add 0x1337, %ecx
    add 0x1337, %edx
    add 0x1337, %ebx
    add 0x1337, %esp
    add 0x1337, %ebp
    add 0x1337, %esi
    add 0x1337, %edi

    add 0x1337, %rax
    add 0x1337, %rcx
    add 0x1337, %rdx
    add 0x1337, %rbx
    add 0x1337, %rsp
    add 0x1337, %rbp
    add 0x1337, %rsi
    add 0x1337, %rdi

    add 0x1337, %r8b
    add 0x1337, %r9b
    add 0x1337, %r10b
    add 0x1337, %r11b
    add 0x1337, %r12b
    add 0x1337, %r13b
    add 0x1337, %r14b
    add 0x1337, %r15b

    add 0x1337, %r8w
    add 0x1337, %r9w
    add 0x1337, %r10w
    add 0x1337, %r11w
    add 0x1337, %r12w
    add 0x1337, %r13w
    add 0x1337, %r14w
    add 0x1337, %r15w

    add 0x1337, %r8d
    add 0x1337, %r9d
    add 0x1337, %r10d
    add 0x1337, %r11d
    add 0x1337, %r12d
    add 0x1337, %r13d
    add 0x1337, %r14d
    add 0x1337, %r15d

    add 0x1337, %r8
    add 0x1337, %r9
    add 0x1337, %r10
    add 0x1337, %r11
    add 0x1337, %r12
    add 0x1337, %r13
    add 0x1337, %r14
    add 0x1337, %r15

    add 0x7fffffff, %eax

    add $1, %al
    add $1, %ax
    add $1, %eax
    add $1, %rax
    add $1, %rdi
    add $1, %r8

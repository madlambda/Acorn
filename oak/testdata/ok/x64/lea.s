.globl _start

_start:
    lea (%rax), %ax
    lea (%rax), %cx
    lea (%rax), %dx
    lea (%rax), %bx
    lea (%rax), %sp
    lea (%rax), %bp
    lea (%rax), %si
    lea (%rax), %di

    lea (%rax), %eax
    lea (%rax), %ecx
    lea (%rax), %edx
    lea (%rax), %ebx
    lea (%rax), %esp
    lea (%rax), %ebp
    lea (%rax), %esi
    lea (%rax), %edi

    lea (%rax), %rax
    lea (%rax), %rcx
    lea (%rax), %rdx
    lea (%rax), %rbx
    lea (%rax), %rsp
    lea (%rax), %rbp
    lea (%rax), %rsi
    lea (%rax), %rdi

    lea (%rax), %r8
    lea (%rax), %r9
    lea (%rax), %r10
    lea (%rax), %r11
    lea (%rax), %r12
    lea (%rax), %r13
    lea (%rax), %r14
    lea (%rax), %r15

    lea (%rax), %r8d
    lea (%rax), %r9d
    lea (%rax), %r10d
    lea (%rax), %r11d
    lea (%rax), %r12d
    lea (%rax), %r13d
    lea (%rax), %r14d
    lea (%rax), %r15d

    lea (%eax), %rax
    lea (%ecx), %rax
    lea (%edx), %rax
    lea (%ebx), %rax
    lea (%esp), %rax
    lea (%ebp), %rax
    lea (%esi), %rax
    lea (%edi), %rax

    lea (%rax), %rax
    lea (%rcx), %rax
    lea (%rdx), %rax
    lea (%rbx), %rax
    lea (%rsp), %rax
    lea (%rbp), %rax
    lea (%rsi), %rax
    lea (%rdi), %rax

    lea 8(%eax), %rax
    lea 8(%ecx), %rax
    lea 8(%edx), %rax
    lea 8(%ebx), %rax
    lea 8(%esp), %rax
    lea 8(%ebp), %rax
    lea 8(%esi), %rax
    lea 8(%edi), %rax

    lea 8(%rax), %rax
    lea 8(%rcx), %rax
    lea 8(%rdx), %rax
    lea 8(%rbx), %rax
    lea 8(%rsp), %rax
    lea 8(%rbp), %rax
    lea 8(%rsi), %rax
    lea 8(%rdi), %rax

    lea 1000(%eax), %rax
    lea 1000(%ecx), %rax
    lea 1000(%edx), %rax
    lea 1000(%ebx), %rax
    lea 1000(%esp), %rax
    lea 1000(%ebp), %rax
    lea 1000(%esi), %rax
    lea 1000(%edi), %rax

    lea 1000(%rax), %rax
    lea 1000(%rcx), %rax
    lea 1000(%rdx), %rax
    lea 1000(%rbx), %rax
    lea 1000(%rsp), %rax
    lea 1000(%rbp), %rax
    lea 1000(%rsi), %rax
    lea 1000(%rdi), %rax

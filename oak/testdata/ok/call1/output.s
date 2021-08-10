.globl call1

call1:
    sub $0x100, %rsp
    mov %rdi, (%rsp)
    mov %rsi, 8(%rsp)

    xor %rax, %rax

    # i32.const 42
    mov $42, %rdi

    # prepare locals

    # lookup function in fn->instance->funcs
    mov (%rsp), %rax
    mov 0x28(%rax), %rax
    mov 0x20(%rax), %rax

    # lookup array->items
    mov 0x10(%rax), %rax
    mov %rax, %rdi
    mov (%rax), %rax

    # rsi = funcall-local
    # rsi->locals = rsi + sizeof(Local)
    mov 0x10(%rsp), %rsi
    mov %rsi, %rcx
    add $0x10, %rcx
    mov %rcx, (%rsi)
    movl $0, 0x10(%rcx)
    call *%rax

    add $0x100, %rsp
    retq

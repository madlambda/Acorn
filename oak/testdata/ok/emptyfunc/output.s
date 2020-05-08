.globl empty

empty:
    sub $0x10, %rsp
    mov %rdi, (%rsp)
    mov %rsi, 8(%rsp)
    xor %rax, %rax
    add $0x10, %rsp
    retq

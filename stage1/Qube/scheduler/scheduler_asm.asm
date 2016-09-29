.intel_syntax noprefix
.text
.code64
.globl push_swap_pop
push_swap_pop:
push rax
push rbx
push rcx
push rdx
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
push rsi
push rdi
push rbp
mov [rsi],rsp # save the old stack
mov rsp,rdi  # and load the new stack
pop rbp
pop rdi
pop rsi
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rdx
pop rcx
pop rbx
pop rax
ret
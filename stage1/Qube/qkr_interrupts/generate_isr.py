# This file generate the file isr.S that contains 256 entries for the IDT.

TEMPLATE = """
.local   isr_wrapper{interrupt_number}
.align   4

isr_wrapper{interrupt_number}:
	{push_dummy_error_code}
	push ${interrupt_number} //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //

"""
ptrs = """
	.data
	.global isrs_list
	isrs_list:
"""

entry_points = ""
for i in xrange(0x100):
	error_code = ""
	if i not in (8, 10,11,12,13,14,17):
		error_code = "push $0 // push dummy error code"
	entry_points += TEMPLATE.format(interrupt_number = i, push_dummy_error_code = error_code)
	ptrs += """.quad isr_wrapper%d
	""" % i

REST_OF_FILE = """
isr_asm:
	// manual push registers:
	sub $0xa0, %rsp   // make room for the manual-pushed registers.
	movq	%r15, 0x90(%rsp)//
	movq	%r14, 0x88(%rsp)//
	movq	%r13, 0x80(%rsp)//
	movq	%r12, 0x78(%rsp)//
	movq	%r11, 0x70(%rsp)//
	movq	%r10, 0x68(%rsp)//
	movq	%r9,  0x60(%rsp)//
	movq	%r8,  0x58(%rsp)//
	movq	%rsi, 0x50(%rsp)//
	movq	%rdi, 0x48(%rsp)//
	movq	%rbp, 0x40(%rsp)//
	movq	%rdx, 0x38(%rsp)//
	movq	%rcx, 0x30(%rsp)//
	movq	%rbx, 0x28(%rsp)//
	movq	%rax, 0x20(%rsp)//
	xorl	%eax, %eax//
	movw	%gs, %ax//
    movq	%rax, 0x18(%rsp)//
    movw	%fs, %ax//
    movq	%rax, 0x10(%rsp)//
    movw	%es, %ax//
    movq	%rax, 0x08(%rsp)//
    movw	%ds, %ax//
    movq	%rax, 0x00(%rsp)//

	// Call the C handle routine
	//push %rsp // move the Registers struct as parameter.
	movq    %rsp, %rdi // move the Registers struct as parameter.
	//add $8, %rdi
	cld // /* C code following the sysV ABI requires DF to be clear on function entry */
	call handle_interrupts // C routine
	//add $8, %rsp//
	
	// Manual pop the registers:
	movq	0x00(%rsp), %rax//
	movw	%ax, %ds//
	movq	0x08(%rsp), %rax//
	movw	%ax, %es//
	movq	0x10(%rsp), %rax//
	movw	%ax, %fs//
	movq	0x18(%rsp), %rax//
	movw	%ax, %gs//

	movq	0x20(%rsp), %rax //
	movq	0x28(%rsp), %rbx //
	movq	0x30(%rsp), %rcx //
	movq	0x38(%rsp), %rdx //
	movq	0x40(%rsp), %rbp //
	movq	0x48(%rsp), %rdi //
	movq	0x50(%rsp), %rsi //
	movq	0x58(%rsp),  %r8  //  
	movq	0x60(%rsp),  %r9  //
	movq	0x68(%rsp), %r10 //
	movq	0x70(%rsp), %r11 //	
	movq	0x78(%rsp), %r12 //
	movq	0x80(%rsp), %r13 //
	movq	0x88(%rsp), %r14 //
	movq	0x90(%rsp), %r15 //
	add $0xa0, %rsp   // restore %rsp
	ret //
"""


all = REST_OF_FILE + entry_points + ptrs
f = open("isrs.S","rb")
old = f.read()
f.close()
if all != old:
	f = open("isrs.S","wb")
	f.write(all)
	f.close()

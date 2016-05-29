
isr_asm:
	// manual push registers:
	sub $0x98, %rsp   // make room for the manual-pushed registers.
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
	add $8, %rdi
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
	add $0x98, %rsp   // restore %rsp
	ret //

.local   isr_wrapper0
.align   4

isr_wrapper0:
	push $0 // push dummy error code
	push $0 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper1
.align   4

isr_wrapper1:
	push $0 // push dummy error code
	push $1 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper2
.align   4

isr_wrapper2:
	push $0 // push dummy error code
	push $2 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper3
.align   4

isr_wrapper3:
	push $0 // push dummy error code
	push $3 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper4
.align   4

isr_wrapper4:
	push $0 // push dummy error code
	push $4 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper5
.align   4

isr_wrapper5:
	push $0 // push dummy error code
	push $5 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper6
.align   4

isr_wrapper6:
	push $0 // push dummy error code
	push $6 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper7
.align   4

isr_wrapper7:
	push $0 // push dummy error code
	push $7 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper8
.align   4

isr_wrapper8:
	
	push $8 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper9
.align   4

isr_wrapper9:
	push $0 // push dummy error code
	push $9 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper10
.align   4

isr_wrapper10:
	
	push $10 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper11
.align   4

isr_wrapper11:
	
	push $11 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper12
.align   4

isr_wrapper12:
	
	push $12 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper13
.align   4

isr_wrapper13:
	
	push $13 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper14
.align   4

isr_wrapper14:
	
	push $14 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper15
.align   4

isr_wrapper15:
	push $0 // push dummy error code
	push $15 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper16
.align   4

isr_wrapper16:
	push $0 // push dummy error code
	push $16 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper17
.align   4

isr_wrapper17:
	
	push $17 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper18
.align   4

isr_wrapper18:
	push $0 // push dummy error code
	push $18 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper19
.align   4

isr_wrapper19:
	push $0 // push dummy error code
	push $19 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper20
.align   4

isr_wrapper20:
	push $0 // push dummy error code
	push $20 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper21
.align   4

isr_wrapper21:
	push $0 // push dummy error code
	push $21 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper22
.align   4

isr_wrapper22:
	push $0 // push dummy error code
	push $22 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper23
.align   4

isr_wrapper23:
	push $0 // push dummy error code
	push $23 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper24
.align   4

isr_wrapper24:
	push $0 // push dummy error code
	push $24 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper25
.align   4

isr_wrapper25:
	push $0 // push dummy error code
	push $25 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper26
.align   4

isr_wrapper26:
	push $0 // push dummy error code
	push $26 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper27
.align   4

isr_wrapper27:
	push $0 // push dummy error code
	push $27 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper28
.align   4

isr_wrapper28:
	push $0 // push dummy error code
	push $28 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper29
.align   4

isr_wrapper29:
	push $0 // push dummy error code
	push $29 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper30
.align   4

isr_wrapper30:
	push $0 // push dummy error code
	push $30 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper31
.align   4

isr_wrapper31:
	push $0 // push dummy error code
	push $31 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper32
.align   4

isr_wrapper32:
	push $0 // push dummy error code
	push $32 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper33
.align   4

isr_wrapper33:
	push $0 // push dummy error code
	push $33 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper34
.align   4

isr_wrapper34:
	push $0 // push dummy error code
	push $34 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper35
.align   4

isr_wrapper35:
	push $0 // push dummy error code
	push $35 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper36
.align   4

isr_wrapper36:
	push $0 // push dummy error code
	push $36 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper37
.align   4

isr_wrapper37:
	push $0 // push dummy error code
	push $37 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper38
.align   4

isr_wrapper38:
	push $0 // push dummy error code
	push $38 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper39
.align   4

isr_wrapper39:
	push $0 // push dummy error code
	push $39 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper40
.align   4

isr_wrapper40:
	push $0 // push dummy error code
	push $40 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper41
.align   4

isr_wrapper41:
	push $0 // push dummy error code
	push $41 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper42
.align   4

isr_wrapper42:
	push $0 // push dummy error code
	push $42 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper43
.align   4

isr_wrapper43:
	push $0 // push dummy error code
	push $43 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper44
.align   4

isr_wrapper44:
	push $0 // push dummy error code
	push $44 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper45
.align   4

isr_wrapper45:
	push $0 // push dummy error code
	push $45 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper46
.align   4

isr_wrapper46:
	push $0 // push dummy error code
	push $46 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper47
.align   4

isr_wrapper47:
	push $0 // push dummy error code
	push $47 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper48
.align   4

isr_wrapper48:
	push $0 // push dummy error code
	push $48 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper49
.align   4

isr_wrapper49:
	push $0 // push dummy error code
	push $49 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper50
.align   4

isr_wrapper50:
	push $0 // push dummy error code
	push $50 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper51
.align   4

isr_wrapper51:
	push $0 // push dummy error code
	push $51 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper52
.align   4

isr_wrapper52:
	push $0 // push dummy error code
	push $52 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper53
.align   4

isr_wrapper53:
	push $0 // push dummy error code
	push $53 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper54
.align   4

isr_wrapper54:
	push $0 // push dummy error code
	push $54 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper55
.align   4

isr_wrapper55:
	push $0 // push dummy error code
	push $55 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper56
.align   4

isr_wrapper56:
	push $0 // push dummy error code
	push $56 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper57
.align   4

isr_wrapper57:
	push $0 // push dummy error code
	push $57 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper58
.align   4

isr_wrapper58:
	push $0 // push dummy error code
	push $58 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper59
.align   4

isr_wrapper59:
	push $0 // push dummy error code
	push $59 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper60
.align   4

isr_wrapper60:
	push $0 // push dummy error code
	push $60 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper61
.align   4

isr_wrapper61:
	push $0 // push dummy error code
	push $61 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper62
.align   4

isr_wrapper62:
	push $0 // push dummy error code
	push $62 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper63
.align   4

isr_wrapper63:
	push $0 // push dummy error code
	push $63 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper64
.align   4

isr_wrapper64:
	push $0 // push dummy error code
	push $64 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper65
.align   4

isr_wrapper65:
	push $0 // push dummy error code
	push $65 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper66
.align   4

isr_wrapper66:
	push $0 // push dummy error code
	push $66 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper67
.align   4

isr_wrapper67:
	push $0 // push dummy error code
	push $67 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper68
.align   4

isr_wrapper68:
	push $0 // push dummy error code
	push $68 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper69
.align   4

isr_wrapper69:
	push $0 // push dummy error code
	push $69 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper70
.align   4

isr_wrapper70:
	push $0 // push dummy error code
	push $70 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper71
.align   4

isr_wrapper71:
	push $0 // push dummy error code
	push $71 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper72
.align   4

isr_wrapper72:
	push $0 // push dummy error code
	push $72 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper73
.align   4

isr_wrapper73:
	push $0 // push dummy error code
	push $73 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper74
.align   4

isr_wrapper74:
	push $0 // push dummy error code
	push $74 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper75
.align   4

isr_wrapper75:
	push $0 // push dummy error code
	push $75 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper76
.align   4

isr_wrapper76:
	push $0 // push dummy error code
	push $76 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper77
.align   4

isr_wrapper77:
	push $0 // push dummy error code
	push $77 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper78
.align   4

isr_wrapper78:
	push $0 // push dummy error code
	push $78 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper79
.align   4

isr_wrapper79:
	push $0 // push dummy error code
	push $79 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper80
.align   4

isr_wrapper80:
	push $0 // push dummy error code
	push $80 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper81
.align   4

isr_wrapper81:
	push $0 // push dummy error code
	push $81 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper82
.align   4

isr_wrapper82:
	push $0 // push dummy error code
	push $82 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper83
.align   4

isr_wrapper83:
	push $0 // push dummy error code
	push $83 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper84
.align   4

isr_wrapper84:
	push $0 // push dummy error code
	push $84 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper85
.align   4

isr_wrapper85:
	push $0 // push dummy error code
	push $85 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper86
.align   4

isr_wrapper86:
	push $0 // push dummy error code
	push $86 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper87
.align   4

isr_wrapper87:
	push $0 // push dummy error code
	push $87 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper88
.align   4

isr_wrapper88:
	push $0 // push dummy error code
	push $88 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper89
.align   4

isr_wrapper89:
	push $0 // push dummy error code
	push $89 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper90
.align   4

isr_wrapper90:
	push $0 // push dummy error code
	push $90 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper91
.align   4

isr_wrapper91:
	push $0 // push dummy error code
	push $91 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper92
.align   4

isr_wrapper92:
	push $0 // push dummy error code
	push $92 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper93
.align   4

isr_wrapper93:
	push $0 // push dummy error code
	push $93 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper94
.align   4

isr_wrapper94:
	push $0 // push dummy error code
	push $94 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper95
.align   4

isr_wrapper95:
	push $0 // push dummy error code
	push $95 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper96
.align   4

isr_wrapper96:
	push $0 // push dummy error code
	push $96 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper97
.align   4

isr_wrapper97:
	push $0 // push dummy error code
	push $97 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper98
.align   4

isr_wrapper98:
	push $0 // push dummy error code
	push $98 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper99
.align   4

isr_wrapper99:
	push $0 // push dummy error code
	push $99 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper100
.align   4

isr_wrapper100:
	push $0 // push dummy error code
	push $100 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper101
.align   4

isr_wrapper101:
	push $0 // push dummy error code
	push $101 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper102
.align   4

isr_wrapper102:
	push $0 // push dummy error code
	push $102 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper103
.align   4

isr_wrapper103:
	push $0 // push dummy error code
	push $103 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper104
.align   4

isr_wrapper104:
	push $0 // push dummy error code
	push $104 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper105
.align   4

isr_wrapper105:
	push $0 // push dummy error code
	push $105 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper106
.align   4

isr_wrapper106:
	push $0 // push dummy error code
	push $106 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper107
.align   4

isr_wrapper107:
	push $0 // push dummy error code
	push $107 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper108
.align   4

isr_wrapper108:
	push $0 // push dummy error code
	push $108 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper109
.align   4

isr_wrapper109:
	push $0 // push dummy error code
	push $109 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper110
.align   4

isr_wrapper110:
	push $0 // push dummy error code
	push $110 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper111
.align   4

isr_wrapper111:
	push $0 // push dummy error code
	push $111 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper112
.align   4

isr_wrapper112:
	push $0 // push dummy error code
	push $112 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper113
.align   4

isr_wrapper113:
	push $0 // push dummy error code
	push $113 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper114
.align   4

isr_wrapper114:
	push $0 // push dummy error code
	push $114 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper115
.align   4

isr_wrapper115:
	push $0 // push dummy error code
	push $115 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper116
.align   4

isr_wrapper116:
	push $0 // push dummy error code
	push $116 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper117
.align   4

isr_wrapper117:
	push $0 // push dummy error code
	push $117 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper118
.align   4

isr_wrapper118:
	push $0 // push dummy error code
	push $118 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper119
.align   4

isr_wrapper119:
	push $0 // push dummy error code
	push $119 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper120
.align   4

isr_wrapper120:
	push $0 // push dummy error code
	push $120 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper121
.align   4

isr_wrapper121:
	push $0 // push dummy error code
	push $121 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper122
.align   4

isr_wrapper122:
	push $0 // push dummy error code
	push $122 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper123
.align   4

isr_wrapper123:
	push $0 // push dummy error code
	push $123 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper124
.align   4

isr_wrapper124:
	push $0 // push dummy error code
	push $124 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper125
.align   4

isr_wrapper125:
	push $0 // push dummy error code
	push $125 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper126
.align   4

isr_wrapper126:
	push $0 // push dummy error code
	push $126 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper127
.align   4

isr_wrapper127:
	push $0 // push dummy error code
	push $127 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper128
.align   4

isr_wrapper128:
	push $0 // push dummy error code
	push $128 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper129
.align   4

isr_wrapper129:
	push $0 // push dummy error code
	push $129 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper130
.align   4

isr_wrapper130:
	push $0 // push dummy error code
	push $130 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper131
.align   4

isr_wrapper131:
	push $0 // push dummy error code
	push $131 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper132
.align   4

isr_wrapper132:
	push $0 // push dummy error code
	push $132 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper133
.align   4

isr_wrapper133:
	push $0 // push dummy error code
	push $133 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper134
.align   4

isr_wrapper134:
	push $0 // push dummy error code
	push $134 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper135
.align   4

isr_wrapper135:
	push $0 // push dummy error code
	push $135 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper136
.align   4

isr_wrapper136:
	push $0 // push dummy error code
	push $136 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper137
.align   4

isr_wrapper137:
	push $0 // push dummy error code
	push $137 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper138
.align   4

isr_wrapper138:
	push $0 // push dummy error code
	push $138 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper139
.align   4

isr_wrapper139:
	push $0 // push dummy error code
	push $139 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper140
.align   4

isr_wrapper140:
	push $0 // push dummy error code
	push $140 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper141
.align   4

isr_wrapper141:
	push $0 // push dummy error code
	push $141 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper142
.align   4

isr_wrapper142:
	push $0 // push dummy error code
	push $142 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper143
.align   4

isr_wrapper143:
	push $0 // push dummy error code
	push $143 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper144
.align   4

isr_wrapper144:
	push $0 // push dummy error code
	push $144 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper145
.align   4

isr_wrapper145:
	push $0 // push dummy error code
	push $145 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper146
.align   4

isr_wrapper146:
	push $0 // push dummy error code
	push $146 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper147
.align   4

isr_wrapper147:
	push $0 // push dummy error code
	push $147 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper148
.align   4

isr_wrapper148:
	push $0 // push dummy error code
	push $148 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper149
.align   4

isr_wrapper149:
	push $0 // push dummy error code
	push $149 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper150
.align   4

isr_wrapper150:
	push $0 // push dummy error code
	push $150 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper151
.align   4

isr_wrapper151:
	push $0 // push dummy error code
	push $151 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper152
.align   4

isr_wrapper152:
	push $0 // push dummy error code
	push $152 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper153
.align   4

isr_wrapper153:
	push $0 // push dummy error code
	push $153 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper154
.align   4

isr_wrapper154:
	push $0 // push dummy error code
	push $154 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper155
.align   4

isr_wrapper155:
	push $0 // push dummy error code
	push $155 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper156
.align   4

isr_wrapper156:
	push $0 // push dummy error code
	push $156 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper157
.align   4

isr_wrapper157:
	push $0 // push dummy error code
	push $157 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper158
.align   4

isr_wrapper158:
	push $0 // push dummy error code
	push $158 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper159
.align   4

isr_wrapper159:
	push $0 // push dummy error code
	push $159 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper160
.align   4

isr_wrapper160:
	push $0 // push dummy error code
	push $160 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper161
.align   4

isr_wrapper161:
	push $0 // push dummy error code
	push $161 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper162
.align   4

isr_wrapper162:
	push $0 // push dummy error code
	push $162 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper163
.align   4

isr_wrapper163:
	push $0 // push dummy error code
	push $163 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper164
.align   4

isr_wrapper164:
	push $0 // push dummy error code
	push $164 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper165
.align   4

isr_wrapper165:
	push $0 // push dummy error code
	push $165 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper166
.align   4

isr_wrapper166:
	push $0 // push dummy error code
	push $166 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper167
.align   4

isr_wrapper167:
	push $0 // push dummy error code
	push $167 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper168
.align   4

isr_wrapper168:
	push $0 // push dummy error code
	push $168 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper169
.align   4

isr_wrapper169:
	push $0 // push dummy error code
	push $169 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper170
.align   4

isr_wrapper170:
	push $0 // push dummy error code
	push $170 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper171
.align   4

isr_wrapper171:
	push $0 // push dummy error code
	push $171 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper172
.align   4

isr_wrapper172:
	push $0 // push dummy error code
	push $172 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper173
.align   4

isr_wrapper173:
	push $0 // push dummy error code
	push $173 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper174
.align   4

isr_wrapper174:
	push $0 // push dummy error code
	push $174 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper175
.align   4

isr_wrapper175:
	push $0 // push dummy error code
	push $175 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper176
.align   4

isr_wrapper176:
	push $0 // push dummy error code
	push $176 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper177
.align   4

isr_wrapper177:
	push $0 // push dummy error code
	push $177 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper178
.align   4

isr_wrapper178:
	push $0 // push dummy error code
	push $178 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper179
.align   4

isr_wrapper179:
	push $0 // push dummy error code
	push $179 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper180
.align   4

isr_wrapper180:
	push $0 // push dummy error code
	push $180 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper181
.align   4

isr_wrapper181:
	push $0 // push dummy error code
	push $181 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper182
.align   4

isr_wrapper182:
	push $0 // push dummy error code
	push $182 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper183
.align   4

isr_wrapper183:
	push $0 // push dummy error code
	push $183 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper184
.align   4

isr_wrapper184:
	push $0 // push dummy error code
	push $184 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper185
.align   4

isr_wrapper185:
	push $0 // push dummy error code
	push $185 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper186
.align   4

isr_wrapper186:
	push $0 // push dummy error code
	push $186 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper187
.align   4

isr_wrapper187:
	push $0 // push dummy error code
	push $187 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper188
.align   4

isr_wrapper188:
	push $0 // push dummy error code
	push $188 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper189
.align   4

isr_wrapper189:
	push $0 // push dummy error code
	push $189 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper190
.align   4

isr_wrapper190:
	push $0 // push dummy error code
	push $190 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper191
.align   4

isr_wrapper191:
	push $0 // push dummy error code
	push $191 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper192
.align   4

isr_wrapper192:
	push $0 // push dummy error code
	push $192 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper193
.align   4

isr_wrapper193:
	push $0 // push dummy error code
	push $193 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper194
.align   4

isr_wrapper194:
	push $0 // push dummy error code
	push $194 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper195
.align   4

isr_wrapper195:
	push $0 // push dummy error code
	push $195 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper196
.align   4

isr_wrapper196:
	push $0 // push dummy error code
	push $196 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper197
.align   4

isr_wrapper197:
	push $0 // push dummy error code
	push $197 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper198
.align   4

isr_wrapper198:
	push $0 // push dummy error code
	push $198 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper199
.align   4

isr_wrapper199:
	push $0 // push dummy error code
	push $199 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper200
.align   4

isr_wrapper200:
	push $0 // push dummy error code
	push $200 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper201
.align   4

isr_wrapper201:
	push $0 // push dummy error code
	push $201 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper202
.align   4

isr_wrapper202:
	push $0 // push dummy error code
	push $202 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper203
.align   4

isr_wrapper203:
	push $0 // push dummy error code
	push $203 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper204
.align   4

isr_wrapper204:
	push $0 // push dummy error code
	push $204 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper205
.align   4

isr_wrapper205:
	push $0 // push dummy error code
	push $205 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper206
.align   4

isr_wrapper206:
	push $0 // push dummy error code
	push $206 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper207
.align   4

isr_wrapper207:
	push $0 // push dummy error code
	push $207 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper208
.align   4

isr_wrapper208:
	push $0 // push dummy error code
	push $208 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper209
.align   4

isr_wrapper209:
	push $0 // push dummy error code
	push $209 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper210
.align   4

isr_wrapper210:
	push $0 // push dummy error code
	push $210 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper211
.align   4

isr_wrapper211:
	push $0 // push dummy error code
	push $211 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper212
.align   4

isr_wrapper212:
	push $0 // push dummy error code
	push $212 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper213
.align   4

isr_wrapper213:
	push $0 // push dummy error code
	push $213 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper214
.align   4

isr_wrapper214:
	push $0 // push dummy error code
	push $214 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper215
.align   4

isr_wrapper215:
	push $0 // push dummy error code
	push $215 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper216
.align   4

isr_wrapper216:
	push $0 // push dummy error code
	push $216 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper217
.align   4

isr_wrapper217:
	push $0 // push dummy error code
	push $217 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper218
.align   4

isr_wrapper218:
	push $0 // push dummy error code
	push $218 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper219
.align   4

isr_wrapper219:
	push $0 // push dummy error code
	push $219 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper220
.align   4

isr_wrapper220:
	push $0 // push dummy error code
	push $220 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper221
.align   4

isr_wrapper221:
	push $0 // push dummy error code
	push $221 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper222
.align   4

isr_wrapper222:
	push $0 // push dummy error code
	push $222 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper223
.align   4

isr_wrapper223:
	push $0 // push dummy error code
	push $223 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper224
.align   4

isr_wrapper224:
	push $0 // push dummy error code
	push $224 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper225
.align   4

isr_wrapper225:
	push $0 // push dummy error code
	push $225 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper226
.align   4

isr_wrapper226:
	push $0 // push dummy error code
	push $226 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper227
.align   4

isr_wrapper227:
	push $0 // push dummy error code
	push $227 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper228
.align   4

isr_wrapper228:
	push $0 // push dummy error code
	push $228 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper229
.align   4

isr_wrapper229:
	push $0 // push dummy error code
	push $229 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper230
.align   4

isr_wrapper230:
	push $0 // push dummy error code
	push $230 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper231
.align   4

isr_wrapper231:
	push $0 // push dummy error code
	push $231 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper232
.align   4

isr_wrapper232:
	push $0 // push dummy error code
	push $232 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper233
.align   4

isr_wrapper233:
	push $0 // push dummy error code
	push $233 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper234
.align   4

isr_wrapper234:
	push $0 // push dummy error code
	push $234 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper235
.align   4

isr_wrapper235:
	push $0 // push dummy error code
	push $235 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper236
.align   4

isr_wrapper236:
	push $0 // push dummy error code
	push $236 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper237
.align   4

isr_wrapper237:
	push $0 // push dummy error code
	push $237 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper238
.align   4

isr_wrapper238:
	push $0 // push dummy error code
	push $238 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper239
.align   4

isr_wrapper239:
	push $0 // push dummy error code
	push $239 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper240
.align   4

isr_wrapper240:
	push $0 // push dummy error code
	push $240 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper241
.align   4

isr_wrapper241:
	push $0 // push dummy error code
	push $241 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper242
.align   4

isr_wrapper242:
	push $0 // push dummy error code
	push $242 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper243
.align   4

isr_wrapper243:
	push $0 // push dummy error code
	push $243 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper244
.align   4

isr_wrapper244:
	push $0 // push dummy error code
	push $244 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper245
.align   4

isr_wrapper245:
	push $0 // push dummy error code
	push $245 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper246
.align   4

isr_wrapper246:
	push $0 // push dummy error code
	push $246 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper247
.align   4

isr_wrapper247:
	push $0 // push dummy error code
	push $247 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper248
.align   4

isr_wrapper248:
	push $0 // push dummy error code
	push $248 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper249
.align   4

isr_wrapper249:
	push $0 // push dummy error code
	push $249 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper250
.align   4

isr_wrapper250:
	push $0 // push dummy error code
	push $250 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper251
.align   4

isr_wrapper251:
	push $0 // push dummy error code
	push $251 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper252
.align   4

isr_wrapper252:
	push $0 // push dummy error code
	push $252 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper253
.align   4

isr_wrapper253:
	push $0 // push dummy error code
	push $253 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper254
.align   4

isr_wrapper254:
	push $0 // push dummy error code
	push $254 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


.local   isr_wrapper255
.align   4

isr_wrapper255:
	push $0 // push dummy error code
	push $255 //
    call isr_asm //
	add $0x10, %rsp // pop the interrupt number and the error code.
    iretq //


	.data
	.global isrs_list
	isrs_list:
.quad isr_wrapper0
	.quad isr_wrapper1
	.quad isr_wrapper2
	.quad isr_wrapper3
	.quad isr_wrapper4
	.quad isr_wrapper5
	.quad isr_wrapper6
	.quad isr_wrapper7
	.quad isr_wrapper8
	.quad isr_wrapper9
	.quad isr_wrapper10
	.quad isr_wrapper11
	.quad isr_wrapper12
	.quad isr_wrapper13
	.quad isr_wrapper14
	.quad isr_wrapper15
	.quad isr_wrapper16
	.quad isr_wrapper17
	.quad isr_wrapper18
	.quad isr_wrapper19
	.quad isr_wrapper20
	.quad isr_wrapper21
	.quad isr_wrapper22
	.quad isr_wrapper23
	.quad isr_wrapper24
	.quad isr_wrapper25
	.quad isr_wrapper26
	.quad isr_wrapper27
	.quad isr_wrapper28
	.quad isr_wrapper29
	.quad isr_wrapper30
	.quad isr_wrapper31
	.quad isr_wrapper32
	.quad isr_wrapper33
	.quad isr_wrapper34
	.quad isr_wrapper35
	.quad isr_wrapper36
	.quad isr_wrapper37
	.quad isr_wrapper38
	.quad isr_wrapper39
	.quad isr_wrapper40
	.quad isr_wrapper41
	.quad isr_wrapper42
	.quad isr_wrapper43
	.quad isr_wrapper44
	.quad isr_wrapper45
	.quad isr_wrapper46
	.quad isr_wrapper47
	.quad isr_wrapper48
	.quad isr_wrapper49
	.quad isr_wrapper50
	.quad isr_wrapper51
	.quad isr_wrapper52
	.quad isr_wrapper53
	.quad isr_wrapper54
	.quad isr_wrapper55
	.quad isr_wrapper56
	.quad isr_wrapper57
	.quad isr_wrapper58
	.quad isr_wrapper59
	.quad isr_wrapper60
	.quad isr_wrapper61
	.quad isr_wrapper62
	.quad isr_wrapper63
	.quad isr_wrapper64
	.quad isr_wrapper65
	.quad isr_wrapper66
	.quad isr_wrapper67
	.quad isr_wrapper68
	.quad isr_wrapper69
	.quad isr_wrapper70
	.quad isr_wrapper71
	.quad isr_wrapper72
	.quad isr_wrapper73
	.quad isr_wrapper74
	.quad isr_wrapper75
	.quad isr_wrapper76
	.quad isr_wrapper77
	.quad isr_wrapper78
	.quad isr_wrapper79
	.quad isr_wrapper80
	.quad isr_wrapper81
	.quad isr_wrapper82
	.quad isr_wrapper83
	.quad isr_wrapper84
	.quad isr_wrapper85
	.quad isr_wrapper86
	.quad isr_wrapper87
	.quad isr_wrapper88
	.quad isr_wrapper89
	.quad isr_wrapper90
	.quad isr_wrapper91
	.quad isr_wrapper92
	.quad isr_wrapper93
	.quad isr_wrapper94
	.quad isr_wrapper95
	.quad isr_wrapper96
	.quad isr_wrapper97
	.quad isr_wrapper98
	.quad isr_wrapper99
	.quad isr_wrapper100
	.quad isr_wrapper101
	.quad isr_wrapper102
	.quad isr_wrapper103
	.quad isr_wrapper104
	.quad isr_wrapper105
	.quad isr_wrapper106
	.quad isr_wrapper107
	.quad isr_wrapper108
	.quad isr_wrapper109
	.quad isr_wrapper110
	.quad isr_wrapper111
	.quad isr_wrapper112
	.quad isr_wrapper113
	.quad isr_wrapper114
	.quad isr_wrapper115
	.quad isr_wrapper116
	.quad isr_wrapper117
	.quad isr_wrapper118
	.quad isr_wrapper119
	.quad isr_wrapper120
	.quad isr_wrapper121
	.quad isr_wrapper122
	.quad isr_wrapper123
	.quad isr_wrapper124
	.quad isr_wrapper125
	.quad isr_wrapper126
	.quad isr_wrapper127
	.quad isr_wrapper128
	.quad isr_wrapper129
	.quad isr_wrapper130
	.quad isr_wrapper131
	.quad isr_wrapper132
	.quad isr_wrapper133
	.quad isr_wrapper134
	.quad isr_wrapper135
	.quad isr_wrapper136
	.quad isr_wrapper137
	.quad isr_wrapper138
	.quad isr_wrapper139
	.quad isr_wrapper140
	.quad isr_wrapper141
	.quad isr_wrapper142
	.quad isr_wrapper143
	.quad isr_wrapper144
	.quad isr_wrapper145
	.quad isr_wrapper146
	.quad isr_wrapper147
	.quad isr_wrapper148
	.quad isr_wrapper149
	.quad isr_wrapper150
	.quad isr_wrapper151
	.quad isr_wrapper152
	.quad isr_wrapper153
	.quad isr_wrapper154
	.quad isr_wrapper155
	.quad isr_wrapper156
	.quad isr_wrapper157
	.quad isr_wrapper158
	.quad isr_wrapper159
	.quad isr_wrapper160
	.quad isr_wrapper161
	.quad isr_wrapper162
	.quad isr_wrapper163
	.quad isr_wrapper164
	.quad isr_wrapper165
	.quad isr_wrapper166
	.quad isr_wrapper167
	.quad isr_wrapper168
	.quad isr_wrapper169
	.quad isr_wrapper170
	.quad isr_wrapper171
	.quad isr_wrapper172
	.quad isr_wrapper173
	.quad isr_wrapper174
	.quad isr_wrapper175
	.quad isr_wrapper176
	.quad isr_wrapper177
	.quad isr_wrapper178
	.quad isr_wrapper179
	.quad isr_wrapper180
	.quad isr_wrapper181
	.quad isr_wrapper182
	.quad isr_wrapper183
	.quad isr_wrapper184
	.quad isr_wrapper185
	.quad isr_wrapper186
	.quad isr_wrapper187
	.quad isr_wrapper188
	.quad isr_wrapper189
	.quad isr_wrapper190
	.quad isr_wrapper191
	.quad isr_wrapper192
	.quad isr_wrapper193
	.quad isr_wrapper194
	.quad isr_wrapper195
	.quad isr_wrapper196
	.quad isr_wrapper197
	.quad isr_wrapper198
	.quad isr_wrapper199
	.quad isr_wrapper200
	.quad isr_wrapper201
	.quad isr_wrapper202
	.quad isr_wrapper203
	.quad isr_wrapper204
	.quad isr_wrapper205
	.quad isr_wrapper206
	.quad isr_wrapper207
	.quad isr_wrapper208
	.quad isr_wrapper209
	.quad isr_wrapper210
	.quad isr_wrapper211
	.quad isr_wrapper212
	.quad isr_wrapper213
	.quad isr_wrapper214
	.quad isr_wrapper215
	.quad isr_wrapper216
	.quad isr_wrapper217
	.quad isr_wrapper218
	.quad isr_wrapper219
	.quad isr_wrapper220
	.quad isr_wrapper221
	.quad isr_wrapper222
	.quad isr_wrapper223
	.quad isr_wrapper224
	.quad isr_wrapper225
	.quad isr_wrapper226
	.quad isr_wrapper227
	.quad isr_wrapper228
	.quad isr_wrapper229
	.quad isr_wrapper230
	.quad isr_wrapper231
	.quad isr_wrapper232
	.quad isr_wrapper233
	.quad isr_wrapper234
	.quad isr_wrapper235
	.quad isr_wrapper236
	.quad isr_wrapper237
	.quad isr_wrapper238
	.quad isr_wrapper239
	.quad isr_wrapper240
	.quad isr_wrapper241
	.quad isr_wrapper242
	.quad isr_wrapper243
	.quad isr_wrapper244
	.quad isr_wrapper245
	.quad isr_wrapper246
	.quad isr_wrapper247
	.quad isr_wrapper248
	.quad isr_wrapper249
	.quad isr_wrapper250
	.quad isr_wrapper251
	.quad isr_wrapper252
	.quad isr_wrapper253
	.quad isr_wrapper254
	.quad isr_wrapper255
	
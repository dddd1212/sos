.intel_syntax noprefix#
# org 0x7C00 # boot sector address
.text
.code16
.globl real_mode


; OK. We starts in real_mode. This is address 0x7c00.
; When we start, only 512 bytes are loaded from the disk.
real_mode:
; First we want to jump a little because we need space for the file system.
    jmp Boot
; This is the space (90 bytes).
    . = real_mode + 90 # space for file system
Boot:
; Now, we want to read from the HD couple of pages that contains the remainder of this bootloader code.
    mov ah,0x02    ; READ command: read sectors into memory
; CR: Gilad: Y 40?!? this is need to be variable.
    mov al,0x28    ; number of sectors to read (40)
; CR: Gilad: Y this line is comment out?	
    ;mov dl,0x80    # drive number
	mov ch,0    ; cylinder number
    mov dh,2    ; head number
; CR: Gilad: WTF?!?!? Y the hell this is writen like this?!?! change it to make it less MAVCHIL.
    mov cl,4    # starting sector number. we need sector number 2+128 (because the MBR part), so we need sector 4 in head 2.
    mov bx, offset Main    ; address to load to
    int 0x13    ; call the interrupt routine
    #
    jmp Main
    #
; Pad the rest of the 512 bytes.
PreviousLabel:

PadOutWithZeroesSectorOne:
    . = real_mode + (0x200 - 2)

BootSectorSignature:
    .word 0xAA55
; CR: Gilad - what is this \?	
	\
#===========================================

Main:
    #
    # set the display to VGA text mode now
    # because interrupts must be disabled
    #
    mov ax, 0x2401
    int 0x15 # enable A20 line
    
    mov ax,3
    int 0x10    # set VGA text mode 3
    
	#
    # setup data for entering protected mode
    #
    xor edx,edx # edx = 0
    mov dx,ds   # get the data segment
    shl edx,4   # shift it left a nibble
    add [GlobalDescriptorTable+2],edx # GDT's base addr = edx

    lgdt [GlobalDescriptorTable] # load the GDT  
    mov eax,cr0 # eax = machine status word (MSW)
    or al,1     # set the protection enable bit of the MSW to 1

    cli         # disable interrupts
    mov cr0,eax # start protected mode    
    jmp 0x8:prot_mode # this will change cs to 0x8 and actually make it works in protected mode (We cant access directly to cs).
############################         
        
; Now we in protected mode! Good. Lets move on to long-mode:
.code32
prot_mode:
        mov ebx,0x10 # the size of a GDT descriptor is 8 bytes
        mov fs,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        mov ds,bx   # ds = the 2nd GDT descriptor, a 4 GB data seg
        mov ss,bx   # ss = the 2nd GDT descriptor, a 4 GB data seg
        mov es,bx   # es = the 2nd GDT descriptor, a 4 GB data seg
        mov gs,bx   # gs = the 2nd GDT descriptor, a 4 GB data seg
        
; Initialize the PXE:

; CR: Gilad - make the number 0x100000 a parameter.
;			  0x400 too. need to be something like - PAGE_SIZE_IN_DWORDS
; CR: GIlad - Are we allow to use the address 0x100000?
    #
    # zero the root PXE page.
    #
    mov edi, 0x100000
	mov ecx, 0x400
	xor eax, eax
	cld
	rep stosd

; CR: Gilad - here too - you need to switch all of these numbers to paramters.
;			  The best is to use only one number in the definitions - the id of the PXE (range(0,256))
    mov eax, 0x100003
    mov ebx,0x100F68
    mov [ebx],eax # set the PXE point to itself
    
    mov eax, 0x100000
    mov cr3, eax # set cr3 point to the PXE root
; CR: Gilad - The address 0x200000 is ok?
; CR: Gilad - Add doc. here that says the map between the physical and the virtual (in what virtual address each of the following pages can be accees? (The same?))
    mov eax, 0x201003
    mov ebx,0x100000
    mov [ebx],eax # set the PXE entry of code
    
    mov eax, 0x202003
    mov ebx, 0x201000
    mov [ebx],eax # set the PPE entry of code
    
    mov eax, 0x203003
    mov ebx, 0x202000
    mov [ebx],eax # set the PDE entry of code
    
    mov eax, 0x7003
    mov ebx, 0x203038
    mov [ebx],eax # set the PTE entry of code: 0x7000.
    #mov eax, 0x8003
    #mov ebx, 0x103040
    #mov [ebx],eax # set the PTE entry of code: 0x8000.
    
    
    mov eax, cr4                 # Set the A-register to control register 4.
    or eax, 1 << 5               # Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 # Set control register 4 to the A-register.
    
    mov ecx, 0xC0000080          # Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        # Read from the model-specific register.
    or eax, 1 << 8               # Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        # Write to the model-specific register.
    
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    lgdt [GDT64_Pointer]
	
    jmp 8:mode64 # 8 is the code selector

; Long mode!
.code64
mode64:
; CR: Gilad - Again - get the numbers out of here :)
    # map address 0xffff800000000000 to same physical pages and continue execution.
    mov rax, 0xFFFFF6FB7DBED800 # pxe
    mov qword ptr [rax], 0x400003
    
    mov rax, 0xFFFFF6FB7DB00000 # ppe
    mov qword ptr [rax],0x401003
    
    mov rax, 0xFFFFF6FB60000000 # pde
    mov qword ptr [rax],0x402003
    
    mov rax, 0xFFFFF6C000000000 # pte
; CR: Gilad - maybe we want to do this in loop, and the number of pages should be variable?	
    mov qword ptr [rax], 0x7003     # map 4 pages. (we read 40 sectors)
    add rax, 8
    mov qword ptr [rax], 0x8003
    add rax, 8
    mov qword ptr [rax], 0x9003
    add rax, 8
    mov qword ptr [rax], 0xA003
	add rax, 8
    mov qword ptr [rax], 0xB003
    
    mov rax, 0xFFFF800000000000
	add rax, (continue_at_kernel_space - real_mode + 0xC00) # this is the address of the same code - mapped to the 0xFFFF800000000000 area. 
    jmp rax

GlobalDescriptorTable: 
NULL_DESC: # Not really NULL. no one use it so we use it.
    .word GlobalDescriptorTableEnd - GlobalDescriptorTable - 1 
    # segment address bits 0-15, 16-23
    .word GlobalDescriptorTable 
    .long 0

CODE_DESC:
    .word 0xFFFF       # limit low
    .word 0            # base low
    .byte 0            # base middle
    .byte 0b10011010   # access
    .byte 0b11001111   # granularity
    .byte 0            # base high

DATA_DESC:
    .word 0xFFFF       # data descriptor
    .word 0            # limit low
    .byte 0            # base low
    .byte 0b10010010   # access
    .byte 0b11001111   # granularity
    .byte 0            # base high

gdtr:
    .word 24          # length of GDT
    .long NULL_DESC   # base of GDT

GlobalDescriptorTableEnd:

GDT64:                           # Global Descriptor Table (64-bit).
    #.set GDT64Null, $ - GDT64         # The null descriptor.
    .word 0                         # Limit (low).
    .word 0                         # Base (low).
    .byte 0                         # Base (middle)
    .byte 0                         # Access.
    .byte 0                         # Granularity.
    .byte 0                         # Base (high).
    #.set GDT64Code, $ - GDT64         # The code descriptor.
    .word 0xFFFF                    # Limit (low).
    .word 0                         # Base (low).
    .byte 0                         # Base (middle)
    .byte 0b10011010                # Access (exec/read).
    .byte 0b00100000                # Granularity.
    .byte 0                         # Base (high).
    #.set GDT64Data, $ - GDT64         # The data descriptor.
    .word 0xFFFF                    # Limit (low).
    .word 0                         # Base (low).
    .byte 0                         # Base (middle)
    .byte 0b10010010                # Access (read/write).
    .byte 0b00000000                # Granularity.
    .byte 0                         # Base (high).
GDT64_Pointer:                    # The GDT-pointer.
    .word GDT64_Pointer - GDT64 - 1             # Limit.
    .quad GDT64                     # Base.
    GDT64_PointerAtKernelSpace:
    .word GDT64_Pointer - GDT64 - 1             # Limit.
    .long GDT64 - real_mode + 0xC00 # Base.
; CR: Gilad - what this comment means?
	.long 0xFFFF8000				# 0xFFFF800000000000 + GDT64 - real_mode + 0xC00 
    
#===========================================

; CR: Gilad - change the numbers into variables..
continue_at_kernel_space:
	mov rax, 0xFFFF800000000000
    add rax,GDT64_PointerAtKernelSpace - real_mode + 0xC00
    lgdt [rax]
    # clean the mapping at address 0x7000
    mov rax, 0xFFFFF6FB7DBED000
    mov qword ptr [rax], 0
    
; CR: Gilad - You need to make the size of the stack a variable.
    # map the stack. (maximum of 4 pages)
    mov rax, 0xFFFFF6C000000028 # start after the 5 pages of the boot code
    mov qword ptr [rax], 0x403003
    add rax, 8
    mov qword ptr [rax], 0x404003
    add rax, 8
    mov qword ptr [rax], 0x405003
    add rax, 8
    mov qword ptr [rax], 0x406003
    
    mov rsp, 0xffff800000008000
	jmp _start

    # now, in the physical space: non-volatile area is the pages up to 0x400000 and we use only 0x100000.
; CR: GIlad - Where you use this fact that the next free page is 0x407000? If we will want to add one more page here? So how this place will know it?
    #                             volatile area is the pages from 0x400000. the next free page is 0x407000.
    # and in the virtual space: non-volatile area is only the page of the root PXEs. (0xFFFFF6FB7DBED000)
    #                           volatile area is anything under the PXE at 0xFFFFF6FB7DBED800
                                
PadOutWithZeroesSectorsAll:
    . = real_mode + 0x2000

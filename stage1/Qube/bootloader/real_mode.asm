.intel_syntax noprefix#
# org 0x7C00 # boot sector address
.text
.code16
.globl real_mode
real_mode:
    jmp Boot
    . = real_mode + 90 # space for file system
Boot:
    mov ah,0x02    # read sectors into memory
    mov al,0x28    # number of sectors to read (40)
    #mov dl,0x80    # drive number
	mov ch,0    # cylinder number
    mov dh,2    # head number
    mov cl,4    # starting sector number. we need sector number 2+128 (because the MBR part), so we need sector 4 in head 2.
    mov bx, offset Main    # address to load to
    int 0x13    # call the interrupt routine
    #
    jmp Main
    #

PreviousLabel:

PadOutWithZeroesSectorOne:
    . = real_mode + (0x200 - 2)

BootSectorSignature:
    .word 0xAA55

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
    # set up data for entering protected mode
    #
	mov edi, 0xF000
	mov edx, 0x534D4150
	xor ebx, ebx
	mov es, bx
	
phy_pages_loop:
	mov eax, 0xE820
	mov ecx,24
	int 0x15
	add edi, 24
	test ebx, ebx
	jnz phy_pages_loop
	mov eax, 0xFFFFFFFF
	cld
	stosd
	stosd


    xor edx,edx # edx = 0
    mov dx,ds   # get the data segment
    shl edx,4   # shift it left a nibble
    add [GlobalDescriptorTable+2],edx # GDT's base addr = edx

    lgdt [GlobalDescriptorTable] # load the GDT  
    mov eax,cr0 # eax = machine status word (MSW)
    or al,1     # set the protection enable bit of the MSW to 1

    cli         # disable interrupts
    mov cr0,eax # start protected mode    
    jmp 0x8:prot_mode # this will change cs to 0x8 and actually make it works in protected mode ( We cant access directly to cs.
############################         
        
        
.code32
prot_mode:
        mov ebx,0x10 # the size of a GDT descriptor is 8 bytes
        mov fs,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        mov ds,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        mov ss,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        mov es,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        mov gs,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        #mov cs,bx   # fs = the 2nd GDT descriptor, a 4 GB data seg
        # TODO: more segments??
        
    #
    # zero the root PXE page.
    #
    mov edi, 0x60000
	mov ecx, 0x400
	xor eax, eax
	cld
	rep stosd

    mov eax, 0x60003
    mov ebx,0x60F68
    mov [ebx],eax # set the PXE point to itself
    
    mov eax, 0x60000
    mov cr3, eax # set cr3 point to the PXE root
    
    mov eax, 0x20003
    mov ebx, 0x60000
    mov [ebx],eax # set the PXE entry of code
    
    mov eax, 0x21003
    mov ebx, 0x20000
    mov [ebx],eax # set the PPE entry of code
    
    mov eax, 0x22003
    mov ebx, 0x21000
    mov [ebx],eax # set the PDE entry of code
    
    mov eax, 0x7003
    mov ebx, 0x22038
    mov [ebx],eax # set the PTE entry of code: 0x7000.
    #mov eax, 0x8003
    #mov ebx, 0x22040
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
.code64
mode64:
    # map address 0xffff800000000000 to same physical pages and continue execution.
    mov rax, 0xFFFFF6FB7DBED800 # pxe
    mov qword ptr [rax], 0x24003
    
    mov rax, 0xFFFFF6FB7DB00000 # ppe
    mov qword ptr [rax],0x25003
    
    mov rax, 0xFFFFF6FB60000000 # pde
    mov qword ptr [rax],0x26003
    
    mov rax, 0xFFFFF6C000000000 # pte
    mov qword ptr [rax], 0x7003     # map 6 pages. (we read 40 sectors, start from 0x7c00)
    add rax, 8
    mov qword ptr [rax], 0x8003
    add rax, 8
    mov qword ptr [rax], 0x9003
    add rax, 8
    mov qword ptr [rax], 0xA003
	add rax, 8
    mov qword ptr [rax], 0xB003
	add rax, 8
    mov qword ptr [rax], 0xC003
    
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
	.long 0xFFFF8000				# 0xFFFF800000000000 + GDT64 - real_mode + 0xC00 
    
#===========================================

continue_at_kernel_space:
	mov rax, 0xFFFF800000000000
    add rax,GDT64_PointerAtKernelSpace - real_mode + 0xC00
    lgdt [rax]
    # clean the mapping at address 0x7000
    mov rax, 0xFFFFF6FB7DBED000
    mov qword ptr [rax], 0
    
    # map the stack. (maximum of 4 pages)
    mov rax, 0xFFFFF6C000000030 # start after the 6 pages of the boot code
    mov qword ptr [rax], 0x20003
    add rax, 8
    mov qword ptr [rax], 0x21003
    add rax, 8
    mov qword ptr [rax], 0x22003
    add rax, 8
    mov qword ptr [rax], 0x23003
    
    mov rsp, 0xffff80000000A000
	jmp _start

    # now, in the physical space: non-volatile area is the pages starting at 0x60000 and we use only 0x60000.
    #                             volatile area is the pages up tp 0x60000. the next free page is 0x27000.
    # and in the virtual space: non-volatile area is only the page of the root PXEs. (0xFFFFF6FB7DBED000)
    #                           volatile area is anything under the PXE at 0xFFFFF6FB7DBED800
                                
PadOutWithZeroesSectorsAll:
    . = real_mode + 0x2000

# CR - Gilad - I Start this file in 12/7/16. Dror, I will CR the whole file altough the file may have code that i wrote.

# CR - Gilad - Write here in breif the tasks that this file fulfill

# CR - Gilad - THis is not good. you need to provide a .h file with all the defines there. minimum numbers as possible. if you name the file *.S instead of *.asm (with capital S) 
#			   you will be able to use the keyword "include" and include header files.
#			   Also, is there a way to choose this number after compiling the bootloader?
.set NUM_OF_BOOT_PAGES, 7 # not including first two sectors! 7 is the maximum value because we have the physical pages list at 0xF000 (if we want more we need to mode that list to other address).
.intel_syntax noprefix#
# org 0x7C00 # boot sector address
.text
.code16
.globl real_mode
real_mode:
    jmp Boot
    . = real_mode + 90 # space for file system
Boot:
	mov bl,dl # BL: DriveNum
	mov ah,0x08
	# CR - Gilad - add a comment before every syscall that say what the call is, what the params are and what are the returnd values.
	int 0x13 # DH: number of heads - 1.
	# CR - Gilad - dh is the num of heads or the num of heads - 1?
	mov bh,dh # BH: NumOfHeads
	and ecx,0x3f # ECX: sectors per track
	xor edx,edx
	# CR - Gilad - explain what is this magic number 28. is it confiruable? if so, put it in the h file.
	mov eax,[0x7c00+28]
	inc eax
	div ecx # EAX=LBA/(SectorsPerTrack), EDX=SectorNumber-1
	inc edx

	mov cl,bh # CL: NumOfHeads
	mov bh,dl # BL: DriveNum, BH: SectorNumber

	xor edx,edx
	div ecx # now: bh=SectorNumber, EDX=HeadNumber, EAX=CylinderNumber

	mov ch,al    # cylinder number
	shr ax,2
	and al,0xC0
    mov cl,bh    # starting sector number.

	or cl,al
    mov dh,dl    # head number
    mov dl,bl    # drive number
	# CR - Gilad - you need to EXPLAIN what you want to do in the above code.. I had enough reversereverse engineering.

    mov ah,0x02    # read sectors into memory
    mov al,2+8*NUM_OF_BOOT_PAGES    # number of sectors to read (80 = 10 pages)
    mov bx, offset Main    # address to load to
    int 0x13    # call the interrupt routine
    jmp Main

PadOutWithZeroesSectorOne:
    . = real_mode + (0x200 - 2)
BootSectorSignature:
    .word 0xAA55

#===========================================
# From here is the code that we read in the above code
Main:
	# CR - Gilad - is this comment correct? We saw that int instrcutions work if the interrupts are disabled..
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
	# CR - Gilad - What are these magic numbers?
	mov edi, 0xF000
	mov edx, 0x534D4150
	xor ebx, ebx
	mov es, bx

# CR - Gilad - you have to explain what you do here and why..	
phy_pages_loop:
# CR - Gilad - magic numbers..
	mov eax, 0xE820
	mov ecx,24
	int 0x15
	add edi, 24
	test ebx, ebx
	jnz phy_pages_loop
	mov eax, 0xFFFFFFFF
	cld
	# CR - Gilad - is this a bug (2 stosd)?
	stosd
	stosd
	# CR - Gilad - add comment that this is the end of the phy_pages_loop loop.

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
    
	# CR - Gilad - you need to explain the details of what is going on here.
	#		       Every number need to get out of here or at least be explained in details..
	#			   Also - what about ASLR?
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
	mov ecx, NUM_OF_BOOT_PAGES+1 # num of pages to map
map_boot_pages:
    mov [ebx],eax # set the PTE entry of code.
	add eax, 0x1000
	add ebx, 8
    loop map_boot_pages
    
    # CR - Gilad - chchchchchchch you copy the comments from somewhere...

	# CR - Gilad - breif what this code do
    mov eax, cr4                 # Set the A-register to control register 4.
    or eax, 1 << 5               # Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 # Set control register 4 to the A-register.
    
	# CR - Gilad - breif what this code do
    mov ecx, 0xC0000080          # Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        # Read from the model-specific register.
    or eax, 1 << 8               # Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        # Write to the model-specific register.
    
	# CR - Gilad - breif what this code do
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    lgdt [GDT64_Pointer]
	
    jmp 8:mode64 # 8 is the code selector
.code64
mode64:
	mov ebx,0x10 # the size of a GDT descriptor is 8 bytes
    mov fs,bx
    mov ds,bx
    mov ss,bx
    mov es,bx
    mov gs,bx

	# CR - Gilad - you need to explain the details of what is going on here.
	#		       Every number need to get out of here or at least be explained in details..
	#			   Also - what about ASLR?
    # map the stack. (maximum of 4 pages)
    mov rax, 0xFFFFF68000000000 # 4 pages of stack, starting at 0
    mov qword ptr [rax], 0x23003
    add rax, 8
    mov qword ptr [rax], 0x24003
    add rax, 8
    mov qword ptr [rax], 0x25003
    add rax, 8
    mov qword ptr [rax], 0x26003

    mov rsp, 0x0000000000004000
	jmp _start

    # now, in the physical space: non-volatile area is the pages starting at 0x60000 and we use only 0x60000.
    #                             volatile area is the pages up tp 0x60000. the next free page is 0x27000.
    # and in the virtual space: non-volatile area is only the page of the root PXEs. (0xFFFFF6FB7DBED000)
    #                           volatile area is anything under the PXE at 0xFFFFF6FB7DBED800

# CR - Gilad - add here a comment that states that this is the table for protected mode and that this is just temporary table beacuse we switch to long-mode
GlobalDescriptorTable: 
NULL_DESC: # Not really NULL. no one use it so we use it.
# CR - Gilad - write here that here we write the data that we need to point to get the lgdt opcode works (with size and pointer).
    .word GlobalDescriptorTableEnd - GlobalDescriptorTable - 1 
    # segment address bits 0-15, 16-23
    .word GlobalDescriptorTable 
    .long 0

CODE_DESC:
    .word 0xFFFF       # limit low
    .word 0            # base low
    .byte 0            # base middle
	# CR - Gilad - what are these accesses and the granularity means?
    .byte 0b10011010   # access
    .byte 0b11001111   # granularity
    .byte 0            # base high

DATA_DESC:
    .word 0xFFFF       # limit low
    .word 0            # base low
    .byte 0            # base middle
	# CR - Gilad - what are these accesses and the granularity means?
    .byte 0b10010010   # access
    .byte 0b11001111   # granularity
    .byte 0            # base high

GlobalDescriptorTableEnd:

GDT64:								# Global Descriptor Table (64-bit).
    #.set GDT64Null, $ - GDT64      # The null descriptor.
    .word 0                         # Limit (low).
    .word 0                         # Base (low).
    .byte 0                         # Base (middle)
    .byte 0                         # Access.
    .byte 0                         # Granularity.
    .byte 0                         # Base (high).
    #.set GDT64Code, $ - GDT64      # The code descriptor.
    .word 0						    # Limit (low).
    .word 0                         # Base (low).
    .byte 0                         # Base (middle)
    .byte 0b10011010                # Access (exec/read).
    .byte 0b00100000                # Granularity.
    .byte 0                         # Base (high).
    #.set GDT64Data, $ - GDT64      # The data descriptor.
    .word 0							# Limit (low).
    .word 0                         # Base (low).
    .byte 0                         # Base (middle)
    .byte 0b10010010                # Access (read/write).
    .byte 0b00000000                # Granularity.
    .byte 0                         # Base (high).
GDT64_Pointer:						# The GDT-pointer.
    .word GDT64_Pointer - GDT64 - 1             # Limit.
    .quad GDT64                     # Base.
    
#===========================================
# CR - Gilad - why you pad? and to 0x2000?                                
PadOutWithZeroesSectorsAll:
    . = real_mode + 0x2000

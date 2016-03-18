org 0x7C00 ; boot sector address
 bits 16
Boot:
	;
	;mov ah,0x00	; reset disk
	;mov dl,0x80	; drive number
	;int 0x13
	;
    ; dl already contains the drive number
	mov ah,0x02	; read sectors into memory
	mov al,0x10	; number of sectors to read (16)
	;mov dl,0x80	; drive number
	mov ch,0	; cylinder number
	mov dh,0	; head number
	mov cl,2	; starting sector number
	mov bx,Main	; address to load to
	int 0x13	; call the interrupt routine
	;
	jmp Main
	;

PreviousLabel:

PadOutWithZeroesSectorOne:
	times ((0x200 - 2) - ($ - $$)) db 0x00

BootSectorSignature:
	dw 0xAA55

;===========================================

Main:
	;
	; set the display to VGA text mode now
	; because interrupts must be disabled
	;
    mov ax, 0x2401
    int 0x15 ; enable A20 line
    
    mov si, here1
    call print_string
	mov ax,3
	int 0x10    ; set VGA text mode 3
	;
	; set up data for entering protected mode
	;
        xor edx,edx ; edx = 0
        mov dx,ds   ; get the data segment
        shl edx,4   ; shift it left a nibble
        add [GlobalDescriptorTable+2],edx ; GDT's base addr = edx
	;
        lgdt [GlobalDescriptorTable] ; load the GDT  
        mov eax,cr0 ; eax = machine status word (MSW)
        or al,1     ; set the protection enable bit of the MSW to 1
	;
        cli         ; disable interrupts
        mov cr0,eax ; start protected mode
	;

        
        jmp 0x8:prot_mode ; this will change cs to 0x8 and actually make it works in protected mode ( We cant access directly to cs.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; Real mode functions
 
 print_string:
   lodsb        ; grab a byte from SI
 
   or al, al  ; logical or AL by itself
   jz .done   ; if the result is zero, get out
 
   mov ah, 0x0E
   int 0x10      ; otherwise, print out the character!
 
   jmp print_string
 
 .done:
   ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;         
        
        
bits 32
prot_mode:
        mov ebx,0x10 ; the size of a GDT descriptor is 8 bytes
        mov fs,bx   ; fs = the 2nd GDT descriptor, a 4 GB data seg
        mov ds,bx   ; fs = the 2nd GDT descriptor, a 4 GB data seg
        mov ss,bx   ; fs = the 2nd GDT descriptor, a 4 GB data seg
        mov es,bx   ; fs = the 2nd GDT descriptor, a 4 GB data seg
        mov gs,bx   ; fs = the 2nd GDT descriptor, a 4 GB data seg
        ;mov cs,bx   ; fs = the 2nd GDT descriptor, a 4 GB data seg
        ; TODO: more segments??
        
	;
	; write a status message
	;
    
    mov eax, 0x100003
    mov ebx,0x100F68
    mov [ebx],eax ; set the PXE point to itself
    
    mov eax, 0x100000
    mov cr3, eax ; set cr3 point to the PXE root
    
    mov eax, 0x101003
    mov ebx,0x100000
    mov [ebx],eax ; set the PXE entry of code
    
    mov eax, 0x102003
    mov ebx, 0x101000
    mov [ebx],eax ; set the PPE entry of code
    
    mov eax, 0x103003
    mov ebx, 0x102000
    mov [ebx],eax ; set the PDE entry of code
    
    mov eax, 0x7003
    mov ebx, 0x103038
    mov [ebx],eax ; set the PDE entry of code
    
    mov eax, cr4                 ; Set the A-register to control register 4.
    or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 ; Set control register 4 to the A-register.
    
    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.
    
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    inc eax
    lgdt [GDT64.Pointer]
    jmp GDT64.Code:mode64
bits 64
mode64:
    mov rax, 0xFFFFF6FB7DBEDFF0 ; pxe
    mov qword [rax], 0x104003
    ;invlpg [0xFFFFF6FB7DBFE000] ; ppe
    
    mov rax, 0xFFFFF6FB7DBFE000 ; ppe
    mov qword [rax],0x105003
    ;invlpg [0xFFFFF6FB7FC00000] ; pde
    
    mov rax, 0xFFFFF6FB7FC00000 ; pde
    mov qword [rax],0x106003
    ;invlpg [0xFFFFF6FF80000000] ; pte
    
    mov rax, 0xFFFFF6FF80000000 ; pte
    mov qword [rax], 0xB8003 ; screen
    ;invlpg [0xffffff0000000000] ; pte
    
    
    
	mov rbx,0xffffff0000000000 ; address of first char for VGA mode 3
    mov word [rbx],0x0f44
	;
	mov rsi,TextProtectedMode ; si = message text
	;
	ForEachChar:
		;
		mov eax, 0
        lodsb		; get next char	
		cmp al,0x00	; if it's null, break       
		je EndForEachChar
        or eax, 0x0f00
		;
		mov [rbx],ax	; write char to display memory
		;
		inc rbx		; 2 bytes per char
		inc rbx		; so increment twice
		;
	jmp ForEachChar
	EndForEachChar:
	;
	LoopForever: jmp LoopForever
	;
	ret
	;
	TextProtectedMode: db 'The processor is in protected and paging 64 bit mode.',0

GlobalDescriptorTable: 
NULL_DESC: ; Not really NULL. no one use it so we use it.
	dw GlobalDescriptorTableEnd - GlobalDescriptorTable - 1 
	; segment address bits 0-15, 16-23
	dw GlobalDescriptorTable 
	dd 0

CODE_DESC:
    dw 0xFFFF       ; limit low
    dw 0            ; base low
    db 0            ; base middle
    db 10011010b    ; access
    db 11001111b    ; granularity
    db 0            ; base high

DATA_DESC:
    dw 0xFFFF       ; data descriptor
    dw 0            ; limit low
    db 0            ; base low
    db 10010010b    ; access
    db 11001111b    ; granularity
    db 0            ; base high

gdtr:
    Limit dw 24         ; length of GDT
    Base dd NULL_DESC   ; base of GDT

GlobalDescriptorTableEnd:

GDT64:                           ; Global Descriptor Table (64-bit).
    .Null: equ $ - GDT64         ; The null descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 0                         ; Granularity.
    db 0                         ; Base (high).
    .Code: equ $ - GDT64         ; The code descriptor.
    dw 0xFFFF                    ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .Data: equ $ - GDT64         ; The data descriptor.
    dw 0xFFFF                    ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.
    
;===========================================
here1 db 'here1', 0x0D, 0x0A, 0


   
PadOutWithZeroesSectorsAll:
	times (0x2000 - ($ - $$)) db 0x00
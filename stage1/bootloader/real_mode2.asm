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
	mov ebx,0xB8000 ; address of first char for VGA mode 3
	;
	mov esi,TextProtectedMode ; si = message text
	;
	ForEachChar:
		;
		mov eax, 0
        lodsb		; get next char	
		cmp al,0x00	; if it's null, break       
		je EndForEachChar
        or eax, 0x0f00
		;
		mov [ebx],ax	; write char to display memory
		;
		inc ebx		; 2 bytes per char
		inc ebx		; so increment twice
		;
	jmp ForEachChar
	EndForEachChar:
	;
	LoopForever: jmp LoopForever
	;
	ret
	;
	TextProtectedMode: db 'The processor is in protected mode. GILAD THE KING',0

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

;===========================================
here1 db 'here1', 0x0D, 0x0A, 0


   
PadOutWithZeroesSectorsAll:
	times (0x2000 - ($ - $$)) db 0x00
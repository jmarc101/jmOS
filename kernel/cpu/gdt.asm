; gdt_flush.asm
; called from C as: gdt_flush(uint32_t gdtr_address)
; separate asm file because C can't do a far jump (only way to reload CS)
; 'global' makes it visible to the linker so C can call it
[BITS 32]
global gdt_flush

gdt_flush:
    ; ESP+0 = return address (4 bytes in 32-bit)
    ; ESP+4 = our argument, skip the return address to get here
    ; EAX = 32-bit general purpose register, used to hold the gdtr address
    MOV EAX, [esp + 4]

    ; swap from GRUB's GDT to ours
    ; segment registers still point at GRUB's entries until we reload below
    LGDT [EAX]

    ; far jump: JMP selector:address
    ; selector 0x08 = byte offset 8 in the GDT array = entry[1*8] = kernel code
    ; colon makes it far, triggers GDT lookup and loads CS automatically
    ; CS is set as a side effect, you can't MOV into CS directly
    ; only works because LGDT already ran
    JMP 0x08:.reload_CS

.reload_CS:
    ; CS = 0x08 = kernel code, ring 0
    ; now reload data registers with kernel data selector
    ; selector 0x10 = byte offset 16 in the GDT array = entry[2*8] = kernel data
    ; AX = lower 16 bits of EAX, used to hold the selector value
    MOV AX, 0x10
    MOV DS, AX  ; DS = data segment, used for memory reads/writes
    MOV ES, AX  ; ES = extra segment, general purpose
    MOV FS, AX  ; FS = general purpose
    MOV GS, AX  ; GS = general purpose
    MOV SS, AX  ; SS = stack segment, used for push/pop

    RET

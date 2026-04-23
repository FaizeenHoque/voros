[BITS 16]
[ORG 0x7c00]

CODE_OFFSET equ 0x8
DATA_OFFSET equ 0x10
KERNEL_LOAD_SEG equ 0x0100

start:
  cli
  mov ax, 0x00
  mov ds, ax 
  mov es, ax
  mov ss, ax
  mov sp, 0x7c00
  sti

  mov al, 'B'
  mov dx, 0x3F8
  out dx, al

  mov ax, 0x0000
  mov ds, ax
  mov ax, KERNEL_LOAD_SEG
  mov es, ax
  mov bx, 0x0000
  mov dh, 0x00
  mov dl, 0x80
  mov cl, 0x02
  mov ch, 0x00
  mov ah, 0x02
  mov al, 8
  int 0x13

  jc disk_read_error

  mov al, 'K'
  mov dx, 0x3F8
  out dx, al

  mov al, 'G'
  mov dx, 0x3F8
  out dx, al

  cli 
  lgdt [gdt_descriptor]

  mov al, 'D'
  mov dx, 0x3F8
  out dx, al

  mov eax, cr0
  or al, 1
  mov cr0, eax

  mov al, 'M'
  mov dx, 0x3F8
  out dx, al
  
  ; Verify we're in protected mode - read CR0
  mov eax, cr0
  test eax, 1
  jz pmode_failed
  
  mov al, '+'
  mov dx, 0x3F8
  out dx, al
  
  ; Set up segment registers for protected mode
  mov ax, DATA_OFFSET    ; Load data segment selector
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  
  mov al, 'S'
  mov dx, 0x3F8
  out dx, al
  
  ; Far jump with correct 16-bit encoding
  db 0xEA
  dw 0x1000  ; offset
  dw 0x0008  ; selector

pmode_failed:
  mov al, 'F'
  mov dx, 0x3F8
  out dx, al
  hlt

disk_read_error:
  mov al, 'E'
  mov dx, 0x3F8
  out dx, al
  hlt

align 16
gdt_start:
  ; Null descriptor
  dq 0x0000000000000000
  
  ; Code segment: base=0x0000, limit=0xFFFF, present, DPL=0, code, readable
  dw 0xFFFF        ; Limit 15:0
  dw 0x0000        ; Base 15:0
  db 0x00          ; Base 23:16
  db 0x9A          ; Access byte (Present=1, DPL=0, Code=1, Exec=1, RW=1)
  db 0xCF          ; Granularity=1, D/B=1, Limit 19:16=F
  db 0x00          ; Base 31:24
  
  ; Data segment: base=0x0000, limit=0xFFFF, present, DPL=0, data, writable
  dw 0xFFFF        ; Limit 15:0
  dw 0x0000        ; Base 15:0
  db 0x00          ; Base 23:16
  db 0x92          ; Access byte (Present=1, DPL=0, Data=0, Exec=0, RW=1)
  db 0xCF          ; Granularity=1, D/B=1, Limit 19:16=F
  db 0x00          ; Base 31:24
gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start

times 510 - ($-$$) db 0
dw 0xAA55

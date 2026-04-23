[BITS 32]

global _start
extern kernel_main

_start:
  ; Immediate serial output to confirm kernel executes
  mov al, 'Y'
  mov dx, 0x3F8
  out dx, al
  
  call kernel_main
  jmp $

times 512-($-$$) db 0

#include <stdint.h>
#include "io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

void pic_remap(void) {
    // ICW1: start initialization sequence
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2: set vector offsets
    outb(PIC1_DATA, 0x20);  // master PIC starts at vector 32
    outb(PIC2_DATA, 0x28);  // slave PIC starts at vector 40

    // ICW3: tell master there's a slave at IRQ2, tell slave its cascade identity
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // ICW4: set 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // mask all IRQs for now
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}
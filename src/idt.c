#include "idt.h"

static idt_entry_t idt[256];
static idtr_t idtr;

extern void idt_flush(idtr_t *idtr);
extern void irq1_stub(void);

static void exception_handler(void) {
    for(;;) asm("hlt");
}

static void idt_set_entry(uint8_t vector, void *handler, uint8_t type_attributes) {
    uint64_t offset = (uint64_t)handler;

    idt[vector].offset_low      = (uint16_t)(offset & 0xFFFF);
    idt[vector].offset_mid      = (uint16_t)((offset >> 16) & 0xFFFF);
    idt[vector].offset_high     = (uint32_t)((offset >> 32) & 0xFFFFFFFF);
    idt[vector].selector        = 0x08;
    idt[vector].ist             = 0;
    idt[vector].type_attributes = type_attributes;
    idt[vector].zero            = 0;
}

void idt_init(void) {
    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, exception_handler, 0x8E);
    }

    idt_set_entry(33, irq1_stub, 0x8E);

    idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtr.base  = (uint64_t)&idt;

    idt_flush(&idtr);
}
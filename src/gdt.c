#include "gdt.h"

static gdt_entry_t gdt[3];
static gdtr_t gdtr;

extern void gdt_flush(gdtr_t *gdtr);

void gdt_init(void) {
    gdt[0].limit_low   = 0;
    gdt[0].base_low    = 0;
    gdt[0].base_mid    = 0;
    gdt[0].access      = 0;
    gdt[0].granularity = 0;
    gdt[0].base_high   = 0;

    gdt[1].limit_low   = 0xFFFF;
    gdt[1].base_low    = 0;
    gdt[1].base_mid    = 0;
    gdt[1].access      = 0x9A;
    gdt[1].granularity = 0xAF;
    gdt[1].base_high   = 0;

    gdt[2].limit_low   = 0xFFFF;
    gdt[2].base_low    = 0;
    gdt[2].base_mid    = 0;
    gdt[2].access      = 0x92;
    gdt[2].granularity = 0xAF;
    gdt[2].base_high   = 0;

    gdtr.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdtr.base  = (uint64_t)&gdt;

    gdt_flush(&gdtr);
}

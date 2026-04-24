#include "kernel.h"

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void print(const char* str) {
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    int i = 0;
    while (1) {
        unsigned char c = (unsigned char)str[i];
        if (c == 0) break;
        vga[i] = (0x0F << 8) | c;
        i++;
    }
}

void kernel_main() {
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;

    // Clear screen - short* so index by cell, not byte
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (0x0F << 8) | ' ';
    }

    print("Hello, World!");
}
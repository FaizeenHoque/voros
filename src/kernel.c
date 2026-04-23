#include "kernel.h"

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void kernel_main() {
    volatile unsigned char *vga = (volatile unsigned char *)VGA_BUFFER;
    int i;
    
    // Clear screen
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga[i] = ' ';
        vga[i + 1] = 0x0F;
    }
    
    vga[0] = 'H';
    vga[1] = 0x0F;
    vga[2] = 'E';
    vga[3] = 0x0F;
    vga[4] = 'L';
    vga[5] = 0x0F;
    vga[6] = 'L';
    vga[7] = 0x0F;
    vga[8] = 'O';
    vga[9] = 0x0F;
    vga[10] = ' ';
    vga[11] = 0x0F;
    vga[12] = 'W';
    vga[13] = 0x0F;
    vga[14] = 'O';
    vga[15] = 0x0F;
    vga[16] = 'R';
    vga[17] = 0x0F;
    vga[18] = 'L';
    vga[19] = 0x0F;
    vga[20] = 'D';
    vga[21] = 0x0F;
}

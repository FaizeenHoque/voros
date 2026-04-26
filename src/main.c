#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"
#include "font.h"
#include "colors.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

uint16_t cursor_x = 0, cursor_y = 0;

void printc(const char* str, uint32_t color);

static void clear_screen(void) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;
    uint64_t pixels = (framebuffer->pitch / 4) * framebuffer->height;

    for (uint64_t i = 0; i < pixels; i++) {
        fb_ptr[i] = COLOR_BLACK;
    }

    cursor_x = 0;
    cursor_y = 0;
}

static bool streq(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return false;
        }
        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

static bool starts_with(const char *s, const char *prefix) {
    size_t i = 0;
    while (prefix[i] != '\0') {
        if (s[i] != prefix[i]) {
            return false;
        }
        i++;
    }

    return true;
}

static void print_prompt(void) {
    printc("user", COLOR_TERMINAL_GREEN);
    printc("@", COLOR_GRAY_LIGHT);
    printc("voros", COLOR_TERMINAL_AMBER);
    printc(":~$ ", COLOR_WHITE);
}

static void run_command(const char *cmd) {
    if (cmd[0] == '\0') {
        return;
    }

    if (streq(cmd, "help")) {
        printc("Commands: help, clear, echo <text>, about, whoami\n", COLOR_CYAN);
        return;
    }

    if (streq(cmd, "clear")) {
        clear_screen();
        return;
    }

    if (starts_with(cmd, "echo ")) {
        printc(cmd + 5, COLOR_WHITE);
        printc("\n", COLOR_WHITE);
        return;
    }

    if (streq(cmd, "about")) {
        printc("Voros kernel terminal (framebuffer + PS/2 input).\n", COLOR_CYAN);
        return;
    }

    if (streq(cmd, "whoami")) {
        printc("root\n", COLOR_WHITE);
        return;
    }

    printc("Unknown command: ", COLOR_RED);
    printc(cmd, COLOR_WHITE);
    printc("\n", COLOR_WHITE);
}

void printc(const char* str, uint32_t color) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;
    uint64_t cols = framebuffer->width / 8;
    uint64_t rows = framebuffer->height / 16;

    if (cols == 0 || rows == 0) {
        return;
    }

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\b')
        {
            if (cursor_x >= 8) {
                cursor_x -= 8;
            } else if (cursor_y >= 16) {
                cursor_y -= 16;
                cursor_x = (uint16_t)((cols - 1) * 8);
            } else {
                continue;
            }

            for (int row = 0; row < 16; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    fb_ptr[(cursor_y + row) * (framebuffer->pitch / 4) + (cursor_x + col)] = 0x000000;
                }
            }

            continue;
        }

        if (str[i] == '\n')
        {
            cursor_x = 0;
            if ((uint64_t)cursor_y + 16 < rows * 16) {
                cursor_y += 16;
            }
            continue;
        }

        if ((uint64_t)(cursor_x + 8) > framebuffer->width) {
            cursor_x = 0;
            if ((uint64_t)cursor_y + 16 < rows * 16) {
                cursor_y += 16;
            } else {
                continue;
            }
        }

        if ((uint64_t)cursor_y + 16 > framebuffer->height) {
            continue;
        }

        const uint8_t *glyph = vga_font[(uint8_t)str[i]];
        for (int row = 0; row < 16; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                uint32_t c = (glyph[row] & (0x80 >> col)) ? color : 0x000000;
                fb_ptr[(cursor_y + row) * (framebuffer->pitch / 4) + (cursor_x + col)] = c;
            }
        }

        cursor_x += 8;

    }

}
void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    gdt_init();
    idt_init();
    pic_remap();
    keyboard_init();
    pic_clear_mask(1);

    asm volatile ("sti");

    clear_screen();
    printc("Voros Terminal\n", COLOR_TERMINAL_GREEN);
    printc("Type 'help' to list commands.\n\n", COLOR_GRAY_LIGHT);
    print_prompt();

    char line_buffer[128];
    size_t line_len = 0;

    for (;;) {
        keyboard_poll();

        char c;
        while (keyboard_read_char(&c)) {
            if (c == '\r' || c == '\n') {
                printc("\n", COLOR_WHITE);
                line_buffer[line_len] = '\0';
                run_command(line_buffer);
                line_len = 0;
                print_prompt();
                continue;
            }

            if (c == '\b') {
                if (line_len > 0) {
                    line_len--;
                    printc("\b", COLOR_WHITE);
                }
                continue;
            }

            if (c < 32 || c > 126) {
                continue;
            }

            if (line_len < sizeof(line_buffer) - 1) {
                line_buffer[line_len++] = c;
                char out[2] = {c, '\0'};
                printc(out, COLOR_WHITE);
            }
        }

        asm volatile ("pause");
    }
}
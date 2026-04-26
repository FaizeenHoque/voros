#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "io.h"
#include "pic.h"
#include "keyboard.h"

static bool shift_pressed = false;

#define KEYBOARD_QUEUE_SIZE 128

static volatile uint8_t queue_head = 0;
static volatile uint8_t queue_tail = 0;
static char key_queue[KEYBOARD_QUEUE_SIZE];

static bool keyboard_queue_push(char c) {
    uint8_t next = (uint8_t)((queue_head + 1) % KEYBOARD_QUEUE_SIZE);
    if (next == queue_tail) {
        return false;
    }

    key_queue[queue_head] = c;
    queue_head = next;
    return true;
}

static bool kbd_wait_input_empty(void) {
    for (uint32_t i = 0; i < 100000; i++) {
        if ((inb(0x64) & 0x02) == 0) {
            return true;
        }
    }

    return false;
}

static bool kbd_wait_output_full(void) {
    for (uint32_t i = 0; i < 100000; i++) {
        if ((inb(0x64) & 0x01) != 0) {
            return true;
        }
    }

    return false;
}

static void kbd_flush_output(void) {
    while ((inb(0x64) & 0x01) != 0) {
        (void)inb(0x60);
    }
}

static const char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
};

static const char scancode_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z',
    'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
};

static void keyboard_process_scancode(uint8_t scancode) {
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return;
    }

    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
        return;
    }

    if (scancode & 0x80) {
        return;
    }

    char c = shift_pressed ? scancode_map_shift[scancode] : scancode_map[scancode];
    if (c != 0) {
        (void)keyboard_queue_push(c);
    }
}

void keyboard_init(void) {
    if (kbd_wait_input_empty()) outb(0x64, 0xAD);
    if (kbd_wait_input_empty()) outb(0x64, 0xA7);

    kbd_flush_output();

    uint8_t config = 0;
    if (kbd_wait_input_empty()) outb(0x64, 0x20);
    if (kbd_wait_output_full()) config = inb(0x60);

    config |= 0x01;
    config |= 0x40;

    if (kbd_wait_input_empty()) outb(0x64, 0x60);
    if (kbd_wait_input_empty()) outb(0x60, config);

    if (kbd_wait_input_empty()) outb(0x64, 0xAE);

    if (kbd_wait_input_empty()) outb(0x60, 0xF4);
    if (kbd_wait_output_full()) {
        (void)inb(0x60);
    }
}

void keyboard_irq_handler(void) {
    uint8_t scancode = inb(0x60);

    keyboard_process_scancode(scancode);
    pic_send_eoi(1);
}

void keyboard_poll(void) {
    uint8_t status = inb(0x64);

    if ((status & 0x01) == 0) {
        return;
    }

    if ((status & 0x20) != 0) {
        (void)inb(0x60);
        return;
    }

    keyboard_process_scancode(inb(0x60));
}

bool keyboard_read_char(char *out_char) {
    if (out_char == NULL) {
        return false;
    }

    if (queue_tail == queue_head) {
        return false;
    }

    *out_char = key_queue[queue_tail];
    queue_tail = (uint8_t)((queue_tail + 1) % KEYBOARD_QUEUE_SIZE);
    return true;
}
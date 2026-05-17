# Voros

A small x86_64 hobby operating system written in C and Assembly.
Built using the Limine bootloader protocol and designed to run in QEMU.

## Features

* x86_64 kernel
* Limine bootloader support
* GDT and IDT setup
* PIC remapping
* Keyboard input handling
* Freestanding kernel environment
* ISO and HDD image generation
* QEMU support

## Project Structure

```text
voros/
├── src/               # Kernel source files
├── limine/            # Limine bootloader files
├── iso_root/          # ISO filesystem root
├── obj/               # Compiled object files
├── bin/               # Built kernel binary
├── GNUmakefile        # Build system
├── linker.lds         # Linker script
├── create_iso.sh      # ISO creation script
├── create_disk_image.sh
└── image.iso          # Generated bootable ISO
```

## Requirements

Install the following tools before building:

* gcc or clang
* ld / lld
* nasm
* make
* xorriso
* qemu-system-x86_64

Arch Linux example:

```bash
sudo pacman -S base-devel nasm qemu-full xorriso
```

## Building

Build the kernel:

```bash
make
```

Create the ISO:

```bash
./create_iso.sh
```

Or use the build script:

```bash
./build.sh
```

Because manually compiling kernels repeatedly is how people accidentally become shell script addicts.

## Running

Run the operating system in QEMU:

```bash
make run
```

Or manually:

```bash
qemu-system-x86_64 -cdrom image.iso
```

## Cleaning

Remove build files:

```bash
make clean
```

## Boot Process

1. Limine loads the kernel
2. GDT and IDT are initialized
3. PIC is remapped
4. Interrupts are enabled
5. Keyboard input handling starts
6. Kernel main loop runs

Tiny operating systems are basically controlled chaos held together by linker scripts and optimism.

## Toolchain Notes

The project supports:

* GCC
* Clang/LLVM

Example LLVM build:

```bash
make TOOLCHAIN=llvm
```

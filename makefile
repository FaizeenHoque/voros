FILES = ./build/kernel.asm.o ./build/kernel.o
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

burn:
	nasm -f bin ./src/boot.asm -o ./bin/boot.bin
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o
	i686-elf-ld -g -relocatable $(FILES) -o ./build/completeKernel.o
	i686-elf-ld -T ./src/linkerScript.ld -o ./bin/kernel.bin ./build/completeKernel.o

	rm -f ./bin/os.bin
	dd if=/dev/zero of=./bin/os.bin bs=512 count=16
	dd if=./bin/boot.bin of=./bin/os.bin conv=notrunc bs=512 count=1
	dd if=./bin/kernel.bin of=./bin/os.bin conv=notrunc bs=512 seek=1

run:
	qemu-system-x86_64 -hda ./bin/os.bin -serial stdio

clean:
	rm -f ./bin/boot.bin
	rm -f ./bin/kernel.bin
	rm -f ./bin/os.bin
	rm -f ./build/kernel.asm.o
	rm -f ./build/kernel.o
	rm -f ./build/completeKernel.o

.PHONY: makepty makepty-debug elf.o code.c

all: code.bin

elf.out: elf.o
	ld -T linker.ld $^ -o $@

elf.o: elf.S elf_codesize.s
	gcc -c $<

code.out: code.c
	gcc $^ -o $@ -m32 -static -nostdlib -Wl,-e,main -ffreestanding -fno-builtin -Oz -fno-stack-protector

elf_codesize.s: code.bin
	echo ".set codesize, `stat -c %s $<`" > $@

code.bin: makepty
	objcopy -O binary -j .text $< $@

code.elf: elf.out code.bin
	cat $^ > $@ && chmod +x $@

makepty: makepty.c
	clang $^ -o $@ -m32 -static -nostdlib -Wl,-e,main -ffreestanding -fno-stack-protector -Oz \
		-Wl,-z,noseparate-code -fno-ident -fno-builtin -Wl,--build-id=none

makepty-debug: makepty.c
	gcc $^ -o $@ -m32 -DDEBUG -g

makepty-strip: makepty
	sstrip $<
	sed -i 's/\x00*$$//' $<

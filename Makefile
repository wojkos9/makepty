all: makepty-tiny

.INTERMEDIATE: elf.out makepty-full

makepty-tiny: elf.out makepty.bin
	cat $^ > makepty && chmod +x makepty

elf.out: elf.o linker.ld
	ld -T linker.ld $< -o $@

elf.o: elf.S elf.h elf_codesize.s
	gcc -c $<

makepty.bin: makepty-full
	objcopy -O binary -j .text $< $@

elf_codesize.s: makepty.bin
	echo ".set codesize, `stat -c %s $<`" > $@

makepty-strip: makepty-full
	sstrip $<
	sed 's/\x00*$$//' $< > makepty

makepty-full: makepty.c
	clang $^ -o $@ -m32 -static -nostdlib -Wl,-e,main -ffreestanding -fno-stack-protector -Oz \
		-Wl,-z,noseparate-code -fno-ident -fno-builtin -Wl,--build-id=none


makepty-debug: makepty.c
	gcc $^ -o makepty -m32 -DDEBUG -g


clean:
	-rm elf.o makepty makepty.bin elf_codesize.s

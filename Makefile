.PHONY: makepty makepty-debug

makepty: makepty.c
	clang $^ -o $@ -m32 -static -nostdlib -Wl,-e,main -ffreestanding -fno-stack-protector -Oz \
		-Wl,-z,noseparate-code -fno-ident -fno-builtin -Wl,--build-id=none

makepty-debug: makepty.c
	gcc $^ -o $@ -m32 -DDEBUG -g

makepty-strip: makepty
	sstrip $<

makepty: makepty.c
	gcc $^ -o $@ -static -nostdlib -Wl,-e,main -fno-stack-protector -Oz -Wl,-z,noseparate-code -Wl,-z,norelro

makepty-strip: makepty
	sstrip $<

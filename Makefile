OBJS = build/start.o build/debug.o build/string.o build/syscall.o build/stdio.o 
INCLUDE = -I head/
DD = of=/home/vir-bryanq-dylan/VMs/bochs/OS/hd60M.img bs=512 conv=notrunc
CFLAGS = -c -m32 -fno-stack-protector -fno-builtin -Wmissing-prototypes -Wstrict-prototypes -Wall $(INCLUDE) 
CC = gcc
LDFLAGS = -m elf_i386

write: prog
	dd if=build/prog.bin $(DD) seek=600

prog: build/prog.bin

build/prog.bin: build/main.o build/simple_crt.a 
	ld -o $@ $^ $(LDFLAGS)  

build/simple_crt.a: $(OBJS)
	ar rcs $@ $^

build/start.o: src/start.s
	nasm -f elf -o $@ $^ 

build/%.o: src/%.c
	$(CC) -o $@ $^ $(CFLAGS)

wrfile:
	dd if=rfile $(DD) seek=600

clean:
	$(RM) build/*








CPP = gcc
CPP_OPTIONS = -m32 -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector -fleading-underscore -fno-asynchronous-unwind-tables -fno-pie

all: kernel.elf

clean:
	rm -f *.o *.elf

start.o: start.asm gdt_low.asm idt_low.asm irq_low.asm
	nasm -f elf -o start.o start.asm

utils.o: utils.C utils.H
	$(CPP) $(CPP_OPTIONS) -g -c -o utils.o utils.C

assert.o: assert.C assert.H
	$(CPP) $(CPP_OPTIONS) -g -c -o assert.o assert.C


# ==== VARIOUS LOW-LEVEL STUFF =====

gdt.o: gdt.C gdt.H
	$(CPP) $(CPP_OPTIONS) -g -c -o gdt.o gdt.C

machine.o: machine.C machine.H
	$(CPP) $(CPP_OPTIONS) -g -c -o machine.o machine.C

machine_low.o: machine_low.asm machine_low.H
	nasm -f elf -o machine_low.o machine_low.asm

# ==== EXCEPTIONS AND INTERRUPTS =====

idt.o: idt.C idt.H
	$(CPP) $(CPP_OPTIONS) -g -c -o idt.o idt.C

irq.o: irq.C irq.H
	$(CPP) $(CPP_OPTIONS) -g -c -o irq.o irq.C

exceptions.o: exceptions.C exceptions.H
	$(CPP) $(CPP_OPTIONS) -g -c -o exceptions.o exceptions.C

interrupts.o: interrupts.C interrupts.H
	$(CPP) $(CPP_OPTIONS) -g -c -o interrupts.o interrupts.C

# ==== DEVICES =====

console.o: console.C console.H
	$(CPP) $(CPP_OPTIONS) -g -c -o console.o console.C

simple_timer.o: simple_timer.C simple_timer.H
	$(CPP) $(CPP_OPTIONS) -g -c -o simple_timer.o simple_timer.C

simple_keyboard.o: simple_keyboard.C simple_keyboard.H
	$(CPP) $(CPP_OPTIONS) -g -c -o simple_keyboard.o simple_keyboard.C

# ==== MEMORY =====

frame_pool.o: frame_pool.C frame_pool.H 
	$(CPP) $(CPP_OPTIONS) -g -c -o frame_pool.o frame_pool.C

mem_pool.o: mem_pool.C mem_pool.H 
	$(CPP) $(CPP_OPTIONS) -g -c -o mem_pool.o mem_pool.C

# ==== THREADS & SCHEDULING =====

threads_low.o: threads_low.asm threads_low.H
	nasm -f elf -o threads_low.o threads_low.asm

thread.o: thread.C thread.H threads_low.H
	$(CPP) $(CPP_OPTIONS) -g -c -o thread.o thread.C

scheduler.o: scheduler.C scheduler.H thread.H
	$(CPP) $(CPP_OPTIONS) -g -c -o scheduler.o scheduler.C

queue.o: queue.C queue.H
	$(CPP) $(CPP_OPTIONS) -g -c -o queue.o queue.C

RRScheduler.o: RRScheduler.C RRScheduler.H
	$(CPP) $(CPP_OPTIONS) -g -c -o RRScheduler.o RRScheduler.C

EOQTimer.o: EOQTimer.C EOQTimer.H
	$(CPP) $(CPP_OPTIONS) -g -c -o EOQTimer.o EOQTimer.C

# ==== KERNEL MAIN FILE =====

kernel.o: kernel.C machine.H console.H gdt.H idt.H irq.H exceptions.H interrupts.H simple_timer.H frame_pool.H mem_pool.H thread.H scheduler.H
	$(CPP) $(CPP_OPTIONS) -g -c -o kernel.o kernel.C

kernel.elf: start.o utils.o kernel.o \
   assert.o console.o gdt.o idt.o irq.o exceptions.o \
   interrupts.o simple_timer.o simple_keyboard.o frame_pool.o mem_pool.o \
   thread.o threads_low.o scheduler.o machine.o machine_low.o queue.o RRScheduler.o EOQTimer.o
	ld -melf_i386 -T linker.ld -o kernel.elf start.o utils.o kernel.o \
   assert.o console.o gdt.o idt.o irq.o exceptions.o interrupts.o \
   simple_timer.o simple_keyboard.o frame_pool.o mem_pool.o \
   thread.o threads_low.o scheduler.o machine.o machine_low.o queue.o RRScheduler.o EOQTimer.o

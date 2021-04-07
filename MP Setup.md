[bochsrc.bxrc]

# Print to terminal
port_e9_hack: enabled=1

# GDB Debugging stub
gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0

[console.C]

Machine::outportb(0xe9, _s[i]);

[copykernel.sh]

make clean
make
bin to elf
...
sleep 1s
gnome-terminal -- bochs -f bochsrc.bxrc

[linker.ld]

Remove OUTPUT_FORMAT("binary")

[makefile]

CPP_OPTIONS = ... -fno-pie
-g
bin to elf
aout to elf

[Terminal]
chmod +x ./copykernel.sh
sudo mount dev_kernel_grub.img /mnt/tmp/
sudo gedit /mnt/tmp/boot/grub/menu.lst
bin to elf
sudo umount -l /mnt/tmp
make clean
make

[CLion]
GDB Remote Debug
localhost:1234

BUILD:=build
BOCHS:=bochs
BOOT:=boot
KERNEL:=kernel

BUILD_BOOT:=$(BUILD)/$(BOOT)
BUILD_KERNEL:=$(BUILD)/$(KERNEL)

dummy_build_folder:=$(shell mkdir -p $(BUILD))
dummy_boot_folder:=$(shell mkdir -p $(BUILD_BOOT))
dummy_kernel_folder:=$(shell mkdir -p $(BUILD_KERNEL))

$(BUILD_BOOT)/boot.bin: $(BOOT)/boot.asm
	nasm -f bin $< -o $@

$(BUILD_BOOT)/loader.bin: $(BOOT)/loader.asm
	nasm -f bin $< -o $@

$(BUILD_KERNEL)/header.o: $(KERNEL)/header.S
	gcc -E $< > $(BUILD_KERNEL)/header.tmp
	as --64 -o $@ $(BUILD_KERNEL)/header.tmp

$(BUILD_KERNEL)/main.o: $(KERNEL)/main.c
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(BUILD_KERNEL)/system: $(BUILD_KERNEL)/header.o \
								 				$(BUILD_KERNEL)/main.o
	ld -b elf64-x86-64 -o $@ $^ -T $(KERNEL)/kernel.lds

$(BUILD_KERNEL)/kernel.bin: $(BUILD_KERNEL)/system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $< $@

$(BUILD)/master.img: $(BUILD_BOOT)/boot.bin \
										 $(BUILD_BOOT)/loader.bin \
										 $(BUILD_KERNEL)/kernel.bin
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$< of=$@ bs=512 count=1 conv=notrunc
	python3 build.py $@ $^

$(BUILD)/master-floppy.img: $(BUILD_BOOT)/boot.bin \
														$(BUILD_BOOT)/loader.bin \
														$(BUILD_KERNEL)/kernel.bin
	yes | bximage -q -fd=1.44M -func=create $@
	dd if=$< of=$@ bs=512 count=1 conv=notrunc
	python3 build.py $@ $^

test: $(BUILD)/master-floppy.img

clean:
	rm -rf $(BUILD)

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -f $(BOCHS)/bochsrc -unlock

.PHONY: bochs
bochs-floppy: $(BUILD)/master-floppy.img
	bochs -q -f $(BOCHS)/bochsrc.floppy -unlock

BUILD:=./build
BOCHS:=./bochs
BOOT:=./boot
KERNEL:=./kernel

$(BUILD)/boot.bin: $(BOOT)/boot.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/loader.bin: $(BOOT)/loader.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/master.img: $(BUILD)/boot.bin \
										$(BUILD)/loader.bin
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc

$(BUILD)/master-floppy.img: $(BUILD)/boot.bin \
														$(BUILD)/loader.bin
	# yes | bximage -q -fd=1.44M -func=create $@
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc

all: $(BUILD)/master-floppy.img

clean:
	rm -rf $(BUILD)

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -f $(BOCHS)/bochsrc -unlock

.PHONY: bochs
bochs-floppy: $(BUILD)/master-floppy.img
	bochs -q -f $(BOCHS)/bochsrc.floppy -unlock

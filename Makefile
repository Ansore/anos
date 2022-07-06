BUILD:=./build
BOCHS:=./bochs
BOOT:=./boot
KERNEL:=./kernel

$(BUILD)/boot.bin: $(BOOT)/boot.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/master.img: $(BUILD)/boot.bin
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc

test: $(BUILD)/master.img

clean:
	rm -rf $(BUILD)

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -f $(BOCHS)/bochsrc -unlock

.PHONY: bochs
bochs-floppy: $(BUILD)/boot.bin
	bochs -q -f $(BOCHS)/bochsrc.floppy -unlock

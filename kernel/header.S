.section .text

.globl _start

_start:
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %ss
  mov $0x7e00, %esp

  // load GDTR
  lgdt GDT_POINTER(%rip)

  // load IDTR
  lidt IDT_POINTER(%rip)

  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  movq $0x7e00, %rsp

  // load cr3
  movq $0x101000, %rax
  movq %rax, %cr3
  movq switch_seg(%rip), %rax
  pushq $0x08
  pushq %rax
  lretq

switch_seg:
  .quad entry64

entry64:
  movq $0x10, %rax
  movq %rax, %ds
  movq %rax, %es
  movq %rax, %gs
  movq %rax, %ss
  movq $0xffff800000007e00, %rsp

  movq go_to_kernel(%rip), %rax
  pushq $0x08
  pushq %rax
  lretq

.go_to_kernel:
  .quad start_kernel

// init page
.align 8
.org 0x1000

__PML4E:
  // bit 0,1,2 is 1, read/write, not allow user-mode access
  .quad 0x102007
  .fill 255, 8, 0
  .quad 0x102007
  .fill 255, 8, 0

.org 0x2000

__PDPTE:
  // bit 0,1 is 1, read/write
  .quad 0x103003
  .fill 511, 8, 0

.org 0x3000
__PDE:
  // bit 0,1 is 1, read/write, bit 7 is 1, maps 2MB
  // 0MB
  .quad 0x000083
  .quad 0x200083
  .quad 0x400083
  .quad 0x600083
  .quad 0x800083
  // 10MB
  .quad 0xe0000083
  .quad 0xe0200083
  .quad 0xe0400083
  .quad 0xe0600083
  .quad 0xe0800083
  .quad 0xe0a00083
  .quad 0xe0c00083
  .quad 0xe0e00083
  // 16MB
  .fill 499, 8, 0

// GDT Table
.section .data

.global GDT_Table

GDT_Table:
  .quad 0x0000000000000000 // 0 null descriptor 00
  .quad 0x0020980000000000 // 1 kernel code 64bit segment 08
  .quad 0x0000920000000000 // 2 kernel data 64bit segment 10
  .quad 0x0020f80000000000 // 3 user code 64bit segment 18
  .quad 0x0000f20000000000 // 4 user data 64bit segment 20
  .quad 0x00cf9a000000ffff // 5 kernel code 32bit segment 28
  .quad 0x00cf92000000ffff // 6 kernel data 32bit segment 30
  .fill 10, 8, 0 // 8~9 TSS(jmp one segment 7) in long-mode 128bit
GDT_END:

GDT_POINTER:
GDT_LIMIT: .word GDT_END - GDT_Table - 1
GDT_BASE: .quad GDT_Table

// IDT Table
.global IDT_Table

IDT_Table:
  .fill 512, 8, 0
IDT_END:

IDT_POINTER:
IDT_LIMIT: .word IDT_END - IDT_Table - 1
IDT_BASE: .quad IDT_Table

// TSS64 Table
.global TSS64_Table

TSS64_Table:
  .fill 13, 8, 0
TSS64_END:

TSS64_POINTER:
TSS64_LIMIT: .word TSS64_END - TSS64_Table - 1
TSS64_BASE: .quad TSS64_Table
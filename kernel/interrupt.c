#include "interrupt.h"
#include "gate.h"
#include "lib.h"
#include "linkage.h"
#include "printk.h"

#define SAVE_ALL                                                               \
  "cld; \n\t"                                                                  \
  "pushq %rax; \n\t"                                                           \
  "pushq %rax; \n\t"                                                           \
  "movq %es, %rax; \n\t"                                                       \
  "pushq %rax; \n\t"                                                           \
  "movq %ds, %rax; \n\t"                                                       \
  "pushq %rax; \n\t"                                                           \
  "xorq %rax, %rax; \n\t"                                                      \
  "pushq %rbp; \n\t"                                                           \
  "pushq %rdi; \n\t"                                                           \
  "pushq %rsi; \n\t"                                                           \
  "pushq %rdx; \n\t"                                                           \
  "pushq %rcx; \n\t"                                                           \
  "pushq %rbx; \n\t"                                                           \
  "pushq %r8; \n\t"                                                            \
  "pushq %r9; \n\t"                                                            \
  "pushq %r10; \n\t"                                                           \
  "pushq %r11; \n\t"                                                           \
  "pushq %r12; \n\t"                                                           \
  "pushq %r13; \n\t"                                                           \
  "pushq %r14; \n\t"                                                           \
  "pushq %r15; \n\t"                                                           \
  "movq $0x10, %rdx; \n\t"                                                     \
  "movq %rdx, %ds; \n\t"                                                       \
  "movq %rdx, %es; \n\t"

#define IRQ_NAME2(nr) nr##_interrupt(void)
#define IRQ_NAME(nr) IRQ_NAME2(IRQ##nr)
#define BUILD_IRQ(nr)                                                          \
  void IRQ_NAME(nr);                                                           \
  __asm__(SYMBOL_NAME_STR(IRQ) #nr "_interrupt: \n\t"                          \
                                   "pushq $0x00 \n\t" SAVE_ALL                 \
                                   "movq %rsp, %rdi \n\t"                      \
                                   "leaq ret_from_intr(%rip), %rax \n\t"       \
                                   "pushq %rax \n\t"                           \
                                   "movq $" #nr ", %rsi \n\t"                  \
                                   "jmp do_irq \n\t");

BUILD_IRQ(0x20)
BUILD_IRQ(0x21)
BUILD_IRQ(0x22)
BUILD_IRQ(0x23)
BUILD_IRQ(0x24)
BUILD_IRQ(0x25)
BUILD_IRQ(0x26)
BUILD_IRQ(0x27)
BUILD_IRQ(0x28)
BUILD_IRQ(0x29)
BUILD_IRQ(0x2a)
BUILD_IRQ(0x2b)
BUILD_IRQ(0x2c)
BUILD_IRQ(0x2d)
BUILD_IRQ(0x2e)
BUILD_IRQ(0x2f)
BUILD_IRQ(0x30)
BUILD_IRQ(0x31)
BUILD_IRQ(0x32)
BUILD_IRQ(0x33)
BUILD_IRQ(0x34)
BUILD_IRQ(0x35)
BUILD_IRQ(0x36)
BUILD_IRQ(0x37)

void (*interrupt[24])(void) = {
    IRQ0x20_interrupt, IRQ0x21_interrupt, IRQ0x22_interrupt, IRQ0x23_interrupt,
    IRQ0x24_interrupt, IRQ0x25_interrupt, IRQ0x26_interrupt, IRQ0x27_interrupt,
    IRQ0x28_interrupt, IRQ0x29_interrupt, IRQ0x2a_interrupt, IRQ0x2b_interrupt,
    IRQ0x2c_interrupt, IRQ0x2e_interrupt, IRQ0x2f_interrupt, IRQ0x30_interrupt,
    IRQ0x31_interrupt, IRQ0x32_interrupt, IRQ0x33_interrupt, IRQ0x34_interrupt,
    IRQ0x35_interrupt, IRQ0x36_interrupt, IRQ0x37_interrupt};

void interrupt_init() {
  int i;
  for (i = 32; i < 56; i++) {
    set_intr_gate(i, 2, interrupt[i - 32]);
  }
  color_printk(RED, BLACK, "8259A init \n");

  // 8259A master ICW1-4
  io_out8(0x20, 0x11);
  io_out8(0x21, 0x20);
  io_out8(0x21, 0x04);
  io_out8(0x21, 0x01);

  // 8259A slave ICW1-4
  io_out8(0xa0, 0x11);
  io_out8(0xa1, 0x28);
  io_out8(0xa1, 0x02);
  io_out8(0xa1, 0x01);

  // 8259A-M/S OCW1
  io_out8(0x21, 0xfd);
  io_out8(0xa1, 0xff);

  sti();
}

void do_irq(unsigned long regs, unsigned long nr) { // regs: rep, nr
  unsigned char x;
  color_printk(RED, BLACK, "do_irq:%#08x\t", nr);
  x = io_in8(0x60);
  color_printk(RED, BLACK, "key code: %#08x\n", x);
  io_out8(0x20, 0x20);
}

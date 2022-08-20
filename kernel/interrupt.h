#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

void interrupt_init();

void do_irq(unsigned long regs, unsigned long nr);

#endif // !__INTERRUPT_H__

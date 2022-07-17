 GDT、LDT和IDT表都是描述符表。描述符表是由若干个描述符组成，每个描述符占用8个字节的内存空间，每个描述符表内最多可以有（8K）8129个描述符。描述符是描述一个段的大小,地址及各种状态的。

# GDT

全局描述表（GDT Global Descriptor Table）。在整个系统中，全局描述符表GDT只有一张(一个处理器对应一个GDT)，GDT可以被放在内存的任何位置。系统用GDTR寄存器存放当前GDT表的基地址。在保护模式下，对一个段的描述包括3个方面的因素：Base Address、Limit、Access，它们加在一起被放在一个64-bit长的数据结构中，被称为段描述符。段描述符结构如下：

![GDT](https://img.ansore.top/2022/07/16/905187cc02cb7aed4156b74d2d9b4e5b.png)

段描述符使用数组存储，使用LGDT指令将GDT的入口地址装入GDTR寄存器。

段选择子是一个16位的数据结构，其索引号作为GDT表的下标，索引号只有13位，所以GDT表最多有8192个元素，其结构如下：

![段选择子](https://img.ansore.top/2022/07/16/f2f5153bed0b2eecb342a35a3186936a.png)

# LDT

局部描述符表LDT（Local Descriptor Table）局部描述符表可以有若干张，每个任务可以有一张。在相应的段寄存器装入段选择子，安装索引号到GDT或LDT中找到对应的段描述符，得到Base Address再加上Offset，就得到了内存地址。

由于每个进程都有自己的一套程序段、数据段、堆栈段，有了局部描述符表则可以将每个进程的程序段、数据段、堆栈段封装在一起，只要改变LDTR就可以实现对不同进程的段进行访问。

# IDT

IDT，Interrupt Descriptor Table，即中断描述符表，和GDT类似，他记录了0~255的中断号和调用函数之间的关系。整个系统IDT表也只有一张，GDT表也可以被放在内存的任何位置寄存器。IDTR寄存器存放IDT表的基地址。x86CPU最大可以支持256种中断ISR(中断处理程序)，每个表项为8字节。Intel指定或保留了前32个中断号的作用，操作系统可以指定其余的中断号的作用。

![IDT](https://img.ansore.top/2022/07/16/da9f7dab19119141e7e889eafcc51adb.png)

# 分页

64位4级页表分页模式如下：

![Screenshot_20220717_123739](https://img.ansore.top/2022/07/17/f3cc66c5587172a41a044ed809c0a0ca.png)

1. PML4：这是IA-32e模式新增的页转换表，每个表4K字节，包含512个PML4E结构
2. PDPT：每个表4K字节，包含512个PDPTE结构
3. PDT：每个表4K字节，包含512个PDE结构
4. PT：每个表4K字节，包含512个PTE结构

当前的x64体系中，如果使用4级分页，处理器64位线性地址空间只实现了48位，高16位被用作Bit 47位的符号扩展位，要么全是0，要么全是1。每个页表项结构都是8个字节64位宽，而虚拟地址中的每个页表项索引值都是9位，因此每个页表都是512 * 8 = 4K字节。

## CR3

- 如果设置寄存器`CR0.PG = 1, CR4.PAE = 1, IA32_EFER.LME = 1, and CR4.LA57 = 0`，则处理器使用4级分页，4级分页将48位线性地址转换位52位物理地址。虽然52位对应4个字节，但是线性地址只有48位，最多可以访问256TB的线性地址空间。
- 如果设置寄存器`CR0.PG = 1, CR4.PAE = 1, IA32_EFER.LME = 1, and CR4.LA57 = 1`，则处理器使用5级分页，5级分页将57位线性地址空间转换为52位物理地址。5级分页的线性地址空间足够访问整个物理地址空间。

如果是4级页表，CR3的第一个寻呼地址PML4；如果是5级页表，CR3第一个寻呼地址为PML4。

CR3的使用取决于是否设置处理器上下文标识符（PCID）。

如果`CR4.PCIDE = 0`，则CR3寄存器位使用如下：

![Screenshot_20220717_132906](https://img.ansore.top/2022/07/17/097c86a46eb1dbb16af9f099b666e518.png)

![Screenshot_20220717_132929](https://img.ansore.top/2022/07/17/a0e0a956f3a8de49f9e70bface60cb78.png)

如果`CR4.PCIDE = 1`，则CR3寄存器位使用如下：

![Screenshot_20220717_133023](https://img.ansore.top/2022/07/17/14dc2c8fa5892b029299fff33ab9ee12.png)

IA-32e模式下CR3以及各页表的结构：

![Screenshot_20220717_131048](https://img.ansore.top/2022/07/17/41220ac1c2c03d6a4e87de24ec446c95.png)

## PML5E

PML5E位说明如下：

![Screenshot_20220717_133121](https://img.ansore.top/2022/07/17/c82829d9e080b4624e1abf5c5a1eea36.png)

## PML4E

PML4E位说明如下：

![Screenshot_20220717_133155](https://img.ansore.top/2022/07/17/1eb94257a6900131a9aca34d69e07e39.png)

![Screenshot_20220717_133207](https://img.ansore.top/2022/07/17/af58053ff951773aa4561ca059df7739.png)

## PDPTE

PDPTE位说明：

![Screenshot_20220717_133325](https://img.ansore.top/2022/07/17/cb95ea538c073a6642d8e3d099a8f907.png)

## PDE

PDE位说明：

![Screenshot_20220717_133401](https://img.ansore.top/2022/07/17/de63bf4142f71470533c79c2545cbe1b.png)

## PTE

PTE位说明：

![Screenshot_20220717_133427](https://img.ansore.top/2022/07/17/1b78c24ed83d3cd64df10f0c0229cf5c.png)


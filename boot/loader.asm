[org 0x10000]
jmp LabelStart

%include "boot/fat12.inc"

BaseOfKernelFile equ 0x00
OffsetOfKernelFile equ 0x100000

BaseTmpOfKernelAddr equ 0x00
OffsetTmpOfKernelFile equ 0x7e00

MemoryStructBufferAddr equ 0x7e00

[SECTION gdt]
GDT: dd 0,0
DESC_CODE32: dd 0x0000FFFF,0x00CF9A00
DESC_DATA32: dd 0x0000FFFF,0x00CF9200

GdtLen equ $ - GDT
GdtPtr dw GdtLen - 1
       dd GDT

SelectorCode32 equ DESC_CODE32 - GDT
SelectorData32 equ DESC_DATA32 - GDT

[SECTION gdt64]
GDT64: dq 0x0000000000000000
DESC_CODE64: dq 0x0020980000000000
DESC_DATA64: dq 0x0000920000000000

GdtLen64 equ $ - GDT64
GdtPtr64 dw GdtLen64 - 1
         dd GDT64

SelectorCode64 equ DESC_CODE64 - GDT64
SelectorData64 equ DESC_DATA64 - GDT64

[SECTION .s16]
[BITS 16]
LabelStart:
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ax, 0x00
  mov ss, ax
  mov sp, 0x7c00

  ; display start loader ...
  mov ax, 0x1301
  mov bx, 0xf
  mov dx, 0x200
  mov cx, 15
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, StartLoaderMessage
  int 0x10

  ; open address A20
  push ax
  in al, 0x92
  or al, 00000010b
  out 0x92, al
  pop ax
  
  cli

  db 0x66
  lgdt [GdtPtr]

  mov eax, cr0
  or eax, 1
  mov cr0, eax

  mov ax, SelectorData32
  mov fs, ax
  mov eax, cr0
  and al, 11111110b
  mov cr0, eax

  sti

  ; reset floppy
  xor ah, ah
  xor dl, dl
  int 0x13

  ; search kernel.bin
  mov word [SectorNo], SectorNumOfRootDirStart

SearchInRootDirBegin:
  cmp word [RootDirSizeForLoop], 0
  jz NoKernelBin
  dec word [RootDirSizeForLoop]
  mov ax, 0x0000
  mov es, ax
  mov bx, 0x8000
  mov ax, [SectorNo]
  mov cl, 1
  call ReadOneSector
  mov si, KernelFileName
  mov di, 0x8000
  cld
  mov dx, 0x10

SearchForKernelBin:
  cmp dx, 0
  jz GotoNextSectorInRoorDir
  dec dx
  mov cx, 11

CmpFileName:
  cmp cx, 0
  jz FileNameFound
  dec cx
  lodsb
  cmp al, byte [es:di]
  jz GoOn
  jmp Different

GoOn:
  inc di
  jmp CmpFileName

Different:
  and di, 0xffe0
  add di, 0x20
  mov si, KernelFileName
  jmp SearchForKernelBin

GotoNextSectorInRoorDir:
  add word [SectorNo], 1
  jmp SearchInRootDirBegin

; display on screen: Error: KERNEL NOT FOUNT
NoKernelBin:
  mov ax, 0x1301
  mov bx, 0x008c
  mov dx, 0x0300 ; row 3
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, NoKernelBinMessage
  int 0x10
  jmp $

; found kernel.bin name in root dir
FileNameFound:
  mov ax, RootDirSectors
  and di, 0xffe0
  add di, 0x1a
  mov cx, word [es:di]
  push cx
  add cx, ax
  add cx, SectorBalance
  mov eax, BaseTmpOfKernelAddr ; base of kernel file
  mov es, ax
  mov bx, OffsetTmpOfKernelFile ; offset of kernel file
  mov ax, cx

GoOnLoadingFile:
  push ax
  push bx
  mov ah, 0x0e
  mov al, '.'
  mov bl, 0x0f
  int 0x10
  pop bx
  pop ax

  mov cl, 1
  call ReadOneSector
  pop ax

  push cx
  push eax
  push fs
  push edi
  push ds
  push esi

  mov cx, 0x200
  mov ax, BaseOfKernelFile
  mov fs, ax
  mov edi, dword [OffsetOfKernelFileCount]

  mov ax, BaseTmpOfKernelAddr
  mov ds, ax
  mov esi, OffsetTmpOfKernelFile

MovKernel:
  mov al, byte [ds:esi]
  mov byte [fs:edi], al
  inc esi
  inc edi

  loop MovKernel

  mov eax, 0x1000
  mov ds, eax
  mov dword [OffsetOfKernelFileCount], edi

  pop esi
  pop ds
  pop edi
  pop fs
  pop eax
  pop cx

  call GetFATEntry
  cmp ax, 0xfff
  jz FileLoaded
  push ax
  mov dx, RootDirSectors
  add ax, dx
  add ax, SectorBalance
  jmp GoOnLoadingFile

FileLoaded:
  mov ax, 0xb800
  mov gs, ax
  mov ah, 0x0f ; 0000 black background, 1111 white word
  mov al, 'G'
  mov [gs:((80*0+39)*2)], ax ; 0 row, 39 colum

KillMotor:
  push dx
  mov dx, 0x03f2
  mov al, 0
  out dx, al
  pop dx

  ; get memory address size type
  mov ax, 0x1301
  mov bx, 0x000f
  mov dx, 0x0400
  mov cx, 24
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, StartGetMemStructMessage
  int 0x10

  mov ebx, 0
  mov ax, 0x00
  mov es, ax
  mov di, MemoryStructBufferAddr

GetMemStruct:
  mov eax, 0x0e820
  mov ecx, 20
  mov edx, 0x534d4150
  int 0x15
  jc GetMemFailed
  add di, 20

  cmp ebx, 0
  jne GetMemStruct
  jmp GetMemOk

GetMemFailed:
  mov ax, 0x1301
  mov bx, 0x008c
  mov dx, 0x0500
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, GetMemStructErrorMessage
  int 0x10
  jmp $

GetMemOk:
  mov ax, 0x1301
  mov bx, 0x000f
  mov dx, 0x0600
  mov cx, 29
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, GetMemStructOkMessage
  int 0x10
  
  ; get SVGA infomation
  mov ax, 0x1301
  mov bx, 0x000f
  mov dx, 0x0800
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, StartGetSVGAVBEInfoMessage
  int 0x10
  
  mov ax, 0x00
  mov es, ax
  mov di, 0x8000
  mov ax, 0x4f00
  int 0x10

  cmp ax, 0x004f
  jz .KO

  ; Failed
  mov ax, 0x1301
  mov bx, 0x008c
  mov dx, 0x0900
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, GetSVGAVBEInfoErrMessage
  int 0x10
  jmp $

.KO:
  mov ax, 0x1301
  mov bx, 0x000f
  mov dx, 0x0a00 ; row 10
  mov cx, 29
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, GetSVGAVBEInfoOKMessage
  int 0x10

; get SVGA mode info
  mov ax, 0x1310
  mov bx, 0x000f
  mov dx, 0x0c00 ; row 12
  mov cx, 24
  push ax
  mov ax, ds
  mov es, ax
  mov ax, StartGetSVGAModeInfoMessage
  int 0x10

  mov ax, 0x00
  mov es, ax
  mov si, 0x800e

  mov esi, dword [es:si]
  mov edi, 0x8200

SVGAModelInfoGet:
  mov cx, word [es:esi]

  ; display SVGA mode info
  push ax
  mov ax, 0x00
  mov al, ch
  call DisplauAL
  mov ax, 0x00
  mov al, cl
  call DisplauAL

  pop ax
  
  cmp cx, 0xffff
  jz SVGAModelInfoFinish

  mov ax, 0x4f01
  int 0x10

  cmp ax, 0x004f
  jnz SVGAModelInfoFailed

  add esi, 2
  add edi, 0x100

  jmp SVGAModelInfoGet

SVGAModelInfoFailed:
  mov ax, 0x1301
  mov bx, 0x008c
  mov dx, 0x0d00
  mov cx, 24
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, GetSVGAModeInfoErrMessage
  int 0x10

SetSVGAModeVesaVbeFailed:
  jmp $

SVGAModelInfoFinish:
  mov ax, 0x1301
  mov bx, 0x000f
  mov dx, 0x0e00
  mov cx, 30
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, GetSVGAModeInfoOKMessage
  int 0x10

  ; set the SVGA mode (VESA VBE)
  mov ax, 0x4f02
  mov bx, 0x4180 ; model: 0x180 or 0x143
  int 0x10

  cmp ax, 0x004f
  jnz SetSVGAModeVesaVbeFailed

  ; init IDT GDT goto protect mode
  cli ; close interrupt

  db 0x66
  lgdt [GdtPtr]
  
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp dword SelectorCode32:GoToTmpProtect

[SECTION .s32]
[BITS 32]
GoToTmpProtect:
  ; go to tmp long mode
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov ss, ax
  mov esp, 0x7e00
  call SupportLongMode
  test eax, eax
  jz NoSupport

  ; init temporary page table 0x90000
  mov dword [0x90000], 0x91007
  mov dword [0x90800], 0x91007
  mov dword [0x91000], 0x92007
  mov dword [0x92000], 0x000083
  mov dword [0x92008], 0x200083
  mov dword [0x92010], 0x400083
  mov dword [0x92018], 0x600083
  mov dword [0x92020], 0x800083
  mov dword [0x92028], 0xa00083

  ; load GDTR
  db 0x66
  lgdt [GdtPtr64]
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov esp, 0x7e00

  ; open PAE
  mov eax, cr4
  bts eax, 5
  mov cr4, eax

  ; load cr3
  mov eax, 0x90000
  mov cr3, eax

  ; enble long-mode
  mov ecx, 0x0c0000080 ; IA32_EFER
  rdmsr

  bts eax, 8
  wrmsr


  ; open PE and paging
  mov eax, cr0
  bts eax, 0
  bts eax, 31
  mov cr0, eax

  jmp SelectorCode64:OffsetOfKernelFile

; test support long mode or not
SupportLongMode:
  mov eax, 0x80000000
  cpuid
  cmp eax, 0x80000001
  setnb al
  jb SupportLongModeDone
  mov eax, 0x80000001
  cpuid
  bt edx, 29
  setc al

SupportLongModeDone:
  movzx eax, al
  ret

NoSupport:
  jmp $

; read one sector from flopy
[SECTION .s16lib]
[BITS 16]

ReadOneSector:
  push bp
  mov bp, sp
  sub esp, 2
  mov byte [bp-2], cl
  push bx
  mov bl, [BPB_SecPerTrk]
  div bl
  inc ah
  mov cl, ah
  mov dh, al
  shr al, 1
  mov	ch,	al
  and dh, 1
  pop bx
  mov dl, [BS_DrvNum]

GoOnReading:
  mov ah, 2
  mov al, byte [bp-2]
  int 0x13
  jc GoOnReading
  add esp, 2
  pop bp
  ret

; get FAT Entry
GetFATEntry:
  push es
  push bx
  push ax
  mov ax, 0x00
  mov es, ax
  pop ax
  mov byte [Odd], 0
  mov bx, 3
  mul bx
  mov bx, 2
  div bx
  cmp dx, 0
  jz Even
  mov byte [Odd], 1

Even:
  xor dx, dx
  mov bx, [BPB_BytesPerSec]
  div bx
  push dx
  mov bx, 0x8000
  add ax, SectorNumOfFAT1Start
  mov cl, 2
  call ReadOneSector

  pop dx
  add bx, dx
  mov ax, [es:bx]
  cmp byte [Odd], 1
  jnz Even2
  shr ax, 4

Even2:
  and ax, 0xfff
  pop bx
  pop es
  ret

; display num in al
DisplauAL:
  push ecx
  push edx
  push edi

  mov edi, [DispalyPosition]
  mov ah, 0x0f
  mov dl, al
  shr al, 4
  mov ecx, 2

.begin:
  and al, 0x0f
  cmp al, 9
  jz .1
  add al, '0'
  jmp .2

.1:
  sub al, 0x0a
  add al, 'A'

.2:
  mov [gs:edi], ax
  add edi, 2

  mov al, dl
  loop .begin
  mov [DispalyPosition], edi

  pop edi
  pop edx
  pop ecx

  ret

; tmp IDT
IDT:
  times 0x50 dq 0
IDT_END:

IDT_POINTER:
  dw IDT_END - IDT - 1
  dd IDT

; temp variable
RootDirSizeForLoop dw RootDirSectors
SectorNo dw 0
Odd db 0
OffsetOfKernelFileCount dd OffsetOfKernelFile
DispalyPosition dd 0

StartLoaderMessage: db "Start Loader..."
NoKernelBinMessage: db "ERROR: KERNEL NOT FOUND"
KernelFileName: db "KERNEL  BIN", 0
StartGetMemStructMessage: db "Start Get Memory Struct."
GetMemStructErrorMessage: db "Get Memory Struct ERROR"
GetMemStructOkMessage: db "Get Memory Struct SUCCESSFUL!"
StartGetSVGAVBEInfoMessage: db "Start Get SVGA VBE Info"
GetSVGAVBEInfoErrMessage: db "Get SVGA VBE Info ERROR"
GetSVGAVBEInfoOKMessage: db "Get SVGA VBE Info SUCCESSFUL!"
StartGetSVGAModeInfoMessage: db "Start Get SVGA Mode Info"
GetSVGAModeInfoErrMessage: db "Get SVGA Mode Info ERROR"
GetSVGAModeInfoOKMessage: db "Get SVGA Mode Info SUCCESSFUL!"

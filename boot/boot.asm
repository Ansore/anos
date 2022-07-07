[org 0x7c00]

BaseOfStack equ 0x7c00
BaseOfLoader equ 0x1000
OffsetOfLoader equ 0x00

RootDirSectors equ 14
SectorNumOfRootDirStart equ 19
SectorNumOfFAT1Start equ 1
SectorBalance equ 17

; FAT12 Head
jmp short LableStart
nop
BS_OEMName	db	'ANOSboot'
BPB_BytesPerSec	dw	512
BPB_SecPerClus	db	1
BPB_RsvdSecCnt	dw	1
BPB_NumFATs	db	2
BPB_RootEntCnt	dw	224
BPB_TotSec16	dw	2880
BPB_Media	db	0xf0
BPB_FATSz16	dw	9
BPB_SecPerTrk	dw	18
BPB_NumHeads	dw	2
BPB_HiddSec	dd	0
BPB_TotSec32	dd	0
BS_DrvNum	db	0
BS_Reserved1	db	0
BS_BootSig	db	0x29
BS_VolID	dd	0
BS_VolLab	db	'boot loader'
BS_FileSysType	db	'FAT12   '

LableStart:
; init reg
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov sp, BaseOfStack

; clear screen
mov ax, 0x600
mov bx, 0x700
mov cx, 0x0
mov dx, 0x184f
int 0x10

; set focus
mov ax, 0x200
mov bx, 0x0
mov dx, 0x0
int 0x10

; display on screen
mov ax, 0x1301
mov bx, 0xf
mov dx, 0x0
mov cx, 15
push ax
mov ax, ds
mov es, ax
pop ax
mov bp, StartBootText
int 0x10

; reset floppy
xor ah, ah
xor dl, dl
int 0x13

; search loader.bin
mov word [SectorNo], SectorNumOfRootDirStart

SearchInRootDirBegin:
  cmp word [RootDirSizeForLoop], 0
  jz NoLoaderBin
  dec word [RootDirSizeForLoop]
  mov ax, 0x0000
  mov es, ax
  mov bx, 0x8000
  mov ax, [SectorNo]
  mov cl, 1
  call ReadOneSector
  mov si, LoaderFileName
  mov di, 0x8000
  cld
  mov dx, 0x10

SearchForLoaderBin:
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
  mov si, LoaderFileName
  jmp SearchForLoaderBin

GotoNextSectorInRoorDir:
  add word [SectorNo], 1
  jmp SearchInRootDirBegin

; display on screen: Error LEADER.BIN file not found
NoLoaderBin:
  mov ax, 0x1301
  mov bx, 0x008c
  mov dx, 0x0100
  mov cx, 32
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, NoLoaderBinText
  int 0x10
  jmp $

; found loader.bin name in root dir
FileNameFound:
  mov ax, RootDirSectors
  and di, 0xffe0
  add di, 0x1a
  mov cx, word [es:di]
  push cx
  add cx, ax
  add cx, SectorBalance
  mov ax, BaseOfLoader
  mov es, ax
  mov bx, OffsetOfLoader
  mov ax, cx

GoOnLoadingFile:
  push ax
  push bx
  mov ah, 0x0e
  mov al, '.'
  mov bl, 0xf
  int 0x10
  pop bx
  pop ax

  mov cl, 1
  call ReadOneSector
  pop ax
  call GetFATEntry
  cmp ax, 0xfff
  jz FileLoaded
  push ax
  mov dx, RootDirSectors
  add ax, dx
  add ax, SectorBalance
  add bx, [BPB_BytesPerSec]
  jmp GoOnLoadingFile

FileLoaded:
  jmp BaseOfLoader:OffsetOfLoader

; read one sector from floppy
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

; temp variable
RootDirSizeForLoop dw RootDirSectors
SectorNo dw 0
Odd db 0

; display message
StartBootText: db "Start Boot Anos"
NoLoaderBinText: db "ERROR: LOADER.BIN FILE NOT FOUND"
LoaderFileName: db "LOADER  BIN", 0

times 510-($-$$) db 0

; dw 0xaa55
db 0x55, 0xaa

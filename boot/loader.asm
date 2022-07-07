[org 0x10000]

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
mov bp, StartLoaderText
int 0x10

jmp $

StartLoaderText: db "Start Loader..."

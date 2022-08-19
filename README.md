# A Operator System

## depend env

- gnu C
- python3
- bochs
- nasm

first, you need to install `pyfatfs` for package images of `fat12` filesystem

```bash
$ pip install pyfatfs
```

package floppy image:

```bash
make test
```

run with bochs:

```bash
make bochs-floppy
```

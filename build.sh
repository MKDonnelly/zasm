#!/bin/bash

gcc test.c
./a.out
cat header.elf hello.bin > myexe.elf
\rm header.elf hello.bin


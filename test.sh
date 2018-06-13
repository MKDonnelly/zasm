#!/bin/bash

executable=./a.out

test_encode(){
   if [[ "$($executable "$1")" != "$2" ]]; then
      echo "$1 test failed!"
      echo "Expected \"$2\" got \"$($executable $1)\""
      exit
   fi
}

test_encode "movrrq rax, rbx" "48 8b d8 "
test_encode "movril rdi, 100" "bf 64 00 00 00 "
test_encode "addrrl ecx, esi" "01 f1 "
test_encode "movrrl edx, eax" "8b c2 "
test_encode "movriq rsp, 12345" "49 bd 39 30 00 00 00 00 00 00 "
test_encode "nop" "90 "
test_encode "andrrq rbp, rdi" "49 21 fd "
test_encode "xorrrq rsi, rdx" "48 31 d6 "
test_encode "syscall" "0f 05 "

echo "All tests passed!"

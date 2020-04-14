  # RUN: llvm-mc -filetype=obj -triple x86_64-pc-linux-gnu --x86-align-branch-boundary=32 --x86-align-branch=fused+jcc --mc-relax-all %s | llvm-objdump -d --no-show-raw-insn - | FileCheck %s
<<<<<<< HEAD
  # RUN: llvm-mc -filetype=obj -triple x86_64-pc-linux-gnu --x86-align-branch-boundary=32 --x86-align-branch=fused+jcc --x86-pad-max-prefix-size=5 --mc-relax-all %s | llvm-objdump -d --no-show-raw-insn - | FileCheck %s ;INTEL
=======
  # RUN: llvm-mc -filetype=obj -triple x86_64-pc-linux-gnu --x86-align-branch-boundary=32 --x86-align-branch=fused+jcc --x86-pad-max-prefix-size=5 --mc-relax-all %s | llvm-objdump -d --no-show-raw-insn - | FileCheck %s
>>>>>>> 5d73f79c54788bc07dabe9ed75c36f12bafe9d28

  # Check instructions can be aligned correctly along with option --mc-relax-all

  .text
  .global foo
foo:
  .p2align  5
  .rept 25
  int3
  .endr
  # CHECK:    19:          jne
  # CHECK:    1f:          int3
  jne   foo
  int3

  .p2align  5
  .rept 27
  int3
  .endr
  # CHECK:    40:          jne
  jne   foo

  .p2align  5
  .rept 22
  int3
  .endr
  # CHECK:    76:          testb    $2, %dl
  # CHECK:    79:          jne
  # CHECK:    7f:          int3
  testb $2, %dl
  jne   foo
  int3

  .p2align  5
  .rept 27
  int3
  .endr
  # CHECK:    a0:          testb    $2, %dl
  # CHECK:    a3:          jne
  testb $2, %dl
  jne   foo

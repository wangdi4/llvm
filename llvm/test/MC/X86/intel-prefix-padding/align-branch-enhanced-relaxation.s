  # RUN: llvm-mc -mcpu=skylake -filetype=obj -triple x86_64-pc-linux-gnu %s -x86-pad-max-prefix-size=5 --x86-align-branch-boundary=32 --x86-align-branch=jmp+indirect | llvm-objdump -d - | FileCheck %s
  # RUN: llvm-mc -mcpu=skylake -filetype=obj -triple x86_64-pc-linux-gnu %s -x86-align-branch-prefix-size=5 --x86-align-branch-boundary=32 --x86-align-branch=jmp+indirect | llvm-objdump -d - | FileCheck %s

  # Exercise cases where we are allowed to increase the length of unrelaxable
  # instructions (by adding prefixes) for alignment purposes.

  # The first test is a basic test, we just check the jmp is aligned by prefix
  # padding the previous instructions.
  .text
  .globl labeled_basic_test
labeled_basic_test:
  .p2align 5
  .rept 30
  int3
  .endr
# CHECK:      1e: 2e cc                            int3
# CHECK:      20: eb 00                            jmp
  int3
  jmp foo
foo:
  ret

   # The second test check the correctness cornercase - can't add prefixes on a
   # prefix or a instruction following by a prefix.
  .globl labeled_prefix_test
labeled_prefix_test:
  .p2align 5
  .rept 28
  int3
  .endr
# CHECK:      5c: 2e cc                            int3
  int3
# CHECK:      5e: 3e cc                            int3
  DS
  int3
# CHECK:      60: eb 00                            jmp
   jmp bar
bar:
   ret

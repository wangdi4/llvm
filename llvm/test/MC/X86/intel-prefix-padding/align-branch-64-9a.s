## Check prefix won't be prepended to instruction that has variant symbol operand.
# RUN: llvm-mc -filetype=obj -triple x86_64-unknown-unknown --x86-align-branch-boundary=32 --x86-align-branch=fused+jcc  --x86-align-branch-prefix-size=5 %s | llvm-objdump -d - | FileCheck %s

# CHECK: 0000000000000000 <_start>:
# CHECK-NEXT:        0: 66 66 48 48                      rex64
# CHECK-NEXT:        4: 8d 3d 00 00 00 00                leal    (%rip), %edi
# CHECK-NEXT:        a: 66 48 8d 3d 00 00 00 00          leaq    (%rip), %rdi
# CHECK-NEXT:       12: e8 00 00 00 00                   callq   {{.*}}
# CHECK-NEXT:       17: 48 8b 98 00 00 00 00             movq    (%rax), %rbx
# CHECK-NEXT:       1e: 90                               nop
# CHECK-NEXT:       1f: 90                               nop
# CHECK-NEXT:       20: 48 85 db                         testq    %rbx, %rbx
# CHECK-NEXT:       23: 74 00                            je       {{.*}}
# CHECK-NEXT:       25: c3                               retq

  .text
  .globl _start
_start:
  data16
  data16
  rex64
  leaq  bar@tlsld(%rip), %rdi
  data16
  leaq  bar@tlsld(%rip), %rdi
  call  __tls_get_addr@PLT
  movq  bar@DTPOFF(%rax), %rbx
  testq   %rbx, %rbx
  je  .L1
.L1:
  ret
  .section ".tdata", "awT", @progbits
bar:
  .long 10

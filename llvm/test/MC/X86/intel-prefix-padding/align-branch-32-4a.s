## Check prefix of instruction is limited by option --x86-align-branch-prefix-size=NUM.
# RUN: llvm-mc -filetype=obj -triple i386-unknown-unknown --x86-align-branch-boundary=32 --x86-align-branch=fused+jcc+jmp  --x86-align-branch-prefix-size=4 %s | llvm-objdump -d  - | FileCheck %s -check-prefixes=CHECK,PREFIX4

# RUN: llvm-mc -filetype=obj -triple i386-unknown-unknown --x86-align-branch-boundary=32 --x86-align-branch=fused+jcc+jmp  --x86-align-branch-prefix-size=5 %s | llvm-objdump -d  - | FileCheck %s -check-prefixes=CHECK,PREFIX5


# CHECK: 00000000 <foo>:
# PREFIX4:           0: 66 0f 3a 60 00 03             pcmpestrm    $3, (%eax), %xmm0
# PREFIX4-NEXT:      6: c4 e3 79 60 00 03             vpcmpestrm   $3, (%eax), %xmm0
# PREFIX4-NEXT:      c: 65 a3 01 00 00 00             movl    %eax, %gs:1
# PREFIX4-COUNT-4:    : 89 75 f4                      movl    %esi, -12(%ebp)
# PREFIX4-COUNT-2:    : 90                            nop

# PREFIX5:           0: 3e 3e 66 0f 3a 60 00 03       pcmpestrm    $3, %ds:(%eax), %xmm0
# PREFIX5-NEXT:      8: c4 e3 79 60 00 03             vpcmpestrm   $3, (%eax), %xmm0
# PREFIX5-NEXT:      e: 65 a3 01 00 00 00             movl    %eax, %gs:1
# PREFIX5-COUNT-4:    : 89 75 f4                      movl    %esi, -12(%ebp)

# CHECK:            20: a8 04                         testb   $4, %al
# CHECK-NEXT:       22: 70 dc                         jo      {{.*}}

  .text
  .globl  foo
  .p2align  4
foo:
.L1:
  pcmpestrm $3, (%eax), %xmm0
  vpcmpestrm $3, (%eax), %xmm0
  movl  %eax, %gs:0x1
  .rept 4
  movl  %esi, -12(%ebp)
  .endr
  testb $0x4,%al
  jo  .L1


# Check the dump information for alignment.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000007: 00 00 00 00 00 00 00 00  # Align
# CHECK-NEXT: 00000008: 0c                       # TB_TAG_RTN64

    .section    .trace,"a",@progbits
    .quad       0                              # Align to boundary 8
    .byte      12                              # TB_TAG_RTN64

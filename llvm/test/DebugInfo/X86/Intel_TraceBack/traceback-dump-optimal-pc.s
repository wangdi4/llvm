# Check that dump tool warns for non-optimal pc.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 07                       # TB_TAG_PC1
# CHECK-NEXT: 00000001: ff                       # TB_AT_PC1
# CHECK-NEXT:           (PC: +0, delta PC: 0x100)
# CHECK-NEXT: 00000002: 08                       # TB_TAG_PC2
# CHECK-NEXT: 00000003: ff 00                    # TB_AT_PC2
# CHECK-NEXT:           (PC: +0x100, delta PC: 0x100)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT: 00000005: 09                       # TB_TAG_PC4
# CHECK-NEXT: 00000006: 00 01 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: +0x200, delta PC: 0x101)
# CHECK-NEXT:           (warning: could use TB_TAG_PC2 here)

    .section    .trace,"a",@progbits
    .byte    7                               # TB_TAG_PC1
    .byte    255                             # TB_AT_PC1
    .byte    8                               # TB_TAG_PC2
    .short   255                             # TB_AT_PC2
    .byte    9                               # TB_TAG_PC4
    .long    256                             # TB_AT_PC4

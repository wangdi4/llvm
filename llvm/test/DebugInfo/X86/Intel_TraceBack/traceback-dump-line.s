# Check the dump information for line.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000001: 42                       # TB_AT_LN1
# CHECK-NEXT:           (line: 66, delta line: 66)
# CHECK-NEXT: 00000002: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000003: 7f                       # TB_AT_LN1
# CHECK-NEXT:           (line: 193, delta line: 127)
# CHECK-NEXT: 00000004: 05                       # TB_TAG_LN2
# CHECK-NEXT: 00000005: 7f ff                    # TB_AT_LN2
# CHECK-NEXT:           (line: 64, delta line: -129)
# CHECK-NEXT: 00000007: 06                       # TB_TAG_LN4
# CHECK-NEXT: 00000008: 00 80 00 00              # TB_AT_LN4
# CHECK-NEXT:           (line: 32832, delta line: 32768)
# CHECK-NEXT: 0000000c: 04                       # TB_TAG_LN1
# CHECK-NEXT: 0000000d: ff                       # TB_AT_LN1
# CHECK-NEXT:           (line: 32831, delta line: -1)
# CHECK-NEXT: 0000000e: 06                       # TB_TAG_LN4
# CHECK-NEXT: 0000000f: ff 7f ff ff              # TB_AT_LN4
# CHECK-NEXT:           (line: 62, delta line: -32769)

    .section    .trace,"a",@progbits
    .byte    4                               # TB_TAG_LN1
    .byte    66                              # TB_AT_LN1
    .byte    4                               # TB_TAG_LN1
    .byte    127                             # TB_AT_LN1
    .byte    5                               # TB_TAG_LN2
    .short   -129                            # TB_AT_LN2
    .byte    6                               # TB_TAG_LN4
    .long    32768                           # TB_AT_LN4
    .byte    4                               # TB_TAG_LN1
    .byte    -1                              # TB_AT_LN1
    .byte    6                               # TB_TAG_LN4
    .long    -32769                          # TB_AT_LN4

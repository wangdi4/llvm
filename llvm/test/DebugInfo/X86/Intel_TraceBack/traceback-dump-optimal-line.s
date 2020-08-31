# Check that dump tool warns for non-optimal line.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000001: 7f                       # TB_AT_LN1
# CHECK-NEXT:           (line: 127, delta line: 127)
# CHECK-NEXT: 00000002: 05                       # TB_TAG_LN2
# CHECK-NEXT: 00000003: 7f 00                    # TB_AT_LN2
# CHECK-NEXT:           (line: 254, delta line: 127)
# CHECK-NEXT:           (warning: could use TB_TAG_LN1 here)
# CHECK-NEXT: 00000005: 06                       # TB_TAG_LN4
# CHECK-NEXT: 00000006: 7f 00 00 00              # TB_AT_LN4
# CHECK-NEXT:           (line: 381, delta line: 127)
# CHECK-NEXT:           (warning: could use TB_TAG_LN1 here)
# CHECK-NEXT: 0000000a: 06                       # TB_TAG_LN4
# CHECK-NEXT: 0000000b: 80 00 00 00              # TB_AT_LN4
# CHECK-NEXT:           (line: 509, delta line: 128)
# CHECK-NEXT:           (warning: could use TB_TAG_LN2 here)

    .section    .trace,"a",@progbits
    .byte    4                               # TB_TAG_LN1
    .byte    127                             # TB_AT_LN1
    .byte    5                               # TB_TAG_LN2
    .short   127                             # TB_AT_LN2
    .byte    6                               # TB_TAG_LN4
    .long    127                             # TB_AT_LN4
    .byte    6                               # TB_TAG_LN4
    .long    128                             # TB_AT_LN4


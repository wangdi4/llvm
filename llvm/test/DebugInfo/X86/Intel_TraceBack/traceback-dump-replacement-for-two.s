# Check that dump tool warns if we could use one tag to replace the previous two tags.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
    .section    .trace,"a",@progbits

# CHECK-NEXT: 00000000: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000001: 01                       # TB_AT_LN1
# CHECK-NEXT:           (line: 1, delta line: 1)
# CHECK-NEXT: 00000002: 07                       # TB_TAG_PC1
# CHECK-NEXT: 00000003: 3f                       # TB_AT_PC1
# CHECK-NEXT:           (PC: +0, delta PC: 0x40)
# CHECK-NEXT:           (warning: could use TB_TAG_CO1 to replace the preivous two tags)

    .byte    4                               # TB_TAG_LN1
    .byte    1                               # TB_AT_LN1
    .byte    7                               # TB_TAG_PC1
    .byte    63                              # TB_AT_PC1

# CHECK-NEXT: 00000004: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000005: 7f                       # TB_AT_LN1
# CHECK-NEXT:           (line: 128, delta line: 127)
# CHECK-NEXT: 00000006: 07                       # TB_TAG_PC1
# CHECK-NEXT: 00000007: 3f                       # TB_AT_PC1
# CHECK-NEXT:           (PC: +0x40, delta PC: 0x40)
# CHECK-NEXT:           (warning: could use TB_TAG_CO2 to replace the preivous two tags)

    .byte    4                               # TB_TAG_LN1
    .byte    127                             # TB_AT_LN1
    .byte    7                               # TB_TAG_PC1
    .byte    63                              # TB_AT_PC1

# CHECK-NEXT: 00000008: 05                       # TB_TAG_LN2
# CHECK-NEXT: 00000009: 7f 00                    # TB_AT_LN2
# CHECK-NEXT:           (line: 255, delta line: 127)
#      CHECK: 0000000b: 07                       # TB_TAG_PC1
# CHECK-NEXT: 0000000c: 3f                       # TB_AT_PC1
# CHECK-NEXT:           (PC: +0x80, delta PC: 0x40)
# CHECK-NEXT:           (warning: could use TB_TAG_CO2 to replace the preivous two tags)

    .byte    5                               # TB_TAG_LN2
    .short   127                             # TB_AT_LN2
    .byte    7                               # TB_TAG_PC1
    .byte    63                              # TB_AT_PC1

# CHECK-NEXT: 0000000d: 05                       # TB_TAG_LN2
# CHECK-NEXT: 0000000e: 7f 00                    # TB_AT_LN2
# CHECK-NEXT:           (line: 382, delta line: 127)
#      CHECK: 00000010: 08                       # TB_TAG_PC2
# CHECK-NEXT: 00000011: 3f 00                    # TB_AT_PC2
# CHECK-NEXT:           (PC: +0xc0, delta PC: 0x40)
#      CHECK:           (warning: could use TB_TAG_CO2 to replace the preivous two tags)

    .byte    5                               # TB_TAG_LN2
    .short   127                             # TB_AT_LN2
    .byte    8                               # TB_TAG_PC2
    .short   63                              # TB_AT_PC2

# Check the dump information for 32-bit routine.

# RUN: llvm-mc -filetype=obj -triple i386 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 02           # TB_TAG_RTN32
# CHECK-NEXT: 00000001: 00           # TB_AT_Padding
# CHECK-NEXT: 00000002: 04 00        # TB_AT_NameLength
# CHECK-NEXT: 00000004: 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT: 00000008: 6d 61 69 6e  # TB_AT_RoutineName
# CHECK-NEXT:           (name: "main")

    .section    .trace,"a",@progbits
    .byte       2                            # TB_TAG_RTN32
    .byte       0                            # TB_AT_Padding
    .short      4                            # TB_AT_NameLength
    .long       0                            # TB_AT_RoutineBegin
    .ascii      "main"                       # TB_AT_RoutineName

# Check the dump information for 64-bit routine.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 0c                       # TB_TAG_RTN64
# CHECK-NEXT: 00000001: 00                       # TB_AT_Padding
# CHECK-NEXT: 00000002: 04 00                    # TB_AT_NameLength
# CHECK-NEXT: 00000004: 00 00 00 00 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT: 0000000c: 6d 61 69 6e              # TB_AT_RoutineName
# CHECK-NEXT:           (name: "main")

    .section    .trace,"a",@progbits
    .byte      12                            # TB_TAG_RTN64
    .byte       0                            # TB_AT_Padding
    .short      4                            # TB_AT_NameLength
    .quad       0                            # TB_AT_RoutineBegin
    .ascii      "main"                       # TB_AT_RoutineName

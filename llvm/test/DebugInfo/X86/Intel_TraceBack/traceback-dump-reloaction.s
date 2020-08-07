# Check the dump information for relocation.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

    .text
main:                                   # @main
.Lfunc_begin0:
    retq
.Lfunc_begin1:
subr1:                                  # @subr1
    retq
.Lfunc_end1:

# CHECK-LABEL: .trace contents:
#      CHECK: 00000008: 00 00 00 00 00 00 00 00  # TB_AT_TextBegin
# CHECK-NEXT:           (relocation: .text)
#      CHECK: 00000026: 00 00 00 00 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT:           (relocation: .text)
# CHECK-NEXT: 0000002e: 6d 61 69 6e              # TB_AT_RoutineName
# CHECK-NEXT:           (name: "main")
#      CHECK: 00000036: 00 00 00 00 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT:           (relocation: .text+0x1)
# CHECK-NEXT: 0000003e: 73 75 62 72 31           # TB_AT_RoutineName
# CHECK-NEXT:           (name: "subr1")

    .section    .trace,"a",@progbits
    .byte    10                              # TB_TAG_Module
    .short   2                               # TB_AT_MajorV
    .byte    0                               # TB_AT_MinorV
    .long    .Lsec_end0-.trace               # TB_AT_TraceSize
    .quad    .Lfunc_begin0                   # TB_AT_TextBegin
    .long    1                               # TB_AT_NumOfFiles
    .long    .Lfunc_end1-.Lfunc_begin0       # TB_AT_TextSize
    .short    0                              # TB_AT_NameLength
    .short    6                              # TB_AT_NameLength
    .ascii    "temp.c"                       # TB_AT_FileName
    .byte     12                             # TB_TAG_RTN64
    .byte     0                              # TB_AT_Padding
    .short    4                              # TB_AT_NameLength
    .quad    .Lfunc_begin0                   # TB_AT_RoutineBegin
    .ascii    "main"                         # TB_AT_RoutineName
    .byte     12                             # TB_TAG_RTN64
    .byte     0                              # TB_AT_Padding
    .short    5                              # TB_AT_NameLength
    .quad    .Lfunc_begin1                   # TB_AT_RoutineBegin
    .ascii    "subr1"                        # TB_AT_RoutineName
.Lsec_end0:

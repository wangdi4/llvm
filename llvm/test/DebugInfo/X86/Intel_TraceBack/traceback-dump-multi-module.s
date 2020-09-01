# Check the accumulated line is reset to zero when we start a new module.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK: 00000023: 01                       # TB_AT_LN1
# CHECK:           (line: 1, delta line: 1)
# CHECK: 00000047: 02                       # TB_AT_LN1
# CHECK:           (line: 2, delta line: 2)

    .section    .trace,"a",@progbits
    .byte    10                              # TB_TAG_Module
    .short    2                              # TB_AT_MajorV
    .byte     0                              # TB_AT_MinorV
    .long     0                              # TB_AT_TraceSize
    .quad     0                              # TB_AT_TextBegin
    .long     1                              # TB_AT_NumOfFiles
    .long     0                              # TB_AT_TextSize
    .short    0                              # TB_AT_NameLength
    .short    6                              # TB_AT_NameLength
    .ascii    "file.c"                       # TB_AT_FileName
    .byte    4                               # TB_TAG_LN1
    .byte     1                              # TB_AT_LN1

    .byte    10                              # TB_TAG_Module
    .short    2                              # TB_AT_MajorV
    .byte     0                              # TB_AT_MinorV
    .long     0                              # TB_AT_TraceSize
    .quad     0                              # TB_AT_TextBegin
    .long     1                              # TB_AT_NumOfFiles
    .long     0                              # TB_AT_TextSize
    .short    0                              # TB_AT_NameLength
    .short    6                              # TB_AT_NameLength
    .ascii    "file.c"                       # TB_AT_FileName
    .byte    4                               # TB_TAG_LN1
    .byte    2                              # TB_AT_LN1

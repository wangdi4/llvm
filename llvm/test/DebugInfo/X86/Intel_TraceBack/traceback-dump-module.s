# Check the dump information for module.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 0a                       # TB_TAG_Module
# CHECK-NEXT: 00000001: 02 00                    # TB_AT_MajorV
# CHECK-NEXT: 00000003: 00                       # TB_AT_MinorV
# CHECK-NEXT: 00000004: 00 00 00 00              # TB_AT_TraceSize
# CHECK-NEXT: 00000008: 00 00 00 00 00 00 00 00  # TB_AT_TextBegin
# CHECK-NEXT: 00000010: 02 00 00 00              # TB_AT_NumOfFiles
# CHECK-NEXT: 00000014: 00 00 00 00              # TB_AT_TextSize
# CHECK-NEXT: 00000018: 00 00                    # TB_AT_NameLength
# CHECK-NEXT: 0000001a: 07 00                    # TB_AT_NameLength
# CHECK-NEXT: 0000001c: 66 69 6c 65 31 2e 63     # TB_AT_FileName
# CHECK-NEXT:           (name: "file1.c")
# CHECK-NEXT: 00000023: 09 00                    # TB_AT_NameLength
# CHECK-NEXT: 00000025: 66 69 6c 65 32 2e 63 70 70  # TB_AT_FileName
# CHECK-NEXT:           (name: "file2.cpp")

    .section    .trace,"a",@progbits
    .byte    10                              # TB_TAG_Module
    .short    2                              # TB_AT_MajorV
    .byte     0                              # TB_AT_MinorV
    .long     0                              # TB_AT_TraceSize
    .quad     0                              # TB_AT_TextBegin
    .long     2                              # TB_AT_NumOfFiles
    .long     0                              # TB_AT_TextSize
    .short    0                              # TB_AT_NameLength
    .short    7                              # TB_AT_NameLength
    .ascii    "file1.c"                      # TB_AT_FileName
    .short    9                              # TB_AT_NameLength
    .ascii    "file2.cpp"                    # TB_AT_FileName

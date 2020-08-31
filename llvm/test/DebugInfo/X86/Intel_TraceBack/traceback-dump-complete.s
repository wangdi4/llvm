# Check the full dump information of .trace section for a small but complete case.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: 0a                       # TB_TAG_Module
# CHECK-NEXT: 00000001: 02 00                    # TB_AT_MajorV
# CHECK-NEXT: 00000003: 00                       # TB_AT_MinorV
# CHECK-NEXT: 00000004: c7 00 00 00              # TB_AT_TraceSize
# CHECK-NEXT: 00000008: 00 00 00 00 00 00 00 00  # TB_AT_TextBegin
# CHECK-NEXT:           (relocation: .text)
# CHECK-NEXT: 00000010: 02 00 00 00              # TB_AT_NumOfFiles
# CHECK-NEXT: 00000014: 8e 00 00 00              # TB_AT_TextSize
# CHECK-NEXT: 00000018: 00 00                    # TB_AT_NameLength
# CHECK-NEXT: 0000001a: 16 00                    # TB_AT_NameLength
# CHECK-NEXT: 0000001c: 74 72 61 63 65 62 61 63 6b 2d 63 6f 6d 70 6c 65 74 65 5f 61 2e 63  # TB_AT_FileName
# CHECK-NEXT:           (name: "traceback-complete_a.c")
# CHECK-NEXT: 00000032: 16 00                    # TB_AT_NameLength
# CHECK-NEXT: 00000034: 74 72 61 63 65 62 61 63 6b 2d 63 6f 6d 70 6c 65 74 65 5f 62 2e 63  # TB_AT_FileName
# CHECK-NEXT:           (name: "traceback-complete_b.c")
# CHECK-NEXT: 0000004f: 00 00 00 00 00 00        # Align
# CHECK-NEXT: 00000050: 0c                       # TB_TAG_RTN64
# CHECK-NEXT: 00000051: 00                       # TB_AT_Padding
# CHECK-NEXT: 00000052: 04 00                    # TB_AT_NameLength
# CHECK-NEXT: 00000054: 00 00 00 00 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT:           (relocation: .text)
# CHECK-NEXT: 0000005c: 6d 61 69 6e              # TB_AT_RoutineName
# CHECK-NEXT:           (name: "main")
# CHECK-NEXT: 00000060: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000061: 06                       # TB_AT_LN1
# CHECK-NEXT:           (line: 6, delta line: 6)
# CHECK-NEXT: 00000062: 09                       # TB_TAG_PC4
# CHECK-NEXT: 00000063: 0e 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: main+0, delta PC: 0xf)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO2 to replace the preivous two tags)
# CHECK-NEXT: 00000067: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000068: 01                       # TB_AT_LN1
# CHECK-NEXT:           (line: 7, delta line: 1)
# CHECK-NEXT: 00000069: 09                       # TB_TAG_PC4
# CHECK-NEXT: 0000006a: 11 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: main+0xf, delta PC: 0x12)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO1 to replace the preivous two tags)
# CHECK-NEXT: 0000006f: 00 00                    # Align
# CHECK-NEXT: 00000070: 0c                       # TB_TAG_RTN64
# CHECK-NEXT: 00000071: 00                       # TB_AT_Padding
# CHECK-NEXT: 00000072: 05 00                    # TB_AT_NameLength
# CHECK-NEXT: 00000074: 00 00 00 00 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT:           (relocation: .text+0x30)
# CHECK-NEXT: 0000007c: 73 75 62 72 31           # TB_AT_RoutineName
# CHECK-NEXT:           (name: "subr1")
# CHECK-NEXT: 00000081: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000082: 03                       # TB_AT_LN1
# CHECK-NEXT:           (line: 10, delta line: 3)
# CHECK-NEXT: 00000083: 09                       # TB_TAG_PC4
# CHECK-NEXT: 00000084: 0a 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: subr1+0, delta PC: 0xb)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO2 to replace the preivous two tags)
# CHECK-NEXT: 00000088: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000089: 01                       # TB_AT_LN1
# CHECK-NEXT:           (line: 11, delta line: 1)
# CHECK-NEXT: 0000008a: 09                       # TB_TAG_PC4
# CHECK-NEXT: 0000008b: 0e 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: subr1+0xb, delta PC: 0xf)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO1 to replace the preivous two tags)
# CHECK-NEXT: 0000008f: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000090: 01                       # TB_AT_LN1
# CHECK-NEXT:           (line: 12, delta line: 1)
# CHECK-NEXT: 00000091: 09                       # TB_TAG_PC4
# CHECK-NEXT: 00000092: 0d 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: subr1+0x1a, delta PC: 0xe)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO1 to replace the preivous two tags)
# CHECK-NEXT: 00000096: 04                       # TB_TAG_LN1
# CHECK-NEXT: 00000097: 01                       # TB_AT_LN1
# CHECK-NEXT:           (line: 13, delta line: 1)
# CHECK-NEXT: 00000098: 09                       # TB_TAG_PC4
# CHECK-NEXT: 00000099: 08 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: subr1+0x28, delta PC: 0x9)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO1 to replace the preivous two tags)
# CHECK-NEXT: 0000009d: 03                       # TB_TAG_File
# CHECK-NEXT: 0000009e: 01 00 00 00              # TB_AT_FileIdx
# CHECK-NEXT: 000000a7: 00 00 00 00 00 00        # Align
# CHECK-NEXT: 000000a8: 0c                       # TB_TAG_RTN64
# CHECK-NEXT: 000000a9: 00                       # TB_AT_Padding
# CHECK-NEXT: 000000aa: 05 00                    # TB_AT_NameLength
# CHECK-NEXT: 000000ac: 00 00 00 00 00 00 00 00  # TB_AT_RoutineBegin
# CHECK-NEXT:           (relocation: .text+0x70)
# CHECK-NEXT: 000000b4: 73 75 62 72 32           # TB_AT_RoutineName
# CHECK-NEXT:           (name: "subr2")
# CHECK-NEXT: 000000b9: 04                       # TB_TAG_LN1
# CHECK-NEXT: 000000ba: 04                       # TB_AT_LN1
# CHECK-NEXT:           (line: 17, delta line: 4)
# CHECK-NEXT: 000000bb: 09                       # TB_TAG_PC4
# CHECK-NEXT: 000000bc: 0a 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: subr2+0, delta PC: 0xb)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO2 to replace the preivous two tags)
# CHECK-NEXT: 000000c0: 04                       # TB_TAG_LN1
# CHECK-NEXT: 000000c1: 01                       # TB_AT_LN1
# CHECK-NEXT:           (line: 18, delta line: 1)
# CHECK-NEXT: 000000c2: 09                       # TB_TAG_PC4
# CHECK-NEXT: 000000c3: 12 00 00 00              # TB_AT_PC4
# CHECK-NEXT:           (PC: subr2+0xb, delta PC: 0x13)
# CHECK-NEXT:           (warning: could use TB_TAG_PC1 here)
# CHECK-NEXT:           (warning: could use TB_TAG_CO1 to replace the preivous two tags)

# To regenerate the test traceback-dump-complete.s
# clang -traceback -target x86_64-linux-gnu -emit-llvm -S traceback-complete_a.c
# clang -traceback -target x86_64-linux-gnu -emit-llvm -S traceback-complete_b.c
# llvm-link traceback-complete_a.ll traceback-complete_b.ll -o traceback-complete.bc
# llc -O0 -mtriple x86_64-linux-gnu traceback-complete.bc -o traceback-dump-complete.s

    .text
    .globl    main                            # -- Begin function main
    .p2align    4, 0x90
    .type    main,@function
main:                                   # @main
.Lfunc_begin0:
    pushq    %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp
    movl    $0, -4(%rbp)
.Ltmp0:
    movl    x, %edi
    callq    subr1
    addq    $16, %rsp
    popq    %rbp
    retq
.Lfunc_end0:
    .size    main, .Lfunc_end0-main

    .globl    subr1                           # -- Begin function subr1
    .p2align    4, 0x90
    .type    subr1,@function
subr1:                                  # @subr1
.Lfunc_begin1:
    pushq    %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp
    movl    %edi, -8(%rbp)
.Ltmp2:
    cmpl    $0, -8(%rbp)
    jne    .LBB1_2
# %bb.1:                                # %if.then
    movl    $0, -4(%rbp)
    jmp    .LBB1_3
.LBB1_2:                                # %if.end
.Ltmp3:
    movl    -8(%rbp), %edi
    callq    subr2
    addl    $1, %eax
    movl    %eax, -4(%rbp)
.LBB1_3:                                # %return
.Ltmp4:
    movl    -4(%rbp), %eax
    addq    $16, %rsp
    popq    %rbp
    retq
.Ltmp5:
.Lfunc_end1:
    .size    subr1, .Lfunc_end1-subr1
                                        # -- End function
    .globl    subr2                           # -- Begin function subr2
    .p2align    4, 0x90
    .type    subr2,@function
subr2:                                  # @subr2
.Lfunc_begin2:
# %bb.0:                                # %entry
    pushq    %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp
    movl    %edi, -4(%rbp)
.Ltmp6:
    movl    -4(%rbp), %eax
    subl    $1, %eax
    movl    %eax, %edi
    callq    subr1
    addq    $16, %rsp
    popq    %rbp
    retq
.Ltmp7:
.Lfunc_end2:
    .size    subr2, .Lfunc_end2-subr2
                                        # -- End function
    .type    x,@object                       # @x
    .data
    .globl    x
    .p2align    2
x:
    .long    2                               # 0x2
    .size    x, 4

    .section    .trace,"a",@progbits
    .byte    10                              # TB_TAG_Module
    .short    2                               # TB_AT_MajorV
    .byte    0                               # TB_AT_MinorV
    .long    .Lsec_end0-.trace               # TB_AT_TraceSize
    .quad    .Lfunc_begin0                   # TB_AT_TextBegin
    .long    2                               # TB_AT_NumOfFiles
    .long    .Lfunc_end2-.Lfunc_begin0       # TB_AT_TextSize
    .short    0                               # TB_AT_NameLength
    .short    22                              # TB_AT_NameLength
    .ascii    "traceback-complete_a.c"        # TB_AT_FileName
    .short    22                              # TB_AT_NameLength
    .ascii    "traceback-complete_b.c"        # TB_AT_FileName
    .p2align    3                               # Align to boundary 8
    .byte    12                              # TB_TAG_RTN64
    .byte    0                               # TB_AT_Padding
    .short    4                               # TB_AT_NameLength
    .quad    .Lfunc_begin0                   # TB_AT_RoutineBegin
    .ascii    "main"                          # TB_AT_RoutineName
    .byte    4                               # TB_TAG_LN1
    .byte    6                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Ltmp0-.Lfunc_begin0)-1        # TB_AT_PC4
    .byte    4                               # TB_TAG_LN1
    .byte    1                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Lfunc_end0-.Ltmp0)-1          # TB_AT_PC4
    .p2align    3                               # Align to boundary 8
    .byte    12                              # TB_TAG_RTN64
    .byte    0                               # TB_AT_Padding
    .short    5                               # TB_AT_NameLength
    .quad    .Lfunc_begin1                   # TB_AT_RoutineBegin
    .ascii    "subr1"                         # TB_AT_RoutineName
    .byte    4                               # TB_TAG_LN1
    .byte    3                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Ltmp2-.Lfunc_begin1)-1        # TB_AT_PC4
    .byte    4                               # TB_TAG_LN1
    .byte    1                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Ltmp3-.Ltmp2)-1               # TB_AT_PC4
    .byte    4                               # TB_TAG_LN1
    .byte    1                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Ltmp4-.Ltmp3)-1               # TB_AT_PC4
    .byte    4                               # TB_TAG_LN1
    .byte    1                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Lfunc_end1-.Ltmp4)-1          # TB_AT_PC4
    .byte    3                               # TB_TAG_File
    .long    1                               # TB_AT_FileIdx
    .p2align    3                               # Align to boundary 8
    .byte    12                              # TB_TAG_RTN64
    .byte    0                               # TB_AT_Padding
    .short    5                               # TB_AT_NameLength
    .quad    .Lfunc_begin2                   # TB_AT_RoutineBegin
    .ascii    "subr2"                         # TB_AT_RoutineName
    .byte    4                               # TB_TAG_LN1
    .byte    4                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Ltmp6-.Lfunc_begin2)-1        # TB_AT_PC4
    .byte    4                               # TB_TAG_LN1
    .byte    1                               # TB_AT_LN1
    .byte    9                               # TB_TAG_PC4
    .long    (.Lfunc_end2-.Ltmp6)-1          # TB_AT_PC4
.Lsec_end0:

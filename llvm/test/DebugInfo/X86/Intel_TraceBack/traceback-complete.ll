; Check the full contents of .trace section for a small but complete case.

; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s

; To regenerate the test traceback-complete.ll
; clang -traceback -target x86_64-linux-gnu -emit-llvm -S traceback-complete_a.c
; clang -traceback -target x86_64-linux-gnu -emit-llvm -S traceback-complete_b.c
; llvm-link traceback-complete_a.ll traceback-complete_b.ll -o traceback-complete.bc
; llvm-dis traceback-complete.bc

; CHECK-LABEL:    .section    .trace,"a",@progbits
; CHECK:         .byte    10                              # TB_TAG_Module
; CHECK-NEXT:    .short    2                              # TB_AT_MajorV
; CHECK-NEXT:    .byte    0                               # TB_AT_MinorV
; CHECK-NEXT:    .long    .L{{.*}}-.L{{.*}}               # TB_AT_ModuleSize
; CHECK-NEXT:    .quad    main                            # TB_AT_CodeBegin
; CHECK-NEXT:    .long    2                               # TB_AT_NumOfFiles
; CHECK-NEXT:    .long    .L{{.*}}-main                   # TB_AT_CodeSize
; CHECK-NEXT:    .short    0                              # TB_AT_NameLength
; CHECK-NEXT:    .short    22                             # TB_AT_NameLength
; CHECK-NEXT:    .ascii    "traceback-complete_a.c"       # TB_AT_FileName
; CHECK-NEXT:    .short    22                             # TB_AT_NameLength
; CHECK-NEXT:    .ascii    "traceback-complete_b.c"       # TB_AT_FileName
; CHECK-NEXT:    .p2align    3, 0x0                       # Align to boundary 8
; CHECK-NEXT:    .byte    12                              # TB_TAG_RTN64
; CHECK-NEXT:    .byte    0                               # TB_AT_Padding
; CHECK-NEXT:    .short    4                              # TB_AT_NameLength
; CHECK-NEXT:    .quad    main                            # TB_AT_RoutineBegin
; CHECK-NEXT:    .ascii    "main"                         # TB_AT_RoutineName
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    6                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-main)-1               # TB_AT_PC4
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    1                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-.L{{.*}})-1           # TB_AT_PC4
; CHECK-NEXT:    .p2align    3, 0x0                       # Align to boundary 8
; CHECK-NEXT:    .byte    12                              # TB_TAG_RTN64
; CHECK-NEXT:    .byte    0                               # TB_AT_Padding
; CHECK-NEXT:    .short    5                              # TB_AT_NameLength
; CHECK-NEXT:    .quad    subr1                           # TB_AT_RoutineBegin
; CHECK-NEXT:    .ascii    "subr1"                        # TB_AT_RoutineName
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    3                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-subr1)-1              # TB_AT_PC4
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    1                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-.L{{.*}})-1           # TB_AT_PC4
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    1                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-.L{{.*}})-1           # TB_AT_PC4
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    1                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-.L{{.*}})-1           # TB_AT_PC4
; CHECK-NEXT:    .byte    3                               # TB_TAG_File
; CHECK-NEXT:    .long    1                               # TB_AT_FileIdx
; CHECK-NEXT:    .p2align    3, 0x0                       # Align to boundary 8
; CHECK-NEXT:    .byte    12                              # TB_TAG_RTN64
; CHECK-NEXT:    .byte    0                               # TB_AT_Padding
; CHECK-NEXT:    .short    5                              # TB_AT_NameLength
; CHECK-NEXT:    .quad    subr2                           # TB_AT_RoutineBegin
; CHECK-NEXT:    .ascii    "subr2"                        # TB_AT_RoutineName
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    -9                              # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-subr2)-1              # TB_AT_PC4
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    1                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-.L{{.*}})-1           # TB_AT_PC4

@x = dso_local global i32 2, align 4

define dso_local i32 @main() !dbg !9 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i32, i32* @x, align 4, !dbg !11
  %call = call i32 @subr1(i32 %0), !dbg !12
  ret i32 %call, !dbg !13
}

define dso_local i32 @subr1(i32 %x) !dbg !14 {
entry:
  %retval = alloca i32, align 4
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4, !dbg !15
  %tobool = icmp ne i32 %0, 0, !dbg !15
  br i1 %tobool, label %if.end, label %if.then, !dbg !16

if.then:                                          ; preds = %entry
  store i32 0, i32* %retval, align 4, !dbg !17
  br label %return, !dbg !17

if.end:                                           ; preds = %entry
  %1 = load i32, i32* %x.addr, align 4, !dbg !18
  %call = call i32 @subr2(i32 %1), !dbg !19
  %add = add nsw i32 1, %call, !dbg !20
  store i32 %add, i32* %retval, align 4, !dbg !21
  br label %return, !dbg !21

return:                                           ; preds = %if.end, %if.then
  %2 = load i32, i32* %retval, align 4, !dbg !22
  ret i32 %2, !dbg !22
}

define dso_local i32 @subr2(i32 %x) !dbg !23 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4, !dbg !24
  %sub = sub nsw i32 %0, 1, !dbg !25
  %call = call i32 @subr1(i32 %sub), !dbg !26
  ret i32 %call, !dbg !27
}

!llvm.dbg.cu = !{!0, !3}
!llvm.ident = !{!5, !5}
!llvm.module.flags = !{!6, !7, !8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-complete_a.c", directory: "/temp")
!2 = !{}
!3 = distinct !DICompileUnit(language: DW_LANG_C99, file: !4, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!4 = !DIFile(filename: "traceback-complete_b.c", directory: "/temp")
!5 = !{!"clang version 11.0.0"}
!6 = !{i32 2, !"TraceBack", i32 1}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !10, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(types: !2)
!11 = !DILocation(line: 7, column: 16, scope: !9)
!12 = !DILocation(line: 7, column: 10, scope: !9)
!13 = !DILocation(line: 7, column: 3, scope: !9)
!14 = distinct !DISubprogram(name: "subr1", scope: !1, file: !1, line: 10, type: !10, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!15 = !DILocation(line: 11, column: 7, scope: !14)
!16 = !DILocation(line: 11, column: 6, scope: !14)
!17 = !DILocation(line: 11, column: 10, scope: !14)
!18 = !DILocation(line: 12, column: 20, scope: !14)
!19 = !DILocation(line: 12, column: 14, scope: !14)
!20 = !DILocation(line: 12, column: 12, scope: !14)
!21 = !DILocation(line: 12, column: 3, scope: !14)
!22 = !DILocation(line: 13, column: 1, scope: !14)
!23 = distinct !DISubprogram(name: "subr2", scope: !4, file: !4, line: 4, type: !10, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !2)
!24 = !DILocation(line: 5, column: 16, scope: !23)
!25 = !DILocation(line: 5, column: 18, scope: !23)
!26 = !DILocation(line: 5, column: 10, scope: !23)
!27 = !DILocation(line: 5, column: 3, scope: !23)

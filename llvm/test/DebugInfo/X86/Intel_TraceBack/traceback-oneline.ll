; Check we emit records for each function even if they are on same line .

; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s

; CHECK:         .quad    f                               # TB_AT_RoutineBegin
; CHECK-NEXT:    .byte    102                             # TB_AT_RoutineName
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    1                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-f)-1                  # TB_AT_PC4

; CHECK:         .quad    main                            # TB_AT_RoutineBegin
; CHECK-NEXT:    .ascii    "main"                         # TB_AT_RoutineName
; CHECK-NEXT:    .byte    4                               # TB_TAG_LN1
; CHECK-NEXT:    .byte    0                               # TB_AT_LN1
; CHECK-NEXT:    .byte    9                               # TB_TAG_PC4
; CHECK-NEXT:    .long    (.L{{.*}}-main)-1               # TB_AT_PC4

; To regenerate the test file traceback-oneline.ll
; clang -traceback -S -emit-llvm traceback-oneline.c -o traceback-oneline.ll

define dso_local i32 @f(i32 %x) !dbg !7 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4, !dbg !9
  ret i32 %0, !dbg !10
}

define dso_local i32 @main() !dbg !11 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i32 @f(i32 0), !dbg !12
  ret i32 %call, !dbg !13
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-oneline.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 1, column: 19, scope: !7)
!10 = !DILocation(line: 1, column: 12, scope: !7)
!11 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!12 = !DILocation(line: 1, column: 44, scope: !11)
!13 = !DILocation(line: 1, column: 37, scope: !11)

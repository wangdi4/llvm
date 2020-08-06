; Check the condition when there are multiple comiple units in one moudle.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s --check-prefixes=MULTICU
; MULTICU: .long 2    # TB_AT_NumOfFiles
; MULTICU: .byte 3    # TB_TAG_File
; MULTICU: .long 1    # TB_AT_FileIdx


; To regenerate the test file traceback-mutiple-cus.ll
; clang -DA_C -traceback -target x86_64-linux-gnu -emit-llvm -S  traceback-mutiple-cus.c -o a.ll
; clang -DB_C -traceback -target x86_64-linux-gnu -emit-llvm -S  traceback-mutiple-cus.c -o b.ll
; llvm-link a.ll b.ll -o traceback-mutiple-cus.bc
; llvm-dis traceback-mutiple-cus.bc

define dso_local i32 @main() #0 !dbg !8 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i32 (...) bitcast (i32 ()* @func to i32 (...)*)(), !dbg !10
  ret i32 %call, !dbg !11
}

define dso_local i32 @func() #0 !dbg !12 {
entry:
  ret i32 0, !dbg !13
}

!llvm.dbg.cu = !{!0, !3}
!llvm.ident = !{!4, !4}
!llvm.module.flags = !{!5, !6, !7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-multiple-cus.c", directory: "/temp")
!2 = !{}
!3 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!4 = !{!"clang version 11.0.0"}
!5 = !{i32 2, !"TraceBack", i32 1}
!6 = !{i32 2, !"Debug Info Version", i32 3}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 4, column: 10, scope: !8)
!11 = !DILocation(line: 4, column: 3, scope: !8)
!12 = distinct !DISubprogram(name: "func", scope: !1, file: !1, line: 8, type: !9, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !2)
!13 = !DILocation(line: 9, column: 3, scope: !12)

; Check we are able to emit the DWARF section and .trace section when flags
; "TraceBack","Dwarf Version" exist at same time.

; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s --check-prefixes=DWARF
; DWARF: .section   .debug_{{.*}}

; To regenerate the test file traceback-dwarf.ll
; clang -traceback -g -S -emit-llvm traceback-basic.c -o traceback-dwarf.ll

define dso_local i32 @main() !dbg !8 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  ret i32 0, !dbg !12
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-basic.c", directory: "/temp")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"TraceBack", i32 1}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"clang version 11.0.0"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocation(line: 1, column: 14, scope: !8)

; Check a nodebug function in a module with TraceBack flag doesn't cause crash
; and .trace section is not emitted.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s
; CHECK-NOT: .section    .trace

; To regenerate the test file traceback-nodebug.ll
; clang -traceback -S -emit-llvm traceback-nodebug.c

define dso_local void @fun() {
entry:
  ret void
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-nodebug.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}

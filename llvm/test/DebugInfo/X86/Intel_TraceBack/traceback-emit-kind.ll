; Check a compile unit without emission kind doesn't cause crash
; and .trace section is not emitted.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s
; CHECK-NOT: .section    .trace

; To regenerate the test file traceback-emit-kind.ll
; clang -cc1 -gtraceback -S -emit-llvm traceback-emit-kind.c

define void @play() {
entry:
  ret void
}

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 2, !"TraceBack", i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 11.0.0"}

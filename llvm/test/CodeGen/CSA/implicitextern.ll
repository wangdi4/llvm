; RUN: llc -march=csa < %s | FileCheck %s
source_filename = "diagnosis.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

@foo = external local_unnamed_addr global i32, align 4
; CHECK: .set implicitextern

; Function Attrs: nounwind
define void @bar() local_unnamed_addr #0 {
; CHECK-LABEL: .globl bar
entry:
  store i32 42, i32* @foo, align 4
; CHECK: mov64 %[[LIC:[a-z0-9_]+]], foo
; CHECK: st32 %[[LIC]], 42
  ret void
}

attributes #0 = { nounwind }

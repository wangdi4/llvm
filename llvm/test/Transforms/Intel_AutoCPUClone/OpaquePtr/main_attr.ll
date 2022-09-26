
; RUN: opt -opaque-pointers -auto-cpu-clone -function-attrs -inline < %s -S | FileCheck %s
; RUN: opt -opaque-pointers -passes=auto-cpu-clone,function-attrs,inline < %s -S | FileCheck %s

; The test checks that main()'s versions have their attributes modified/updated
; by those phases that modify/update main()'s attributes.


; CHECK-NOT: define dso_local void @main !llvm.auto.cpu.dispatch !3 {
; CHECK-DAG: @main.A() #0 !llvm.acd.clone 
; CHECK-DAG: @main.a() #1 !llvm.acd.clone
; CHECK-DAG: @main.resolver

; CHECK: attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn
; CHECK: attributes #1 = { mustprogress nofree norecurse nosync nounwind readnone willreturn 


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 0
}

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!""}
!3 = !{!4}
!4 = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}

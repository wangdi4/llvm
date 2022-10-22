; REQUIRES: asserts

; Checks that the CallTree Cloning transformation will pass the preliminary test stage even with a large number of
; (2450+) user-defined calls.
;
; This models a recent performance regression that is caused by an excessive number of user-defined calls
; on the module level.
;
; Instead of crafting a testcase with a large number of user calls, an internal flag is introduced to allow
; passing the value on cmdline.
;

; RUN: opt < %s -debug-only=call-tree-clone -passes='module(call-tree-clone)' -ctcmv-model-user-calls=1 -ctcmv-num-user-calls-modeled=2450 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=0 -disable-output 2>&1 | FileCheck %s

;
; CHECK:        Call-Tree Cloning preliminary analysis:  good
; CHECK-NEXT:   Call-Tree Cloning collection: NOT good


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@str = private unnamed_addr constant [13 x i8] c"inside hello\00", align 1

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @hello() local_unnamed_addr #0 {
entry:
  %puts = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([13 x i8], [13 x i8]* @str, i64 0, i64 0))
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  tail call void @hello()
  ret void
}

; Function Attrs: nofree nounwind
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #1

attributes #0 = { nofree noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}

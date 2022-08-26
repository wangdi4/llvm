; LLVM IR generated from following test using clang -O1 -S -emit-llvm
; int arr[1024];
;
; int foo()
; {
;   int index;
;
;   for (index = 0; index < 1024; index++)
;     arr[index] += index;
;
;   return 0;
; }
;
; Test to check that we remove auto vectorization directives even if we fail to
; vectorize the loop.
; NOTE: We use the switch -vplan-vectorizer-min-trip-count to make sure that loop
; is not vectorized. VPlan does not vectorize loops with constant trip counts lesser
; than the value specified in the switch.
; RUN: opt -enable-new-pm=0 -vplan-vectorizer-min-trip-count=1030 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after-all -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-vectorizer-min-trip-count=1030 -print-after-all -S < %s 2>&1 | FileCheck %s

;
; HIR Test.
; CHECK: IR Dump After{{.+}}Vec{{.*}}Dir{{.*}}Insert{{.*}}Pass{{.*}}
; CHECK: BEGIN REGION
; CHECK: llvm.directive.region.entry
; CHECK: DO i1 = 0, 1023, 1
; CHECK: llvm.directive.region.exit
; CHECK: END REGION
;
;
; CHECK: BEGIN REGION
; CHECK-NOT: llvm.directive.region.entry
; CHECK: DO i1 = 0, 1023, 1
; CHECK-NOT: llvm.directive.region.exit
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !0
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx, align 4, !tbaa !0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}

; This  fails had a compfail in  delineraize test called from Loop Fusion
; void foo(float *yarrrr, int n) {
;    int i;
;    for (i = 0; i < n; i++)
;        yarrrr[i] = 1;
; }
; void bar(float *ar, int n, int stride) {
;   int i, j;
;    for (i = 0; i < 2; i++) {
;        foo(ar, n - i);
;         for (j = i; j < n; j++)
;            ar[stride * j] = 0;
;    }
; }
;  Check for IR after loop fusion is sufficent to indicate it passes
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; CHECK: Function
; CHECK: DO i1
; CHECK: DO i2

;Module Before HIR; ModuleID = 'bug.cpp'
source_filename = "bug.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @_Z3barPfii(ptr nocapture %ar, i32 %n, i32 %stride) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %stride to i64
  %1 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.inc4, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc4 ]
  %2 = sub nsw i64 %1, %indvars.iv
  %cmp4.i = icmp sgt i64 %2, 0
  br i1 %cmp4.i, label %for.body.lr.ph.i, label %_Z3fooPfi.exit

for.body.lr.ph.i:                                 ; preds = %for.body
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ 0, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.body.i ]
  %arrayidx.i = getelementptr inbounds float, ptr %ar, i64 %indvars.iv.i
  store float 1.000000e+00, ptr %arrayidx.i, align 4, !tbaa !2
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, %2
  br i1 %exitcond.i, label %_Z3fooPfi.exit.loopexit, label %for.body.i

_Z3fooPfi.exit.loopexit:                          ; preds = %for.body.i
  br label %_Z3fooPfi.exit

_Z3fooPfi.exit:                                   ; preds = %_Z3fooPfi.exit.loopexit, %for.body
  %cmp216 = icmp slt i64 %indvars.iv, %1
  br i1 %cmp216, label %for.body3.lr.ph, label %for.inc4

for.body3.lr.ph:                                  ; preds = %_Z3fooPfi.exit
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv19 = phi i64 [ %indvars.iv, %for.body3.lr.ph ], [ %indvars.iv.next20, %for.body3 ]
  %3 = mul nsw i64 %indvars.iv19, %0
  %arrayidx = getelementptr inbounds float, ptr %ar, i64 %3
  store float 0.000000e+00, ptr %arrayidx, align 4, !tbaa !2
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond = icmp eq i64 %indvars.iv.next20, %1
  br i1 %exitcond, label %for.inc4.loopexit, label %for.body3

for.inc4.loopexit:                                ; preds = %for.body3
  br label %for.inc4

for.inc4:                                         ; preds = %for.inc4.loopexit, %_Z3fooPfi.exit
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond23, label %for.end6, label %for.body

for.end6:                                         ; preds = %for.inc4
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5f0935fd223fb291a49bd8e9b9c2c0664774e0b2)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}

; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; We do not know whether A and B alias, so we skip this case for dead store elimination
;
; Source code
;void foo(int *A, int *B, int N){
;  int i;
;  for (i = 0; i < N; ++i){
;    A[i] = i * i;
;    B[i]++;
;    A[i] = 2 * i;
;  }
;}
;
;
;*** IR Dump Before HIR Dead Store Elimination ***
;
;<0>       BEGIN REGION { }
;<19>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
;<3>             |   %mul = i1  *  i1;
;<5>             |   (%A)[i1] = %mul;
;<7>             |   %1 = (%B)[i1];
;<9>             |   (%B)[i1] = %1 + 1;
;<12>            |   (%A)[i1] = 2 * i1;
;<19>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;
; CHECK:   BEGIN REGION { }
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:        |   %mul = i1  *  i1;
; CHECK:        |   (%A)[i1] = %mul;
; CHECK:        |   %1 = (%B)[i1];
; CHECK:        |   (%B)[i1] = %1 + 1;
; CHECK:        |   (%A)[i1] = 2 * i1;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;Module Before HIR; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp16 = icmp sgt i32 %N, 0
  br i1 %cmp16, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %mul = mul nsw i32 %0, %0
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %mul, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx2, align 4, !tbaa !2
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx2, align 4, !tbaa !2
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %2 = shl i32 %indvars.iv.tr, 1
  store i32 %2, ptr %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 1114600a24c1bc4aa1934a8668c07c956a0831ec)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

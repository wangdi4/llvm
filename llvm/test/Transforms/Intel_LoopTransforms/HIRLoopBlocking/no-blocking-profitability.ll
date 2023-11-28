; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s --check-prefix=DEFAULT

; Verify that blocking does not happen for the following code.
; No outer loop IVs are in the contiguous memory access direction, and no missing IVs are present.
; Thus, no profitable refs were found by the loop blocking.

; DEFAULT-NOT: { modified }

;void matmul (int N) {
;  double  c[N*N];
;  double  a[N*N];
;  double  b[N*N];
;  int i,j;
;  for(i=0; i<N; i++) {
;    for(j=0; j<N; j++) {
;      c[N*i + j] += b[N*i + j] * a[N*i + j];
;    }
;  }
;}

; *** IR Dump After HIR Loop Blocking ***
; Function: matmul
;
;         BEGIN REGION { }
;               + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
;               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
;               |   |   %mul13 = (%vla4)[sext.i32.i64(%N) * i1 + i2]  *  (%vla2)[sext.i32.i64(%N) * i1 + i2];
;               |   |   %add18 = (%vla)[sext.i32.i64(%N) * i1 + i2]  +  %mul13;
;               |   |   (%vla)[sext.i32.i64(%N) * i1 + i2] = %add18;
;               |   + END LOOP
;               + END LOOP
;         END REGION

; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -hir-loop-blocking-skip-anti-pattern-check=true -hir-loop-blocking-no-delinear=true -disable-hir-loop-blocking-loop-depth-check=true 2>&1 < %s -disable-output | FileCheck %s --check-prefix=DEFAULT
; Verify also that blocking can be avoided without the help of none of checking anti-pattern, delinearization or loop-depth checks.

; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking" -print-changed -disable-output 2>&1 < %s -disable-output | FileCheck %s --check-prefix=CHECK-CHANGED
; Verify that pass is not dumped with print-changed if it bails out.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLoopBlocking

;Module Before HIR
; ModuleID = 'matmul2.c'
source_filename = "matmul2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local void @matmul(i32 %N) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %N, %N
  %0 = zext i32 %mul to i64
  %vla = alloca double, i64 %0, align 16
  %vla2 = alloca double, i64 %0, align 16
  %vla4 = alloca double, i64 %0, align 16
  %cmp44 = icmp sgt i32 %N, 0
  br i1 %cmp44, label %for.cond5.preheader.lr.ph, label %for.end21

for.cond5.preheader.lr.ph:                        ; preds = %entry
  %1 = sext i32 %N to i64
  br label %for.body7.lr.ph

for.body7.lr.ph:                                  ; preds = %for.cond5.preheader.lr.ph, %for.inc19
  %indvars.iv48 = phi i64 [ 0, %for.cond5.preheader.lr.ph ], [ %indvars.iv.next49, %for.inc19 ]
  %2 = mul nsw i64 %indvars.iv48, %1
  br label %for.body7

for.body7:                                        ; preds = %for.body7, %for.body7.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body7.lr.ph ], [ %indvars.iv.next, %for.body7 ]
  %3 = add nsw i64 %indvars.iv, %2
  %arrayidx = getelementptr inbounds double, ptr %vla4, i64 %3
  %4 = load double, ptr %arrayidx, align 8, !tbaa !2
  %arrayidx12 = getelementptr inbounds double, ptr %vla2, i64 %3
  %5 = load double, ptr %arrayidx12, align 8, !tbaa !2
  %mul13 = fmul double %4, %5
  %arrayidx17 = getelementptr inbounds double, ptr %vla, i64 %3
  %6 = load double, ptr %arrayidx17, align 8, !tbaa !2
  %add18 = fadd double %6, %mul13
  store double %add18, ptr %arrayidx17, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond, label %for.inc19, label %for.body7

for.inc19:                                        ; preds = %for.body7
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next49, %1
  br i1 %exitcond52, label %for.end21.loopexit, label %for.body7.lr.ph

for.end21.loopexit:                               ; preds = %for.inc19
  br label %for.end21

for.end21:                                        ; preds = %for.end21.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

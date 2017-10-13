; RUN: llc -O1 -csa-opt-df-pass=1 -csa-seq-opt=2 -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @simple_seq0(i32 %x, i32 %y, i32 %z, double* nocapture %a) #0 {
; CSA_CHECK-DAG: seqotgts64

entry:
  %0 = sext i32 %x to i64
  %1 = sext i32 %z to i64
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body ], [ %0, %entry ]
  %i.0 = phi i32 [ %add1, %do.body ], [ %x, %entry ]
  %2 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %2 to double
  %arrayidx = getelementptr inbounds double, double* %a, i64 %indvars.iv
  %3 = load double, double* %arrayidx, align 8, !tbaa !1
  %add = fadd double %3, %conv
  store double %add, double* %arrayidx, align 8, !tbaa !1
  %add1 = add nsw i32 %i.0, %z
  %cmp = icmp sle i32 %add1, %y
  %indvars.iv.next = add i64 %indvars.iv, %1
  br i1 %cmp, label %do.end, label %do.body

do.end:                                           ; preds = %do.body
  ret void
}

;; Second version: Identical to the first, except it reverses the order
;; of arguments to the compare
; Function Attrs: nounwind
define void @simple_seq1(i32 %x, i32 %y, i32 %z, double* nocapture %a) #0 {
; CSA_CHECK-DAG: seqotgts64

entry:
  %0 = sext i32 %x to i64
  %1 = sext i32 %z to i64
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body ], [ %0, %entry ]
  %i.0 = phi i32 [ %add1, %do.body ], [ %x, %entry ]
  %2 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %2 to double
  %arrayidx = getelementptr inbounds double, double* %a, i64 %indvars.iv
  %3 = load double, double* %arrayidx, align 8, !tbaa !1
  %add = fadd double %3, %conv
  store double %add, double* %arrayidx, align 8, !tbaa !1
  %add1 = add nsw i32 %i.0, %z
  %cmp = icmp sgt i32 %y, %add1
  %indvars.iv.next = add i64 %indvars.iv, %1
  br i1 %cmp, label %do.end, label %do.body

do.end:                                           ; preds = %do.body
  ret void
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 "}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

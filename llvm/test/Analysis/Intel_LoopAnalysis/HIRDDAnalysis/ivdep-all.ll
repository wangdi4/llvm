;Source code:
;
;void matmul(double *a, double *b, double *c) {
;  int i, j, k;
;  int n = 1024;
;  for (i = 0; i < 1024; i++) {
;    for (j = 0; j < 1024; j++) {
;      for (k = 0; k < 1024; k++){
;        c[i * n + j] += a[i * n + k] * b[k * n + j];
;      }
;    }
;  }
;}
;
; Test case for ivdep:
; when -hir-dd-test-assume-no-loop-carried-dep=2, DV for all nesting for memory refs should be set as  = = =
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=2 -analyze | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=2 -disable-output 2>&1 < %s | FileCheck %s
;
; CHECK-DAG: (%c)[1024 * i1 + i2] --> (%c)[1024 * i1 + i2] OUTPUT (= = =) (0 0 0)
; CHECK-DAG: (%a)[1024 * i1 + i3] --> (%c)[1024 * i1 + i2] ANTI (= = =) (0 0 0)
; CHECK-DAG: (%b)[i2 + 1024 * i3] --> (%c)[1024 * i1 + i2] ANTI (= = =) (0 0 0)
;
;Module Before HIR
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @matmul(double* nocapture readonly %a, double* nocapture readonly %b, double* nocapture %c) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc20, %entry
  %indvars.iv48 = phi i64 [ 0, %entry ], [ %indvars.iv.next49, %for.inc20 ]
  %0 = shl nsw i64 %indvars.iv48, 10
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv44 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next45, %for.inc17 ]
  %1 = add nuw nsw i64 %indvars.iv44, %0
  %arrayidx15 = getelementptr inbounds double, double* %c, i64 %1
  %.pre = load double, double* %arrayidx15, align 8, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %2 = phi double [ %.pre, %for.cond4.preheader ], [ %add16, %for.body6 ]
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %3 = add nuw nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds double, double* %a, i64 %3
  %4 = load double, double* %arrayidx, align 8, !tbaa !2
  %5 = shl i64 %indvars.iv, 10
  %6 = add nuw nsw i64 %5, %indvars.iv44
  %arrayidx10 = getelementptr inbounds double, double* %b, i64 %6
  %7 = load double, double* %arrayidx10, align 8, !tbaa !2
  %mul11 = fmul double %4, %7
  %add16 = fadd double %2, %mul11
  store double %add16, double* %arrayidx15, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.inc17, label %for.body6

for.inc17:                                        ; preds = %for.body6
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next45, 1024
  br i1 %exitcond47, label %for.inc20, label %for.cond4.preheader

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next49, 1024
  br i1 %exitcond51, label %for.end22, label %for.cond1.preheader

for.end22:                                        ; preds = %for.inc20
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

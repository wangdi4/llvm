; XFAIL: *
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; This one is not covered currently, becase it requires reordering of execution.
; Sequences in summary are generated as
;  A[2i]
;  A[2i+1]
;  B[2i]
;  B[2i+1]
;
; we can reroll if it were
;  A[2i]
;  B[2i]
;  A[2i+1]
;  B[2i+1]
;
; A TODO

;#define T int
;#define N 100
;T A[N];
;T B[N];
;T C[N];
;
;T foo() {
;  T S = 0;
;  T Y = 0;
;  for (int i = 0; i < N ; i=i+2) {
;    S += A[i] + A[i+1];
;    Y += B[i] + B[i+1];
;  }
;  return S + Y;
;}

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK:              |   %0 = (@A)[0][2 * i1];
; CHECK:              |   %S.024 = %0 + %S.024  +  (@A)[0][2 * i1 + 1];
; CHECK:              |   %3 = (@B)[0][2 * i1];
; CHECK:              |   %Y.025 = %3 + %Y.025  +  (@B)[0][2 * i1 + 1];
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:              |   %0 = (@A)[0][i1];
; CHECK:              |   %S.024 = %0 + %S.024
; CHECK:              |   %3 = (@B)[0][i1];
; CHECK:              |   %Y.025 = %3 + %Y.025
; CHECK:              + END LOOP
; CHECK:        END REGION


;Module Before HIR
; ModuleID = 'red-store.c'
source_filename = "red-store.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add4.lcssa = phi i32 [ %add4, %for.body ]
  %add11.lcssa = phi i32 [ %add11, %for.body ]
  %add13 = add nsw i32 %add11.lcssa, %add4.lcssa
  ret i32 %add13

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %Y.025 = phi i32 [ 0, %entry ], [ %add11, %for.body ]
  %S.024 = phi i32 [ 0, %entry ], [ %add4, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx, align 8, !tbaa !2
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %1, !intel-tbaa !2
  %2 = load i32, ptr %arrayidx2, align 4, !tbaa !2
  %add3 = add i32 %0, %S.024
  %add4 = add i32 %add3, %2
  %arrayidx6 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx6, align 8, !tbaa !2
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %1, !intel-tbaa !2
  %4 = load i32, ptr %arrayidx9, align 4, !tbaa !2
  %add10 = add i32 %3, %Y.025
  %add11 = add i32 %add10, %4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 4d9491ed638c64d173c02b77ca3a29cba2d9d1ef) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 84d2d36f7cbbf8594469caa5e415e2f0c5141e2e)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}


; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

;#define N 100
;#define T double
;T A[N];
;T B[N];
;T C[N];
;
;T foo() {
;  T S = 0;
;  for (int i = 0; i < N ; i=i+2) {
;    // A[i] = C[i] + B[i];
;    S -= A[i];
;    // A[i+1] = C[i+1] + B[i+1];
;    S -= A[i+1];
;  }
;  return S;
;}

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK:              |   %sub = %S.012  -  (@A)[0][2 * i1];
; CHECK:              |   %S.012 = %sub  -  (@A)[0][2 * i1 + 1];
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:              |   %S.012 = %S.012  -  (@A)[0][i1];
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'red-sub.c'
source_filename = "red-sub.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local double @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %sub3.lcssa = phi double [ %sub3, %for.body ]
  ret double %sub3.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %S.012 = phi double [ 0.000000e+00, %entry ], [ %sub3, %for.body ]
  %arrayidx = getelementptr inbounds [100 x double], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load double, ptr %arrayidx, align 16, !tbaa !2
  %sub = fsub double %S.012, %0
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [100 x double], ptr @A, i64 0, i64 %1, !intel-tbaa !2
  %2 = load double, ptr %arrayidx2, align 8, !tbaa !2
  %sub3 = fsub double %sub, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 4d9491ed638c64d173c02b77ca3a29cba2d9d1ef) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fcfa754a97800b6ffd2a8ba7284978104920f0ef)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

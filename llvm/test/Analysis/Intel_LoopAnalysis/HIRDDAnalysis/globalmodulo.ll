; RUN: opt < %s -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
;
; //C source:
; float Ag[100], Bg[100];
;
; void foo() {
;   float *A = Ag;
;   float *B = Bg;
;   for (int t = 0; t < 100; ++t) {
;     for (int i = 10; i < 90; ++i)
;       A[i] = B[i] + 1;
;     A = t % 5 ? Ag : Bg;
;     B = t % 5 ? Bg : Ag;
;   }
; }

; We should determine that there is a dependence through memory across the
; outer loop.
; CHECK-DAG: (%A.024)[i2 + 10] --> (%B.025)[i2 + 10] FLOW (*
; CHECK-DAG: (%B.025)[i2 + 10] --> (%A.024)[i2 + 10] ANTI (*
; Verify that we find an output dependence via the outer loop for the store.
; CHECK-DAG: (%A.024)[i2 + 10] --> (%A.024)[i2 + 10] OUTPUT (*

@Ag = common global [100 x float] zeroinitializer, align 16
@Bg = common global [100 x float] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %t.026 = phi i32 [ 0, %entry ], [ %inc11, %for.cond.cleanup3 ]
  %B.025 = phi float* [ getelementptr inbounds ([100 x float], [100 x float]* @Bg, i64 0, i64 0), %entry ], [ %cond9, %for.cond.cleanup3 ]
  %A.024 = phi float* [ getelementptr inbounds ([100 x float], [100 x float]* @Ag, i64 0, i64 0), %entry ], [ %cond, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %rem = urem i32 %t.026, 5
  %tobool = icmp ne i32 %rem, 0
  %cond = select i1 %tobool, float* getelementptr inbounds ([100 x float], [100 x float]* @Ag, i64 0, i64 0), float* getelementptr inbounds ([100 x float], [100 x float]* @Bg, i64 0, i64 0)
  %cond9 = select i1 %tobool, float* getelementptr inbounds ([100 x float], [100 x float]* @Bg, i64 0, i64 0), float* getelementptr inbounds ([100 x float], [100 x float]* @Ag, i64 0, i64 0)
  %inc11 = add nuw nsw i32 %t.026, 1
  %exitcond27 = icmp eq i32 %inc11, 100
  br i1 %exitcond27, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 10, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds float, float* %B.025, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %add = fadd float %0, 1.000000e+00
  %arrayidx6 = getelementptr inbounds float, float* %A.024, i64 %indvars.iv
  store float %add, float* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 90
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}


; RUN: opt < %s -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
;
; // C source:
;  extern double Ag[1000];
;  extern double Bg[1000];
;  ...
;    for (int i = 0; i < 97; ++i) {
;      for (int j = 0; j < N; ++j)
;        B[j] = A[j] + 1;
;      // Swap A and B pointers:
;      double *tmp = A; A = B; B = tmp;
;    }
;
; Verify dependences between the load and store via the outer loop are found.
; TODO: it is possible to prove that there are no dependencies across the inner
;       loop. This test doesn't check for that.
; CHECK-DAG: (%B.026)[i2] --> (%A.028.out)[i2] FLOW (*
; CHECK-DAG: (%A.028.out)[i2] --> (%B.026)[i2] ANTI (*
; Verify that we find an output dependence via the outer loop for the store.
; CHECK-DAG: (%B.026)[i2] --> (%B.026)[i2] OUTPUT (*

@Ag = external global [1000 x double], align 16
@Bg = external global [1000 x double], align 16

define void @swap_deps() {
entry:
  call void @initialize(double* getelementptr inbounds ([1000 x double], [1000 x double]* @Ag, i64 0, i64 0), double* getelementptr inbounds ([1000 x double], [1000 x double]* @Bg, i64 0, i64 0))
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %A.028 = phi double* [ getelementptr inbounds ([1000 x double], [1000 x double]* @Ag, i64 0, i64 0), %entry ], [ %B.026, %for.cond.cleanup3 ]
  %i.027 = phi i32 [ 0, %entry ], [ %inc8, %for.cond.cleanup3 ]
  %B.026 = phi double* [ getelementptr inbounds ([1000 x double], [1000 x double]* @Bg, i64 0, i64 0), %entry ], [ %A.028, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  call void @consume(double* %B.026, double* %A.028)
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc8 = add nuw nsw i32 %i.027, 1
  %exitcond29 = icmp eq i32 %inc8, 97
  br i1 %exitcond29, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds double, double* %A.028, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %add = fadd double %0, 1.000000e+00
  %arrayidx6 = getelementptr inbounds double, double* %B.026, i64 %indvars.iv
  store double %add, double* %arrayidx6, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

declare void @initialize(double*, double*)

declare void @consume(double*, double*)

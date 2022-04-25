; RUN: opt < %s -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
;
; // C source:
;  extern double Ag[1000];
;  extern double Bg[1000];
;  ...
;    for(int i=1; i<N; i++) {
;      double *A = (i > 5) ? Ag : Bg;
;      double *B = (i > 5) ? Bg : Ag;
;      A[i] = B[i-1];
;    }
;
; Verify dependences between the load and store are found.
; CHECK-DAG: (i64*)(%cond)[i1 + 1] --> (i64*)(%cond3)[i1] FLOW (*
; CHECK-DAG: (i64*)(%cond3)[i1] --> (i64*)(%cond)[i1 + 1] ANTI (*

@Ag = external dso_local local_unnamed_addr global [129 x double], align 16
@Bg = external dso_local local_unnamed_addr global [129 x double], align 16

define void @foo() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %cmp1 = icmp ugt i64 %indvars.iv, 5
  %cond = select i1 %cmp1, double* getelementptr inbounds ([129 x double], [129 x double]* @Ag, i64 0, i64 0), double* getelementptr inbounds ([129 x double], [129 x double]* @Bg, i64 0, i64 0)
  %cond3 = select i1 %cmp1, double* getelementptr inbounds ([129 x double], [129 x double]* @Bg, i64 0, i64 0), double* getelementptr inbounds ([129 x double], [129 x double]* @Ag, i64 0, i64 0)
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds double, double* %cond3, i64 %0
  %1 = bitcast double* %arrayidx to i64*
  %2 = load i64, i64* %1, align 8
  %arrayidx5 = getelementptr inbounds double, double* %cond, i64 %indvars.iv
  %3 = bitcast double* %arrayidx5 to i64*
  store i64 %2, i64* %3, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 129
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; C Source Code:
;
;int A[1000];
;int C[1000];
;int foo(int N) {
;  int i;
;  for (i = 0; i < N; ++i) {
;      C[5] = A[i] +1;
;      if(A[i] > 0){
;        break;
;      }
;  }
;  return 1;
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>       BEGIN REGION { }
;<18>            + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>
;<4>             |   %1 = (@A)[0][i1];
;<6>             |   (@C)[0][5] = %1 + 1;
;<9>             |   if (%1 > 0)
;<9>             |   {
;<10>            |      goto for.body.for.end_crit_edge;
;<9>             |   }
;<18>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK:     BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>
; CHECK:        |   %1 = (@A)[0][i1];
; CHECK:        |   %limm = %1 + 1;
; CHECK:        |   if (%1 > 0)
; CHECK:        |   {
; CHECK:        |      (@C)[0][5] = %limm;
; CHECK:        |      goto for.body.for.end_crit_edge;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:           (@C)[0][5] = %limm;
; CHECK:  END REGION
;
; ModuleID = 'new.ll'
source_filename = "new.ll"

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp1 = icmp slt i32 0, %N
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond:                                         ; preds = %for.body
  %i.0 = phi i32 [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.body:                                         ; preds = %for.body.lr.ph, %for.cond
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %i.0, %for.cond ]
  %0 = zext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %0
  %1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %1, 1
  store i32 %add, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4
  %cmp3 = icmp sgt i32 %1, 0
  %inc = add nuw nsw i32 %i.02, 1
  br i1 %cmp3, label %for.body.for.end_crit_edge, label %for.cond

for.body.for.end_crit_edge:                       ; preds = %for.body
  br label %for.end

for.cond.for.end_crit_edge:                       ; preds = %for.cond
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %for.body.for.end_crit_edge, %entry
  ret i32 1
}

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; store and load are in the different groups
; C Source Code:
;
;int A[1000];
;int B[1000];
;int C[1000];
;int foo(int N) {
;  int i;
;  for (i = 0; i < N; ++i) {
;      A[i+1] = B[4] + N ;
;      if(A[i] > 0){
;        break;
;      }
;      C[5] = A[i] + 1;
;  }
;  return 1;
;}

;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>       BEGIN REGION { }
;<23>            + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 999>
;<2>             |   %0 = (@B)[0][4];
;<7>             |   (@A)[0][i1 + 1] = %N + %0;
;<10>            |   %3 = (@A)[0][i1];
;<12>            |   if (%3 > 0)
;<12>            |   {
;<13>            |      goto for.body.for.end_crit_edge;
;<12>            |   }
;<17>            |   (@C)[0][5] = %3 + 1;
;<23>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:           %0 = (@B)[0][4];
; CHECK:           %limm = 0;
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 999>
; CHECK:        |   (@A)[0][i1 + 1] = %N + %0;
; CHECK:        |   %3 = (@A)[0][i1];
; CHECK:        |   if (%3 > 0)
; CHECK:        |   {
; CHECK:        |      if (i1 != 0)
; CHECK:        |      {
; CHECK:        |         (@C)[0][5] = %limm;
; CHECK:        |      }
; CHECK:        |      goto for.body.for.end_crit_edge;
; CHECK:        |   }
; CHECK:        |   %limm = %3 + 1;
; CHECK:        + END LOOP
; CHECK:           (@C)[0][5] = %limm;
; CHECK:  END REGION
;
; ModuleID = 'tt.ll'
source_filename = "tt.ll"

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp1 = icmp slt i32 0, %N
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %add1, %if.end ]
  %0 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @B, i64 0, i64 4), align 16
  %add = add nsw i32 %0, %N
  %add1 = add nuw nsw i32 %i.02, 1
  %1 = zext i32 %add1 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %1
  store i32 %add, ptr %arrayidx, align 4
  %2 = zext i32 %i.02 to i64
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %2
  %3 = load i32, ptr %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %3, 0
  br i1 %cmp4, label %for.body.for.end_crit_edge, label %if.end

if.end:                                           ; preds = %for.body
  %add7 = add nsw i32 %3, 1
  store i32 %add7, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4
  %cmp = icmp slt i32 %add1, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.body.for.end_crit_edge:                       ; preds = %for.body
  br label %for.end

for.cond.for.end_crit_edge:                       ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %for.body.for.end_crit_edge, %entry
  ret i32 1
}

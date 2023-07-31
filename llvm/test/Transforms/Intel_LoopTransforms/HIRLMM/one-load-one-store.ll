; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; store and load are in the same groups
; C Source Code:
;
;int A[1000];
;int C[1000];
;int foo(int N) {
;  int i;
;  for (i = 0; i < N; ++i) {
;      A[i+1] = C[5] + N ;
;      if(A[i] > 0){
;        break;
;      }
;      C[5] = A[i] + 1;
;  }
;  return 1;
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>       BEGIN REGION { }
;<23>            + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 999>
;<2>             |   %0 = (@C)[0][5];
;<7>             |   (@A)[0][i1 + 1] = %N + %0;
;<10>            |   %3 = (@A)[0][i1];
;<12>            |   if (%3 > 0)
;<12>            |   {
;<13>            |      goto for.end.loopexit;
;<12>            |   }
;<17>            |   (@C)[0][5] = %3 + 1;
;<23>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           %limm = (@C)[0][5];
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 999>
; CHECK:        |   %0 = %limm;
; CHECK:        |   (@A)[0][i1 + 1] = %N + %0;
; CHECK:        |   %3 = (@A)[0][i1];
; CHECK:        |   if (%3 > 0)
; CHECK:        |   {
; CHECK:        |      if (i1 != 0)
; CHECK:        |      {
; CHECK:        |         (@C)[0][5] = %limm;
; CHECK:        |      }
; CHECK:        |      goto for.end.loopexit;
; CHECK:        |   }
; CHECK:        |   %limm = %3 + 1;
; CHECK:        + END LOOP
; CHECK:            (@C)[0][5] = %limm;
; CHECK:  END REGION
;
; ModuleID = 'new.ll'
source_filename = "new.ll"

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp14 = icmp sgt i32 %N, 0
  br i1 %cmp14, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.preheader
  %i.015 = phi i32 [ %add1, %if.end ], [ 0, %for.body.preheader ]
  %0 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4
  %add = add nsw i32 %0, %N
  %add1 = add nuw nsw i32 %i.015, 1
  %1 = zext i32 %add1 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %1
  store i32 %add, ptr %arrayidx, align 4
  %2 = zext i32 %i.015 to i64
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %2
  %3 = load i32, ptr %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %3, 0
  br i1 %cmp4, label %for.end.loopexit, label %if.end

if.end:                                           ; preds = %for.body
  %add7 = add nsw i32 %3, 1
  store i32 %add7, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4
  %cmp = icmp slt i32 %add1, %N
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %if.end, %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 1
}

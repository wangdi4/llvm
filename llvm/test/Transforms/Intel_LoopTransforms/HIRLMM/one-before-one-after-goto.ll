; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; C Source Code:
;
;int A[1000];
;int B[1000];
;int C[1000];
;int foo(int N) {
;  int i;
;  for (i = 0; i < N; ++i) {
;      C[4] = B[i] * 4;
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
;<22>            + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>
;<4>             |   %1 = (@B)[0][i1];
;<6>             |   (@C)[0][4] = 4 * %1;
;<8>             |   %2 = (@A)[0][i1];
;<10>            |   if (%2 > 0)
;<10>            |   {
;<11>            |      goto for.body.for.end_crit_edge;
;<10>            |   }
;<15>            |   (@C)[0][5] = %2 + 1;
;<22>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           %limm1 = 0;
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>
; CHECK:        |   %1 = (@B)[0][i1];
; CHECK:        |   %limm = 4 * %1;
; CHECK:        |   %2 = (@A)[0][i1];
; CHECK:        |   if (%2 > 0)
; CHECK:        |   {
; CHECK:        |      (@C)[0][4] = %limm;
; CHECK:        |      if (i1 != 0)
; CHECK:        |      {
; CHECK:        |         (@C)[0][5] = %limm1;
; CHECK:        |      }
; CHECK:        |      goto for.body.for.end_crit_edge;
; CHECK:        |   }
; CHECK:        |   %limm1 = %2 + 1;
; CHECK:        + END LOOP
; CHECK:           (@C)[0][5] = %limm1;
; CHECK:           (@C)[0][4] = %limm;
; CHECK:  END REGION
;
; ModuleID = 'new.ll'
source_filename = "new.ll"

@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp1 = icmp slt i32 0, %N
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %if.end ]
  %0 = zext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %0
  %1 = load i32, ptr %arrayidx, align 4
  %mul = shl nsw i32 %1, 2
  store i32 %mul, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 4), align 16
  %arrayidx2 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %0
  %2 = load i32, ptr %arrayidx2, align 4
  %cmp3 = icmp sgt i32 %2, 0
  br i1 %cmp3, label %for.body.for.end_crit_edge, label %if.end

if.end:                                           ; preds = %for.body
  %add = add nsw i32 %2, 1
  store i32 %add, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4
  %inc = add nuw nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.body.for.end_crit_edge:                       ; preds = %for.body
  br label %for.end

for.cond.for.end_crit_edge:                       ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %for.body.for.end_crit_edge, %entry
  ret i32 1
}

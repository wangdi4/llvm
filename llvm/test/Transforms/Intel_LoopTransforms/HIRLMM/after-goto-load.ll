; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED
;
; Suppress the case when the load is after goto.
; C Source Code:
;
;int A[1000];
;int B[1000];
;int foo(int N) {
;  int i;
;  for (i = 0; i < N; ++i) {
;      if(A[i] > 0){
;        break;
;      }
;      A[i+1] = B[4] + N ;
;  }
;  return 1;
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>       BEGIN REGION { }
;<19>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 999>
;<5>             |   if ((@A)[0][i1] > 0)
;<5>             |   {
;<6>             |      goto for.end.loopexit;
;<5>             |   }
;<11>            |   %t0 = (@B)[0][4];
;<13>            |   (@A)[0][i1 + 1] = %N + %t0;
;<19>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK-NOT:  BEGIN REGION { modified }

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLMM


; ModuleID = 'new.ll'
source_filename = "new.ll"

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp10 = icmp sgt i32 %N, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %t1 = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv
  %t2 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %t2, 0
  br i1 %cmp1, label %for.end.loopexit, label %if.end

if.end:                                           ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv.next
  %t0 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @B, i64 0, i64 4), align 16
  %add = add nsw i32 %t0, %N
  store i32 %add, ptr %arrayidx4, align 4
  %cmp = icmp slt i64 %indvars.iv.next, %t1
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %if.end, %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 1
}

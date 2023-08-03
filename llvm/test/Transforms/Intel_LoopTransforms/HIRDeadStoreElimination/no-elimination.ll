; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; The first store (@A)[0][i1] should not be eliminated because of the load between the two stores.
;
; C Source Code:
;int A[50];
;int B[50];
;int foo(int N, int M){
;  int i;
;  for(i = 0; i < N; i++){
;    A[i] = 0;
;    int t = 0;
;    if(i < M){
;      t = A[i] + B[i];
;    }else{
;      t = t + B[i];
;    }
;    A[i] = t;
;  }
;  return A[0];
;}

;*** IR Dump Before HIR Dead Store Elimination ***
;Function: foo
;
;<0>       BEGIN REGION { }
;<36>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
;<4>             |   (@A)[0][i1] = 0;
;<6>             |   if (i1 < %M)
;<6>             |   {
;<19>            |      %0 = (@A)[0][i1];
;<22>            |      %1 = (@B)[0][i1];
;<24>            |      %t.0 = %0 + %1;
;<6>             |   }
;<6>             |   else
;<6>             |   {
;<12>            |      %2 = (@B)[0][i1];
;<14>            |      %t.0 = %2;
;<6>             |   }
;<29>            |   (@A)[0][i1] = %t.0;
;<36>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;Function: foo
;
; CHECK-NOT:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK:        |   (@A)[0][i1] = 0;
; CHECK:        |   if (i1 < %M)
; CHECK:        |   {
; CHECK:        |      %0 = (@A)[0][i1];
; CHECK:        |      %1 = (@B)[0][i1];
; CHECK:        |      %t.0 = %0 + %1;
; CHECK:        |   }
; CHECK:        |   else
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][i1];
; CHECK:        |      %t.0 = %2;
; CHECK:        |   }
; CHECK:        |   (@A)[0][i1] = %t.0;
; CHECK:        + END LOOP
; CHECK:  END REGION

; ModuleID = 'no-elimination-test.ll'
source_filename = "no-elimination-test.ll"

@A = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N, i32 %M) {
entry:
  %cmp2 = icmp slt i32 0, %N
  br i1 %cmp2, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %i.03 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %if.end ]
  %idxprom = sext i32 %i.03 to i64
  %arrayidx = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %idxprom
  store i32 0, ptr %arrayidx, align 4, !tbaa !0
  %cmp1 = icmp slt i32 %i.03, %M
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %idxprom2 = sext i32 %i.03 to i64
  %arrayidx3 = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %idxprom2
  %0 = load i32, ptr %arrayidx3, align 4, !tbaa !0
  %idxprom4 = sext i32 %i.03 to i64
  %arrayidx5 = getelementptr inbounds [50 x i32], ptr @B, i64 0, i64 %idxprom4
  %1 = load i32, ptr %arrayidx5, align 4, !tbaa !0
  %add = add nsw i32 %0, %1
  br label %if.end

if.else:                                          ; preds = %for.body
  %idxprom6 = sext i32 %i.03 to i64
  %arrayidx7 = getelementptr inbounds [50 x i32], ptr @B, i64 0, i64 %idxprom6
  %2 = load i32, ptr %arrayidx7, align 4, !tbaa !0
  %add8 = add nsw i32 0, %2
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %t.0 = phi i32 [ %add, %if.then ], [ %add8, %if.else ]
  %idxprom9 = sext i32 %i.03 to i64
  %arrayidx10 = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %idxprom9
  store i32 %t.0, ptr %arrayidx10, align 4, !tbaa !0
  %inc = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %3 = load i32, ptr @A, align 16, !tbaa !0
  ret i32 %3
}

!0 = !{!1, !2, i64 0}
!1 = !{!"array@_ZTSA50_i", !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

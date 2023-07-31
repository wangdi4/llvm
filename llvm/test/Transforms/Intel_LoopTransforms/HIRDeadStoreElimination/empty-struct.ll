; Check there is no compfail when the size of ref is 0
;
; RUN: opt -aa-pipeline="tbaa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir>" -hir-create-function-level-region 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Dead Store Elimination ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<19>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<7>                |   (@A)[0][i1] = i1;
;<8>                |   @bar(&((%struc)[0]));
;<11>               |   (@A)[0][i1] = i1 + 1;
;<19>               + END LOOP
;<19>
;<18>               ret ;
;<0>          END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (@A)[0][i1] = i1;
; CHECK:           |   @bar(&((%struc)[0]));
; CHECK:           |   (@A)[0][i1] = i1 + 1;
; CHECK:           + END LOOP
;
; CHECK:           ret ;
; CHECK:     END REGION
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local global [100 x i32] zeroinitializer, align 16

define dso_local void @foo(ptr %struc) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  call void @bar(ptr %struc)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %1 = trunc i64 %indvars.iv.next to i32
  store i32 %1, ptr %arrayidx, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

declare void @bar(ptr);

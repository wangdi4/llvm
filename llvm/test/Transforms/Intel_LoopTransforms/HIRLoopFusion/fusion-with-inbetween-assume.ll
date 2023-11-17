; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Verify that we can fuse the two loops when there is an assume intrinsic
; between them. Asusme intrinsic should not be classified as unsafe inst.

; INPUT:
; BEGIN REGION { }
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   (%a)[i1] = i1;
;    + END LOOP
;
;    @llvm.assume(-1);
;
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   %1 = (%a)[i1];
;    |   (%b)[i1] = %1 + 1;
;    + END LOOP
;
;    ret ;
; END REGION

; CHECK: BEGIN REGION { modified }

; CHECK:      + DO i1 = 0, 99, 1
; CHECK-NEXT: |   (%a)[i1] = i1;
; CHECK-NEXT: |   %1 = (%a)[i1];
; CHECK-NEXT: |   (%b)[i1] = %1 + 1;
; CHECK-NEXT: + END LOOP

; CHECK-NOT: + DO i1

; CHECK:      @llvm.assume(1);

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture %a, ptr nocapture %b, i32 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv24
  %0 = trunc i64 %indvars.iv24 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next25, 100
  br i1 %exitcond26, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  call void @llvm.assume(i1 true)
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  store i32 %add, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  ret void
}


declare void @llvm.assume(i1 noundef) #2

attributes #2 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }


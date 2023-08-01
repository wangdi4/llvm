; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-cost-model-throttling=0 -disable-output -hir-details 2>&1 | FileCheck %s

; Verify that DD forms edges for fake call refs.

; We should form fake lval ref for &((%A)[0] in @bar1 and fake rval ref for &((%B)[0]) in @bar2.

; CHECK: + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   @bar1(&((%A)[0]),  &((@C)[0]));
; CHECK: |   <FAKE-LVAL-REG> (LINEAR ptr %A)[i64 0]

; CHECK-NOT: FAKE

; CHECK: |   @bar2(&((%A)[0]),  &((%B)[0]));
; CHECK: |   <FAKE-RVAL-REG> (LINEAR ptr %B)[i64 0]

; CHECK-NOT: FAKE

; CHECK: |   @bar3(&((%B)[0]));

; CHECK-NOT: FAKE

; CHECK: + END LOOP

; Fake refs should lead to following edges-

; CHECK-DAG: (%A)[0] --> (%A)[0] OUTPUT (*) (?)
; CHECK-DAG: (%A)[0] --> (%B)[0] FLOW (*) (?)
; CHECK-DAG: (%B)[0] --> (%A)[0] ANTI (*) (?)


; ModuleID = 'func.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = internal unnamed_addr constant [4 x i32] [i32 1, i32 2, i32 3, i32 4], align 16

define void @foo(ptr %A, ptr %B, i32 %n) {
entry:
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.05 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  call void @bar1(ptr %A, ptr @C)
  call void @bar2(ptr %A, ptr %B)
  call void @bar3(ptr %B)
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare void @bar1(ptr, ptr)
declare void @bar2(ptr readnone, ptr readonly)
declare void @bar3(ptr) readnone

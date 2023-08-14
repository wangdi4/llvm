; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-array-transpose,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that transpose kicks in with opaque pointers when there are no bitcasts
; in the incoming IR and the malloc offset &((%call)[8]) is computed in bytes.

; CHECK: Function

; CHECK: BEGIN REGION { }

; CHECK: %call = @malloc(80);
; CHECK: %base = &((%call)[8]);

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%base)[0][5 * i1 + -2] = i1;
; CHECK: + END LOOP

; CHECK: @free(&((%base)[-1]));
; CHECK: ret ;
; CHECK: END REGION


; CHECK: Function

; CHECK: BEGIN REGION { modified }
; CHECK: %call = @malloc(80);
; CHECK: %base = &((%call)[16]);

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%base)[0][i1 + -4] = i1;
; CHECK: + END LOOP

; CHECK: @free(&((%base)[-2]));
; CHECK: ret ;
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() local_unnamed_addr #0 {
entry:
  br label %preheader

preheader:                                        ; preds = %entry
  %call = tail call noalias ptr @malloc(i64 80)
  %base = getelementptr inbounds i8, ptr %call, i64 8
  br label %for.body

for.body:                                         ; preds = %for.body, %preheader
  %indvars.iv = phi i64 [ 0, %preheader ], [ %indvars.iv.next, %for.body ]
  %t1 = mul nuw nsw i64 %indvars.iv, 5
  %t2 = add nsw i64 %t1, -2
  %arrayidx = getelementptr inbounds [10 x i64], ptr %base, i64 0, i64 %t2
  store i64 %indvars.iv, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %gep = getelementptr inbounds i64, ptr %base, i64 -1
  call void @free(ptr nonnull %gep)
  ret void
}

declare noalias ptr @malloc(i64) local_unnamed_addr

declare void @free(ptr nocapture) local_unnamed_addr

attributes #0 = { "may_have_huge_local_malloc" }

; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-array-transpose,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; RUN: opt -xmain-opt-level=3 -opaque-pointers -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-array-transpose,print<hir>" -disable-output 2>&1 < %s | FileCheck %s --check-prefix=CHECK-OPAQUE

; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-array-transpose" -print-changed -disable-output 2>&1 < %s | FileCheck %s --check-prefix=CHECK-CHANGED

; Verify that array transpose kicks in when this sequence-

; %intptr = ptrtoint.i8*.i64(&((%call)[8]));
; %base = inttoptr.i64.[10 x i64]*(%intptr);

; Is replaced in the incoming IR by this bitcast instead-

; %base = bitcast.i8*.[10 x i64]*(&((%call)[8]));

; CHECK: Function

; CHECK: BEGIN REGION { }
; CHECK: %call = @malloc(80);
; CHECK: %base = bitcast.i8*.[10 x i64]*(&((%call)[8]));

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%base)[0][5 * i1 + -2] = i1;
; CHECK: + END LOOP

; CHECK: %tfree = bitcast.[10 x i64]*.i64*(&(([10 x i64]*)(%call)[8]));
; CHECK: @free(&((i8*)(%tfree)[-1]));
; CHECK: ret ;
; CHECK: END REGION


; CHECK: Function

; CHECK: BEGIN REGION { modified }
; CHECK: %call = @malloc(80);
; CHECK: %base = bitcast.i8*.[10 x i64]*(&((%call)[16]));

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%base)[0][i1 + -4] = i1;
; CHECK: + END LOOP

; CHECK: %tfree = bitcast.[10 x i64]*.i64*(&(([10 x i64]*)(%call)[16]));
; CHECK: @free(&((i8*)(%tfree)[-2]));
; CHECK: ret ;
; CHECK: END REGION

; Verify that transformation is triggered with opaque pointers

; CHECK-OPAQUE: BEGIN REGION { modified }
; CHECK-OPAQUE: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK-OPAQUE: |   (%base)[0][i1 + -4] = i1;
; CHECK-OPAQUE: + END LOOP

; Verify that pass is dumped with print-changed when it triggers.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRArrayTranspose

;Module Before HIR; ModuleID = 'transpose.c'
source_filename = "transpose.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() local_unnamed_addr "may_have_huge_local_malloc" {
entry:
  br label %preheader

preheader:
  %call = tail call noalias i8* @malloc(i64 80)
  %add.ptr = getelementptr inbounds i8, i8* %call, i64 8
  %base = bitcast i8* %add.ptr to [10 x i64]*
  br label %for.body

for.body:                                         ; preds = %for.body, %preheader
  %indvars.iv = phi i64 [ 0, %preheader ], [ %indvars.iv.next, %for.body ]
  %t1 = mul nuw nsw i64 %indvars.iv, 5
  %t2 = add nsw i64 %t1, -2
  %arrayidx = getelementptr inbounds [10 x i64], [10 x i64]* %base, i64 0, i64 %t2
  store i64 %indvars.iv, i64* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %tfree = bitcast [10 x i64]* %base to i64*
  %gep = getelementptr inbounds i64, i64* %tfree, i64 -1
  %bc = bitcast i64* %gep to i8*
  call void @free(i8* nonnull %bc)
  ret void
}

declare noalias i8* @malloc(i64) local_unnamed_addr

declare void @free(i8* nocapture) local_unnamed_addr


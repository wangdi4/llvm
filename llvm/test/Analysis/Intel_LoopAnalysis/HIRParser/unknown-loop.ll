; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 | opt -analyze -hir-parser -hir-cost-model-throttling=0 | FileCheck %s

; Check that we correctly parse the unknown loop. The bottom test "if (i1 < %n.addr.012)" has been shifted by -1 to adjust for the IV update copy which will be generated just before it during code gen.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body:
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   if (%0 < 0)
; CHECK: |   {
; CHECK: |      (%A)[i1] = %0 + -1;
; CHECK: |      %n.addr.012 = %n.addr.012  +  -1;
; CHECK: |   }
; CHECK: |   if (i1 + 1 < %n.addr.012)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body;
; CHECK: |   }
; CHECK: + END LOOP

; Check CG for unknown loop
; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-cg -force-hir-cg -S | FileCheck -check-prefix=CHECK-CG %s

; CHECK-CG: region.0
; Get the symbase of %n
; CHECK-CG: store i32 %n, i32* [[NSYM:%t[0-9]+]]
; Check that we generate an IV for unknown loops starting from 0.
; CHECK-CG: store i64 0, i64* %i1.i64

; CHECK-CG: [[LOOPLABEL:.*]]:
; Check the first merge block where the IV update and bottom test are supposed to be generated
; CHECK-CG: ifmerge{{.*}}:

; IV update
; CHECK-CG: [[IVLOAD:%.*]] = load i64, i64* %i1.i64
; CHECK-CG: [[NEXTIV:%.*]] = add nuw nsw i64 [[IVLOAD]], 1

; Bottom test generation
; CHECK-CG: [[IVLOAD1:%.*]] = load i64, i64* %i1.i64
; CHECK-CG: [[IVADD:%.*]] = add i64 [[IVLOAD1]], 1
; CHECK-CG: [[NLOAD:%.*]] = load i32, i32* [[NSYM]]
; CHECK-CG: [[NSEXT:%.*]] = sext i32 [[NLOAD]] to i64
; CHECK-CG: [[CMP:%.*]] = icmp slt i64 [[IVADD]], [[NSEXT]]
; CHECK-CG: br i1 [[CMP]], label %[[BEJUMP:then.[0-9]+]], label %ifmerge.{{[0-9]+}}

; Check that then case updates IV and jumps back to loop header

; CHECK-CG: [[BEJUMP]]:
; CHECK-CG-NEXT: store i64 [[NEXTIV]], i64* %i1.i64
; CHECK-CG-NEXT: br label %[[LOOPLABEL]]


;Module Before HIR; ModuleID = 'unknown.c'
source_filename = "unknown.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr {
entry:
  %cmp11 = icmp sgt i32 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %n.addr.012 = phi i32 [ %n.addr.1, %for.inc ], [ %n, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp slt i32 %0, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %dec = add nsw i32 %0, -1
  store i32 %dec, i32* %arrayidx, align 4
  %dec4 = add nsw i32 %n.addr.012, -1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %n.addr.1 = phi i32 [ %dec4, %if.then ], [ %n.addr.012, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %1 = sext i32 %n.addr.1 to i64
  %cmp = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}


; RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s
;Instruction on <4> is an HInst with an underlying GetElementPtrInst
;Check it is CG'd correctly

;          BEGIN REGION { }
;<10>         + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;<3>          |   (%add.ptr)[0] = i1;
;<4>          |   %add.ptr = &((%add.ptr)[i1]);
;<10>         + END LOOP
;          END REGION
;


; Check that CG leaves HIR metadata in the old region as they become unreachable and will be removed later by simplifycfg. 
; CHECK: in.de.ssa


; CHECK: region.0:
; CHECK: {{loop.[0-9]+:}}
; IV is for rhs of <3>
; CHECK: load i64, i64* %i1.i64

; CG for RHS, &((%add.ptr)[i1]);
; CHECK: [[IV_LOAD:%.*]] = load i64, i64* %i1.i64
; CHECK: [[ARRAY_ADDR:%.*]] = getelementptr inbounds i64, i64* [[A_SYM:%t[0-9+]]]{{.*}}, i64 [[IV_LOAD]]
; We should be storing it into lhs, %add.ptr's symbase memory slot
; CHECK-NEXT: store i64* [[ARRAY_ADDR]], i64** [[A_SYM]]

; Check that HIR metadata is cleaned up by CG when we do not generate code for the region.
; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -S | FileCheck %s -check-prefix=NOCG

; NOCG-NOT: in.de.ssa
; NOCG-NOT: out.de.ssa


; ModuleID = 'poly-ptr-64.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64* nocapture %p, i64 %n) {
entry:
  %cmp.6 = icmp sgt i64 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.08 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi i64* [ %add.ptr, %for.body ], [ %p, %for.body.preheader ]
  store i64 %i.08, i64* %p.addr.07, align 8
  %add.ptr = getelementptr inbounds i64, i64* %p.addr.07, i64 %i.08
  %inc = add nuw nsw i64 %i.08, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

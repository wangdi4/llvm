; Check that nsw, exact flags and fpmath metadata attached to the LLVM instructions are preserved after HIR CG.

; RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -print-before=hir-cg -print-after=hir-cg < %s 2>&1 | FileCheck %s 

; CHECK: IR Dump Before
; CHECK: !fpmath
; CHECK: exact
; CHECK: add nuw nsw i32 %c.05

; CHECK: IR Dump After
; CHECK: region.0:
; CHECK: !fpmath
; CHECK: exact
; CHECK: add nuw nsw i32 

; ModuleID = 'fp.ll'
source_filename = "fp.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define float @foo(i32 %n, i64* %base, i64* %ptr) #0 {
entry:
  %conv = sext i32 %n to i64
  %cmp1 = icmp ult i64 0, %conv
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %c.05 = phi i32 [ 0, %for.body.lr.ph ], [ %add8, %for.inc ]
  %b.04 = phi i32 [ 0, %for.body.lr.ph ], [ %conv5, %for.inc ]
  %a.03 = phi float [ 0.000000e+00, %for.body.lr.ph ], [ %add, %for.inc ]
  %i.02 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %conv2 = uitofp i64 %i.02 to float
  %mul = fmul float %conv2, 3.000000e+00, !fpmath !0
  %add = fadd float %a.03, %mul
  %add.ptr = getelementptr inbounds i64, i64* %ptr, i64 %i.02
  %sub.ptr.lhs.cast = ptrtoint i64* %add.ptr to i64
  %sub.ptr.rhs.cast = ptrtoint i64* %base to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 8
  %div = udiv i64 %sub.ptr.div, 8
  %conv3 = sext i32 %b.04 to i64
  %add4 = add i64 %conv3, %div
  %conv5 = trunc i64 %add4 to i32
  %conv6 = fptosi float %add to i32
  %add7 = add nuw nsw i32 %conv6, %conv5
  %add8 = add nuw nsw i32 %c.05, %add7
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i64 %i.02, 1
  %cmp = icmp ult i64 %inc, %conv
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  %split = phi i32 [ %add8, %for.inc ]
  %split6 = phi i32 [ %conv5, %for.inc ]
  %split7 = phi float [ %add, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %c.0.lcssa = phi i32 [ %split, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  %b.0.lcssa = phi i32 [ %split6, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  %a.0.lcssa = phi float [ %split7, %for.cond.for.end_crit_edge ], [ 0.000000e+00, %entry ]
  %conv9 = sitofp i32 %b.0.lcssa to float
  %add10 = fadd float %a.0.lcssa, %conv9
  %conv11 = sitofp i32 %c.0.lcssa to float
  %add12 = fadd float %add10, %conv11
  ret float %add12
}

!0 = !{ float 2.5 }


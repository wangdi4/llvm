; REQUIRES: asserts
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=8 -vplan-print-after-vpentity-instrs -vplan-entities-dump -disable-output 2>&1 | FileCheck %s

; This test case checks that the reduction with the intrinsic llvm.umax
; was vectorized correctly. It was created from the following test case:

; unsigned foo(unsigned *a, unsigned n) {
;   unsigned max = 1000;
;   for (unsigned i = 0; i < n; i++) {
;     max = a[i] > max ? a[i] : max;
;   }
;   return max;
; }

; HIR generated

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;       |   %max.013 = @llvm.umax.i32((%a)[i1],  %max.013);
;       + END LOOP
; END REGION

; Check VPlan reduction

; CHECK: Reduction list
; CHECK:  (UIntMax) Start: i32 %max.013 Exit: i32 [[VP_LOAD:%.*]]
; CHECK:   Linked values: i32 [[VP1:%.*]], i32 [[VP_LOAD:%.*]], i32 [[VP2:%.*]], i32 [[VP3:%.*]],

; CHECK: i32 [[VP2:%.*]] = reduction-init i32 %max.013
; CHECK: i32 [[VP_LOAD:%.*]] = call i32 [[VP4:%.*]] i32 [[VP1:%.*]] i32 (i32, i32)* @llvm.umax.i32
; CHECK: i32 [[VP3:%.*]] = reduction-final{u_umax} i32 [[VP_LOAD:%.*]]

; Check for vectorizer code

; CHECK: %red.init = %max.013;
; CHECK: %phi.temp5 = %red.init;
; CHECK: %loop.ub = %vec.tc4  -  1;
; CHECK: + DO i1 = 0, %loop.ub, 8   <DO_LOOP>  <MAX_TC_EST = 536870911>  <LEGAL_MAX_TC = 536870911> <auto-vectorized> <nounroll> <novectorize>
; CHECK: |   %.vec7 = (<8 x i32>*)(%a)[i1];
; CHECK: |   %llvm.umax.v8i32 = @llvm.umax.v8i32(%.vec7,  %phi.temp5);
; CHECK: |   %phi.temp5 = %llvm.umax.v8i32;
; CHECK: + END LOOP
; CHECK: %max.013 = @llvm.vector.reduce.umax.v8i32(%llvm.umax.v8i32);
; CHECK: %ind.final = 0  +  %vec.tc4;


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @_Z3fooPjj(i32* nocapture noundef readonly %a, i32 noundef %n) {
entry:
  %cmp12.not = icmp eq i32 %n, 0
  br i1 %cmp12.not, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %.lcssa = phi i32 [ %1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %max.0.lcssa = phi i32 [ 0, %entry ], [ %.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %max.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %max.013 = phi i32 [ 0, %for.body.preheader ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = tail call i32 @llvm.umax.i32(i32 %0, i32 %max.013)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.umax.i32(i32, i32)
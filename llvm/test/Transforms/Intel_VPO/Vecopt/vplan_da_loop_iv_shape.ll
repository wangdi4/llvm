; Test to check that DA initializes correct VectorShape for different types of IVs in loop.

; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -debug-only=vplan-divergence-analysis -S 2>&1 | FileCheck %s

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header><latch><exiting>
; CHECK: Basic Block: BB3
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i64 [[STRIDED_IV_PHI:%vp.*]] = phi  [ i64 0, [[PREHEADER:BB.*]] ],  [ i64 [[STRIDED_IV_ADD:%vp.*]], [[BODY:BB.*]] ]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[UNIT_STRIDE_IV_PHI:%vp.*]] = phi  [ i64 0, [[PREHEADER]] ],  [ i64 [[UNIT_STRIDE_IV_ADD:%vp.*]], [[BODY]] ]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 -1] i64 [[UNIT_NEG_STRIDE_IV_PHI:%vp.*]] = phi  [ i64 0, [[PREHEADER]] ],  [ i64 [[UNIT_NEG_STRIDE_IV_ADD:%vp.*]], [[BODY]] ]
; CHECK-NEXT: Divergent: [Shape: Random] i64 [[VAR_STRIDE_PHI:%vp.*]] = phi  [ i64 0, [[PREHEADER]] ],  [ i64 [[VAR_STRIDE_ADD:%vp.*]], [[BODY]] ]
; Check users
; CHECK: Divergent: [Shape: Strided, Stride: i64 16] i32* {{%vp.*}} = getelementptr inbounds i32* %ary i64 [[STRIDED_IV_PHI]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i32* {{%vp.*}} = getelementptr inbounds i32* %ary i64 [[UNIT_STRIDE_IV_PHI]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 -4] i32* {{%vp.*}} = getelementptr inbounds i32* %ary i64 [[UNIT_NEG_STRIDE_IV_PHI]]
; CHECK: Divergent: [Shape: Random] i32* {{%vp.*}} = getelementptr inbounds i32* %ary i64 [[VAR_STRIDE_PHI]]
; Check updates
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i64 [[STRIDED_IV_ADD]] = add i64 [[STRIDED_IV_PHI]] i64 4
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[UNIT_STRIDE_IV_ADD]] = add i64 [[UNIT_STRIDE_IV_PHI]] i64 1
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 -1] i64 [[UNIT_NEG_STRIDE_IV_ADD]] = add i64 [[UNIT_NEG_STRIDE_IV_PHI]] i64 -1
; CHECK-NEXT: Divergent: [Shape: Random] i64 [[VAR_STRIDE_ADD]] = add i64 [[VAR_STRIDE_PHI]] i64 %var.step


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary, i32 %x, i32 %y) {
pre.entry:
  %i = alloca i64, align 4
  br label %entry

entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR"(i64* %i, i32 -1) ]
  br label %for.preheader

for.preheader:
  %add.neg = sub i32 2, %x
  %sub = sub i32 %add.neg, %y
  %mul = mul nsw i32 %y, %x
  %var.step = sext i32 %sub to i64
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %strided.iv = phi i64 [ 0, %for.preheader ], [ %strided.iv.next, %for.body ]
  %unit.stride.iv = phi i64 [ 0, %for.preheader ], [ %unit.stride.iv.next, %for.body ]
  %unit.neg.stride.iv = phi i64 [ 0, %for.preheader ], [ %unit.neg.stride.iv.next, %for.body ]
  %var.stride.iv = phi i64 [ 0, %for.preheader ], [ %var.stride.iv.next, %for.body ]

  ; Users of IVs
  %strided.gep = getelementptr inbounds i32, i32* %ary, i64 %strided.iv
  %0 = load i32, i32* %strided.gep, align 4
  %unit.stride.gep = getelementptr inbounds i32, i32* %ary, i64 %unit.stride.iv
  %1 = load i32, i32* %unit.stride.gep, align 4
  %unit.neg.stride.gep = getelementptr inbounds i32, i32* %ary, i64 %unit.neg.stride.iv
  %2 = load i32, i32* %unit.neg.stride.gep, align 4
  %var.stride.gep = getelementptr inbounds i32, i32* %ary, i64 %var.stride.iv
  %3 = load i32, i32* %var.stride.gep, align 4

  ; Stride updates for IVs
  %strided.iv.next = add nuw nsw i64 %strided.iv, 4
  %unit.stride.iv.next = add nuw nsw i64 %unit.stride.iv, 1
  %unit.neg.stride.iv.next = add nuw nsw i64 %unit.neg.stride.iv, -1
  %var.stride.iv.next = add nuw nsw i64 %var.stride.iv, %var.step

  %cmp = icmp ult i64 %strided.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

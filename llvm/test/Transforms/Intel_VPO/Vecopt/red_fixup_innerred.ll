; Verify that we can fix up an incomplete reduction with a missing Exit
; instruction when the reduction occurs in a loop deeper than the outermost.
; Test reduced from customer code.  See CMPLRLLVM-44206.

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -S -disable-output -vplan-print-after-vpentity-instrs < %s 2>&1 | FileCheck %s

; CHECK: VPlan after insertion of VPEntities instructions:
; CHECK-NEXT: VPlan IR for: foo:HIR.#1
; CHECK-NEXT: External Defs Start:
; CHECK-NEXT:  {{%.*}} = {%add.32.lcssa239}

; CHECK: BB2:
; CHECK: [[REDINIT:%.*]] = reduction-init double 0.000000e+00
; CHECK: br BB3

; CHECK: BB3:
; CHECK: double [[PHI1:%.*]] = phi [ double [[REDINIT]], BB2 ],  [ double [[PHI3:%.*]], BB8 ]
; CHECK: br i1 {{%.*}}, BB4, BB8

; CHECK: BB4:
; CHECK: br BB5

; CHECK: BB5:
; CHECK: br BB6

; CHECK: BB6:
; CHECK: double [[PHI2:%.*]] = phi [ double [[PHI1]], BB5 ],  [ double [[FADD:%.*]], BB6 ]
; CHECK: double [[FADD]] = fadd double [[PHI2]] double 5.000000e+00
; CHECK: br i1 {{%.*}}, BB6, BB7

; CHECK: BB7:
; CHECK: br BB8

; CHECK: BB8:
; CHECK: [[PHI3]] = phi [ double [[FADD]], BB7 ],  [ double [[PHI1]], BB3 ]
; CHECK: br i1 {{%.*}}, BB3, BB9

; CHECK: BB9:
; CHECK: double {{%.*}} = reduction-final{fadd} double [[PHI3]] double %add.32.lcssa239

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @foo() #0 {
bb1:
  %sum = alloca double, align 8
  %outeriv = alloca i32, align 4
  %inneriv = alloca i32, align 4
  br label %bb2

bb2:
  %tok0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum, double zeroinitializer, i32 1) ]
  store i32 0, ptr %outeriv, align 4
  store double 0.000000e+00, ptr %sum, align 8
  br label %bb3

bb3:
  %oiv = phi i32 [ 0, %bb2 ], [ %oiv.1, %bb7 ]
  %add.32.lcssa239 = phi double [ 0.000000e+00, %bb2 ], [ %sum.outer, %bb7 ]
  %test0 = fcmp oge double %add.32.lcssa239, 9.233514e+22
  br i1 %test0, label %bb7, label %bb4

bb4:
  store i32 0, ptr %inneriv, align 4
  br label %bb5

bb5:
  %iv = phi i32 [ %iv.1, %bb5 ], [ 0, %bb4 ]
  %sum.inner = phi double [ %add.32, %bb5 ], [ %add.32.lcssa239, %bb4 ]
  %add.32 = fadd reassoc ninf nsz arcp contract afn double %sum.inner, 5.000000e+00
  %iv.1 = add nsw nuw i32 %iv, 1
  %test1 = icmp eq i32 %iv.1, 1024
  br i1 %test1, label %bb6, label %bb5

bb6:
  %add.32.lcssa = phi double [ %add.32, %bb5 ]
  br label %bb7

bb7:
  %sum.outer = phi double [ %add.32.lcssa, %bb6 ], [ %add.32.lcssa239, %bb3 ]
  %oiv.1 = add nsw nuw i32 %oiv, 1
  %test2 = icmp eq i32 %oiv.1, 512
  br i1 %test2, label %bb8, label %bb3

bb8:
  %sum.lcssa = phi double [ %sum.outer, %bb7 ]
  store double %sum.lcssa, ptr %sum, align 8
  br label %bb9

bb9:
  call void @llvm.directive.region.exit(token %tok0) [ "DIR.OMP.END.SIMD"() ]
ret double %sum.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

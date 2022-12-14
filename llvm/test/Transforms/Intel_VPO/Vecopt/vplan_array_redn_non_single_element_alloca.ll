; Verify that VPlan vectorizer bails out from loops containing array reductions
; represented via non-single element allocas.

; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality -print-after=vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; Checks for LLVM-IR vectorizer
; CHECK: VPlan LLVM-IR Driver for Function: test
; CHECK: Cannot handle array reductions.
; CHECK: VD: Not vectorizing: Cannot prove legality.

; CHECK: define i32 @test
; CHECK: %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %sum, i32 0, i32 42) ]
;
; Checks for HIR vectorizer
; HIR: VPlan HIR Driver for Function: test
; HIR: Cannot handle array reductions.
; HIR: VD: Not vectorizing: Cannot prove legality.

; HIR: Function: test
; HIR: %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD:TYPED(&((%sum)[0])042) ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @test(i32* nocapture readonly %A, i64 %N, i32 %init) {
entry:
  %sum = alloca i32, i32 42, align 4
  br label %fill.sum

fill.sum:
  %arr.begin = getelementptr inbounds i32, i32* %sum, i32 0
  %arr.end = getelementptr i32, i32* %arr.begin, i32 42
  %red.init.isempty = icmp eq i32* %arr.begin, %arr.end
  br i1 %red.init.isempty, label %begin.simd.1, label %red.init.body

red.init.body:
  %red.curr.ptr = phi i32* [ %arr.begin, %fill.sum ], [ %red.next.ptr, %red.init.body ]
  store i32 %init, i32* %red.curr.ptr, align 4
  %red.next.ptr = getelementptr inbounds i32, i32* %red.curr.ptr, i32 1
  %red.init.done = icmp eq i32* %red.next.ptr, %arr.end
  br i1 %red.init.done, label %begin.simd.1, label %red.init.body

begin.simd.1:
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %sum, i32 0, i32 42) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %begin.simd ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %A.i = load i32, i32* %arrayidx, align 4
  %sum.gep = getelementptr inbounds i32, i32* %sum, i64 %indvars.iv
  %sum.ld = load i32, i32* %sum.gep, align 4
  %add = add nsw i32 %A.i, %sum.ld
  store i32 %add, i32* %sum.gep, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp slt i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.body, label %for.cond.cleanup.loopexit

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  %fin.gep = getelementptr inbounds i32, i32* %sum, i32 42
  %fin = load i32, i32* %fin.gep, align 4
  ret i32 %fin

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

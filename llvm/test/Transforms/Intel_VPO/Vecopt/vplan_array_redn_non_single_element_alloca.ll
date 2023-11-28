; Verify that VPlan vectorizer bails out from loops containing array reductions
; represented via non-single element allocas.

; REQUIRES: asserts
; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -debug-only=VPlanDriver -debug-only=VPlanLegality -print-after=vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -debug-only=VPlanLegality -debug-only=VPlanDriver -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIRVEC
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI-HIR

; Checks for LLVM-IR vectorizer
; CHECK: VPlan LLVM-IR Driver for Function: test
; CHECK: Array alloca detected.
; CHECK: VD: Not vectorizing: Cannot prove legality.

; CHECK: define i32 @test
; CHECK: %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum, i32 0, i32 42) ]
;
; Checks for HIR vectorizer
; HIRVEC: VPlan HIR Driver for Function: test
; HIRVEC: Array alloca detected.
; HIRVEC: VD: Not vectorizing: Cannot prove legality.

; HIRVEC: Function: test
; HIRVEC: %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD:TYPED(&((%sum)[0]), 0, 42) ]

; OPTRPTMED: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized: Array alloca detected.
; OPTRPTHI-HIR: remark #15436: loop was not vectorized: HIR: Array alloca detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @test(ptr nocapture readonly %A, i64 %N, i32 %init) {
entry:
  %sum = alloca i32, i32 42, align 4
  br label %fill.sum

fill.sum:
  %arr.end = getelementptr i32, ptr %sum, i32 42
  %red.init.isempty = icmp eq ptr %sum, %arr.end
  br i1 %red.init.isempty, label %begin.simd.1, label %red.init.body

red.init.body:
  %red.curr.ptr = phi ptr [ %sum, %fill.sum ], [ %red.next.ptr, %red.init.body ]
  store i32 %init, ptr %red.curr.ptr, align 4
  %red.next.ptr = getelementptr inbounds i32, ptr %red.curr.ptr, i32 1
  %red.init.done = icmp eq ptr %red.next.ptr, %arr.end
  br i1 %red.init.done, label %begin.simd.1, label %red.init.body

begin.simd.1:
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum, i32 0, i32 42) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %begin.simd ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %A.i = load i32, ptr %arrayidx, align 4
  %sum.gep = getelementptr inbounds i32, ptr %sum, i64 %indvars.iv
  %sum.ld = load i32, ptr %sum.gep, align 4
  %add = add nsw i32 %A.i, %sum.ld
  store i32 %add, ptr %sum.gep, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp slt i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.body, label %for.cond.cleanup.loopexit

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  %fin.gep = getelementptr inbounds i32, ptr %sum, i32 42
  %fin = load i32, ptr %fin.gep, align 4
  ret i32 %fin

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; Verify that VPlan captures and preserves FastMathFlags for reductions
; specifically during the finalization process. VPlan IR and outgoing
; LLVM-IR/HIR are checked.

; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-print-after-vpentity-instrs -vplan-dump-details -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-print-after-vpentity-instrs -vplan-dump-details -print-after=hir-vplan-vec -hir-details-llvm-inst -disable-output < %s 2>&1 | FileCheck %s

; Checks for VPReductionFinal in VPlan IR
; CHECK:        float [[RED_FINAL:%vp.*]] = reduction-final{fadd} float [[VEC:%vp.*]]  float [[START:%.*]]
; CHECK-NEXT:    DbgLoc: 
; CHECK-NEXT:    OperatorFlags -
; CHECK-NEXT:      FMF: 1, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:    end of details

; Checks for finalization intrinsic in generated code
; CHECK: [[VEC_REDUCE:%.*]] = call fast float @llvm.vector.reduce.fadd.v8f32(float {{.*}}, <8 x float> {{.*}})


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @expl_reduction_add(float* nocapture %a) {
entry:
  %x = alloca float, align 4
  store float 0.000000e+00, float* %x, align 4
  br label %entry.split

entry.split:                                      ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.REDUCTION.ADD:TYPED"(float* %x, float zeroinitializer, i32 1) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %entry.split
  %x.promoted = load float, float* %x, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %DIR.QUAL.LIST.END.2
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.body ]
  %add7 = phi float [ %x.promoted, %DIR.QUAL.LIST.END.2 ], [ %add, %for.body ]
  %add = fadd fast float %add7, 5.000000e-01
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  br label %for.end1

for.end1:                                         ; preds = %for.end
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %for.end
  store float %add.lcssa, float* %x, align 4
  %x1 = load float, float* %x, align 4
  ret float %x1
}

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token) #1

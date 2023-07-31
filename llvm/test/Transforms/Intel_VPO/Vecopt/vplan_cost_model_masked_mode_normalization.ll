; This test verifies that the plan computes cost zero for masked mode loop
; normalization instructions, i.e. instructions of the form:
;
;     <add/sub> ..., <orig-lower-bound>
;
; When computing the cost of these instructions in a peel loop. This is due to
; the fact that, in the peel loop, <orig-lower-bound> will be replaced with '0',
; and hence the instruction will be folded away (zero-cost).

; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-vplan-vec' -disable-output \
; RUN:     -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 \
; RUN:     -vplan-enable-evaluators-cost-model-dumps \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; HIR before VPlan:
; <20>         + DO i1 = 0, 249, 1   <DO_LOOP> <simd>
; <10>         |   (@A)[0][i1 + 1] = i1 + 1;
; <20>         + END LOOP
; <20>

@A = external dso_local local_unnamed_addr global [256 x i64], align 16

define void @foo() local_unnamed_addr {
; Peel loop
; CHECK-LABEL:  Cost Model for VPlan foo:HIR.#{{[0-9]+}} with VF = 4:
; CHECK:         Cost 0 for i64 [[VP_NORM_UB:%vp.*]] = sub i64 250 i64 live-in0
; CHECK:         Cost 0 for i64 [[VP_NEW_IND:%vp.*]] = add i64 [[OLD_IV:%vp.*]] i64 live-in0
; Main loop
; CHECK:       Cost Model for VPlan foo:HIR.#{{[0-9]+}}.cloned.masked with VF = 4:
; CHECK:         Cost 1 for i64 [[VP_NORM_UB]] = sub i64 250 i64 live-in0
; CHECK:         Cost 1 for i64 [[VP_NEW_IND]] = add i64 [[OLD_IV]] i64 live-in0
; Remainder loop
; CHECK:       Cost Model for VPlan foo:HIR.#{{[0-9]+}}.cloned.masked with VF = 4:
; CHECK:         Cost 1 for i64 [[VP_NORM_UB]] = sub i64 250 i64 live-in0
; CHECK:         Cost 1 for i64 [[VP_NEW_IND]] = add i64 [[OLD_IV]] i64 live-in0
;
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.preheader ], [ %iv.next, %for.body ]

  %offset = add i64 %iv, 1
  %A.elem = getelementptr [256 x i64], ptr @A, i64 0, i64 %offset
  store i64 %offset, ptr %A.elem, align 8

  %iv.next = add nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 250
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() nounwind

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) nounwind

attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

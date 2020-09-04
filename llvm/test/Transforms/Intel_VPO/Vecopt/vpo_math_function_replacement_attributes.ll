; Test to check function attributes attached to the OCL builtin calls emitted by
; MathLibraryFunctionsReplacement. Subsequent effects of these attributes in VPlan
; vectorizer are also checked.

; RUN: opt -S -replace-with-math-library-functions -vector-library=SVML %s | FileCheck %s --check-prefix=MFREPLACE

; MFREPLACE-LABEL: @foo
; MFREPLACE:        %_Z4uremjj = call i32 @_Z4uremjj(i32 %dividend, i32 %divisor)
; MFREPLACE-NEXT:   %call = tail call i32 @_Z19sub_group_broadcastjj(i32 %iv, i32 %_Z4uremjj) #2

; FIXME: Function should be declared with nounwind/readnone attributes to indicate
; no side-effects.
; MFREPLACE: declare i32 @_Z4uremjj(i32, i32)

; RUN: opt -replace-with-math-library-functions -VPlanDriver -vplan-print-after-call-vec-decisions -vector-library=SVML -disable-output %s | FileCheck %s --check-prefix=VPLAN

; VPLAN-LABEL: VPlan after CallVecDecisions analysis for VF=8
; FIXME: Uniform call was marked Div by DA because call could have side-effects (missing attributes)
; VPLAN:        [DA: Div] i32 [[UREM_CALL:%vp.*]] = call i32 %dividend i32 %divisor _Z4uremDv8_jS_ [x 1]
; FIXME: Subgroup bcast call vector-variant matching fails because of divergent 2nd operand
; VPLAN-NEXT:   [DA: Uni] i32 [[SG_BCAST:%vp.*]] = call i32 [[IV:%vp.*]] i32 [[UREM_CALL]] i32 (i32, i32)* @_Z19sub_group_broadcastjj [Serial]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @foo(i32 %dividend, i32 %divisor) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %simd.loop ]
  %remainder = urem i32 %dividend, %divisor
  %call = tail call i32 @_Z19sub_group_broadcastjj(i32 %iv, i32 %remainder) #0
  %iv.next = add i32 %iv, 1
  %iv.cmp = icmp ult i32 %iv.next, 1024
  br i1 %iv.cmp, label %simd.loop, label %exit

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
declare i32 @_Z19sub_group_broadcastjj(i32, i32) local_unnamed_addr #1

; FIXME: Add "kernel-call-once" attribute here after attributes of uremjj are fixed.
attributes #0 = { nounwind "has-vplan-mask" "vector-variants"="_ZGVbM8vu_(_Z19sub_group_broadcastDv8_jjS_)" }
attributes #1 = { nounwind "opencl-vec-uniform-return" }

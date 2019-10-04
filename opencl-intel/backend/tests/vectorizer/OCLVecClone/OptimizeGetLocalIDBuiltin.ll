; Test to check that function calls to get_local_id() are moved and uses replaced in an optimized
; manner if it is known that max work group size is less than 2GB.

; Check for default case i.e. max work group size < 2GB. Note that %trunc.user is removed from function,
; its uses replaced by %add. Similarly %shl.user and %ashr.inst are removed, with all uses of %ashr.inst
; replaced by %add.sext.
; RUN: %oclopt --ocl-vecclone --ocl-vec-clone-isa-encoding-override=AVX512Core < %s -S -o - | FileCheck %s --check-prefix=LT2GB
; LT2GB-LABEL: @_ZGVeN8uu_foo

; LT2GB-LABEL: entry:
; LT2GB: [[LID_CALL:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; LT2GB-NEXT: [[LID_CALL_TRUNC:%.*]] = trunc i64 [[LID_CALL]] to i32

; LT2GB-LABEL: simd.loop:
; LT2GB-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
; LT2GB-NEXT: %add = add nuw i32 [[LID_CALL_TRUNC]], %index
; LT2GB-NEXT: [[ADD_SEXT:%.*]] = sext i32 %add to i64
; LT2GB-NEXT: %non.trunc.user = add i64 [[ADD_SEXT]], 42
; LT2GB-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; LT2GB-NEXT: %ret0 = mul i32 %other.trunc, %add
; LT2GB-NEXT: %ret1 = mul i64 %non.trunc.user, [[ADD_SEXT]]
; LT2GB-NEXT: br label %simd.loop.exit


; Check for non-default case i.e. max work group size > 2GB.
; RUN: %oclopt --ocl-vecclone --ocl-vec-clone-isa-encoding-override=AVX512Core --less-than-two-gig-max-work-group-size=false < %s -S -o - | FileCheck %s --check-prefix=GT2GB
; GT2GB-LABEL: @_ZGVeN8uu_foo

; GT2GB-LABEL: entry:
; GT2GB: [[LID_CALL:%.*]] = call i64 @_Z12get_local_idj(i32 0)

; GT2GB-LABEL: simd.loop:
; GT2GB-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
; GT2GB-NEXT: [[IDX_SEXT:%.*]] = sext i32 %index to i64
; GT2GB-NEXT: %add = add nuw i64 [[IDX_SEXT]], %lid_call
; GT2GB-NEXT: %non.trunc.user = add i64 %add, 42
; GT2GB-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; GT2GB-NEXT: %trunc.user = trunc i64 %add to i32
; GT2GB-NEXT: %shl.user = shl i64 %add, 32
; GT2GB-NEXT: %ashr.inst = ashr exact i64 %shl.user, 32
; GT2GB-NEXT: %ret0 = mul i32 %other.trunc, %trunc.user
; GT2GB-NEXT: %ret1 = mul i64 %non.trunc.user, %ashr.inst
; GT2GB-NEXT: br label %simd.loop.exit



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: nounwind
define void @foo(i32 addrspace(1)* %a, float addrspace(1)* %b) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_type_qual !9 !kernel_arg_base_type !10 !no_barrier_path !11 !ocl_recommended_vector_length !12 {
entry:
   %lid_call = call i64 @_Z12get_local_idj(i32 0) #1
   %non.trunc.user = add i64 %lid_call, 42
   %other.trunc = trunc i64 %non.trunc.user to i32
   %trunc.user = trunc i64 %lid_call to i32
   %shl.user = shl i64 %lid_call, 32
   %ashr.inst = ashr exact i64 %shl.user, 32
   %ret0 = mul i32 %other.trunc, %trunc.user
   %ret1 = mul i64 %non.trunc.user, %ashr.inst
   ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="true" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!spirv.Generator = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{}
!4 = !{i16 6, i16 14}
!5 = !{void (i32 addrspace(1)*, float addrspace(1)*)* @foo}
!6 = !{i32 1, i32 1}
!7 = !{!"none", !"none"}
!8 = !{!"int*", !"float*"}
!9 = !{!"", !""}
!10 = !{!"int*", !"float*"}
!11 = !{i1 true}
!12 = !{i32 8}

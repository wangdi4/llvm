; Test to check that function calls to get_global_id() are moved and uses replaced in an optimized
; manner if assumption hints about range of the ID value is provided.

; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

; Check that %gid.trunc is removed from function and its uses replaced by %add1.
; CHECK-LABEL: @_ZGVeN8uuu_foo

; CHECK-LABEL: entry:
; CHECK:      [[GID_CALL:%.*]] = tail call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: [[GID_CALL_TRUNC:%.*]] = trunc i64 [[GID_CALL]] to i32

; CHECK-LABEL: simd.loop.header:
; CHECK-NEXT:  [[IDX:%.*]] = phi i32 [ 0, %simd.loop.preheader ], [ [[INDVAR:%.*]], %simd.loop.latch ]
; CHECK-NEXT:  [[ADD_IDX_GID:%.*]] = add nuw i32 [[GID_CALL_TRUNC]], [[IDX]]
; CHECK-NEXT:  [[ADD_IDX_GID_SEXT:%.*]] = sext i32 [[ADD_IDX_GID]] to i64
; CHECK-NEXT:  [[ASSUME_CMP:%.*]] = icmp ult i64 [[ADD_IDX_GID_SEXT]], 2147483648
; CHECK-NEXT:  tail call void @llvm.assume(i1 [[ASSUME_CMP]])
; CHECK-NEXT:  [[INT_ADD:%.*]] = add i32 [[ADD_IDX_GID]], [[LOAD_INTVAL:%.*]]
; CHECK-NEXT:  [[INT_ADD_SEXT:%.*]] = sext i32 [[INT_ADD]] to i64
; CHECK-NEXT:  [[SRC_PTR:%.*]] = getelementptr inbounds float, float addrspace(1)* [[LOAD_A:%.*]], i64 [[INT_ADD_SEXT]]
; CHECK-NEXT:  [[LOAD:%.*]] = load float, float addrspace(1)* [[SRC_PTR]], align 4
; CHECK-NEXT:  [[DST_PTR:%.*]] = getelementptr inbounds float, float addrspace(1)* [[LOAD_B:%.*]], i64 [[INT_ADD_SEXT]]
; CHECK-NEXT:  store float [[LOAD]], float addrspace(1)* [[DST_PTR]], align 4


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: nounwind
define void @foo(float addrspace(1)* %a, float addrspace(1)* %b, i32 %intval) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_type_qual !9 !kernel_arg_base_type !10 !no_barrier_path !11 !recommended_vector_length !12 {
entry:
  %gid = tail call i64 @_Z13get_global_idj(i32 0) #1
  %assume.cmp = icmp ult i64 %gid, 2147483648
  tail call void @llvm.assume(i1 %assume.cmp)
  %gid.trunc = trunc i64 %gid to i32
  %add = add i32 %gid.trunc, %intval
  %add.sext = sext i32 %add to i64
  %src.ptr = getelementptr inbounds float, float addrspace(1)* %a, i64 %add.sext
  %load = load float, float addrspace(1)* %src.ptr, align 4
  %dst.ptr = getelementptr inbounds float, float addrspace(1)* %b, i64 %add.sext
  store float %load, float addrspace(1)* %dst.ptr, align 4
  ret void
}

; Function Attrs: nounwind willreturn
declare void @llvm.assume(i1)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="true" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!spirv.Generator = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{}
!4 = !{i16 6, i16 14}
!5 = !{void (float addrspace(1)*, float addrspace(1)*, i32)* @foo}
!6 = !{i32 1, i32 1}
!7 = !{!"none", !"none"}
!8 = !{!"int*", !"float*"}
!9 = !{!"", !""}
!10 = !{!"int*", !"float*"}
!11 = !{i1 true}
!12 = !{i32 8}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uuu_foo {{.*}} br
; DEBUGIFY-NOT: WARNING

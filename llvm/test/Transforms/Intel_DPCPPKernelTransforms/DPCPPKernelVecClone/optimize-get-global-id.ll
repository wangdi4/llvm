; Test to check that function calls to get_global_id() are moved and uses replaced in an optimized
; manner if it is known that max number of work items is less than 2Gig elements.

; Check for non-default case i.e. max number of work items < 2Gig. Note that %trunc.user is removed from function,
; its uses replaced by %add. Similarly %shl.user and %ashr.inst are removed, with all uses of %ashr.inst
; replaced by %add.sext.
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core --dpcpp-less-than-two-gig-max-global-work-size=true %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core --dpcpp-less-than-two-gig-max-global-work-size=true %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core --dpcpp-less-than-two-gig-max-global-work-size=true %s -S -o - | FileCheck %s -check-prefix=LT2GIG
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core --dpcpp-less-than-two-gig-max-global-work-size=true %s -S -o - | FileCheck %s -check-prefix=LT2GIG
; LT2GIG-LABEL: @_ZGVeN8uu_foo

; LT2GIG-LABEL: entry:
; LT2GIG: [[GID_CALL:%.*]] = call i64 @_Z13get_global_idj(i32 0)
; LT2GIG-NEXT: [[GID_CALL_TRUNC:%.*]] = trunc i64 [[GID_CALL]] to i32

; LT2GIG-LABEL: simd.loop.header:
; LT2GIG-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
; LT2GIG-NEXT: %add = add nuw i32 [[GID_CALL_TRUNC]], %index
; LT2GIG-NEXT: [[ADD_SEXT:%.*]] = sext i32 %add to i64
; LT2GIG-NEXT: %non.trunc.user = add i64 [[ADD_SEXT]], 42
; LT2GIG-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; LT2GIG-NEXT: %ret0 = mul i32 %other.trunc, %add
; LT2GIG-NEXT: %ret1 = mul i64 %non.trunc.user, [[ADD_SEXT]]
; LT2GIG-NEXT: %mul.shl = mul i64 2, [[ADD_SEXT]]
; LT2GIG-NEXT: %add.shl = add i64 %mul.shl, 1
; LT2GIG-NEXT: %sub.shl = add i64 %mul.shl, -1
; LT2GIG-NEXT: %ret3 = mul i64 %non.trunc.user, %add.shl
; LT2GIG-NEXT: %ret4 = mul i64 %non.trunc.user, %sub.shl
; LT2GIG-NEXT: %shl.keep = shl i64 [[ADD_SEXT]], 32
; LT2GIG-NEXT: %add2.shl = add i64 %shl.keep, 4294967296
; LT2GIG-NEXT: %call = call i64 @dummy(i64 %add2.shl)
; LT2GIG-NEXT: %call.ashr = ashr exact i64 %call, 32
; LT2GIG-NEXT: %ret5 = mul i64 %non.trunc.user, %call.ashr
; LT2GIG-NEXT: %add2 = ashr exact i64 %add2.shl, 32
; LT2GIG-NEXT: %ret6 = mul i64 %non.trunc.user, %add2
; LT2GIG-NEXT: br label %simd.loop.latch


; Check for default case i.e. max number of work items is assumed to be > 2Gig.
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core < %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefixes=DEBUGIFY,DEBUGIFY2 %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core < %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefixes=DEBUGIFY,DEBUGIFY2 %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core < %s -S -o - | FileCheck %s -check-prefix=GT2GIG
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core < %s -S -o - | FileCheck %s -check-prefix=GT2GIG
; GT2GIG-LABEL: @_ZGVeN8uu_foo

; GT2GIG-LABEL: entry:
; GT2GIG: [[GID_CALL:%.*]] = call i64 @_Z13get_global_idj(i32 0)

; GT2GIG-LABEL: simd.loop.header:
; GT2GIG-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
; GT2GIG-NEXT: [[IDX_SEXT:%.*]] = sext i32 %index to i64
; GT2GIG-NEXT: %add = add nuw i64 [[IDX_SEXT]], [[GID_CALL]]
; GT2GIG-NEXT: %non.trunc.user = add i64 %add, 42
; GT2GIG-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; GT2GIG-NEXT: %trunc.user = trunc i64 %add to i32
; GT2GIG-NEXT: %shl.user = shl i64 %add, 32
; GT2GIG-NEXT: %ashr.inst = ashr exact i64 %shl.user, 32
; GT2GIG-NEXT: %ret0 = mul i32 %other.trunc, %trunc.user
; GT2GIG-NEXT: %ret1 = mul i64 %non.trunc.user, %ashr.inst
; GT2GIG-NEXT: %mul.shl = mul i64 2, %shl.user
; GT2GIG-NEXT: %add.shl = add i64 %mul.shl, 4294967296
; GT2GIG-NEXT: %sub.shl = add i64 %mul.shl, -4294967296
; GT2GIG-NEXT: %gid.add = ashr exact i64 %add.shl, 32
; GT2GIG-NEXT: %gid.sub = ashr exact i64 %sub.shl, 32
; GT2GIG-NEXT: %ret3 = mul i64 %non.trunc.user, %gid.add
; GT2GIG-NEXT: %ret4 = mul i64 %non.trunc.user, %gid.sub
; GT2GIG-NEXT: %shl.keep = shl i64 %add, 32
; GT2GIG-NEXT: %add2.shl = add i64 %shl.keep, 4294967296
; GT2GIG-NEXT: %call = call i64 @dummy(i64 %add2.shl)
; GT2GIG-NEXT: %call.ashr = ashr exact i64 %call, 32
; GT2GIG-NEXT: %ret5 = mul i64 %non.trunc.user, %call.ashr
; GT2GIG-NEXT: %add2 = ashr exact i64 %add2.shl, 32
; GT2GIG-NEXT: %ret6 = mul i64 %non.trunc.user, %add2
; GT2GIG-NEXT: br label %simd.loop.latch



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: nounwind
define void @foo(i32 addrspace(1)* %a, float addrspace(1)* %b) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_type_qual !9 !kernel_arg_base_type !10 !no_barrier_path !11 !recommended_vector_length !12 {
entry:
   %gid_call = call i64 @_Z13get_global_idj(i32 0) #1
   %non.trunc.user = add i64 %gid_call, 42
   %other.trunc = trunc i64 %non.trunc.user to i32
   %trunc.user = trunc i64 %gid_call to i32
   %shl.user = shl i64 %gid_call, 32
   %ashr.inst = ashr exact i64 %shl.user, 32
   %ret0 = mul i32 %other.trunc, %trunc.user
   %ret1 = mul i64 %non.trunc.user, %ashr.inst

   %mul.shl = mul i64 2, %shl.user ; %mul = 2 * gid
   %add.shl = add i64 %mul.shl, 4294967296  ; %gid.add = %mul + 1
   %sub.shl = add i64 %mul.shl, -4294967296 ; %gid.sub = %mul - 1
   %gid.add = ashr exact i64 %add.shl, 32
   %gid.sub = ashr exact i64 %sub.shl, 32
   %ret3 = mul i64 %non.trunc.user, %gid.add
   %ret4 = mul i64 %non.trunc.user, %gid.sub

   %shl.keep = shl i64 %gid_call, 32
   %add2.shl = add i64 %shl.keep, 4294967296
   %call = call i64 @dummy(i64 %add2.shl) ; %call blocks handling this path
   %call.ashr = ashr exact i64 %call, 32
   %ret5 = mul i64 %non.trunc.user, %call.ashr
   %add2 = ashr exact i64 %add2.shl, 32
   %ret6 = mul i64 %non.trunc.user, %add2
   ret void
}

declare i64 @dummy(i64)

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
!5 = !{void (i32 addrspace(1)*, float addrspace(1)*)* @foo}
!6 = !{i32 1, i32 1}
!7 = !{!"none", !"none"}
!8 = !{!"int*", !"float*"}
!9 = !{!"", !""}
!10 = !{!"int*", !"float*"}
!11 = !{i1 true}
!12 = !{i32 8}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} add
; DEBUGIFY2-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_foo {{.*}} br
; DEBUGIFY-NOT: WARNING

; RUN: %oclopt --ocl-vec-clone-isa-encoding-override=AVX512Core -ChooseVectorizationDimensionModulePass -ocl-vecclone -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt --ocl-vec-clone-isa-encoding-override=AVX512Core -ChooseVectorizationDimensionModulePass -ocl-vecclone -S %s -o %t
; The order of generated functions may vary, so we run FileCheck multiple times.
; RUN: FileCheck -check-prefix CHECK0 %s -input-file=%t
; RUN: FileCheck -check-prefix CHECK1 %s -input-file=%t
; RUN: FileCheck -check-prefix CHECK2 %s -input-file=%t
; RUN: FileCheck -check-prefix CHECK3 %s -input-file=%t
; RUN: FileCheck -check-prefix CHECK4 %s -input-file=%t
; RUN: FileCheck -check-prefix CHECK5 %s -input-file=%t

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

define void @test_0(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !ocl_recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %mul
  %load = load i32, i32 addrspace(1)* %load.addr  ; good for dim 1 (uniform), bad for dim 0 (strided)
  %store.addr = getelementptr i32, i32 addrspace(1)* %out, i64 %gid.0
  store i32 %load, i32 addrspace(1)* %store.addr  ; good for dim 1 (uniform), good for dim 0 (consecutive)
  ret void
}
; CHECK0-LABEL: define void @_ZGVeN16uu_test_0
; CHECK0-SAME:  !vectorization_dimension [[VEC_DIM_1:![0-9]+]]
; CHECK0: [[VEC_DIM_1]] = !{i32 1}


define void @test_1(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !ocl_recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %gid.0
  %load = load i32, i32 addrspace(1)* %load.addr  ; good for both dim 0 (consecutive) and 1 (uniform)
  %store.addr = getelementptr i32, i32 addrspace(1)* %out, i64 %gid.1
  store i32 %load, i32 addrspace(1)* %store.addr  ; good for both dim 0 (uniform) and 1 (consecutive)
  ret void
}
; CHECK1-LABEL: define void @_ZGVeN16uu_test_1
; CHECK1-SAME:  !vectorization_dimension [[VEC_DIM_0:![0-9]+]]
; CHECK1: [[VEC_DIM_0]] = !{i32 0}


; Same as test_0, but with no_barrier_path being false, choose dim 0.
define void @test_2(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %in) !no_barrier_path !2 !kernel_has_sub_groups !2 !ocl_recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %mul
  %load = load i32, i32 addrspace(1)* %load.addr
  %store.addr = getelementptr i32, i32 addrspace(1)* %out, i64 %gid.0
  store i32 %load, i32 addrspace(1)* %store.addr
  ret void
}
; CHECK2-LABEL: define void @_ZGVeN16uu_test_2
; CHECK2-SAME:  !vectorization_dimension [[VEC_DIM_0:![0-9]+]]
; CHECK2: [[VEC_DIM_0]] = !{i32 0}


; Same as test_0, but with kernel_has_sub_groups being true, choose dim 0.
define void @test_3(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !1 !ocl_recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %mul
  %load = load i32, i32 addrspace(1)* %load.addr
  %store.addr = getelementptr i32, i32 addrspace(1)* %out, i64 %gid.0
  store i32 %load, i32 addrspace(1)* %store.addr
  ret void
}
; CHECK3-LABEL: define void @_ZGVeN16uu_test_3
; CHECK3-SAME:  !vectorization_dimension [[VEC_DIM_0:![0-9]+]]
; CHECK3: [[VEC_DIM_0]] = !{i32 0}


define void @test_4(<4 x i32> addrspace(1)* nocapture %out, <4 x i32> addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !ocl_recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %load.addr = getelementptr <4 x i32>, <4 x i32> addrspace(1)* %in, i64 %gid.0
  %load = load <4 x i32>, <4 x i32> addrspace(1)* %load.addr  ; good for dim 1 (uniform), bad for dim 0 (consecutive on vector type)
  %store.addr = getelementptr <4 x i32>, <4 x i32> addrspace(1)* %out, i64 %gid.0
  store <4 x i32> %load, <4 x i32> addrspace(1)* %store.addr  ; good for dim 1 (uniform), bad for dim 0 (consecutive on vector type)
  ret void
}
; CHECK4-LABEL: define void @_ZGVeN16uu_test_4
; CHECK4-SAME:  !vectorization_dimension [[VEC_DIM_1:![0-9]+]]
; CHECK4: [[VEC_DIM_1]] = !{i32 1}


define void @test_5(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !ocl_recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %gid.0
  %load = load i32, i32 addrspace(1)* %load.addr  ; good for both dim 0 (consecutive) and 1, 2 (uniform)
  %cond = icmp ne i64 %gid.0, %gid.1
  br i1 %cond, label %t, label %return

t: ; non-divergent block for dim 0 and dim 1, thus choose dim 2
  %store.addr = getelementptr i32, i32 addrspace(1)* %out, i64 %gid.1
  store i32 %load, i32 addrspace(1)* %store.addr  ; good for both dim 0, 2 (uniform) and 1 (consecutive)
  br label %return

return:
  ret void
}
; CHECK5-LABEL: define void @_ZGVeN16uu_test_5
; CHECK5-SAME:  !vectorization_dimension [[VEC_DIM_2:![0-9]+]]
; CHECK5: [[VEC_DIM_2]] = !{i32 2}


attributes #0 = { nounwind readnone }
attributes #0 = { nounwind }

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_0, void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_1, void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_2, void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_3, void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*)* @test_4, void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_5}
!1 = !{i1 true}
!2 = !{i1 false}
!3 = !{i32 16}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_0 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_1 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_2 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uu_test_3 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_4 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_5 {{.*}} br
; DEBUGIFY-NOT: WARNING

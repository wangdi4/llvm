; RUN: opt -dpcpp-kernel-vec-dim-analysis -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='require<dpcpp-kernel-vec-dim-analysis>,dpcpp-kernel-vec-clone' -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; RUN: opt -dpcpp-kernel-vec-dim-analysis -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s --check-prefixes=CHECK,WithVecDimAnalysis
; RUN: opt -passes='require<dpcpp-kernel-vec-dim-analysis>,dpcpp-kernel-vec-clone' -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s --check-prefixes=CHECK,WithVecDimAnalysis

; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s --check-prefixes=CHECK,WithoutVecDimAnalysis
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s --check-prefixes=CHECK,WithoutVecDimAnalysis

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

define void @test_dim0(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %gid.0
  %load = load i32, i32 addrspace(1)* %load.addr, align 4 ; good for both dim 0 (consecutive) and 1 (uniform)
  %store.addr = getelementptr i32, i32 addrspace(1)* %out, i64 %gid.1
  store i32 %load, i32 addrspace(1)* %store.addr, align 4 ; good for both dim 0 (uniform) and 1 (consecutive)
  ret void
}

; CHECK: define void @_ZGVeN16uu_test_dim0
; CHECK-SAME: !vectorization_dimension [[VD0:![0-9]+]]
; CHECK-SAME: !can_unite_workgroups [[CUW:![0-9]+]]

define void @test_dim1(i32 addrspace(1)* nocapture %out0, i32 addrspace(1)* nocapture %out1, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %mul
  %load = load i32, i32 addrspace(1)* %load.addr, align 4 ; good for dim 1 (uniform), bad for dim 0 (strided)
  %store.addr = getelementptr i32, i32 addrspace(1)* %out0, i64 %gid.0
  store i32 %load, i32 addrspace(1)* %store.addr, align 4 ; good for dim 1 (uniform), good for dim 0 (consecutive)
  %store.addr.1 = getelementptr i32, i32 addrspace(1)* %out1, i64 %gid.1
  store i32 0, i32 addrspace(1)* %store.addr.1, align 4 ; good for dim 1 (consecutive), good for dim 0 (uniform)
  ret void
}

; CHECK: define void @_ZGVeN16uuu_test_dim1
; WithVecDimAnalysis-SAME: !vectorization_dimension [[VD1:![0-9]+]]
; WithoutVecDimAnalysis-SAME: !vectorization_dimension [[VD0:![0-9]+]]
; CHECK-SAME: !can_unite_workgroups [[CUW:![0-9]+]]

define void @test_dim2(i32 addrspace(1)* nocapture %out1, i32 addrspace(1)* nocapture %out2, i32 addrspace(1)* nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 {
entry:
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %load.addr = getelementptr i32, i32 addrspace(1)* %in, i64 %gid.0
  %load = load i32, i32 addrspace(1)* %load.addr, align 4 ; good for both dim 0 (consecutive) and 1, 2 (uniform)
  %cond = icmp ne i64 %gid.0, %gid.1
  br i1 %cond, label %t, label %return

t:                                                ; preds = %entry ; divergent block for dim 0 and dim 1, thus choose dim 2
  %store.addr = getelementptr i32, i32 addrspace(1)* %out1, i64 %gid.1
  store i32 %load, i32 addrspace(1)* %store.addr, align 4 ; good for both dim 0, 2 (uniform) and 1 (consecutive)
  br label %return

return:                                           ; preds = %t, %entry
  %add = add i64 %gid.2, 2
  %store.addr.2 = getelementptr i32, i32 addrspace(1)* %out2, i64 %add
  store i32 0, i32 addrspace(1)* %store.addr.2, align 4 ; good for both dim0, 1 (uniform) and 2 (consecutive)
  ret void
}

; CHECK: define void @_ZGVeN16uuu_test_dim2
; WithVecDimAnalysis-SAME: !vectorization_dimension [[VD2:![0-9]+]]
; WithoutVecDimAnalysis-SAME: !vectorization_dimension [[VD0:![0-9]+]]
; CHECK-SAME: !can_unite_workgroups [[CUW:![0-9]+]]

attributes #0 = { nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_dim0, void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @test_dim1, void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @test_dim2}
!1 = !{i1 true}
!2 = !{i1 false}
!3 = !{i32 16}

; CHECK-DAG: [[VD0]] = !{i32 0}

; WithVecDimAnalysis-DAG: [[CUW]] = !{i1 true}
; WithVecDimAnalysis-DAG: [[VD1]] = !{i32 1}
; WithVecDimAnalysis-DAG: [[VD2]] = !{i32 2}

; WithoutVecDimAnalysis-DAG: [[CUW]] = !{i1 false}

; DEBUGIFY-COUNT-8: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test_dim0
; DEBUGIFY-COUNT-8: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuu_test_dim1
; DEBUGIFY-COUNT-8: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuu_test_dim2
; DEBUGIFY-NOT: WARNING

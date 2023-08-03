; RUN: opt -passes='sycl-kernel-analysis,print<sycl-kernel-vec-dim-analysis>' %s -disable-output 2>&1 | FileCheck %s

; The test checks that vec dim won't be set to 1 or 2 even if there is
; get_global_id(1 or 2) but it's not used.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.input_pair_t = type { i32, i32 }

define void @sample_test(ptr addrspace(1) %src, ptr addrspace(1) %dst) local_unnamed_addr #0 !recommended_vector_length !1 !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %0 = tail call i64 @_Z13get_global_idj(i32 0) #1
  %1 = tail call i64 @_Z13get_global_idj(i32 1) #1
  %2 = tail call i64 @_Z13get_global_idj(i32 2) #1
  %sext = shl i64 %0, 32
  %idxprom = ashr exact i64 %sext, 32
  %A = getelementptr inbounds %struct.input_pair_t, ptr addrspace(1) %src, i64 %idxprom, i32 0
  %3 = load i32, ptr addrspace(1) %A, align 4
  %B = getelementptr inbounds %struct.input_pair_t, ptr addrspace(1) %src, i64 %idxprom, i32 1
  %4 = load i32, ptr addrspace(1) %B, align 4
  %add = add i32 %4, %3
  %ptridx4 = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %idxprom
  store i32 %add, ptr addrspace(1) %ptridx4, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{ptr @sample_test}
!1 = !{i32 16}
!2 = !{!"%struct.input_pair_t*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr null}

; CHECK: VectorizationDimensionAnalysis for function sample_test
; CHECK:   VectorizeDim 0

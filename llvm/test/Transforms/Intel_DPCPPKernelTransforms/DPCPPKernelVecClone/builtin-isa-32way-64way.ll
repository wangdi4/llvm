; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=CHECK,COMMON %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=CHECK,COMMON %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX2 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=CHECK,COMMON %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX2 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=CHECK,COMMON %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX1 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=AVX1_SSE,COMMON %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX1 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=AVX1_SSE,COMMON %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=SSE42 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=AVX1_SSE,COMMON %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=SSE42 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck -check-prefixes=AVX1_SSE,COMMON %s

; This test checks that on avx1/sse42 only v32/v64 kernel-call-once builtin is used.

; COMMON-DAG: _Z14work_group_allDv32_i
; COMMON-DAG: _Z14work_group_allDv64_i

; CHECK-DAG: _ZGVbN32vvv__Z6selectffj
; CHECK-DAG: _ZGVbN64vvv__Z6selectffj

; AVX1_SSE-NOT: _ZGVbN32vvv__Z6selectffj
; AVX1_SSE-NOT: _ZGVbN64vvv__Z6selectffj

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test_v32_wg_builtin(i32 noundef %a, i32 addrspace(1)* noundef align 4 %b) !no_barrier_path !1 !kernel_has_sub_groups !1 !recommended_vector_length !2 {
entry:
  %call = tail call i32 @_Z14work_group_alli(i32 noundef %a) #2
  store i32 %call, i32 addrspace(1)* %b, align 4, !tbaa !3
  ret void
}

define void @test_v64_wg_builtin(i32 noundef %a, i32 addrspace(1)* noundef align 4 %b) !no_barrier_path !1 !kernel_has_sub_groups !1 !recommended_vector_length !7 {
entry:
  %call = tail call i32 @_Z14work_group_alli(i32 noundef %a) #1
  store i32 %call, i32 addrspace(1)* %b, align 4, !tbaa !8
  ret void
}

; Function Attrs: convergent
declare i32 @_Z14work_group_alli(i32 noundef) local_unnamed_addr #0

define void @test_v32(float addrspace(1)* noundef align 4 %a, float addrspace(1)* noundef align 4 %b) !no_barrier_path !10 !kernel_has_sub_groups !1 !recommended_vector_length !2 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %conv = trunc i64 %call to i32
  %idxprom = and i64 %call, 4294967295
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i64 %idxprom
  %0 = load float, float addrspace(1)* %arrayidx, align 4, !tbaa !11
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %b, i64 %idxprom
  %1 = load float, float addrspace(1)* %arrayidx2, align 4, !tbaa !11
  %call3 = tail call float @_Z6selectffj(float noundef %0, float noundef %1, i32 noundef %conv) #3
  store float %call3, float addrspace(1)* %arrayidx, align 4, !tbaa !11
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare float @_Z6selectffj(float noundef, float noundef, i32 noundef) local_unnamed_addr #1

define void @test_v64(float addrspace(1)* noundef align 4 %a, float addrspace(1)* noundef align 4 %b) !no_barrier_path !10 !kernel_has_sub_groups !1 !recommended_vector_length !7 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %conv = trunc i64 %call to i32
  %idxprom = and i64 %call, 4294967295
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i64 %idxprom
  %0 = load float, float addrspace(1)* %arrayidx, align 4, !tbaa !11
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %b, i64 %idxprom
  %1 = load float, float addrspace(1)* %arrayidx2, align 4, !tbaa !11
  %call3 = tail call float @_Z6selectffj(float noundef %0, float noundef %1, i32 noundef %conv) #3
  store float %call3, float addrspace(1)* %arrayidx, align 4, !tbaa !11
  ret void
}

attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn }
attributes #2 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #3 = { convergent nounwind readnone willreturn }

!sycl.kernels = !{!0}

!0 = !{void (i32, i32 addrspace(1)*)* @test_v32_wg_builtin, void (i32, i32 addrspace(1)*)* @test_v64_wg_builtin, void (float addrspace(1)*, float addrspace(1)*)* @test_v32, void (float addrspace(1)*, float addrspace(1)*)* @test_v64}
!1 = !{i1 false}
!2 = !{i32 32}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{i32 64}
!8 = !{!9, !9, i64 0, i64 0}
!9 = !{!"omnipotent char", !6}
!10 = !{i1 true}
!11 = !{!12, !12, i64 0}
!12 = !{!"float", !5, i64 0}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} call
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} icmp
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} call
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_test_v32 {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} call
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} icmp
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} call
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN64uu_test_v64 {{.*}} br
; DEBUGIFY-NOT: WARNING

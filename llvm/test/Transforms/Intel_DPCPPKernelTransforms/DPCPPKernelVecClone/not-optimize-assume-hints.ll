; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

; Check that if get_global_id call has range assumption but has no trunc/shl
; users, the call is not optimized and truncated.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @test(double addrspace(1)* noalias align 8 %A, double addrspace(1)* noalias align 8 %B) local_unnamed_addr #0 !recommended_vector_length !4 {
entry:
; CHECK-LABEL: @test
; CHECK-NOT: trunc
  %0 = tail call i64 @_Z13get_global_idj(i32 0) #3
  %cmp.i.i = icmp ult i64 %0, 2147483648
  tail call void @llvm.assume(i1 %cmp.i.i)
  %1 = getelementptr inbounds double, double addrspace(1)* %A, i64 %0
  %2 = load double, double addrspace(1)* %1, align 8, !noalias !5
  %3 = getelementptr inbounds double, double addrspace(1)* %B, i64 %0
  store double %2, double addrspace(1)* %3, align 8, !noalias !5
  ret void
}

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

; Function Attrs: mustprogress nofree nosync nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

attributes #0 = { nounwind }
attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { mustprogress nofree nosync nounwind readnone willreturn }
attributes #3 = { nounwind readnone willreturn }

!opencl.spir.version = !{!0}
!opencl.ocl.version = !{!1}
!opencl.used.optional.core.features = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 1, i32 2}
!1 = !{i32 1, i32 0}
!2 = !{!"cl_doubles"}
!3 = !{void (double addrspace(1)*, double addrspace(1)*)* @test}
!4 = !{i32 16}
!5 = !{!6}
!6 = distinct !{!6, !7}
!7 = distinct !{!7}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} call
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} icmp
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} br
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} call
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} br
; DEBUGIFY-NOT: WARNING

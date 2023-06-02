; RUN: opt -passes=sycl-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that shl/ashr of gid call is replaced with the call.

; IR is dumped from OpenCL kernel and then simplified:
;
; kernel void initialize_variables(global float *variables,
;                                  constant float *ff_variable, int nelr) {
;   const int i = get_global_id(0);
;   if (i >= nelr)
;     return;
;   for (int j = 0; j < 5; j++)
;     variables[i + j * nelr] = ff_variable[j];
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @initialize_variables(ptr addrspace(1) nocapture noundef writeonly align 4 %variables, ptr addrspace(2) nocapture noundef readonly align 4 %ff_variable, i32 noundef %nelr) local_unnamed_addr #0 !no_barrier_path !2 !vectorized_kernel !3 !vectorized_width !4 !recommended_vector_length !5 {
entry:
  ret void
}

declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

define dso_local void @_ZGVeN16uuu_initialize_variables(ptr addrspace(1) nocapture noundef writeonly align 4 %variables, ptr addrspace(2) nocapture noundef readonly align 4 %ff_variable, i32 noundef %nelr) local_unnamed_addr #2 !no_barrier_path !2 !vectorized_width !5 !recommended_vector_length !5 !scalar_kernel !1 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %sext = shl i64 %call, 32
  %.extract.0. = ashr exact i64 %sext, 32
  %0 = load float, ptr addrspace(2) %ff_variable, align 4
  %broadcast.splatinsert3 = insertelement <16 x float> poison, float %0, i64 0
  %broadcast.splat4 = shufflevector <16 x float> %broadcast.splatinsert3, <16 x float> poison, <16 x i32> zeroinitializer

; CHECK: %scalar.gep = getelementptr inbounds float, ptr addrspace(1) %variables, i64 %call

  %scalar.gep = getelementptr inbounds float, ptr addrspace(1) %variables, i64 %.extract.0.
  store <16 x float> %broadcast.splat4, ptr addrspace(1) %scalar.gep, align 4
  ret void
}

attributes #0 = { convergent mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { convergent mustprogress nofree nounwind willreturn memory(none) }
attributes #2 = { convergent mustprogress nofree norecurse nounwind willreturn memory(readwrite) }
attributes #3 = { convergent nounwind willreturn memory(none) }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 3, i32 0}
!1 = !{ptr @initialize_variables}
!2 = !{i1 true}
!3 = !{ptr @_ZGVeN16uuu_initialize_variables}
!4 = !{i32 1}
!5 = !{i32 16}

; DEBUGIFY-NOT: WARNING

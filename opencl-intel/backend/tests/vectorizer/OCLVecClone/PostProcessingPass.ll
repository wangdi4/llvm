; If the kernel is not vectorized, then the cloned kernel is removed.
; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -ocl-postvect -verify %s -S -o - \
; RUN: | FileCheck %s

; CHECK-NOT: define void @_ZGVdN8uuu_f

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @f(i32 addrspace(1)* %A, i32 addrspace(1)* %B, i32 addrspace(1)* %C) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !12 !no_barrier_path !13 !vectorized_width !14 !scalarized_kernel !15 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %A, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %B, i64 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4, !tbaa !16
  %add = add nsw i32 %1, %0
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %C, i64 %call
  store i32 %add, i32 addrspace(1)* %arrayidx2, align 4, !tbaa !16
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.f(i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*) !ocl_recommended_vector_length !20 {
entry:
  %3 = call i64 @_Z14get_local_sizej(i32 0)
  %4 = call i64 @get_base_global_id.(i32 0)
  %5 = call i64 @_Z14get_local_sizej(i32 1)
  %6 = call i64 @get_base_global_id.(i32 1)
  %7 = call i64 @_Z14get_local_sizej(i32 2)
  %8 = call i64 @get_base_global_id.(i32 2)
  %9 = insertvalue [7 x i64] undef, i64 %3, 2
  %10 = insertvalue [7 x i64] %9, i64 %4, 1
  %11 = insertvalue [7 x i64] %10, i64 %5, 4
  %12 = insertvalue [7 x i64] %11, i64 %6, 3
  %13 = insertvalue [7 x i64] %12, i64 %7, 6
  %14 = insertvalue [7 x i64] %13, i64 %8, 5
  %15 = insertvalue [7 x i64] %14, i64 1, 0
  ret [7 x i64] %15
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8uuu_f(i32 addrspace(1)* %A, i32 addrspace(1)* %B, i32 addrspace(1)* %C) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !15 !no_barrier_path !13 !ocl_recommended_vector_length !20 !vectorized_width !20 !vectorization_dimension !21 !scalarized_kernel !4 !can_unite_workgroups !22 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 addrspace(1)* %A, i32 addrspace(1)* %B, i32 addrspace(1)* %C) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add1 = add nuw i64 %0, %call
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %A, i64 %add1
  %1 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %B, i64 %add1
  %2 = load i32, i32 addrspace(1)* %arrayidx1, align 4, !tbaa !16
  %add = add nsw i32 %2, %1
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %C, i64 %add1
  store i32 %add, i32 addrspace(1)* %arrayidx2, align 4, !tbaa !16
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !23

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuu_f" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 1204ea834c6c1403298b9f5f46f16bed5d501177) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 3c8f0f343ff31be74d0fa48122d06a50d86b13e2)"}
!4 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @f}
!5 = !{i32 1, i32 1, i32 1}
!6 = !{!"none", !"none", !"none"}
!7 = !{!"int*", !"int*", !"int*"}
!8 = !{!"", !"", !""}
!9 = !{i1 false, i1 false, i1 false}
!10 = !{i32 0, i32 0, i32 0}
!11 = !{!"A", !"B", !"C"}
!12 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @_ZGVdN8uuu_f}
!13 = !{i1 true}
!14 = !{i32 1}
!15 = !{null}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{i32 8}
!21 = !{i32 0}
!22 = !{i1 false}
!23 = distinct !{!23, !24}
!24 = !{!"llvm.loop.unroll.disable"}



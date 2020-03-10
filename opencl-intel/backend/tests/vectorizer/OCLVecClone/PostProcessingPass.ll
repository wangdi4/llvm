; If the kernel is not vectorized, then the cloned kernel is removed.
; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -ocl-postvect -verify %s -S -o - \
; RUN: | FileCheck %s

; CHECK-NOT: define void @_ZGVcN4u_test
; CHECK-NOT: define void @_ZGVcM4u_test
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(i32 addrspace(1)* noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !12 !vectorized_masked_kernel !13 !no_barrier_path !14 !kernel_has_sub_groups !14 !ocl_recommended_vector_length !15 !vectorized_width !5 !scalarized_kernel !16 {
entry:
  %call = tail call i32 @_Z22get_max_sub_group_sizev() #3
  store i32 %call, i32 addrspace(1)* %a, align 4, !tbaa !17
  ret void
}

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #1

define [7 x i64] @WG.boundaries.test(i32 addrspace(1)* %0) !ocl_recommended_vector_length !15 {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = insertvalue [7 x i64] undef, i64 %1, 2
  %8 = insertvalue [7 x i64] %7, i64 %2, 1
  %9 = insertvalue [7 x i64] %8, i64 %3, 4
  %10 = insertvalue [7 x i64] %9, i64 %4, 3
  %11 = insertvalue [7 x i64] %10, i64 %5, 6
  %12 = insertvalue [7 x i64] %11, i64 %6, 5
  %13 = insertvalue [7 x i64] %12, i64 1, 0
  ret [7 x i64] %13
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVcN4u_test(i32 addrspace(1)* noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !16 !no_barrier_path !14 !kernel_has_sub_groups !14 !ocl_recommended_vector_length !15 !vectorized_width !15 !vectorization_dimension !10 !scalarized_kernel !4 !can_unite_workgroups !9 {
entry:
  %alloca.a = alloca i32 addrspace(1)*
  store i32 addrspace(1)* %a, i32 addrspace(1)** %alloca.a
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM"(i32 addrspace(1)** %alloca.a) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.a = load i32 addrspace(1)*, i32 addrspace(1)** %alloca.a
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %call = tail call i32 @_Z22get_max_sub_group_sizev() #3
  store i32 %call, i32 addrspace(1)* %load.a, align 4, !tbaa !17
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !21

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

; Function Attrs: convergent nounwind
define void @_ZGVcM4u_test(i32 addrspace(1)* noalias %a, <4 x i32> %mask) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !16 !no_barrier_path !14 !kernel_has_sub_groups !14 !ocl_recommended_vector_length !15 !vectorized_width !15 !vectorization_dimension !10 !scalarized_kernel !4 !can_unite_workgroups !9 {
entry:
  %alloca.a = alloca i32 addrspace(1)*
  store i32 addrspace(1)* %a, i32 addrspace(1)** %alloca.a
  %vec.mask = alloca <4 x i32>
  %mask.cast = bitcast <4 x i32>* %vec.mask to i32*
  store <4 x i32> %mask, <4 x i32>* %vec.mask
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM"(i32 addrspace(1)** %alloca.a) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.a = load i32 addrspace(1)*, i32 addrspace(1)** %alloca.a
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %mask.gep = getelementptr i32, i32* %mask.cast, i32 %index
  %mask.parm = load i32, i32* %mask.gep
  %mask.cond = icmp ne i32 %mask.parm, 0
  br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else

simd.loop.then:                                   ; preds = %simd.loop
  %call = tail call i32 @_Z22get_max_sub_group_sizev() #3
  store i32 %call, i32 addrspace(1)* %load.a, align 4, !tbaa !17
  br label %simd.loop.exit

simd.loop.else:                                   ; preds = %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop.else, %simd.loop.then
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !23

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVcN4u_test,_ZGVcM4u_test" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { convergent nounwind }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{void (i32 addrspace(1)*)* @test}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"uint*"}
!8 = !{!""}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{!"a"}
!12 = !{void (i32 addrspace(1)*)* @_ZGVcN4u_test}
!13 = !{void (i32 addrspace(1)*, <4 x i32>)* @_ZGVcM4u_test}
!14 = !{i1 true}
!15 = !{i32 4}
!16 = !{null}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.unroll.disable"}
!23 = distinct !{!23, !22}

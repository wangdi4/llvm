; RUN: %oclopt --ocl-vecclone --ocl-vec-clone-isa-encoding-override=AVX512Core < %s -S -o - | FileCheck %s

; CHECK-LABEL:@_ZGVeN8uuu_f
; CHECK-LABEL: entry:
; Checks that we do not hit a label from the WRN region.
; CHECK-NOT: {{^[._a-zA-Z0-9]*}}:
; Checks tha the work-item built-ins are moved at the entry block.
; CHECK: [[call_gid1:%call[0-9]+]] = tail call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: [[call_gid0:%call[0-9]+]] = tail call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: [[call_lid0:%call[0-9]+]] = tail call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT: [[lid0_trunc:%.*]] = trunc i64 [[call_lid0]] to i32
; CHECK-NEXT: [[call_lid1:%call[0-9]+]] = tail call i64 @_Z15get_global_sizej(i32 1)
; CHECK-NEXT: [[call_global_size:%call[0-9]+]] = tail call i64 @_Z15get_global_sizej(i32 0)
; CHECK-NEXT: [[call_group_id1:%call[0-9]+]] = tail call i64 @_Z12get_group_idj(i32 1)
; CHECK-NEXT: [[call_group_id0:%call[0-9]+]] = tail call i64 @_Z12get_group_idj(i32 0)
; CHECK-NEXT: [[call_local_size1:%call[0-9]+]] = tail call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT: [[call_local_size0:%call]] = tail call i64 @_Z14get_local_sizej(i32 0)

; CHECK-LABEL: simd.loop:

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @f([16384 x i32] addrspace(1)* %A, [16384 x i32] addrspace(1)* %B, [16384 x i32] addrspace(1)* %C) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !13 !no_barrier_path !14 !ocl_recommended_vector_length !15 {
entry:
  %call = tail call i64 @_Z14get_local_sizej(i32 0) #2
  %call1 = tail call i64 @_Z12get_group_idj(i32 0) #2
  %mul = mul i64 %call1, %call
  %call2 = tail call i64 @_Z15get_global_sizej(i32 0) #2
  %cmp = icmp eq i64 %mul, %call2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call3 = tail call i64 @_Z13get_global_idj(i32 0) #2
  %call4 = tail call i64 @_Z13get_global_idj(i32 1) #2
  %arrayidx5 = getelementptr inbounds [16384 x i32], [16384 x i32] addrspace(1)* %A, i64 %call3, i64 %call4
  %0 = load i32, i32 addrspace(1)* %arrayidx5, align 4, !tbaa !16
  %arrayidx7 = getelementptr inbounds [16384 x i32], [16384 x i32] addrspace(1)* %B, i64 %call3, i64 %call4
  %1 = load i32, i32 addrspace(1)* %arrayidx7, align 4, !tbaa !16
  %add = add nsw i32 %1, %0
  %arrayidx9 = getelementptr inbounds [16384 x i32], [16384 x i32] addrspace(1)* %C, i64 %call3, i64 %call4
  br label %if.end24.sink.split

if.else:                                          ; preds = %entry
  %call10 = tail call i64 @_Z14get_local_sizej(i32 1) #2
  %call11 = tail call i64 @_Z12get_group_idj(i32 1) #2
  %mul12 = mul i64 %call11, %call10
  %call13 = tail call i64 @_Z15get_global_sizej(i32 1) #2
  %cmp14 = icmp eq i64 %mul12, %call13
  br i1 %cmp14, label %if.then15, label %if.end24

if.then15:                                        ; preds = %if.else
  %call16 = tail call i64 @_Z12get_local_idj(i32 0) #2
  %arrayidx19 = getelementptr inbounds [16384 x i32], [16384 x i32] addrspace(1)* %A, i64 %call16, i64 %call16
  %2 = load i32, i32 addrspace(1)* %arrayidx19, align 4, !tbaa !16
  %arrayidx21 = getelementptr inbounds [16384 x i32], [16384 x i32] addrspace(1)* %B, i64 %call16, i64 %call16
  %3 = load i32, i32 addrspace(1)* %arrayidx21, align 4, !tbaa !16
  %sub = sub nsw i32 %2, %3
  %arrayidx23 = getelementptr inbounds [16384 x i32], [16384 x i32] addrspace(1)* %C, i64 %call16, i64 %call16
  br label %if.end24.sink.split

if.end24.sink.split:                              ; preds = %if.then, %if.then15
  %arrayidx23.sink = phi i32 addrspace(1)* [ %arrayidx23, %if.then15 ], [ %arrayidx9, %if.then ]
  %sub.sink = phi i32 [ %sub, %if.then15 ], [ %add, %if.then ]
  store i32 %sub.sink, i32 addrspace(1)* %arrayidx23.sink, align 4, !tbaa !16
  br label %if.end24

if.end24:                                         ; preds = %if.end24.sink.split, %if.else
  ret void
  }

; Function Attrs: convergent nounwind readnone
declare i64 @_Z14get_local_sizej(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_group_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z15get_global_sizej(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.f([16384 x i32] addrspace(1)*, [16384 x i32] addrspace(1)*, [16384 x i32] addrspace(1)*) !ocl_recommended_vector_length !15 {
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

declare i64 @get_base_global_id.(i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="true" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="true" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"-cl-denorms-are-zero"}
!4 = !{!"icx (ICX) dev.8.x.0"}
!5 = !{void ([16384 x i32] addrspace(1)*, [16384 x i32] addrspace(1)*, [16384 x i32] addrspace(1)*)* @f}
!6 = !{i32 1, i32 1, i32 1}
!7 = !{!"none", !"none", !"none"}
!8 = !{!"__global int [16384]*", !"__global int [16384]*", !"__global int [16384]*"}
!9 = !{!"int __global[16384]*", !"int __global[16384]*", !"int __global[16384]*"}
!10 = !{!"", !"", !""}
!11 = !{i1 false, i1 false, i1 false}
!12 = !{i32 0, i32 0, i32 0}
!13 = !{!"A", !"B", !"C"}
!14 = !{i1 true}
!15 = !{i32 8}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}

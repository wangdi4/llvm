; RUN: opt -passes='print<sycl-kernel-soa-alloca-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check SoaAllocaAnalysis decides not to optimize unsupported base types.

; CHECK: SoaAllocaAnalysis for function test
; CHECK-NOT: alloca i32

; CHECK: SoaAllocaAnalysis for function test_struct
; CHECK-NOT: alloca %struct.ST

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

%struct.ST = type { i32 }

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test(ptr addrspace(1) %dst) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !arg_type_null_val !14 {
entry:
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8, !tbaa !8
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !tbaa !8
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #2
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !tbaa !12
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test_struct() #0 {
entry:
  %s = alloca %struct.ST, align 4
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare spir_func i64 @_Z13get_global_idj(i32) #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent nounwind readnone willreturn }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}

!0 = !{i32 2, i32 0}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!4 = !{!""}
!5 = !{!"dst"}
!6 = !{i1 false}
!7 = !{i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"any pointer", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !10, i64 0}
!14 = !{ptr addrspace(1) null}

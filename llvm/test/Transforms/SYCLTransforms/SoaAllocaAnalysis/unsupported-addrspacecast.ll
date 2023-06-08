; RUN: opt -passes='print<sycl-kernel-soa-alloca-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check SoaAllocaAnalysis decides not to optimize unsupported alloca or
; addrspacecast of alloca that is used in unsupported function call.

; CHECK: SoaAllocaAnalysis for function test_pipe
; CHECK-NOT: alloca ptr addrspace(1)
; CHECK-NOT: alloca i32

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local void @test_pipe(ptr addrspace(1) %p, i32 noundef %data) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !9 {
entry:
  %p.addr = alloca ptr addrspace(1), align 8
  %data.addr = alloca i32, align 4
  store ptr addrspace(1) %p, ptr %p.addr, align 8, !tbaa !10
  store i32 %data, ptr %data.addr, align 4, !tbaa !13
  %0 = load ptr addrspace(1), ptr %p.addr, align 8, !tbaa !10
  %1 = addrspacecast ptr %data.addr to ptr addrspace(4)
  %2 = call i32 @__write_pipe_2(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4)
  ret void
}

declare i32 @__write_pipe_2(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test_pipe}
!1 = !{i32 1, i32 0}
!2 = !{!"write_only", !"none"}
!3 = !{!"int", !"int"}
!4 = !{!"pipe", !""}
!5 = !{!"p", !"data"}
!6 = !{i1 false, i1 false}
!7 = !{i32 0, i32 0}
!8 = !{!"", !""}
!9 = !{target("spirv.Pipe", 1) zeroinitializer, i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}

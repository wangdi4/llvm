; RUN: opt -passes='print<dpcpp-kernel-soa-alloca-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -enable-new-pm=0 -dpcpp-kernel-soa-alloca-analysis %s -S -o - | FileCheck %s

; Check SoaAllocaAnalysis decides not to optimize unsupported alloca or
; addrspacecast of alloca that is used in unsupported function call.

; CHECK: SoaAllocaAnalysis for function test_pipe
; CHECK-NOT: alloca %opencl.pipe_wo_t.9 addrspace(1)*
; CHECK-NOT: alloca i32

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

%opencl.pipe_wo_t.9 = type opaque

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test_pipe(%opencl.pipe_wo_t.9 addrspace(1)* %p, i32 %data) #0 {
entry:
  %p.addr = alloca %opencl.pipe_wo_t.9 addrspace(1)*, align 8
  %data.addr = alloca i32, align 4
  store %opencl.pipe_wo_t.9 addrspace(1)* %p, %opencl.pipe_wo_t.9 addrspace(1)** %p.addr, align 8, !tbaa !1
  store i32 %data, i32* %data.addr, align 4, !tbaa !4
  %0 = load %opencl.pipe_wo_t.9 addrspace(1)*, %opencl.pipe_wo_t.9 addrspace(1)** %p.addr, align 8, !tbaa !1
  %1 = addrspacecast i32* %data.addr to i8 addrspace(4)*
  %2 = call spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t.9 addrspace(1)* %0, i8 addrspace(4)* %1, i32 4, i32 4)
  ret void
}

declare spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t.9 addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }

!opencl.ocl.version = !{!0, !0}
!opencl.spir.version = !{!0, !0}

!0 = !{i32 2, i32 0}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !2, i64 0}

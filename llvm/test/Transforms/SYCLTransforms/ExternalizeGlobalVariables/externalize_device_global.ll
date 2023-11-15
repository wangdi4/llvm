; RUN: opt -passes=sycl-kernel-externalize-global-variables -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-externalize-global-variables -S %s -o - | FileCheck %s

; This test check if we can successflly externalize device global

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%structtype = type { ptr addrspace(1) }

@_ZL15DeviceGlobalVar = internal addrspace(1) global %structtype zeroinitializer, align 8, !spirv.Decorations !0
; CHECK: @_ZL15DeviceGlobalVar = dso_local addrspace(1) global %structtype zeroinitializer, align 8, !spirv.Decorations !0, !spirv.Decorations.HostAccess !2

; Function Attrs: nounwind
define void @_ZTSZ4mainEUlvE_() #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !7 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !7 !arg_type_null_val !7 !spirv.ParameterDecorations !7 {
entry:
  %0 = getelementptr inbounds %structtype, ptr addrspace(1) @_ZL15DeviceGlobalVar, i64 0, i32 0
  %1 = load ptr addrspace(1), ptr addrspace(1) %0, align 8
  %arrayidx1.i = getelementptr inbounds [4 x i32], ptr addrspace(1) %1, i64 0, i64 0
  store i32 42, ptr addrspace(1) %arrayidx1.i, align 4
  ret void
}

attributes #0 = { nounwind }

!spirv.MemoryModel = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!4}
!opencl.spir.version = !{!5}
!opencl.ocl.version = !{!6}
!opencl.used.extensions = !{!7}
!opencl.used.optional.core.features = !{!7}
!spirv.Generator = !{!8}
!sycl.kernels = !{!9}

!0 = !{!1, !2}
!1 = !{i32 44, i32 8}
!2 = !{i32 6147, i32 2, !"uida19ced7082b9fc5c____ZL15DeviceGlobalVar"}
!3 = !{i32 2, i32 2}
!4 = !{i32 4, i32 100000}
!5 = !{i32 1, i32 2}
!6 = !{i32 1, i32 0}
!7 = !{}
!8 = !{i16 6, i16 14}
!9 = !{ptr @_ZTSZ4mainEUlvE_}

; DEBUGIFY-NOT: WARNING

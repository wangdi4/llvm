; RUN: opt -passes=sycl-kernel-remove-devicelib-assert-fallback -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-remove-devicelib-assert-fallback -S %s | FileCheck %s

target triple = "x86_64-pc-linux"

%struct.AssertHappened = type { i32, [257 x i8], [257 x i8], [129 x i8], i32, i64, i64, i64, i64, i64, i64 }

@SPIR_AssertHappenedMem = addrspace(1) global %struct.AssertHappened zeroinitializer, align 8, !spirv.Decorations !3

; Function Attrs: nounwind
define spir_func void @__devicelib_assert_fail(ptr addrspace(4) %expr, ptr addrspace(4) %file, i32 %line, ptr addrspace(4) %func, i64 %gid0, i64 %gid1, i64 %gid2, i64 %lid0, i64 %lid1, i64 %lid2) #0 {
entry:
  ret void
}

; CHECK-NOT: define spir_func void @__devicelib_assert_fail(ptr addrspace(4) %expr, ptr addrspace(4) %file, i32 %line, ptr addrspace(4) %func, i64 %gid0, i64 %gid1, i64 %gid2, i64 %lid0, i64 %lid1, i64 %lid2) #0 {

; CHECK: declare spir_func void @__devicelib_assert_fail_opencl(ptr addrspace(4), ptr addrspace(4), i32, ptr addrspace(4), i64, i64, i64, i64, i64, i64) #0

attributes #0 = { nounwind }

!spirv.MemoryModel = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!7}
!opencl.spir.version = !{!8}
!opencl.ocl.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!spirv.Generator = !{!11}
!sycl.stat.type = !{!12}
!sycl.stat.exec_time = !{!13}
!sycl.stat.run_time_version = !{!14}
!sycl.stat.workload_name = !{!15}
!sycl.stat.module_name = !{!16}

!0 = !{!1, !2}
!1 = !{i32 22}
!2 = !{i32 44, i32 1}
!3 = !{!4, !5}
!4 = !{i32 41, !"SPIR_AssertHappenedMem", i32 0}
!5 = !{i32 44, i32 8}
!6 = !{i32 2, i32 2}
!7 = !{i32 4, i32 100000}
!8 = !{i32 1, i32 2}
!9 = !{i32 1, i32 0}
!10 = !{}
!11 = !{i16 6, i16 14}
!12 = !{!"all"}
!13 = !{!"2023-11-13 18:02:31"}
!14 = !{!"2023.17.11.0"}
!15 = !{!"a.out"}
!16 = !{!"a.out1"}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING

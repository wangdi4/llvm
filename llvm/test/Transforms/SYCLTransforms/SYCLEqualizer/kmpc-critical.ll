; RUN: opt -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-equalizer -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__tgt_offload_entry.0 = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }
@.omp_offloading.entry_name = internal unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_804_2300026__Z4main_l30\00"
@__omp_offloading_entries_table = addrspace(1) constant [1 x %struct.__tgt_offload_entry.0] [%struct.__tgt_offload_entry.0 { ptr addrspace(4) null, ptr addrspace(2) @.omp_offloading.entry_name, i64 0, i32 0, i32 0, i64 41 }]

define spir_func void @__kmpc_critical(ptr addrspace(4)) {
entry:
  ret void
}

define void @foo() {
entry:
  call spir_func void @__kmpc_critical(ptr addrspace(4) null)
  ret void
}

; CHECK: define void @__omp_offloading_804_2300026__Z4main_l30()
; CHECK-SAME: !intel_vec_len_hint [[VLHint:![0-9]+]]

define spir_kernel void @__omp_offloading_804_2300026__Z4main_l30() {
entry:
  call void @foo()
  ret void
}

; CHECK: define void @test_kernel()
; CHECK-SAME: !intel_vec_len_hint [[VLHint]]

define spir_kernel void @test_kernel() {
entry:
  call void @__omp_offloading_804_2300026__Z4main_l30()
  ret void
}

!spirv.Source = !{!0}
!0 = !{i32 4, i32 100000}

; CHECK: [[VLHint]] = !{i32 1}

; DEBUGIFY-NOT: WARNING

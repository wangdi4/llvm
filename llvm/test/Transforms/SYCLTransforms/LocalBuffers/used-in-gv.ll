; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.__tgt_offload_entry.0 = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }

; CHECK: @x = dso_local addrspace(3) global i32 0, align 4,

@x = dso_local addrspace(3) global i32 0, align 4, !spirv.Decorations !0
@.omp_offloading.entry_name = external addrspace(2) constant [5 x i8]
@.omp_offloading.entry_name.1 = external addrspace(2) constant [41 x i8]
@__omp_offloading_entries_table = local_unnamed_addr addrspace(1) constant [2 x %struct.__tgt_offload_entry.0] [%struct.__tgt_offload_entry.0 { ptr addrspace(4) addrspacecast (ptr addrspace(3) @x to ptr addrspace(4)), ptr addrspace(2) @.omp_offloading.entry_name, i64 4, i32 0, i32 0, i64 5 }, %struct.__tgt_offload_entry.0 { ptr addrspace(4) null, ptr addrspace(2) @.omp_offloading.entry_name.1, i64 0, i32 0, i32 0, i64 41 }], !spirv.Decorations !2

!0 = !{!1}
!1 = !{i32 44, i32 4}
!2 = !{!3, !4}
!3 = !{i32 22}
!4 = !{i32 41, !"__omp_offloading_entries_table", i32 0}

; DEBUGIFY-NOT: WARNING

; RUN: opt -sycl-kernel-builtin-lib=%s.rtl -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%s.rtl -passes=sycl-kernel-analysis %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; CHECK: define {{.*}} void @test1({{.*}} !kernel_has_global_sync [[HasGlobalSync:![0-9]+]]

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test1(ptr addrspace(1) noundef align 4 %v) #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = call spir_func i32 @_Z8atom_addPU3AS1Vii(ptr addrspace(1) noundef %v, i32 noundef 1) #2
  ret void
}

; Function Attrs: convergent
declare spir_func i32 @_Z8atom_addPU3AS1Vii(ptr addrspace(1) noundef, i32 noundef) #1

; Function Attrs: convergent norecurse nounwind
define dso_local spir_func void @foo(ptr addrspace(1) noundef %v) #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = call spir_func i32 @_Z8atom_addPU3AS1Vii(ptr addrspace(1) noundef %v, i32 noundef 2) #2
  ret void
}

; CHECK: define {{.*}} void @test2({{.*}} !kernel_has_global_sync [[HasGlobalSync]]

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test2(ptr addrspace(1) noundef align 4 %v) #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  call spir_func void @foo(ptr addrspace(1) noundef %v) #1
  ret void
}

; CHECK: define {{.*}} void @test3({{.*}} !kernel_has_global_sync [[NoGlobalSync:![0-9]+]]

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test3(ptr addrspace(1) noundef align 4 %v) #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  ret void
}

; CHECK: define {{.*}} void @test_atomic_work_item_fence1({{.*}} !kernel_has_global_sync [[NoGlobalSync]]

define dso_local void @test_atomic_work_item_fence1() local_unnamed_addr #0 {
entry:
  tail call void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 noundef 4, i32 noundef 4, i32 noundef 0) #2
  ret void
}

; CHECK: define {{.*}} void @test_atomic_work_item_fence2({{.*}} !kernel_has_global_sync [[HasGlobalSync]]

define dso_local void @test_atomic_work_item_fence2() local_unnamed_addr #0 {
entry:
  tail call void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 noundef 2, i32 noundef 4, i32 noundef 0) #2
  ret void
}

declare void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent }
attributes #2 = { convergent nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test1, ptr @test2, ptr @test3, ptr @test_atomic_work_item_fence1, ptr @test_atomic_work_item_fence2}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}

; CHECK-DAG: [[HasGlobalSync]] = !{i1 true}
; CHECK-DAG: [[NoGlobalSync]] = !{i1 false}

; DEBUGIFY-NOT: WARNING

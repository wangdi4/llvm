; RUN: opt -dpcpp-kernel-builtin-lib=%s.rtl -passes=dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%s.rtl -passes=dpcpp-kernel-analysis %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; CHECK: define {{.*}} void @test1({{.*}} !kernel_has_global_sync [[HasGlobalSync:![0-9]+]]

define dso_local spir_kernel void @test1(i32 addrspace(1)* noundef align 4 %v) #0 {
entry:
  %call = call spir_func i32 @_Z8atom_addPU3AS1Vii(i32 addrspace(1)* noundef %v, i32 noundef 1) #2
  ret void
}

declare spir_func i32 @_Z8atom_addPU3AS1Vii(i32 addrspace(1)* noundef, i32 noundef) #1

define dso_local spir_func void @foo(i32 addrspace(1)* noundef %v) #0 {
entry:
  %call = call spir_func i32 @_Z8atom_addPU3AS1Vii(i32 addrspace(1)* noundef %v, i32 noundef 2) #2
  ret void
}

; CHECK: define {{.*}} void @test2({{.*}} !kernel_has_global_sync [[HasGlobalSync]]

define dso_local spir_kernel void @test2(i32 addrspace(1)* noundef align 4 %v) #0 {
entry:
  call spir_func void @foo(i32 addrspace(1)* noundef %v) #1
  ret void
}

; CHECK: define {{.*}} void @test3({{.*}} !kernel_has_global_sync [[NoGlobalSync:![0-9]+]]

define dso_local spir_kernel void @test3(i32 addrspace(1)* noundef align 4 %v) #0 {
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

!0 = !{void (i32 addrspace(1)*)* @test1, void (i32 addrspace(1)*)* @test2, void (i32 addrspace(1)*)* @test3, void ()* @test_atomic_work_item_fence1, void ()* @test_atomic_work_item_fence2 }

; CHECK-DAG: [[HasGlobalSync]] = !{i1 true}
; CHECK-DAG: [[NoGlobalSync]] = !{i1 false}

; DEBUGIFY-NOT: WARNING

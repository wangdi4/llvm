; RUN: opt -S -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' %s | FileCheck %s

@var = internal addrspace(3) global i8 42

define void @test() {
; CHECK-LABEL: define void @test
; CHECK-NEXT: [[GEP:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NEXT: [[ASC:%[0-9]+]] = addrspacecast ptr addrspace(3) [[GEP]] to ptr
; CHECK-NEXT: %[[INSERT:.*]] = insertvalue { ptr, i8 } poison, ptr [[ASC]], 0
; CHECK-NEXT: %[[INSERT1:.*]] = insertvalue { ptr, i8 } %[[INSERT]], i8 0, 1
; CHECK-NEXT: extractvalue { ptr, i8 } %[[INSERT1]], 0
; CHECK-NEXT: ret void
  %1 = extractvalue { ptr, i8 } { ptr addrspacecast (ptr addrspace(3) @var to ptr), i8 0 }, 0
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS

; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s -check-prefixes=CHECK,OPAQUE

@var = internal addrspace(3) global i8 42

define void @test() {
; CHECK-LABEL: define void @test
; NONOPAQUE-NEXT: [[GEP:%[0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; NONOPAQUE-NEXT: [[ASC:%[0-9]+]] = addrspacecast i8 addrspace(3)* [[GEP]] to i8*
; NONOPAQUE-NEXT: %[[INSERT:.*]] = insertvalue { i8*, i8 } undef, i8* [[ASC]], 0
; NONOPAQUE-NEXT: %[[INSERT1:.*]] = insertvalue { i8*, i8 } %[[INSERT]], i8 0, 1
; NONOPAQUE-NEXT: extractvalue { i8*, i8 } %[[INSERT1]], 0
;; Check for opaque pointer
; OPAQUE-NEXT: [[GEP:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; OPAQUE-NEXT: [[ASC:%[0-9]+]] = addrspacecast ptr addrspace(3) [[GEP]] to ptr
; OPAQUE-NEXT: %[[INSERT:.*]] = insertvalue { ptr, i8 } undef, ptr [[ASC]], 0
; OPAQUE-NEXT: %[[INSERT1:.*]] = insertvalue { ptr, i8 } %[[INSERT]], i8 0, 1
; OPAQUE-NEXT: extractvalue { ptr, i8 } %[[INSERT1]], 0
; CHECK-NEXT: ret void
  %1 = extractvalue { i8*, i8 } { i8* addrspacecast (i8 addrspace(3)* @var to i8*), i8 0 }, 0
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @test}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS

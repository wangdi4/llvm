; RUN: opt -S -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -S -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -S -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers %s | FileCheck %s -check-prefixes=CHECK,OPAQUE
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s -check-prefixes=CHECK,OPAQUE

@var = internal addrspace(3) global i8 42

define void @test() {
; CHECK-LABEL: define void @test
; NONOPAQUE-NEXT: %1 = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; NONOPAQUE-NEXT: %2 = addrspacecast i8 addrspace(3)* %1 to i8 addrspace(3)**
; NONOPAQUE-NEXT: %3 = load i8 addrspace(3)*, i8 addrspace(3)** %2
; NONOPAQUE-NEXT: %4 = addrspacecast i8 addrspace(3)* %3 to i8*
; NONOPAQUE-NEXT: %[[INSERT:.*]] = insertvalue { i8*, i8 } undef, i8* %4, 0
; NONOPAQUE-NEXT: %[[INSERT1:.*]] = insertvalue { i8*, i8 } %[[INSERT]], i8 0, 1
; NONOPAQUE-NEXT: %5 = extractvalue { i8*, i8 } %[[INSERT1]], 0
;; Check for opaque pointer
; OPAQUE-NEXT: %1 = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; OPAQUE-NEXT: %2 = addrspacecast ptr addrspace(3) %1 to ptr
; OPAQUE-NEXT: %3 = load ptr addrspace(3), ptr %2
; OPAQUE-NEXT: %4 = addrspacecast ptr addrspace(3) %3 to ptr
; OPAQUE-NEXT: %[[INSERT:.*]] = insertvalue { ptr, i8 } undef, ptr %4, 0
; OPAQUE-NEXT: %[[INSERT1:.*]] = insertvalue { ptr, i8 } %[[INSERT]], i8 0, 1
; OPAQUE-NEXT: %5 = extractvalue { ptr, i8 } %[[INSERT1]], 0
; CHECK-NEXT: ret void
  %1 = extractvalue { i8*, i8 } { i8* addrspacecast (i8 addrspace(3)* @var to i8*), i8 0 }, 0
  ret void
}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS

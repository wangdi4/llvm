; RUN: opt -S -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers %s | FileCheck %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s

@var = internal addrspace(3) global i8 42

define void @test() {
; CHECK-LABEL: define void @test
; CHECK-NEXT: %1 = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: %2 = addrspacecast i8 addrspace(3)* %1 to i8 addrspace(3)**
; CHECK-NEXT: %3 = load i8 addrspace(3)*, i8 addrspace(3)** %2
; CHECK-NEXT: %4 = addrspacecast i8 addrspace(3)* %3 to i8*
; CHECK-NEXT: %[[INSERT:.*]] = insertvalue { i8*, i8 } undef, i8* %4, 0
; CHECK-NEXT: %[[INSERT1:.*]] = insertvalue { i8*, i8 } %[[INSERT]], i8 0, 1
; CHECK-NEXT: %5 = extractvalue { i8*, i8 } %[[INSERT1]], 0
; CHECK-NEXT: ret void
  %1 = extractvalue { i8*, i8 } { i8* addrspacecast (i8 addrspace(3)* @var to i8*), i8 0 }, 0
  ret void
}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS

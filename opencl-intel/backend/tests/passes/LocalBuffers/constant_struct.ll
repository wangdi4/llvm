; RUN: %oclopt -S -add-implicit-args -debugify -local-buffers-debug -check-debugify %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -S -add-implicit-args -local-buffers-debug %s | FileCheck %s

@var = internal addrspace(3) global i8 42

define void @test() {
; CHECK-LABEL: define void @test
; CHECK-NEXT: %1 = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: %2 = bitcast i8 addrspace(3)* %1 to i8 addrspace(3)*
; CHECK-NEXT: %3 = addrspacecast i8 addrspace(3)* %2 to i8*
; CHECK-NEXT: %[[INSERT:.*]] = insertvalue { i8*, i8 } undef, i8* %3, 0
; CHECK-NEXT: %[[INSERT1:.*]] = insertvalue { i8*, i8 } %[[INSERT]], i8 0, 1
; CHECK-NEXT: %4 = extractvalue { i8*, i8 } %[[INSERT1]], 0
; CHECK-NEXT: ret void
  %1 = extractvalue { i8*, i8 } { i8* addrspacecast (i8 addrspace(3)* @var to i8*), i8 0 }, 0
  ret void
}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- {{.*}} addrspacecast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- {{.*}} insertvalue
; DEBUGIFY-NOT: WARNING

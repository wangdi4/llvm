; RUN: opt -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck %s

; CHECK: define spir_func i32 @foo(i32 %arg, i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-SAME:          { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim
; CHECK-SAME:          i64* noalias %pWGId,
; CHECK-SAME:          [4 x i64] %BaseGlbId,
; CHECK-SAME:          i8* noalias %pSpecialBuf,
; CHECK-SAME:          {}* noalias %RuntimeHandle)
;
; CHECK: define void @test
; CHECK: %[[BITCAST:[0-9]+]] = bitcast i32 (i32)* %fp to i32 (i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)*
; CHECK: call spir_func i32 %[[BITCAST]](i32 %0,

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

define spir_func i32 @foo(i32 %arg) {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  %0 = load i32, i32* %arg.addr, align 4
  %add = add nsw i32 %0, 10
  ret i32 %add
}

define void @test(i32 (i32)* %fp, i32 addrspace(1)* %data) #0 {
entry:
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 1
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call = call spir_func i32 %fp(i32 %0)
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 0
  store i32 %call, i32 addrspace(1)* %arrayidx1, align 4
  ret void
}

attributes #0 = { "sycl-kernel" }

; DEBUGIFY: Instruction with empty DebugLoc in function {{.*}} getelementptr
; DEBUGIFY-NEXT: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{void (i32 (i32)*, i32 addrspace(1)*)* @test}

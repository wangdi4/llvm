; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

; CHECK: define spir_func i32 @foo(i32 %arg, ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-SAME:          ptr noalias %pWorkDim
; CHECK-SAME:          ptr noalias %pWGId,
; CHECK-SAME:          [4 x i64] %BaseGlbId,
; CHECK-SAME:          ptr noalias %pSpecialBuf,
; CHECK-SAME:          ptr noalias %RuntimeHandle)
;
; CHECK: define void @test
; CHECK: [[FUNCPTR:%.*]] = bitcast ptr %fp to ptr
; CHECK: call spir_func i32 [[FUNCPTR]](i32 %0,

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

define spir_func i32 @foo(i32 %arg) {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, ptr %arg.addr, align 4
  %0 = load i32, ptr %arg.addr, align 4
  %add = add nsw i32 %0, 10
  ret i32 %add
}

define void @test(ptr %fp, ptr addrspace(1) %data) {
entry:
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %data, i64 1
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4
  %call = call spir_func i32 %fp(i32 %0)
  store i32 %call, ptr addrspace(1) %data, align 4
  ret void
}


; DEBUGIFY: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{ptr @test}

; RUN: opt -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-resolve-wi-call -check-debugify -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-resolve-wi-call -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind
define void @A(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b, float addrspace(1)* nocapture %c, i32 %iNumElements) {
; CHECK: define void @A
; CHECK: %temp_arg_buf = alloca [12 x i8]
; CHECK-NEXT: [[GEP0:%[0-9]+]] = getelementptr inbounds [12 x i8], [12 x i8]* %temp_arg_buf, i32 0, i32 0
; CHECK-NEXT: %arg_buf_size = bitcast i8* [[GEP0]] to i32*
; CHECK-NEXT: store i32 12, i32* %arg_buf_size, align 4
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr inbounds [12 x i8], [12 x i8]* %temp_arg_buf, i32 0, i32 4
; CHECK-NEXT: %arg_size = bitcast i8* [[GEP1]] to i32*
; CHECK-NEXT: store i32 4, i32* %arg_size, align 4
; CHECK-NEXT: [[GEP2:%[0-9]+]] = getelementptr inbounds [12 x i8], [12 x i8]* %temp_arg_buf, i32 0, i32 8
; CHECK-NEXT: %arg_val = bitcast i8* [[GEP2]] to i32*
; CHECK-NEXT: store i32 %iNumElements, i32* %arg_val, align 1
; CHECK-NEXT: %translated_opencl_printf_call = call i32 @__opencl_printf(i8 addrspace(2)* addrspacecast (i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0) to i8 addrspace(2)*), i8* [[GEP0]]
  %call1 = tail call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* addrspacecast (i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0) to i8 addrspace(2)*), i32 %iNumElements)
  ret void
}

declare i32 @printf(i8 addrspace(2)*, ...)


; The pass will generate some instructions without debug info. Since these
; instructions are not from user code and they will not impact debuggability.
; We can ignore the warnings related to them.
; DEBUGIFY-NOT: WARNING: Missing line

!sycl.kernels = !{!0}
!0 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @A}

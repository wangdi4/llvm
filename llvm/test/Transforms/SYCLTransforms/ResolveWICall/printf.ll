; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind
define void @A(ptr addrspace(1) nocapture %a, ptr addrspace(1) nocapture %b, ptr addrspace(1) nocapture %c, i32 %iNumElements) !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK: define void @A
; CHECK: %temp_arg_buf = alloca [12 x i8]
; CHECK-NEXT: [[GEP0:%[0-9]+]] = getelementptr inbounds [12 x i8], ptr %temp_arg_buf, i32 0, i32 0
; CHECK-NEXT: store i32 12, ptr [[GEP0]], align 4
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr inbounds [12 x i8], ptr %temp_arg_buf, i32 0, i32 4
; CHECK-NEXT: store i32 4, ptr [[GEP1]], align 4
; CHECK-NEXT: [[GEP2:%[0-9]+]] = getelementptr inbounds [12 x i8], ptr %temp_arg_buf, i32 0, i32 8
; CHECK-NEXT: store i32 %iNumElements, ptr [[GEP2]], align 1
; CHECK-NEXT: %translated_opencl_printf_call = call i32 @__opencl_printf(ptr addrspace(2) addrspacecast (ptr @.str to ptr addrspace(2)), ptr [[GEP0]]
  %call1 = tail call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) addrspacecast (ptr @.str to ptr addrspace(2)), i32 %iNumElements)
  ret void
}

declare i32 @printf(ptr addrspace(2), ...)


; The pass will generate some instructions without debug info. Since these
; instructions are not from user code and they will not impact debuggability.
; We can ignore the warnings related to them.
; DEBUGIFY-NOT: WARNING: Missing line

!sycl.kernels = !{!0}
!0 = !{ptr @A}
!1 = !{!"float*", !"float*", !"float*", !"int"}
!2 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null, i32 0}

; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

; CHECK: define void @_ZTS1K
; CHECK: %[[ALLOCA:.*]] = alloca ptr
; CHECK: store ptr @_Z3addii, ptr %[[ALLOCA]]
; CHECK: %[[LOAD:.*]] = load ptr, ptr %[[ALLOCA]]
; CHECK: [[FUNCPTR1:%.*]] = bitcast ptr %[[LOAD]] to ptr
; CHECK: call spir_func i32 [[FUNCPTR1]] 

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-linux-sycldevice"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

@__spirv_BuiltInGlobalInvocationId = external dso_local local_unnamed_addr addrspace(2) constant <3 x i64>, align 32

define spir_func i32 @_Z3addii(i32 %a, i32 %b) {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

define void @_ZTS1K(i32 %_arg_, ptr addrspace(1) %_arg_1, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset, ptr addrspace(1) %_arg_3, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange5, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange6, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset7, ptr addrspace(1) %_arg_8, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange10, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange11, ptr byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset12) {
entry:
  %alloca = alloca ptr, align 8
  %0 = load i64, ptr %_arg_Offset, align 8
  %add.ptr.i = getelementptr inbounds i32, ptr addrspace(1) %_arg_1, i64 %0
  %1 = load i64, ptr %_arg_Offset7, align 8
  %add.ptr.i15 = getelementptr inbounds i32, ptr addrspace(1) %_arg_3, i64 %1
  %2 = load i64, ptr %_arg_Offset12, align 8
  %add.ptr.i26 = getelementptr inbounds i32, ptr addrspace(1) %_arg_8, i64 %2
  %3 = load <3 x i64>, ptr addrspace(2) @__spirv_BuiltInGlobalInvocationId, align 32
  %4 = extractelement <3 x i64> %3, i64 0
  %cmp.i = icmp eq i32 %_arg_, 0
  store ptr @_Z3addii, ptr %alloca
  %arrayidx.i.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %4
  %5 = load i32, ptr addrspace(1) %arrayidx.i.i, align 4
  %arrayidx.i10.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i15, i64 %4
  %6 = load i32, ptr addrspace(1) %arrayidx.i10.i, align 4
  %7 = load ptr, ptr %alloca
  %call4.i = tail call spir_func i32 %7(i32 %5, i32 %6)
  store i32 %call4.i, ptr addrspace(1) %arrayidx.i.i, align 4
  store i32 %_arg_, ptr addrspace(1) %add.ptr.i26, align 4
  ret void
}


; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K {{.*}} bitcast
; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{ptr @_ZTS1K}

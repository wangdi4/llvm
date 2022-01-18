; RUN: opt -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck %s

; CHECK: define void @_ZTS1K
; CHECK: %[[ALLOCA:.*]] = alloca i32
; CHECK: %[[PTRTOINT:.*]] = ptrtoint i32 (i32, i32, {{.*}})* @_Z3addii to i32
; CHECK: %{{.*}} = bitcast i32 (i32, i32, {{.*}})* @_Z3addii to i8*
; CHECK: %{{.*}} = addrspacecast i32 (i32, i32, {{.*}})* @_Z3addii to i32 (i32, i32) addrspace(4)*
; CHECK: store i32 %[[PTRTOINT]], i32* %[[ALLOCA]]
; CHECK: %[[INT:.*]] = load i32, i32* %[[ALLOCA]]
; CHECK: %[[INTTOPTR:.*]] = inttoptr i32 %[[INT]] to i32 (i32, i32)*
; CHECK: %[[BITCAST:.*]] = bitcast i32 (i32, i32)* %[[INTTOPTR]] to i32 (i32, i32, {{.*}})*
; CHECK: call spir_func i32 %[[BITCAST]]
; CHECK: store i32 ptrtoint (i32 (i32, i32, {{.*}})* @_Z3addii to i32), i32* %[[ALLOCA]]
; CHECK: %[[INT:.*]] = load i32, i32* %[[ALLOCA]]
; CHECK: %[[INTTOPTR:.*]] = inttoptr i32 %[[INT]] to i32 (i32, i32)*
; CHECK: %[[BITCAST:.*]] = bitcast i32 (i32, i32)* %[[INTTOPTR]] to i32 (i32, i32, {{.*}})*
; CHECK: call spir_func i32 %[[BITCAST]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-linux-sycldevice"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

@__spirv_BuiltInGlobalInvocationId = external dso_local local_unnamed_addr addrspace(2) constant <3 x i64>, align 32

define spir_func i32 @_Z3addii(i32 %a, i32 %b) local_unnamed_addr {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

define void @_ZTS1K(i32 %_arg_, i32 addrspace(1)* %_arg_1, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset, i32 addrspace(1)* %_arg_3, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange5, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange6, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset7, i32 addrspace(1)* %_arg_8, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange10, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange11, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset12) {
entry:
  %alloca = alloca i32, align 8
  %0 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %_arg_Offset, i64 0, i32 0, i32 0, i64 0
  %1 = load i64, i64* %0, align 8
  %add.ptr.i = getelementptr inbounds i32, i32 addrspace(1)* %_arg_1, i64 %1
  %2 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %_arg_Offset7, i64 0, i32 0, i32 0, i64 0
  %3 = load i64, i64* %2, align 8
  %add.ptr.i15 = getelementptr inbounds i32, i32 addrspace(1)* %_arg_3, i64 %3
  %4 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %_arg_Offset12, i64 0, i32 0, i32 0, i64 0
  %5 = load i64, i64* %4, align 8
  %add.ptr.i26 = getelementptr inbounds i32, i32 addrspace(1)* %_arg_8, i64 %5
  %6 = load <3 x i64>, <3 x i64> addrspace(2)* @__spirv_BuiltInGlobalInvocationId, align 32
  %7 = extractelement <3 x i64> %6, i64 0
  %cmp.i = icmp eq i32 %_arg_, 0
  %int = ptrtoint i32 (i32, i32)* @_Z3addii to i32
  %i8ptr = bitcast i32 (i32, i32)* @_Z3addii to i8*
  %addr = addrspacecast i32 (i32, i32)* @_Z3addii to i32 (i32, i32) addrspace(4)*
  store i32 %int, i32* %alloca
  %arrayidx.i.i = getelementptr inbounds i32, i32 addrspace(1)* %add.ptr.i, i64 %7
  %8 = load i32, i32 addrspace(1)* %arrayidx.i.i, align 4
  %arrayidx.i10.i = getelementptr inbounds i32, i32 addrspace(1)* %add.ptr.i15, i64 %7
  %9 = load i32, i32 addrspace(1)* %arrayidx.i10.i, align 4
  %10 = load i32, i32* %alloca
  %11 = inttoptr i32 %10 to i32 (i32, i32)*
  %call4.i = tail call spir_func i32 %11(i32 %8, i32 %9)
  store i32 %call4.i, i32 addrspace(1)* %arrayidx.i.i, align 4
  store i32 %_arg_, i32 addrspace(1)* %add.ptr.i26, align 4
  store i32 ptrtoint(i32 (i32, i32)* @_Z3addii to i32), i32* %alloca
  %12 = load i32, i32* %alloca
  %13 = inttoptr i32 %12 to i32 (i32, i32)*
  %call6.i = tail call spir_func i32 %13(i32 %8, i32 %9)
  store i32 %call4.i, i32 addrspace(1)* %arrayidx.i.i, align 4
  store i32 %_arg_, i32 addrspace(1)* %add.ptr.i26, align 4
  ret void
}


; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{void (i32, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @_ZTS1K}

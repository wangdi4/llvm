; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-OPAQUE %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK -check-prefix=CHECK-OPAQUE %s

; For opaque pointer, there is no bitcast from function with implicit args
; to original function when select function pointer

; CHECK: define spir_func i32 @_Z3addii(i32 %a, i32 %b,
; CHECK: define spir_func i32 @_Z3subii(i32 %a, i32 %b,
; CHECK: define void @_ZTS1K
; CHECK-NONOPAQUE: %[[ADD:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3addii to i32 (i32, i32)*
; CHECK-NONOPAQUE: %[[SUB:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3subii to i32 (i32, i32)*
; CHECK-NONOPAQUE: %[[SELECT:.*]] = select i1 %{{.*}}, i32 (i32, i32)* %[[ADD]], i32 (i32, i32)* %[[SUB]]
; CHECK-OPAQUE: %{{.*}} = select i1 %{{.*}}, ptr @{{.*}}, ptr @{{.*}}
; CHECK-NONOPAQUE: %[[BITCAST:.*]] = bitcast i32 (i32, i32)* %[[SELECT]] to i32 (i32, i32,
; CHECK-OPAQUE: %[[BITCAST:.*]] = bitcast ptr %{{.*}} to ptr
; CHECK: call spir_func i32 %[[BITCAST]]
; CHECK-NONOPAQUE: %[[ADD2:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3addii to i32 (i32, i32)*
; CHECK-NONOPAQUE: %[[SUB2:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3subii to i32 (i32, i32)*
; CHECK-NONOPAQUE: %[[SELECT2:.*]] = select i1 %{{.*}}, i32 (i32, i32)* %[[ADD2]], i32 (i32, i32)* %[[SUB2]]
; CHECK-OPAQUE: %{{.*}} = select i1 %{{.*}}, ptr @{{.*}}, ptr @{{.*}}
; CHECK-NONOPAQUE: %[[BITCAST2:.*]] = bitcast i32 (i32, i32)* %[[SELECT2]] to i32 (i32, i32,
; CHECK-OPAQUE: %[[BITCAST2:.*]] = bitcast ptr %{{.*}} to ptr
; CHECK: call spir_func i32 %[[BITCAST2]]

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

define spir_func i32 @_Z3subii(i32 %a, i32 %b) local_unnamed_addr {
entry:
  %sub = sub nsw i32 %a, %b
  ret i32 %sub
}

define void @_ZTS1K(i32 %_arg_, i32 addrspace(1)* %_arg_1, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset, i32 addrspace(1)* %_arg_3, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange5, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange6, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset7, i32 addrspace(1)* %_arg_8, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange10, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange11, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset12) {
entry:
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
  %_Z3addii._Z3subii.i = select i1 %cmp.i, i32 (i32, i32)* @_Z3addii, i32 (i32, i32)* @_Z3subii
  %arrayidx.i.i = getelementptr inbounds i32, i32 addrspace(1)* %add.ptr.i, i64 %7
  %8 = load i32, i32 addrspace(1)* %arrayidx.i.i, align 4
  %arrayidx.i10.i = getelementptr inbounds i32, i32 addrspace(1)* %add.ptr.i15, i64 %7
  %9 = load i32, i32 addrspace(1)* %arrayidx.i10.i, align 4
  %call4.i = tail call spir_func i32 %_Z3addii._Z3subii.i(i32 %8, i32 %9)
  %cmp.i2 = icmp ne i32 %_arg_, 0
  %_Z3addii._Z3subii.i2 = select i1 %cmp.i2, i32 (i32, i32)* @_Z3addii, i32 (i32, i32)* @_Z3subii
  %call4.i2 = tail call spir_func i32 %_Z3addii._Z3subii.i2(i32 %8, i32 %9)
  %result = add i32 %call4.i, %call4.i2
  store i32 %result, i32 addrspace(1)* %arrayidx.i.i, align 4
  store i32 %_arg_, i32 addrspace(1)* %add.ptr.i26, align 4
  ret void
}


; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-NOT: WARNING

; DEBUGIFY-OPAQUE-COUNT-2: WARNING: Instruction with empty DebugLoc in function {{.*}} bitcast
; DEBUGIFY-OPAQUE-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{void (i32, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @_ZTS1K}

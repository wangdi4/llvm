; This test case is to check that __cxa_atexit function call is removed
; by remove-atexit pass.
; RUN: opt -passes=sycl-kernel-remove-atexit -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-remove-atexit -S %s | FileCheck %s

; CHECK: call void @_ZN4globC1Ei(ptr @globobj, i32 4)
; CHECK-NEXT:  ret void

target triple = "x86_64-pc-linux"

%struct.glob = type { %"class.std::deque" }
%"class.std::deque" = type { %"class.std::_Deque_base" }
%"class.std::_Deque_base" = type { %"struct.std::_Deque_base<int, std::allocator<int> >::_Deque_impl" }
%"struct.std::_Deque_base<int, std::allocator<int> >::_Deque_impl" = type { ptr, i64, %"struct.std::_Deque_iterator", %"struct.std::_Deque_iterator" }
%"struct.std::_Deque_iterator" = type { ptr, ptr, ptr, ptr }

$_ZN4globC1Ei = comdat any

@globobj = global %struct.glob zeroinitializer, align 8
@__dso_handle = external hidden global i8
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_lib.cpp.15072.i, ptr null }]

; Function Attrs: nounwind
declare i32 @__cxa_atexit(ptr, ptr, ptr) #2

; Function Attrs: nounwind
define internal void @__cxx_global_var_init() #0 {
entry:
  call void @_ZN4globC1Ei(ptr @globobj, i32 4)
  %0 = call i32 @__cxa_atexit(ptr @_ZN4globC1Ei, ptr @globobj, ptr @__dso_handle) #2
  ret void
}

; Function Attrs: nounwind
define linkonce_odr void @_ZN4globC1Ei(ptr %this, i32 %x) unnamed_addr #0 comdat align 2 !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %this.addr = alloca ptr, align 8
  %x.addr = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store i32 %x, ptr %x.addr, align 4
  ret void
}

; Function Attrs: nounwind
define internal void @_GLOBAL__sub_I_lib.cpp.15072.i() #0  {
entry:
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { nounwind "not-ocl-sycl"="true"}

!0 = !{!"%struct.glob*", !"int"}
!1 = !{ptr null, i32 0}

; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY-NOT: WARNING

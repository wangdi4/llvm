; This test case is to check that __cxa_atexit function call is removed
; by remove-atexit pass.
; RUN: %oclopt -remove-atexit -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -remove-atexit -S %s | FileCheck %s

; CHECK: call void @_ZN4globC1Ei(%struct.glob* @globobj, i32 4)
; CHECK-NEXT:  ret void

; ModuleID = 'main'
target triple = "x86_64-pc-linux"

%struct.glob = type { %"class.std::deque" }
%"class.std::deque" = type { %"class.std::_Deque_base" }
%"class.std::_Deque_base" = type { %"struct.std::_Deque_base<int, std::allocator<int> >::_Deque_impl" }
%"struct.std::_Deque_base<int, std::allocator<int> >::_Deque_impl" = type { i32**, i64, %"struct.std::_Deque_iterator", %"struct.std::_Deque_iterator" }
%"struct.std::_Deque_iterator" = type { i32*, i32*, i32*, i32** }

$_ZN4globC1Ei = comdat any

@globobj = global %struct.glob zeroinitializer, align 8
@__dso_handle = external hidden global i8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_lib.cpp.15072.i, i8* null }]

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) #2

; Function Attrs: nounwind
define internal void @__cxx_global_var_init() #0 {
entry:
  call void @_ZN4globC1Ei(%struct.glob* @globobj, i32 4)
  %0 = call i32 @__cxa_atexit(void (i8*)* bitcast (void (%struct.glob*, i32)* @_ZN4globC1Ei to void (i8*)*), i8* bitcast (%struct.glob* @globobj to i8*), i8* @__dso_handle) #2
  ret void
}

; Function Attrs: nounwind
define linkonce_odr void @_ZN4globC1Ei(%struct.glob* %this, i32 %x) unnamed_addr #0 comdat align 2  {
entry:
  %this.addr = alloca %struct.glob*, align 8
  %x.addr = alloca i32, align 4
  store %struct.glob* %this, %struct.glob** %this.addr, align 8
  store i32 %x, i32* %x.addr, align 4
  ret void
}

; Function Attrs: nounwind
define internal void @_GLOBAL__sub_I_lib.cpp.15072.i() #0  {
entry:
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { nounwind "not-ocl-dpcpp"="true"}

; The pass is to remove __cxa_atexit call instruction.
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY-NOT: WARNING

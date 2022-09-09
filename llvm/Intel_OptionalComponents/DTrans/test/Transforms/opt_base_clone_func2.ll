; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -S -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.type01a 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -S -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.type01a 2>&1 | FileCheck %s

; This test verifies that the base class updates the parameter attribute types
; on the cloned function when the type used is remapped. Specifically, the
; byval attribute can take an optional type argument, which needs to be
; updated when remapping types.

%struct.type01a = type { i32, i32, i32 }
%struct.type01b = type { i32, %struct.type01a }
; CHECK-DAG: %__DTT_struct.type01a = type { i32, i32, i32 }
; CHECK-DAG: %__DDT_struct.type01b = type { i32, %__DTT_struct.type01a }

; Test updating the 'byval' attribute.
define internal void @test01caller() {
  %alloc = alloca %struct.type01b*
  %p = load %struct.type01b*, %struct.type01b** %alloc
  call void @test01callee(%struct.type01b* byval(%struct.type01b) %p)
  ret void
}
; The CloneFunctionInto functionality should handle the modification of
; the function call. Check this to be sure.
; CHECK-LABEL: define internal void @test01caller
; CHECK: call void @test01callee.1(%__DDT_struct.type01b* byval(%__DDT_struct.type01b) %p)

; Remapping the dependent type will require the attribute type to be updated
define internal void @test01callee(%struct.type01b* byval(%struct.type01b) %in) {
  ret void
}

; Test updating the 'byref' attribute.
define internal void @test02caller() {
  %alloc = alloca %struct.type01b*
  %p = load %struct.type01b*, %struct.type01b** %alloc
  call void @test02callee(%struct.type01b* byref(%struct.type01b) %p)
  ret void
}
; CHECK-LABEL: define internal void @test02caller
; CHECK: call void @test02callee.2(%__DDT_struct.type01b* byref(%__DDT_struct.type01b) %p)

define internal void @test02callee(%struct.type01b* byref(%struct.type01b) %test.result) {
  %f = getelementptr %struct.type01b, %struct.type01b* %test.result, i64 0, i32 0
  store i32 0, i32* %f
  ret void
}

; Test updating the 'sret' attribute.
define internal void @test03caller() {
  %alloc = alloca %struct.type01b*
  %p = load %struct.type01b*, %struct.type01b** %alloc
  call void @test03callee(%struct.type01b* sret(%struct.type01b) %p)
  ret void
}
; CHECK-LABEL: define internal void @test03caller
; CHECK: call void @test03callee.3(%__DDT_struct.type01b* sret(%__DDT_struct.type01b) %p)

define internal void @test03callee(%struct.type01b* sret(%struct.type01b) %test.result) {
  %f = getelementptr %struct.type01b, %struct.type01b* %test.result, i64 0, i32 0
  store i32 0, i32* %f
  ret void
}

; Test updating the 'preallocated' attribute on a cloned routine.
define i32 @test04callee(<{ %struct.type01a, i32, %struct.type01a }>* preallocated(<{ %struct.type01a, i32, %struct.type01a }>) %p) {
  ret i32 0
}

; Cloned functions are printed last.

; CHECK: define internal void @test01callee.1(%__DDT_struct.type01b* byval(%__DDT_struct.type01b) %in)
; CHECK: define internal void @test02callee.2(%__DDT_struct.type01b* byref(%__DDT_struct.type01b) %test.result)
; CHECK: define internal void @test03callee.3(%__DDT_struct.type01b* sret(%__DDT_struct.type01b) %test.result)
; CHECK: define internal i32 @test04callee.4(<{ %__DTT_struct.type01a, i32, %__DTT_struct.type01a }>* preallocated(<{ %__DTT_struct.type01a, i32, %__DTT_struct.type01a }>) %p)

; UNSUPPORTED: enable-opaque-pointers

; This test verifies that the base class clones functions which have parameter
; types or return values modified as a result of replacing types.
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.type01a,struct.type02a 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -S -passes='internalize,dtrans-optbasetest' -dtrans-optbasetest-typelist=struct.type01a,struct.type02a 2>&1 | FileCheck %s

; Test when base class is used without dtrans analysis parameter to
; be sure all the types and dependent types are found without relying
; on the analysis pass.
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.type01a,struct.type02a -dtrans-optbasetest-use-analysis=false 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -S -passes='internalize,dtrans-optbasetest' -dtrans-optbasetest-typelist=struct.type01a,struct.type02a -dtrans-optbasetest-use-analysis=false 2>&1 | FileCheck %s

%struct.type01a = type { i32, i32, i32 }
%struct.type01b = type { i32, %struct.type01a* }

%struct.type02a = type { i16, i16, i16 }
%struct.type02b = type { i16, %struct.type02a* }

%struct.nochangetype = type { i64, i32, i16, i8 }

; CHECK-DAG: %__DDT_struct.type01b = type { i32, %__DTT_struct.type01a* }
; CHECK-DAG: %__DDT_struct.type02b = type { i16, %__DTT_struct.type02a* }


; Test with pointer type that is converted passed as a parameter.
; The types in the calling function should be changed, and the called
; function should be cloned. The original called function should be
; completely removed (i.e. no 'define' or 'declare' version of it)
define void @test01callee(%struct.type01b* %in) {
  ret void
}
define void @test01caller() {
  %a = alloca %struct.type01b*
  %p = load %struct.type01b*, %struct.type01b** %a
  call void @test01callee(%struct.type01b* %p)
  ret void
}
; CHECK-NOT: void @test01callee(%struct.type01b*)
; CHECK: define internal void @test01caller() {
; CHECK: %a = alloca %__DDT_struct.type01b*
; CHECK: %p = load %__DDT_struct.type01b*, %__DDT_struct.type01b** %a
; CHECK: call void @test01callee.{{[0-9]+}}(%__DDT_struct.type01b* %p)


; Test with multiple parameters that are converted being passed as parameters.
; The types in the calling function should be changed, and the called
; function should be cloned.
define void @test02callee(%struct.type01b* %in1, %struct.type02b* %in2) {
  ret void
}
define void @test02caller() {
  %a1 = alloca %struct.type01b*
  %p1 = load %struct.type01b*, %struct.type01b** %a1
  %a2 = alloca %struct.type02b*
  %p2 = load %struct.type02b*, %struct.type02b** %a2
  call void @test02callee(%struct.type01b* %p1, %struct.type02b* %p2)
  ret void
}
; CHECK-NOT: void @test02callee(%struct.type01b*)
; CHECK: define internal void @test02caller() {
; CHECK: %a1 = alloca %__DDT_struct.type01b*
; CHECK: %p1 = load %__DDT_struct.type01b*, %__DDT_struct.type01b** %a1
; CHECK: %a2 = alloca %__DDT_struct.type02b*
; CHECK: %p2 = load %__DDT_struct.type02b*, %__DDT_struct.type02b** %a2
; CHECK: call void @test02callee.{{[0-9]+}}(%__DDT_struct.type01b* %p1, %__DDT_struct.type02b* %p2)


; Test with multiple parameters where some are not converted types being
; passed as parameters.
; Verify only specific types in the calling function should be changed,
; and the called function gets cloned.
define void @test03callee(%struct.nochangetype* %in1, %struct.type02b* %in2) {
  ret void
}
define void @test03caller() {
  %a1 = alloca %struct.nochangetype*
  %p1 = load %struct.nochangetype*, %struct.nochangetype** %a1
  %a2 = alloca %struct.type02b*
  %p2 = load %struct.type02b*, %struct.type02b** %a2
  call void @test03callee(%struct.nochangetype* %p1, %struct.type02b* %p2)
  ret void
}
; CHECK-NOT: void @test03callee(%struct.nochangetype* %in1, %struct.type02b* %in2)
; CHECK: define internal void @test03caller() {
; CHECK: %a1 = alloca %struct.nochangetype*
; CHECK: %p1 = load %struct.nochangetype*, %struct.nochangetype** %a1
; CHECK: %a2 = alloca %__DDT_struct.type02b*
; CHECK: %p2 = load %__DDT_struct.type02b*, %__DDT_struct.type02b** %a2
; CHECK: call void @test03callee.{{[0-9]+}}(%struct.nochangetype* %p1, %__DDT_struct.type02b* %p2)


; Test with calling a vararg function with a parameter that is pointer
; to a type to be converted to be sure the cloned function is still
; a var arg function
define void @test04callee(%struct.type01b* %in, i32 %count, ...) {
  ret void
}
define void @test04caller() {
  %a = alloca %struct.type01b*
  %p = load %struct.type01b*, %struct.type01b** %a
  call void (%struct.type01b*, i32, ...) @test04callee(%struct.type01b* %p, i32 2, i32 4, i32 16)
  ret void
}
; CHECK-NOT: void @test04callee(%struct.type01b* %in, i32 %count, ...)
; CHECK: define internal void @test04caller() {
; CHECK: %a = alloca %__DDT_struct.type01b*
; CHECK: %p = load %__DDT_struct.type01b*, %__DDT_struct.type01b** %a
; CHECK: call void (%__DDT_struct.type01b*, i32, ...) @test04callee.{{[0-9]+}}(%__DDT_struct.type01b* %p, i32 2, i32 4, i32 16)


; Test with function that needs the return value type to be converted.
; This should cause a cloned function to be created.
define %struct.type01b* @test05callee() {
  %p = call i8* @malloc(i64 256)
  %ptype = bitcast i8* %p to %struct.type01b*
  ret %struct.type01b* %ptype
}

define void @test05caller() {
  %p = call %struct.type01b* @test05callee()
  ret void
}
; CHECK-NOT: %struct.type01b* @test05callee()
; CHECK: define internal void @test05caller() {
; CHECK: %p = call %__DDT_struct.type01b* @test05callee.{{[0-9]+}}()


; Test that a function that does not need cloning does not get cloned.
define void @test06callee(%struct.nochangetype* %in) {
  ret void
}
define void @test06caller() {
  %a = alloca %struct.nochangetype*
  %p = load %struct.nochangetype*, %struct.nochangetype** %a
  call void @test06callee(%struct.nochangetype* %p)
  ret void
}
; CHECK: define internal void @test06callee(%struct.nochangetype* %in)
; CHECK: define internal void @test06caller() {
; CHECK: %a = alloca %struct.nochangetype*
; CHECK: %p = load %struct.nochangetype*, %struct.nochangetype** %a
; CHECK: call void @test06callee(%struct.nochangetype* %p)


; We expect all the clone functions to come out at the end of the input because
; they are new functions added to the module. But, we don't care which order
; they get processed in, which could changes the suffixes or order, so we
; use CHECK-DAG and regular expressions here.

; CHECK-DAG: define internal void @test01callee.{{[0-9]+}}(%__DDT_struct.type01b* %in)
; CHECK-DAG: define internal void @test02callee.{{[0-9]+}}(%__DDT_struct.type01b* %in1, %__DDT_struct.type02b* %in2)
; CHECK-DAG: define internal void @test03callee.{{[0-9]+}}(%struct.nochangetype* %in1, %__DDT_struct.type02b* %in2)
; CHECK-DAG: define internal void @test04callee.{{[0-9]+}}(%__DDT_struct.type01b* %in, i32 %count, ...)
; CHECK-DAG: define internal %__DDT_struct.type01b* @test05callee.{{[0-9]+}}()

declare i8* @malloc(i64)

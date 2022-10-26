; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -opaque-pointers -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the base class updates the parameter attribute types
; when the type used is remapped.

%struct.test01a = type { i32, i32, i32 }
%struct.test01b = type { i32, %struct.test01a }
; CHECK-DAG: %__DTT_struct.test01a = type { i32, i32, i32 }
; CHECK-DAG: %__DDT_struct.test01b = type { i32, %__DTT_struct.test01a }

; Test updating the 'byval' attribute.
define internal void @test01caller() {
  %alloc = alloca %struct.test01b*, !intel_dtrans_type !3
  %p = load %struct.test01b*, %struct.test01b** %alloc
  call void @test01callee(%struct.test01b* byval(%struct.test01b) %p)
  ret void
}
; The CloneFunctionInto functionality should handle the modification of
; the function call. Check this to be sure.
; CHECK-LABEL: define internal void @test01caller
; CHECK: call void @test01callee
; CHECK-SAME: byval(%__DDT_struct.test01b) %p

; Remapping the dependent type will require the attribute type to be updated
define internal void @test01callee(%struct.test01b* "intel_dtrans_func_index"="1" byval(%struct.test01b) %in) !intel.dtrans.func.type !4 {
  ret void
}
; The function will not be cloned when using opaque pointers.
; CHECK-OPAQUE: define internal void @test01callee
; CHECK-OPAQUE-SAME: byval(%__DDT_struct.test01b)

; Test updating the 'byref' attribute.
define internal void @test02caller() {
  %alloc = alloca %struct.test01b*, !intel_dtrans_type !3
  %p = load %struct.test01b*, %struct.test01b** %alloc
  call void @test02callee(%struct.test01b* byref(%struct.test01b) %p)
  ret void
}
; CHECK-LABEL: define internal void @test02caller
; CHECK: call void @test02callee
; CHECK-SAME: byref(%__DDT_struct.test01b) %p

define internal void @test02callee(%struct.test01b* "intel_dtrans_func_index"="1" byref(%struct.test01b) %test.result) !intel.dtrans.func.type !5 {
  %f = getelementptr %struct.test01b, %struct.test01b* %test.result, i64 0, i32 0
  store i32 0, i32* %f
  ret void
}
; The function will not be cloned when using opaque pointers.
; CHECK-OPAQUE: define internal void @test02callee
; CHECK-OPAQUE-SAME: byref(%__DDT_struct.test01b)

; Test updating the 'sret' attribute.
define internal void @test03caller() {
  %alloc = alloca %struct.test01b*, !intel_dtrans_type !3
  %p = load %struct.test01b*, %struct.test01b** %alloc
  call void @test03callee(%struct.test01b* sret(%struct.test01b) %p)
  ret void
}
; CHECK-LABEL: define internal void @test03caller
; CHECK: call void @test03callee
; CHECK-SAME: sret(%__DDT_struct.test01b) %p

define internal void @test03callee(%struct.test01b* "intel_dtrans_func_index"="1" sret(%struct.test01b) %test.result) !intel.dtrans.func.type !6 {
  %f = getelementptr %struct.test01b, %struct.test01b* %test.result, i64 0, i32 0
  store i32 0, i32* %f
  ret void
}
; The function will not be cloned when using opaque pointers.
; CHECK-OPAQUE: define internal void @test03callee
; CHECK-OPAQUE-SAME: sret(%__DDT_struct.test01b)

; Test updating the 'preallocated' attribute on a cloned routine.
define i32 @test04callee(<{ %struct.test01a, i32, %struct.test01a }>* "intel_dtrans_func_index"="1" preallocated(<{ %struct.test01a, i32, %struct.test01a }>) %p) !intel.dtrans.func.type !13 {
  ret i32 0
}
; The function will not be cloned when using opaque pointers.
; CHECK-OPAQUE: define i32 @test04callee
; CHECK-OPAQUE-SAME: preallocated(<{ %__DTT_struct.test01a, i32, %__DTT_struct.test01a }>

; Cloned functions are printed last.

; CHECK-NONOPAQUE: define internal void @test01callee.1
; CHECK-NONOPAQUE-SAME: byval(%__DDT_struct.test01b)

; CHECK-NONOPAQUE: define internal void @test02callee.2
; CHECK-NONOPAQUE-SAME: byref(%__DDT_struct.test01b)

; CHECK-NONOPAQUE: define internal void @test03callee.3
; CHECK-NONOPAQUE-SAME: sret(%__DDT_struct.test01b)

; CHECK-NONOPAQUE: define internal i32 @test04callee.4
; CHECK-NONOPAQUE-SAME: preallocated(<{ %__DTT_struct.test01a, i32, %__DTT_struct.test01a }>

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = distinct !{!3}
!7 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!10 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test01a }
!11 = !{!12, i32 1}
!12 = !{!"L", i32 3, !2, !1, !2 }
!13 = distinct !{!11}

!intel.dtrans.types = !{!9, !10}

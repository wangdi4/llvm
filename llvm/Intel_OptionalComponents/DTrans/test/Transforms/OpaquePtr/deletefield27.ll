; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete field pass correctly transforms
; structures with global arrays of instances with non-default initializers
; and operator getelementptr accesses.

%struct.test = type { i32, i64, i32 }
@g_test = private global [4 x %struct.test] [
  %struct.test { i32 100, i64 1000, i32 10000 },
  %struct.test { i32 200, i64 2000, i32 20000 },
  %struct.test { i32 300, i64 3000, i32 30000 },
  %struct.test { i32 400, i64 4000, i32 40000 } ]

define i32 @f() {
  ; read A and C
  %valA = load i32, ptr getelementptr ([4 x %struct.test], ptr @g_test, i64 0, i32 2, i32 0)
  %valC = load i32, ptr getelementptr ([4 x %struct.test], ptr @g_test, i64 0, i32 2, i32 2)
  %sum = add i32 %valA, %valC

  ; write B
  store i64 3, ptr getelementptr ([4 x %struct.test], ptr @g_test, i64 0, i32 2, i32 1)

  ret i32 %sum
}

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  %tmp = call i32 @f()
  ret i32 0
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK: @g_test = private global [4 x %__DFT_struct.test] [
; CHECK-SAME: %__DFT_struct.test { i32 100, i32 10000 },
; CHECK-SAME: %__DFT_struct.test { i32 200, i32 20000 },
; CHECK-SAME: %__DFT_struct.test { i32 300, i32 30000 },
; CHECK-SAME: %__DFT_struct.test { i32 400, i32 40000 }]

; CHECK-LABEL: define i32 @f() {
; CHECK: %valA = load i32, ptr getelementptr inbounds ([4 x %__DFT_struct.test], ptr @g_test, i64 0, i32 2, i32 0)
; CHECK: %valC = load i32, ptr getelementptr inbounds ([4 x %__DFT_struct.test], ptr @g_test, i64 0, i32 2, i32 1)
; CHECK-NOT: store i64 3

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!5}

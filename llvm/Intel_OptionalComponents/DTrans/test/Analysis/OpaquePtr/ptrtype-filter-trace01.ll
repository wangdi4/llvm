; REQUIRES: asserts

; Test the ability to filter verbose messages based on the function
; name list specified by -dtrans-pta-filter-funcs

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -dtrans-pta-filter-print-funcs=test01,test03 < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -dtrans-pta-filter-print-funcs=test01,test03 < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR


; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test = type { i64, i32, i32 }

; This function will have traces enabled.
define internal void @test01() {
  %test1_local = alloca %struct.test
  %test1_gep = getelementptr %struct.test, %struct.test* %test1_local, i64 0, i32 1
  ret void
}
; CHECK: Added alias [DECL]: [test01]   %test1_local = alloca %struct.test, align 8 -- %struct.test*
; CHECK-CUR: Added alias [DECL]: [test01]   %test1_gep = getelementptr %struct.test, %struct.test* %test1_local, i64 0, i32 1 -- i32*
; CHECK-CUR: Added element:[DECL]: [test01]   %test1_gep = getelementptr %struct.test, %struct.test* %test1_local, i64 0, i32 1 -- %struct.test @ 1
; CHECK-FUT: Added alias [DECL]: [test01]   %test1_gep = getelementptr %struct.test, p0 %test1_local, i64 0, i32 1 -- i32*
; CHECK-FUT: Added element:[DECL]: [test01]   %test1_gep = getelementptr %struct.test, p0 %test1_local, i64 0, i32 1 -- %struct.test @ 1

; This function will NOT have traces enabled.
define internal void @test02() {
  %test2_local = alloca %struct.test
  %test2_gep = getelementptr %struct.test, %struct.test* %test2_local, i64 0, i32 1
  ret void
}
; CHECK-NOT: Added alias [DECL]: [test02]   %test2_local = alloca %struct.test -- %struct.test*
; CHECK-CUR-NOT: Added alias [DECL]: [test02]   %test2_gep = getelementptr %struct.test, %struct.test* %test2_local, i64 0, i32 1 -- i32*
; CHECK-CUR-NOT: Added element:[DECL]: [test02]   %test2_gep = getelementptr %struct.test, %struct.test* %test2_local, i64 0, i32 1 -- %struct.test @ 1
; CHECK-FUT-NOT: Added alias [DECL]: [test02]   %test2_gep = getelementptr %struct.test, p0 %test2_local, i64 0, i32 1 -- i32*
; CHECK-FUT-NOT: Added element:[DECL]: [test02]   %test2_gep = getelementptr %struct.test, p0 %test2_local, i64 0, i32 1 -- %struct.test @ 1

; This function will have traces enabled.
define internal void @test03() {
  %test3_local = alloca %struct.test
  %test3_gep = getelementptr %struct.test, %struct.test* %test3_local, i64 0, i32 1
  ret void
}
; CHECK: Added alias [DECL]: [test03]   %test3_local = alloca %struct.test, align 8 -- %struct.test*
; CHECK-CUR: Added alias [DECL]: [test03]   %test3_gep = getelementptr %struct.test, %struct.test* %test3_local, i64 0, i32 1 -- i32*
; CHECK-CUR: Added element:[DECL]: [test03]   %test3_gep = getelementptr %struct.test, %struct.test* %test3_local, i64 0, i32 1 -- %struct.test @ 1
; CHECK-FUT: Added alias [DECL]: [test03]   %test3_gep = getelementptr %struct.test, p0 %test3_local, i64 0, i32 1 -- i32*
; CHECK-FUT: Added element:[DECL]: [test03]   %test3_gep = getelementptr %struct.test, p0 %test3_local, i64 0, i32 1 -- %struct.test @ 1

declare i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !2} ; { i64, i32, i32 }

!dtrans_types = !{!3}

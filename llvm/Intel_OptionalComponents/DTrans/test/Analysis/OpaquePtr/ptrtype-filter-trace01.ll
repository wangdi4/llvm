; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; Test the ability to filter verbose messages based on the function
; name list specified by -dtrans-pta-filter-funcs

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -dtrans-pta-filter-print-funcs=test01,test03 < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -dtrans-pta-filter-print-funcs=test01,test03 < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -dtrans-pta-filter-print-funcs=test01,test03 < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -dtrans-pta-filter-print-funcs=test01,test03 < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE


; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
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
; CHECK-NONOPAQUE: Added alias [DECL]: [test01]   %test1_gep = getelementptr %struct.test, %struct.test* %test1_local, i64 0, i32 1 -- i32*
; CHECK-NONOPAQUE: Added element:[DECL]: [test01]   %test1_gep = getelementptr %struct.test, %struct.test* %test1_local, i64 0, i32 1 -- %struct.test @ 1
; CHECK-OPAQUE: Added alias [DECL]: [test01]   %test1_gep = getelementptr %struct.test, ptr %test1_local, i64 0, i32 1 -- i32*
; CHECK-OPAQUE: Added element:[DECL]: [test01]   %test1_gep = getelementptr %struct.test, ptr %test1_local, i64 0, i32 1 -- %struct.test @ 1

; This function will NOT have traces enabled.
define internal void @test02() {
  %test2_local = alloca %struct.test
  %test2_gep = getelementptr %struct.test, %struct.test* %test2_local, i64 0, i32 1
  ret void
}
; CHECK-NOT: Added alias [DECL]: [test02]   %test2_local = alloca %struct.test -- %struct.test*
; CHECK-NONOPAQUE-NOT: Added alias [DECL]: [test02]   %test2_gep = getelementptr %struct.test, %struct.test* %test2_local, i64 0, i32 1 -- i32*
; CHECK-NONOPAQUE-NOT: Added element:[DECL]: [test02]   %test2_gep = getelementptr %struct.test, %struct.test* %test2_local, i64 0, i32 1 -- %struct.test @ 1
; CHECK-OPAQUE-NOT: Added alias [DECL]: [test02]   %test2_gep = getelementptr %struct.test, ptr %test2_local, i64 0, i32 1 -- i32*
; CHECK-OPAQUE-NOT: Added element:[DECL]: [test02]   %test2_gep = getelementptr %struct.test, ptr %test2_local, i64 0, i32 1 -- %struct.test @ 1

; This function will have traces enabled.
define internal void @test03() {
  %test3_local = alloca %struct.test
  %test3_gep = getelementptr %struct.test, %struct.test* %test3_local, i64 0, i32 1
  ret void
}
; CHECK: Added alias [DECL]: [test03]   %test3_local = alloca %struct.test, align 8 -- %struct.test*
; CHECK-NONOPAQUE: Added alias [DECL]: [test03]   %test3_gep = getelementptr %struct.test, %struct.test* %test3_local, i64 0, i32 1 -- i32*
; CHECK-NONOPAQUE: Added element:[DECL]: [test03]   %test3_gep = getelementptr %struct.test, %struct.test* %test3_local, i64 0, i32 1 -- %struct.test @ 1
; CHECK-OPAQUE: Added alias [DECL]: [test03]   %test3_gep = getelementptr %struct.test, ptr %test3_local, i64 0, i32 1 -- i32*
; CHECK-OPAQUE: Added element:[DECL]: [test03]   %test3_gep = getelementptr %struct.test, ptr %test3_local, i64 0, i32 1 -- %struct.test @ 1

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !2} ; { i64, i32, i32 }

!intel.dtrans.types = !{!5}

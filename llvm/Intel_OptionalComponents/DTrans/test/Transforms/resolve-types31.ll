; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes correctly combines
; types that have the same base name but different suffixes if the types
; have the same layout. In the case that a type is declared as being
; passed to an external routine that will not be used, it checks that
; the types still get combined.

; These types should all be combined.
%class.test01 = type { i32, i64, i32 }
%class.test01.123 = type { i32, i64, i32 }
%class.test01.456 = type { i32, i64, i32 }

; CHECK: %__DTRT_class.test01 = type { i32, i64, i32 }

; Because there are no uses of this external function, it should not prevent
; resolve types from combining %class.test01.123 and %class.test01.456
declare %class.test01* @llvm.ssa.copy.p0s_class.test01(%class.test01* returned)

define void @test01_b(%class.test01.123* %p) {
  ret void
}
; CHECK-NOT: void @test01_b(%class.test01.123* %p)

define void @test01_c(%class.test01.456* %p) {
  ret void
}
; CHECK-NOT: void @test01_c(%class.test01.456* %p)

define void @test01() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %class.test01*
  call void bitcast (void (%class.test01.123*)* @test01_b
              to void (%class.test01*)*) (%class.test01* %p)
  call void bitcast (void (%class.test01.456*)* @test01_c
              to void (%class.test01*)*) (%class.test01* %p)
  call void @free(i8* %buf)
  ret void
}

; CHECK-LABEL: void @test01()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %__DTRT_class.test01*
; CHECK:  call void @test01_b.1(%__DTRT_class.test01* %p)
; CHECK:  call void @test01_c.2(%__DTRT_class.test01* %p)
; CHECK:  call void @free(i8* %buf)

declare i8* @malloc(i32)
declare void @free(i8*)

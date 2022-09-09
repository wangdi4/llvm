; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test is to verify the checks on uses of the pointer returned by the
; allocation call for types being transformed. In this case, the pointer is
; used in a non-null comparison, and will therefore be rejected.

%struct.test01 = type { i16, i64*, i32, i64 }
%struct.test02 = type { i32, i32, i64 }
; CHECK-DAG: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test01
; CHECK-DAG: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test02

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i8* null)
  call void @test02()
  ret i32 0
}

; Verify that comparison requires constant NULL to allow transformation.
define void @test01(i8* %other) {
  %mem1 = call i8* @malloc(i64 320)

  ; This could be safe, but there is no need to support
  ; anything but a comparison with NULL currently, and
  ; and it's not expected the code would contain other
  ; comparisons.
  %cmp1 = icmp eq i8* %mem1, %other

  %st_mem1 = bitcast i8* %mem1 to %struct.test01*
  ret void
}

; Verify that only [non]equality comparisons with NULL are supported.
define void @test02() {
  %mem1 = call i8* @malloc(i64 160)
  %cmp1 = icmp ugt i8* %mem1, null
  %st_mem1 = bitcast i8* %mem1 to %struct.test02*
  ret void
}

declare i8* @malloc(i64)

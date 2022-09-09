; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test is to verify the checks on uses of the pointer returned by the
; allocation call for types being transformed. In this case, the pointer is
; used for phi/select instructions, and will therefore be rejected. These
; cases could be supported in the future, but are not needed at the moment.

%struct.test01 = type { i16, i64*, i32, i64 }
%struct.test02 = type { i32, i32, i64 }
; CHECK-DAG: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test01
; CHECK-DAG: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test02

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i8* null)
  call void @test02()
  ret i32 0
}

; Verify that select instructions on the result of the allocation rejected
; the structure from being transformed.
define void @test01(i8* %other) {
  %mem1 = call i8* @malloc(i64 320)
  %st_mem1 = bitcast i8* %mem1 to %struct.test01*

  ; Because we are not tracking through the select instructions that uses
  ; of the result of the select, this should cause the structure to be
  ; rejected.
  %sel = select i1 undef, i8* %mem1, i8* null
  ret void
}

; Verify that phi instructions on the result of the allocation rejected
; the structure from being transformed.
define void @test02() {
entry:
  br i1 undef, label %alloc, label %merge

alloc:
  %mem1 = call i8* @malloc(i64 160)
  %st_mem1 = bitcast i8* %mem1 to %struct.test02*
  br label %merge

merge:
  ; Because we are not tracking through the phi instructions that use the
  ; allocation call result, this should cause the structure to be rejected.
  %mem2 = phi i8* [%mem1, %alloc], [null, %entry]
  ret void
}

declare i8* @malloc(i64)

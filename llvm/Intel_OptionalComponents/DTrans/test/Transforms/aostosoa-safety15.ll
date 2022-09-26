; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test is to verify the checks on uses of the pointer returned by the
; allocation call for types being transformed. In this case, the pointer passed
; to a memfunc routine. This is not currently supported, but could be in the
; future, but may require additional support.

%struct.test01 = type { i32, i32, i64 }
; CHECK: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test01

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 15, i8* null)
  ret i32 0
}

define void @test01(i64 %num, i8* %other) {
  %mem1 = call i8* @malloc(i64 160)
  ; This could be considered safe, but would require changing
  ; the size parameter of the memfunc to reflect the updated
  ; allocation size. Currently, we will reject the structure due
  ; to this.
  call void @llvm.memset.p0i8.i64(i8* %mem1, i8 0, i64 160, i1 false)

  store i8* %mem1, i8** bitcast (%struct.test01** @g_test01ptr to i8**)
  ret void
}

declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

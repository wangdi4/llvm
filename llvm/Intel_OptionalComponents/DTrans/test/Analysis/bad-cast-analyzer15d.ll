; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-bca -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-bca -disable-output 2>&1 | FileCheck %s

; Test that %struct.test01a and %struct.test01b aren't marked as bad casting
; since the BitCast will be used as a dead argument. This is the same test case
; as bad-cast-analyzer15.ll but with the DTrans bad casting analysis trace.

%struct.test01a = type { i32, i64, i32 }
%struct.test01b = type { i32, i64, i32 }

; CHECK-NOT: dtrans-bca: Bad casting -- unsafe cast of aliased pointer

define void @test01_a(%struct.test01b* %p2) {
  ret void
}

define void @test01(%struct.test01a* %p) {
  call void bitcast (void (%struct.test01b*)* @test01_a
               to void (%struct.test01a*)*) (%struct.test01a* %p)
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test01a*
  call void @test01(%struct.test01a* %p)
  call void @free(i8* %buf)
  ret i32 0
}

declare i8* @malloc(i32)
declare void @free(i8*)
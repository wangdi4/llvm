; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes=dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that memset size is adjusted and fields are removed from both types
%struct.test01t = type { i16, i32 }
%struct.test01 = type { i32, i16, %struct.test01t }
define i32 @test01(%struct.test01* %s1, %struct.test01* %s2) {
  %p1 = bitcast %struct.test01* %s1 to i8*
  %p2 = bitcast %struct.test01* %s2 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p1, i8 0, i64 16, i1 false)
  %p3 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %r1 = load i32, i32* %p3
  %p4 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 1
  %r2 = load i32, i32* %p4
  %r = add i32 %r1, %r2
  ret i32 %r
}

; CHECK: %__DFT_struct.test01 = type { i32, %__DFT_struct.test01t }
; CHECK: %__DFT_struct.test01t = type { i32 }
; CHECK: call void @llvm.memset
; CHECK-SAME: i64 8
; CHECK: getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %s1, i64 0, i32 0
; CHECK: getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %s1, i64 0, i32 1, i32 0

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)


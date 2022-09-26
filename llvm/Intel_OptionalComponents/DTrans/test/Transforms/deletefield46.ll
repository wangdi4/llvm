; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes=dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the memcpy sizes are adjusted for cases of nested structures when
; fields are deleted from both the outer and inner structures.

; One field should be removed from each structure.
%struct.test01 = type { i32, i16, %struct.test01t }
%struct.test01t = type { i16, i32 }
; CHECK: %__DFT_struct.test01 = type { i32, %__DFT_struct.test01t }
; CHECK: %__DFT_struct.test01t = type { i32 }

; This case should update the memcpy size parameter based on the new outer
; structure type.
define i32 @test01(%struct.test01* %s1, %struct.test01* %s2) {
  %p1 = bitcast %struct.test01* %s1 to i8*
  %p2 = bitcast %struct.test01* %s2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 16, i1 false)
  %p3 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %r1 = load i32, i32* %p3
  %p4 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 1
  %r2 = load i32, i32* %p4
  %r = add i32 %r1, %r2
  ret i32 %r
}
; CHECK-LABEL: define internal i32 @test01
; CHECK: call void @llvm.memcpy
; CHECK-SAME: i64 8
; CHECK: getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %s1, i64 0, i32 0
; CHECK: getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %s1, i64 0, i32 1, i32 0

; This case should update the memcpy size parameter based on the new inner
; structure type.
define i32 @test02(%struct.test01t* %s1, %struct.test01* %s2) {
  %pDst = bitcast %struct.test01t* %s1 to i8*
  %pField = getelementptr %struct.test01, %struct.test01* %s2, i64 0, i32 2
  %pSrc = bitcast %struct.test01t* %pField to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret i32 0
}
; CHECK-LABEL: define internal i32 @test02
; CHECK: getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %s2, i64 0, i32 1
; CHECK: call void @llvm.memcpy
; CHECK-SAME: i64 4

; This case should update the memcpy size parameter based on the new inner
; structure type.
define i32 @test03(%struct.test01t* %s1, %struct.test01* %s2) {
  %pSrc = bitcast %struct.test01t* %s1 to i8*
  %pField = getelementptr %struct.test01, %struct.test01* %s2, i64 0, i32 2
  %pDst = bitcast %struct.test01t* %pField to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret i32 0
}
; CHECK-LABEL: define internal i32 @test03
; CHECK: getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %s2, i64 0, i32 1
; CHECK: call void @llvm.memcpy
; CHECK-SAME: i64 4

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

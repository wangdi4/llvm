; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes="require<dtransanalysis>" -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Check that memset call adds a zero value to each field of %struct.type0.
; type { i32, i32, i32 }, memset size 12 bytes.

; CHECK-LABEL: LLVMType: %struct.type0 = type
; CHECK: Multiple Value: [ 0, 6 ]
; CHECK-SAME: complete
; CHECK: Multiple Value: [ 0, 8 ]
; CHECK-SAME: complete
; CHECK: Multiple Value: [ 0, 16 ]
; CHECK-SAME: complete
; CHECK: Safety data:

%struct.type0 = type { i32, i32, i32 }
@gType0 = internal dso_local global %struct.type0 { i32 6, i32 8, i32 16 }, align 4
define dso_local i32 @test0() {
  %p1 = getelementptr %struct.type0, %struct.type0* @gType0, i64 0, i32 0
  %p2 = bitcast i32* %p1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p2, i8 0, i64 12, i1 false)
  ret i32 0
}

; The test checks that only first two fields are impacted by the bad-size memset call.
; type { i32, i32, i32 }, memset size 6 bytes.

; CHECK-LABEL: LLVMType: %struct.type1 = type
; CHECK: Multiple Value: [ 0, 6 ]
; CHECK-SAME: complete
; CHECK: Multiple Value: [ 8 ]
; CHECK-SAME: incomplete
; CHECK: Single Value: i32 16
; CHECK: Safety data:
; CHECK-SAME: Bad memfunc size

%struct.type1 = type { i32, i32, i32 }
@gType1 = internal dso_local global %struct.type1 { i32 6, i32 8, i32 16 }, align 4
define dso_local i32 @test1() {
  %p1 = getelementptr %struct.type1, %struct.type1* @gType1, i64 0, i32 0
  %p2 = bitcast i32* %p1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p2, i8 0, i64 6, i1 false)
  ret i32 0
}

; The test checks that only first two fields are impacated by the partial memset call.
; type { i32, i32, i32 }, memset size 8 bytes.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.type2 = type
; CHECK: Multiple Value: [ 0, 6 ]
; CHECK-SAME: complete
; CHECK: Multiple Value: [ 0, 8 ]
; CHECK-SAME: complete
; CHECK: Single Value: i32 16
; CHECK: Safety data:
; CHECK-SAME: Memfunc partial write

%struct.type2 = type { i32, i32, i32 }
@gType2 = internal dso_local global %struct.type2 { i32 6, i32 8, i32 16 }, align 4
define dso_local i32 @test2() {
  %p1 = getelementptr %struct.type2, %struct.type2* @gType2, i64 0, i32 0
  %p2 = bitcast i32* %p1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p2, i8 0, i64 8, i1 false)
  ret i32 0
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)


; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-outofboundsok=true -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes="require<dtransanalysis>" -dtrans-print-types -dtrans-outofboundsok=true -disable-output < %s 2>&1 | FileCheck %s

; Checks that the constant value set of the non-address-taken fields
; in the structure changes to 'incomplete' under DTransOutOfBoundsOK=true.

; CHECK-LABEL: LLVMType: %struct.type0 = type
; CHECK: Multiple Value: [ 0, 6 ]
; CHECK-SAME: incomplete
; CHECK: Multiple Value: [ 0, 8 ]
; CHECK-SAME: incomplete
; CHECK: Multiple Value: [ 0, 16 ]
; CHECK-SAME: incomplete
; CHECK: Safety data:

%struct.type0 = type { i32, i32, i32 }
@gType0 = internal dso_local global %struct.type0 { i32 6, i32 8, i32 16 }, align 4
define dso_local i32 @test0() {
  %p1 = getelementptr %struct.type0, %struct.type0* @gType0, i64 0, i32 0
  %p2 = bitcast i32* %p1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p2, i8 0, i64 12, i1 false)
  %p3 = getelementptr %struct.type0, %struct.type0* @gType0, i64 0, i32 2
  call void @toFour(i32* %p3)
  ret i32 0
}

define dso_local void @toFour(i32* %arg) {
  store i32 4, i32* %arg
  ret void;
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)

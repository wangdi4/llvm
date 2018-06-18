; RUN: opt -dtransanalysis -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="require<dtransanalysis>" -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; The test checks that only first two fields are impacted by the partial memset call.
; type { i32, i32, i32 }, memset size 8 bytes.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.type1 = type
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Single Value: i32 16
; CHECK: Safety data:
; CHECK-SAME: Memfunc partial write

%struct.type1 = type { i32, i32, i32 }

@gType1 = internal dso_local global %struct.type1 { i32 6, i32 8, i32 16 }, align 4

define dso_local i32 @main() {
  %p1 = getelementptr %struct.type1, %struct.type1* @gType1, i64 0, i32 0
  %p2 = bitcast i32* %p1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p2, i8 0, i64 8, i1 false)
  ret i32 0
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)


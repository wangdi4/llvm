; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s

; Test the LocalPointerAnalysis tracking of GEPs that access field elements
; and byte-offsets into the aggregate

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; Test accesses to fields and padding bytes.
%struct.atest01sub = type { i64, i64, i32, i64 }
%struct.atest01 = type { i64, %struct.atest01sub, i64 }
define void @atest01() {
  %mem = call i8* @malloc(i64 48)
  %st_ptr = bitcast i8* %mem to %struct.atest01*

  ; Track by element number
  %field1 = getelementptr %struct.atest01, %struct.atest01 *%st_ptr, i64 0, i32 1

  ; Track by byte-offset into structure
  %pad = getelementptr i8, i8* %mem, i64 28
  call void @llvm.memset.p0i8.i64(i8* %pad, i8 0, i64 12, i1 false)

  ; Track byte-flattened GEP that resolves to field element start
  %bgep = getelementptr i8, i8* %mem, i64 32
  call void @llvm.memset.p0i8.i64(i8* %bgep, i8 0, i64 8, i1 false)

  ; Track by element number
  %fieldsub3 = getelementptr %struct.atest01sub, %struct.atest01sub* %field1, i64 0, i32 3
  %subval = load i64, i64* %fieldsub3

  ; Track by byte-offset into structure
  %mem2 = bitcast %struct.atest01sub* %field1 to i8*
  %pad2 = getelementptr i8, i8* %mem2, i64 20
  call void @llvm.memset.p0i8.i64(i8* %pad2, i8 0, i64 12, i1 false)

  ; Track byte-flattened GEP that resolves to field element start
  %bgep2 = getelementptr i8, i8* %mem, i64 24
  call void @llvm.memset.p0i8.i64(i8* %bgep2, i8 0, i64 8, i1 false)

  ret void
}
; CHECK-LABEL: void @atest01()
; Field element should not report a ByteOffset value
; CHECK: %field1 = getelementptr %struct.atest01, %struct.atest01* %st_ptr, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.atest01sub*
; CHECK:      Element pointees:
; CHECK:        %struct.atest01 @ 1
; CHECK-NOT: ByteOffset:

; Padding location should report a ByteOffset value
; CHECK: %pad = getelementptr i8, i8* %mem, i64 28
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i8*
; CHECK:      Element pointees:
; CHECK:        %struct.atest01 @ not-field ByteOffset: 28

; Byte-GEP to field element should not report a ByteOffset value
; CHECK: %bgep = getelementptr i8, i8* %mem, i64 32
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:        i8*
; CHECK:      Element pointees:
; CHECK:        %struct.atest01sub @ 3
; CHECK-NOT: ByteOffset:

; Should not report a ByteOffset value
; CHECK: %fieldsub3 = getelementptr %struct.atest01sub, %struct.atest01sub* %field1, i64 0, i32 3
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      Element pointees:
; CHECK:        %struct.atest01sub @ 3
; CHECK-NOT: ByteOffset:

; Should report a ByteOffset value
; CHECK: %pad2 = getelementptr i8, i8* %mem2, i64 20
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i8*
; CHECK:      Element pointees:
; CHECK:        %struct.atest01sub @ not-field ByteOffset: 20


; Test with offset that is not to a padding location. This will result in a
; safety violation, but the pointer analyzer should track the GEP location.
%struct.atest02 = type { i64, i64, i64 }
define void @atest02() {
  %mem = call i8* @malloc(i64 24)
  %st_ptr = bitcast i8* %mem to %struct.atest02*

  ; Track by byte-offset into structure that is within element
  %bad = getelementptr i8, i8* %mem, i64 12
  call void @llvm.memset.p0i8.i64(i8* %bad, i8 0, i64 12, i1 false)

 ret void
}
; CHECK-LABEL: void @atest02()
; Should report a ByteOffset value
; CHECK: %bad = getelementptr i8, i8* %mem, i64 12
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i8*
; CHECK:      Element pointees:
; CHECK:        %struct.atest02 @ not-field ByteOffset: 12

declare dso_local noalias i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This tests analysis for memset cases that begin or end within the padding
; bytes between fields of structures.

; Test with valid memset that covers region starting from padding after the
; first field until the end of the structure.
%struct.atest01 = type { i32, i64, i8, i32, i32 }
define void @atest01() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.atest01*

   ; Clear all but the first 4 bytes
  %pad = getelementptr inbounds i8, i8* %mem, i64 4
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 28, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest01 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Memfunc partial write

; Test with valid memset that covers region starting from padding after the
; third field, and covers the next field.
%struct.atest02 = type { i32, i64, i8, i32, i32 }
define void @atest02() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.atest02*

  ; Clear padding, and 1 field
  %pad = getelementptr inbounds i8, i8* %mem, i64 17
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 7, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest02 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Memfunc partial write

; Test with access that covers fields, and the padding bytes that follow.
%struct.atest03 = type { i32, i64, i8, i32, i32 }
define void @atest03() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.atest03*

  %pad = getelementptr inbounds i8, i8* %mem, i64 8
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 12, i1 false)

  ret void
}

; Test with invalid memset that covers region starting before the structure
%struct.ztest01 = type { i32, i64, i8, i32, i32 }
define void @ztest01() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.ztest01*

  %pad = getelementptr inbounds i8, i8* %mem, i64 -2
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 34, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.ztest01 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Test with invalid memset that covers region starting after the structure
%struct.ztest02 = type { i32, i64, i8, i32, i32 }
define void @ztest02() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.ztest02*

  %pad = getelementptr inbounds i8, i8* %mem, i64 33
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 32, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.ztest02 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Test with memet that starts within the structure, but extends beyond the end
%struct.ztest03 = type { i32, i64, i8, i32, i32 }
define void @ztest03() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.ztest03*

  %pad = getelementptr inbounds i8, i8* %mem, i64 4
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 32, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.ztest03 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Bad memfunc size

; Test with memset just affecting padding bytes. This could be supported, but
; is not, since it's not expected to occur.
%struct.ztest04 = type { i32, i64, i8, i32, i32 }
define void @ztest04() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.ztest04*

  %pad = getelementptr inbounds i8, i8* %mem, i64 4
  call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 4, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.ztest04 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Bad memfunc size

; Test with use of padding field address for something other than memset.
%struct.ztest05 = type { i32, i64, i8, i32, i32 }
define void @ztest05() {
  %mem = call i8* @malloc(i64 32)
  %st_ptr = bitcast i8* %mem to %struct.ztest05*

  %pad = getelementptr inbounds i8, i8* %mem, i64 4
  store i8 1, i8* %pad

  ret void
}
; CHECK-LABEL: LLVMType: %struct.ztest05 = type { i32, i64, i8, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation


; Memfunc info is printed after the structure safety checks
; CHECK-LABEL: Function: atest01
; CHECK: Instruction:   call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 28, i1 false)
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: false
; CHECK:     PrePad:     4
; CHECK:     FirstField: 1
; CHECK:     LastField:  4
; CHECK:     PostPad:    4
; CHECK:     Type: %struct.atest01 = type { i32, i64, i8, i32, i32 }

; CHECK-LABEL: Function: atest02
; CHECK: Instruction:   call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 7, i1 false)
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: false
; CHECK:     PrePad:     3
; CHECK:     FirstField: 3
; CHECK:     LastField:  3
; CHECK:     PostPad:    0
; CHECK:     Type: %struct.atest02 = type { i32, i64, i8, i32, i32 }

; CHECK-LABEL: Function: atest03
; CHECK: Instruction:   call void @llvm.memset.p0i8.i64(i8* align 1 %pad, i8 0, i64 12, i1 false)
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: false
; CHECK:     PrePad:     0
; CHECK:     FirstField: 1
; CHECK:     LastField:  2
; CHECK:     PostPad:    3
; CHECK:     Type: %struct.atest03 = type { i32, i64, i8, i32, i32 }

declare dso_local noalias i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

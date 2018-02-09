; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct identification and processing of various memory
; functions (memset, memcpy, memmove) by the DTransAnalysis and verifies
; that real legality checks are correctly identified while patterns which
; do not present any legality concerns are not incorrectly marked as unsafe.

; FIXME: All of these cases currently report "unhandled use" because
;        analysis of function calls is not yet implemented. They should
;        report either "No issues found" or "Unsafe function call".

; Call memset with a struct pointer.
%struct.test01 = type { i32, i32 }
define void @test1(%struct.test01* %s) {
  %p = bitcast %struct.test01* %s to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i32 8, i1 false)
  ret void
}

; FIXME: This should be a safe use.
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Call memcpy with matched struct pointers.
%struct.test02 = type { i32, i32 }
define void @test2(%struct.test02* %s1, %struct.test02* %s2) {
  %p1 = bitcast %struct.test02* %s1 to i8*
  %p2 = bitcast %struct.test02* %s2 to i8*
  call void @llvm.memcpy.p0i8.i64(i8* %p1, i8* %p2, i64 8, i32 8, i1 false)
  ret void
}

; FIXME: This should be a safe use.
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Call memcpy with mismatched struct pointers.
%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i16, i16, i32 }
define void @test3(%struct.test03.a* %sa, %struct.test03.b* %sb) {
  %pa = bitcast %struct.test03.a* %sa to i8*
  %pb = bitcast %struct.test03.b* %sb to i8*
  call void @llvm.memcpy.p0i8.i64(i8* %pa, i8* %pb, i64 8, i32 8, i1 false)
  ret void
}

; CHECK: LLVMType: %struct.test03.a = type { i32, i32 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test03.b = type { i16, i16, i32 }
; CHECK: Safety data: Unhandled use

; Call memmove with matched struct pointers.
%struct.test04 = type { i32, i32 }
define void @test4(%struct.test04* %s1, %struct.test04* %s2) {
  %p1 = bitcast %struct.test04* %s1 to i8*
  %p2 = bitcast %struct.test04* %s2 to i8*
  call void @llvm.memmove.p0i8.i64(i8* %p1, i8* %p2, i64 8, i32 8, i1 false)
  ret void
}

; FIXME: This should be a safe use.
; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Call memmove with mismatched struct pointers.
%struct.test05.a = type { i32, i32 }
%struct.test05.b = type { i16, i16, i32 }
define void @test5(%struct.test05.a* %sa, %struct.test05.b* %sb) {
  %pa = bitcast %struct.test05.a* %sa to i8*
  %pb = bitcast %struct.test05.b* %sb to i8*
  call void @llvm.memmove.p0i8.i64(i8* %pa, i8* %pb, i64 8, i32 8, i1 false)
  ret void
}

; CHECK: LLVMType: %struct.test05.a = type { i32, i32 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test05.b = type { i16, i16, i32 }
; CHECK: Safety data: Unhandled use

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1)
declare void @llvm.memcpy.p0i8.i64(i8* nocapture writeonly,
                                   i8* nocapture readonly, i64, i32, i1)
declare void @llvm.memmove.p0i8.i64(i8* nocapture writeonly,
                                    i8* nocapture readonly, i64, i32, i1)

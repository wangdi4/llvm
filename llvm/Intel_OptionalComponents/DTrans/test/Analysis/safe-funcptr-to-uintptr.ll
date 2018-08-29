; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that storing pointer to pointer-sized int doesn't cause 'Unsafe pointer store'
%struct.with_struct_ptr = type { i64, {}* }
%struct.with_uintptr = type { i8, i8, i64 }

define internal i64 @foo(%struct.with_uintptr* %s_uintptr, %struct.with_struct_ptr* %s_ptr) {
  %sp = getelementptr inbounds %struct.with_struct_ptr, %struct.with_struct_ptr* %s_ptr, i64 0, i32 1
  %bc = bitcast {}** %sp to i64*
  %ld = load i64, i64* %bc, align 8
  %uintptr = getelementptr inbounds %struct.with_uintptr, %struct.with_uintptr* %s_uintptr, i64 0, i32 2
  store i64 %ld, i64* %uintptr, align 8
  ret i64 %ld
}


%struct.A = type { i32, i32, i32, i32 }
%struct.B = type { i64, i64 }
%struct.C = type { i8, i8, i64 }

define void @storeB(%struct.B* %b, %struct.C* %c) {
  %intptrb = ptrtoint %struct.B* %b to i64
  %uiptr = getelementptr %struct.C, %struct.C* %c, i64 0, i32 2
  store i64 %intptrb, i64* %uiptr
  ret void
}


define %struct.A* @loadA(%struct.C* %c) {
  %uiptr = getelementptr %struct.C, %struct.C* %c, i64 0, i32 2
  %uint = load i64, i64* %uiptr
  %a = inttoptr i64 %uint to %struct.A*
  ret %struct.A* %a
}


; CHECK: LLVMType: %struct.A
; CHECK: Bad casting
; CHECK: LLVMType: %struct.B
; CHECK: Address taken

; CHECK: LLVMType: %struct.with_struct_ptr
; CHECK-NOT: Unsafe pointer store
; CHECK: LLVMType: %struct.with_uintptr
; CHECK-NOT: Unsafe pointer store


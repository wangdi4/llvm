; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that we correctly handle bitcasts involving vtables.

; B is a class that derives from C
%class.B = type { %class.C }

; CHECK:  LLVMType: %class.B = type { %class.C }
; CHECK:  Safety data: Contains nested structure

; The first element of C is a vtable
%class.C = type { i32 (...)** }

; CHECK:  LLVMType: %class.C = type { i32 (...)** }
; CHECK:  Safety data: Nested structure | Has vtable

%struct.A = type { i64, i64, %class.B*, i64 }

; CHECK:  LLVMType: %struct.A = type { i64, i64, %class.B*, i64 }
; CHECK:  Safety data: No issues found

; This is the vtable for C
@_ZTV1C = external constant { [3 x i8*] }

define void @test(%struct.A* %arg) {
  ; The IR below includes some simplifications, including replacing
  ; 'new B' with a call to malloc. Those are not relevant to what this
  ; IR is intended to test.
  %pA_B = getelementptr inbounds %struct.A, %struct.A* %arg, i64 0, i32 2
  %bufB = call i8* @malloc(i64 40)
  %tmpA_B = bitcast %class.B** %pA_B to i8**
  %vtableB = bitcast i8* %bufB to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV1C, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %vtableB
  store i8* %bufB, i8** %tmpA_B
  ret void
}

declare i8* @malloc(i64)

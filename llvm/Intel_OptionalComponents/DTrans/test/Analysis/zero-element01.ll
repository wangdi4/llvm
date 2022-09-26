; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKUNHUSE
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKMISS
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKBC
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKREAD

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKUNHUSE
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKMISS
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKBC
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKREAD

; Test that checks that the type %class.TestClass isn't marked as
; "Unhandled use", "Mismatched element access" or "Bad casting" because the
; 0 element is computed with a BitCast and a Load. It also makes sure that
; entries 0 and 1 in %"struct.std::_Vector_base<TestClass,
; std::allocator<TestClass>>::_Vector_impl.67" are marked as Read. This test
; case checks when the source is an argument.

; Check Unhandled use
; CHECKUNHUSE-LABEL:  LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECKUNHUSE-NOT: Safety data:{{.*}}Unhandled use{{.*}}

; Check Mismatched element access
; CHECKMISS-LABEL:  LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECKMISS-NOT: Safety data:{{.*}}Mismatched element access{{.*}}

; Check Bad casting
; CHECKBC-LABEL:  LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECKBC-NOT: Safety data:{{.*}}Bad casting{{.*}}

; Check Read
; CHECKREAD-LABEL:  LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
; CHECKREAD: 0)Field LLVM Type: %class.TestClass*
; CHECKREAD: Field info: Read

; CHECKREAD: 1)Field LLVM Type: %class.TestClass*
; CHECKREAD: Field info: Read

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal %class.TestClass* @make_new(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp1 = getelementptr inbounds %"struct.std::_Vector_base.13", %"struct.std::_Vector_base.13"* %tmp, i64 0, i32 0
  %tmp2 = getelementptr inbounds %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67", %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"* %tmp1, i64 0, i32 1
  %tmp3 = load %class.TestClass*, %class.TestClass** %tmp2
  br label %bb4

bb4:                                              ; preds = %bb
  %tmp5 = ptrtoint %class.TestClass* %tmp3 to i64
  %tmp6 = bitcast %"class.std::vector.12"* %arg to i64*
  %tmp7 = load i64, i64* %tmp6
  %tmp8 = sub i64 %tmp5, %tmp7
  %tmp9 = sdiv exact i64 %tmp8, 24
  br label %bb10

bb10:                                             ; preds = %bb4
  %tmp11 = tail call noalias nonnull i8* @_Znwm(i64 %tmp8)
  %tmp12 = bitcast i8* %tmp11 to %class.TestClass*
  ret %class.TestClass* %tmp12
}

declare noalias i8* @_Znwm(i64)


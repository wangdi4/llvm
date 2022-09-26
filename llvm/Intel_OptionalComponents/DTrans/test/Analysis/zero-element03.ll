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
; "Unhandled use", "Bad casting" or "Mismatched element access" because the
; 0 element is computed with a BitCast and a Load. It also makes sure that
; entries 0 and 1 in %"struct.std::_Vector_base<TestClass,
; std::allocator<TestClass>>::_Vector_impl.67" are marked as Read. This test
; case checks when the use is a PHI.

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

define internal i64 @isSize24(%"class.std::vector.12"* nocapture %arg, i1 %arg1) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
  %tmp3 = load i64, i64* %tmp2
  %tmp4 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0, i32 0, i32 1
  %tmp5 = icmp eq i1 %arg1, false
  br i1 %tmp5, label %bb6, label %bb8

bb6:                                              ; preds = %bb
  %tmp7 = load %class.TestClass*, %class.TestClass** %tmp4
  br label %bb12

bb8:                                              ; preds = %bb
  %tmp9 = bitcast %class.TestClass** %tmp4 to i64*
  %tmp10 = load i64, i64* %tmp9
  %tmp11 = inttoptr i64 %tmp10 to %class.TestClass*
  br label %bb12

bb12:                                             ; preds = %bb8, %bb6
  %tmp13 = phi %class.TestClass* [ %tmp7, %bb6 ], [ %tmp11, %bb8 ]
  %tmp14 = ptrtoint %class.TestClass* %tmp13 to i64
  %tmp15 = sub i64 %tmp14, %tmp3
  %tmp16 = sdiv exact i64 %tmp15, 24
  ret i64 %tmp16
}

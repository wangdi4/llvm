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
; case checks when there is a GEP with multiple indices, and then it is treated
; as a combination of bitcast and load to collect the 0 element.

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

%"class.std::vector.12" = type { %"class.std::vector.13" }
%"class.std::vector.13" = type { %"struct.std::_Vector_base.13", %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal i64 @isSize24(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0, i32 1
  %tmp1 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0, i32 1 , i32 0, i32 1
  %tmp2 = bitcast %class.TestClass** %tmp1 to i64*
  %tmp3 = load i64, i64* %tmp2
  %tmp4 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
  %tmp5 = load i64, i64* %tmp4
  %tmp6 = sub i64 %tmp3, %tmp5
  %tmp7 = sdiv exact i64 %tmp6, 24
  ret i64 %tmp7
}
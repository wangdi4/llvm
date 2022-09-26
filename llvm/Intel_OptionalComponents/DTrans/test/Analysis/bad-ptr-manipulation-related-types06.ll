; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKBPTR
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKBPTR

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKUNSAFEPTR
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKUNSAFEPTR

; This test checks that "Bad pointer manipulation for related types" and
; "Unsafe pointer store for related types" are not set for the types since
; the pointer to the beginning of the memory allocated space is stored
; in %"struct.std::_Vector_impl", and the pointer that points to the end
; is stored in %"struct.std::_Vector_impl2". The test case should produce
; "Bad pointer manipulation" and "Unsafe pointer store" for these types.

%class.TestClass = type { i32, %"class.std::vector" }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_impl" }
%"struct.std::_Vector_impl" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }

%class.TestClass2 = type { i32, %"class.std::vector2" }
%"class.std::vector2" = type { %"struct.std::_Vector_base2" }
%"struct.std::_Vector_base2" = type { %"struct.std::_Vector_impl2" }
%"struct.std::_Vector_impl2" = type { %class.TestClass2*, %class.TestClass2*, %class.TestClass2* }

define void @foo(%class.TestClass* %0, %class.TestClass2* %1) {
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1, i32 0, i32 0, i32 0
  %tmp2 = getelementptr inbounds %class.TestClass2, %class.TestClass2* %1, i64 0, i32 1, i32 0, i32 0, i32 1

  %tmp3 = tail call noalias nonnull i8* @_Znwm(i64 32)
  %tmp4 = bitcast i8* %tmp3 to %class.TestClass*

  %tmp5 = bitcast %class.TestClass** %tmp1 to i8**
  store i8* %tmp3, i8** %tmp5
  %tmp6 = getelementptr inbounds i8, i8* %tmp3, i64 32
  %tmp7 = bitcast %class.TestClass2** %tmp2 to i8**
  store i8* %tmp6, i8** %tmp7
  ret void
}

declare noalias i8* @_Znwm(i64)

; Check for not "Bad pointer manipulation for related types"
; CHECKBPTR-LABEL: LLVMType: %"struct.std::_Vector_impl" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
; CHECKBPTR-NOT: {{.*}}Bad pointer manipulation (related types){{.*}}
; CHECKBPTR: {{.*}}Bad pointer manipulation{{.*}}

; CHECKBPTR-LABEL: LLVMType: %class.TestClass = type { i32, %"class.std::vector" }
; CHECKBPTR-NOT: {{.*}}Bad pointer manipulation (related types){{.*}}
; CHECKBPTR: {{.*}}Bad pointer manipulation{{.*}}

; Check for not "Unsafe pointer store for related types"
; CHECKUNSAFEPTR-LABEL: LLVMType: %"struct.std::_Vector_impl" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
; CHECKUNSAFEPTR-NOT: {{.*}}Unsafe pointer store (related types){{.*}}
; CHECKUNSAFEPTR: {{.*}}Unsafe pointer store{{.*}}

; CHECKUNSAFEPTR-LABEL: LLVMType: %class.TestClass = type { i32, %"class.std::vector" }
; CHECKUNSAFEPTR-NOT: {{.*}}Unsafe pointer store (related types){{.*}}
; CHECKUNSAFEPTR: {{.*}}Unsafe pointer store{{.*}}
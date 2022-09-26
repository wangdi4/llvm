; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=true -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=true -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T

; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F

; Test that storing the address of a field to memory results in "Field address taken memory"
; being cascaded and pointer carried only when "-dtrans-outofboundsok=true",
; and not when "-dtrans-outofboundsok=false"

%struct.test01a = type { i32, i32, %struct.test01b, %struct.test01d* }
%struct.test01b = type { i32, %struct.test01c }
%struct.test01c = type { i32 }
%struct.test01d = type { i32, %struct.test01e }
%struct.test01e = type { i32 }

define void @test01() {
  %local_struct = alloca %struct.test01a
  %local_ptr = alloca i32*
  %field_addr = getelementptr %struct.test01a, %struct.test01a* %local_struct, i64 0, i32 1
  store  i32* %field_addr, i32** %local_ptr
  ret void
}

; CHECK_ALWAYS-LABEL: LLVMType: %struct.test01a = type { i32, i32, %struct.test01b, %struct.test01d* }
; CHECK_ALWAYS: Field address taken memory | Contains nested structure | Local instance{{ *$}}

; CHECK_ALWAYS-LABEL: LLVMType: %struct.test01b = type { i32, %struct.test01c }
; CHECK_OOB_T: Field address taken memory | Nested structure | Contains nested structure | Local instance{{ *$}}
; CHECK_OOB_F: Nested structure | Contains nested structure | Local instance{{ *$}}

; CHECK_ALWAYS-LABEL: LLVMType: %struct.test01c = type { i32 }
; CHECK_OOB_T: Field address taken memory | Nested structure | Local instance{{ *$}}
; CHECK_OOB_F: Nested structure | Local instance{{ *$}}

; CHECK_ALWAYS-LABEL: LLVMType: %struct.test01d = type { i32, %struct.test01e }
; CHECK_OOB_T: Field address taken memory | Contains nested structure{{ *$}}
; CHECK_OOB_F: Contains nested structure{{ *$}}

; CHECK_ALWAYS-LABEL: LLVMType: %struct.test01e = type { i32 }
; CHECK_OOB_T: Field address taken memory | Nested structure{{ *$}}
; CHECK_OOB_F: Nested structure{{ *$}}

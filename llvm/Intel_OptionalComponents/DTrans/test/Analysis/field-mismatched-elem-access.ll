; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-outofboundsok=false -dtrans-print-types -disable-output 2>&1 | FileCheck --check-prefix=CHECK --check-prefix=OOB_OFF %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output 2>&1 | FileCheck --check-prefix=CHECK --check-prefix=OOB_OFF %s

; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-outofboundsok=true -dtrans-print-types -disable-output 2>&1 | FileCheck --check-prefix=CHECK --check-prefix=OOB_ON %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-outofboundsok=true -dtrans-print-types -disable-output 2>&1 | FileCheck --check-prefix=CHECK --check-prefix=OOB_ON %s


; Test setting field member property "mismatched element access" when field is
; accessed as a type different than it's declared type. This allows for some
; cases to be accessed as pointer sized integer or pointers to pointer sized
; integers as described in the DTransAnalysis analyzeElementLoadOrStore function.

%struct.test01 = type { i64, double, i64*, double*, %struct.test01* }

define void @test01(%struct.test01* %in) {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %f2 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 2
  %f3 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 3
  %f4 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 4

  %v0 = load i64, i64* %f0

  ; mismatched load for field 1
  %f1bc = bitcast double* %f1 to i64*
  %v1 = load i64, i64* %f1bc
  %v2 = load i64*, i64** %f2
  %f3bc = bitcast double** %f3 to i64**
  %v3 = load i64*, i64** %f3bc

  ; mismatched load for field 4
  %f4bc = bitcast %struct.test01** %f4 to i64**
  %v4 = load i64*, i64** %f4bc

  ret void
}

; Without outofboundsok, only fields which are mismatched get marked as
; mismatched.
; With outofboundsok, all fields of the structure get marked as mismatched
; if any field is mismatched.

; CHECK-LABEL: LLVMType: %struct.test01 = type { i64, double, i64*, double*, %struct.test01* }
; CHECK: 0)Field LLVM Type: i64
; OOB_OFF-NOT: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess
; CHECK: 1)Field LLVM Type: double
; OOB_OFF: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess
; CHECK: 2)Field LLVM Type: i64*
; OOB_OFF-NOT: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess
; CHECK: 3)Field LLVM Type: double*
; OOB_OFF-NOT: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess
; CHECK: 4)Field LLVM Type: %struct.test01*
; OOB_OFF: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess


; Check the special case where the mismatched element access occurs when
; accessing the first element of the structure using a pointer to the
; structure, rather than a GEP result. We only need to check 'load'
; instructions, because 'store' instructions would treat the structure as an
; 'unsafe pointer store', rather than an element access.
%struct.test02 = type { i64, double*, %struct.test02* }

define void @test02(%struct.test02* %in) {
  %as_i32 = bitcast %struct.test02* %in to i32*
  %half = load i32, i32* %as_i32
  ret void
}
; CHECK-LABEL: %struct.test02 = type { i64, double*, %struct.test02* }
; CHECK: 0)Field LLVM Type: i64
; OOB_OFF: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess
; CHECK: 1)Field LLVM Type: double*
; OOB_OFF-NOT: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess
; CHECK: 2)Field LLVM Type: %struct.test02*
; OOB_OFF-NOT: MismatchedElementAccess
; OOB_ON: MismatchedElementAccess


; Check the special case where the mismatched element access occurs when
; the access is using a pointer to an empty structure to ensure there
; are not errors caused by trying to access non-existent field information.
%struct.test03 = type {}
%struct.test03base = type { i32 (...)** }

define void @test03(%struct.test03** %in) {
  %func_pppp = bitcast %struct.test03** %in to void (%struct.test03base*, i8*)****
  %func_ppp = load void (%struct.test03base*, i8*)***, void (%struct.test03base*, i8*)**** %func_pppp
  %func_pp = load void (%struct.test03base*, i8*)**, void (%struct.test03base*, i8*)*** %func_ppp
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test03 = type {}
; CHECK: Safety data:
; CHECK-SAME: Mismatched element access


; Test a mismatched element load that is larger than the field size.
; To simplify the implementation, a conservative approach is taken of marking
; all the fields of the structure.
%struct.test04 = type { i64, i32, i32 }
define void @test04(%struct.test04* %in) {
  %f0 = getelementptr %struct.test04, %struct.test04* %in, i64 0, i32 0
  %f1 = getelementptr %struct.test04, %struct.test04* %in, i64 0, i32 1
  %v0 = load i64, i64* %f0

  ; mismatched load that covers multiple fields.
  %f1bc = bitcast i32* %f1 to i64*
  %v1 = load i64, i64* %f1bc
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04 = type { i64, i32, i32 }
; CHECK: 0)Field LLVM Type: i64
; CHECK: MismatchedElementAccess
; CHECK: 1)Field LLVM Type: i32
; CHECK: MismatchedElementAccess
; CHECK: 2)Field LLVM Type: i32
; CHECK: MismatchedElementAccess


; If the access size is larger than the size of the first field, then all
; the affected fields would need to be marked as having the "mismatched
; element access". To simplify the implementation, a conservative approach
; is taken of marking all the fields of the structure.
 %struct.test05 = type { i32, i32, %struct.test05* }

define void @test05(%struct.test05* %in) {
  %as_i64 = bitcast %struct.test05* %in to i64*
  %half = load i64, i64* %as_i64
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test05 = type { i32, i32, %struct.test05* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: MismatchedElementAccess
; CHECK: 1)Field LLVM Type: i32
; CHECK: MismatchedElementAccess
; CHECK: 2)Field LLVM Type: %struct.test05
; CHECK: MismatchedElementAccess

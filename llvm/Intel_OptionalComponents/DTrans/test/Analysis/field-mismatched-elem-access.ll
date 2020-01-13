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
  %f1bc = bitcast double* %f1 to i64*
  %v1 = load i64, i64* %f1bc
  %v2 = load i64*, i64** %f2
  %f3bc = bitcast double** %f3 to i64**
  %v3 = load i64*, i64** %f3bc
  %f4bc = bitcast %struct.test01** %f4 to i64**
  %v4 = load i64*, i64** %f4bc

  ret void
}

; Without outofboundsok, only fields which are mismatched get marked as
; mismatched.
; With outofboundsok, all fields of the structure get marked as mismatched
; if any field is mismatched.

; CHECK: LLVMType: %struct.test01 = type { i64, double, i64*, double*, %struct.test01* }
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

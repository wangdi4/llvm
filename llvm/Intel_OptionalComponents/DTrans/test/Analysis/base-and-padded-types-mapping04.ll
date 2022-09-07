; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKUS
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKGEP

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKUS
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKGEP

; This test verifies that unsafe pointer store and ambiguous GEP don't
; show up in the safety data since we are casting from a base type to a
; padded type.

%"struct.test.array" = type { [4 x i32] }
%struct.test.a = type <{ %"struct.test.array" , i32, [4 x i8] }>
%struct.test.a.base = type <{ %"struct.test.array" , i32 }>
%struct.test.b = type { %struct.test.a.base, i64 }

define void @foo(%struct.test.a* %bcast) {
  %agep = getelementptr %struct.test.a, %struct.test.a* %bcast, i64 0, i32 0
  %boostgep = getelementptr %"struct.test.array", %"struct.test.array"* %agep, i64 0, i32 0
  %arrgep = getelementptr [4 x i32], [4 x i32]* %boostgep, i64 0, i32 0
  store i32 2, i32* %arrgep

  ret void
}

define void @test() {
  %bptr = alloca %struct.test.b
  %bcast = bitcast %struct.test.b* %bptr to %struct.test.a*
  call void @foo(%struct.test.a* %bcast)
  ret void
}

; Check Unsafe store
; CHECKUS-LABEL: %struct.test.a = type <{ %struct.test.array, i32, [4 x i8] }>
; CHECKUS: Related base structure: struct.test.a.base
; CHECKUS-NOT: Safety data:{{.*}}Unsafe pointer store{{.*}}

; CHECKUS-LABEL: %struct.test.a.base = type <{ %struct.test.array, i32 }>
; CHECKUS: Related padded structure: struct.test.a
; CHECKUS-NOT: Safety data:{{.*}}Unsafe pointer store{{.*}}

; CHECKUS-LABEL: %struct.test.array = type { [4 x i32] }
; CHECKUS-NOT: Unsafe pointer store

; CHECKUS-LABEL: %struct.test.b = type { %struct.test.a.base, i64 }
; CHECKUS-NOT: Unsafe pointer store

; Check Ambiguous GEP
; CHECKGEP-LABEL: %struct.test.a = type <{ %struct.test.array, i32, [4 x i8] }>
; CHECKGEP: Related base structure: struct.test.a.base
; CHECKGEP-NOT: Safety data:{{.*}}Ambiguous GEP{{.*}}

; CHECKGEP-LABEL: %struct.test.a.base = type <{ %struct.test.array, i32 }>
; CHECKGEP: Related padded structure: struct.test.a
; CHECKGEP-NOT: Safety data:{{.*}}Ambiguous GEP{{.*}}

; CHECKGEP-LABEL: %struct.test.array = type { [4 x i32] }
; CHECKGEP-NOT: Ambiguous GEP

; CHECKGEP-LABEL: %struct.test.b = type { %struct.test.a.base, i64 }
; CHECKGEP-NOT: Ambiguous GEP
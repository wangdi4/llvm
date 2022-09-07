; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKBC
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKBC

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKAT
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKAT


; This test verifies that bad casting and address taken doesn't show up in
; the safety data since we are casting from a base type to a padded type.
; Also, it checks that %struct.test.a is related to %struct.test.a.base.
; The safety data results from %struct.test.a and %struct.test.a.base
; should be the same.

%"struct.test.array" = type { [4 x i32] }
%struct.test.a = type <{ %"struct.test.array" , i32, [4 x i8] }>
%struct.test.a.base = type <{ %"struct.test.array" , i32 }>
%struct.test.b = type { %struct.test.a.base, i64 }

define i32 @foo(%struct.test.a* %bcast) {
  %agep = getelementptr %struct.test.a, %struct.test.a* %bcast, i64 0, i32 0
  %boostgep = getelementptr %"struct.test.array", %"struct.test.array"* %agep, i64 0, i32 0
  %arrgep = getelementptr [4 x i32], [4 x i32]* %boostgep, i64 0, i32 0
  %ret = load i32, i32* %arrgep

  ret i32 %ret
}

define i32 @test() {
  %bptr = alloca %struct.test.b
  %bcast = bitcast %struct.test.b* %bptr to %struct.test.a*
  %ret = call i32 @foo(%struct.test.a* %bcast)
  ret i32 %ret
}

; Check Bad casting
; CHECKBC-LABEL: %struct.test.a = type <{ %struct.test.array, i32, [4 x i8] }>
; CHECKBC: Related base structure: struct.test.a.base
; CHECKBC-NOT: Safety data:{{.*}}Bad casting{{.*}}

; CHECKBC-LABEL: %struct.test.a.base = type <{ %struct.test.array, i32 }>
; CHECKBC: Related padded structure: struct.test.a
; CHECKBC-NOT: Safety data:{{.*}}Bad casting{{.*}}

; CHECKBC-LABEL: %struct.test.array = type { [4 x i32] }
; CHECKBC-NOT: Safety data:{{.*}}Bad casting{{.*}}

; CHECKBC-LABEL: %struct.test.b = type { %struct.test.a.base, i64 }
; CHECKBC-NOT: Safety data:{{.*}}Bad casting{{.*}}


; Check Address taken
; CHECKAT-LABEL: %struct.test.a = type <{ %struct.test.array, i32, [4 x i8] }>
; CHECKAT: Related base structure: struct.test.a.base
; CHECKAT-NOT: Safety data:{{.*}}Address taken{{.*}}

; CHECKAT-LABEL: %struct.test.a.base = type <{ %struct.test.array, i32 }>
; CHECKAT: Related padded structure: struct.test.a
; CHECKAT-NOT: Safety data:{{.*}}Address taken{{.*}}

; CHECKAT-LABEL: %struct.test.array = type { [4 x i32] }
; CHECKAT-NOT: Safety data:{{.*}}Address taken{{.*}}

; CHECKAT-LABEL: %struct.test.b = type { %struct.test.a.base, i64 }
; CHECKAT-NOT: Safety data:{{.*}}Address taken{{.*}}
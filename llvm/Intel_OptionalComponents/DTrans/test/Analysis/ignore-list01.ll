; REQUIRES: asserts

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output -debug-only=dtransanalysis -dtrans-nosafetychecks-list="first;deletefields:one,two;aostosoa:three" -dtrans-nosafetychecks-list="" 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output -debug-only=dtransanalysis -dtrans-nosafetychecks-list="first;deletefields:one,two;aostosoa:three" -dtrans-nosafetychecks-list="" 2>&1 | FileCheck %s

; The test checks '-dtrans-nosafetychecks-list=' option functionality. This options allows ignoring safety check violations for types.


%struct.first = type { i32, i32 }
@a.first = internal unnamed_addr global %struct.first zeroinitializer

%struct.second = type { i32, i32 }
@b.second = internal unnamed_addr global %struct.second zeroinitializer

%struct.third = type { i32, i32 }
@c.third = internal unnamed_addr global %struct.third zeroinitializer

%struct.three = type { i32, i32 }
@d.three = internal unnamed_addr global %struct.three zeroinitializer

define dso_local i32 @main() {
  ret i32 0
}

; CHECK: dtrans-nosafetychecks-list:
; CHECK-NEXT:        Skipping 'first': transform name or types list is missing
; CHECK-NEXT:        Skipping 'deletefields:one,two': bad transformation name
; CHECK-NEXT:        Adding   'aostosoa:three'

; CHECK:  LLVMType: %struct.three = type { i32, i32 }
; CHECK-NEXT:  Name: struct.three
; CHECK-NEXT:  (will be ignored in  aostosoa)

; RUN: llvm-as %s -o %t.bc
; RUN: llvm-link -irmover-type-merging=false -S %t.bc -o - | FileCheck %s
; RUN: llvm-link -irmover-type-merging=false -S %s -o -    | FileCheck %s
; RUN: llvm-link -irmover-type-merging=true -S %t.bc -o -  | FileCheck --check-prefix=CHECK-TM %s
; RUN: llvm-link -irmover-type-merging=true -S %s -o -     | FileCheck --check-prefix=CHECK-TM %s
; See type-unique-src-type.ll

; Test that we don't try to map %C_c and C and then try to map %C to a new type.
; This used to happen when lazy loading since we wouldn't then identify %C
; as a destination type until it was too late.

; CHECK: %C_c = type { %B }
; CHECK-NEXT: %B = type { %A }
; CHECK-NEXT: %A = type { i8 }

; CHECK-TM: %C_c = type { %B }
; CHECK-TM-NEXT: %B = type { %A }
; CHECK-TM-NEXT: %A = type { i8 }

; CHECK: @g1 = global %C_c zeroinitializer
; CHECK:  getelementptr %C, %C* null, i64 0, i32 0, i32 0

; CHECK-TM: @g1 = global %C_c zeroinitializer
; CHECK-TM: getelementptr %C_c, %C_c* null, i64 0, i32 0, i32 0

%A   = type { i8 }
%B   = type { %A }
%C   = type { %B }
%C_c = type { %B }
define void @f1() {
  getelementptr %C, %C* null, i64 0, i32 0, i32 0
  ret void
}
@g1 = global %C_c zeroinitializer

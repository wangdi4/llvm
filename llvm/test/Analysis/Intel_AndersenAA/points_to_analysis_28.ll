; Verifies that points-to info for pr<mem> is computed as <universal>
; since integer type value (i.e i64) is stored to @pr.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK:  pr<mem>        --> ({{[0-9a-f]+}}): <universal>

%struct.S = type { i32* }

@pr = internal dso_local global %struct.S* null, align 8

define internal void @foo(i64 %ptr1) {
entry:
  store i64 %ptr1, i64* bitcast (%struct.S** @pr to i64*), align 8
  ret void
}

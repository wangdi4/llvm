; CMPLRLLVM-12033: Verifies that points-to info for pr<mem> is computed as
; "malloc" call in "foo" routine even though the pointer is stored as
; integer value.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK:  pr<mem>        --> ({{[0-9a-f]+}}): foo:call

%struct.S = type { i32* }

@pr = internal dso_local global %struct.S* null, align 8
@temp = internal dso_local global %struct.S** null, align 8

define internal void @foo(%struct.S** %q) {
entry:
  %call = call i8* @malloc(i64 4)
  %0 = bitcast i8* %call to %struct.S**
  store %struct.S** %0, %struct.S*** @temp, align 8
  %ptr1 = bitcast %struct.S*** @temp to i64*
  %ld = load i64, i64* %ptr1
  store i64 %ld, i64* bitcast (%struct.S** @pr to i64*), align 8
  ret void
}

declare dso_local i8* @malloc(i64)


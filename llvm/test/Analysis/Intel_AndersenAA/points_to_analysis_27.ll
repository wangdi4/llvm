; CMPLRLLVM-12033: Verifies that points-to info for pr<mem> is computed as
; "malloc" call in "foo" routine even though the pointer is stored as
; integer value.
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s
; CHECK:  pr<mem>        --> ({{[0-9a-f]+}}): foo:call

@pr = internal global ptr null, align 8
@temp = internal global ptr null, align 8

define internal void @foo(ptr %q) {
entry:
  %call = call ptr @malloc(i64 4)
  %i = bitcast ptr %call to ptr
  store ptr %i, ptr @temp, align 8
  %ptr1 = bitcast ptr @temp to ptr
  %ld = load i64, ptr %ptr1, align 4
  store i64 %ld, ptr @pr, align 8
  ret void
}

declare dso_local ptr @malloc(i64)

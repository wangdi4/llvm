; CMPLRLLVM-9064: Verifies that points-to info for Glob2 shouldn't have
; <universal> even though non-pointer is stored to Glob2.
; RUN: opt < %s -anders-aa -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; Andersens Analysis shouldn't go conservative for Glob2's store instruction
; even though non-pointer value (i.e i64)is stored since non-pointer
; loads/stores of global variables are treated as pointers.

; CHECK: Glob2<mem>     --> ({{[0-9a-f]+}}): foo:call
; CHECK-NOT: Glob2<mem>     --> ({{[0-9a-f]+}}): <universal>, ({{[0-9a-f]+}}): foo:call

%struct.S = type { i32* }

@Glob1 = internal dso_local global %struct.S* null, align 8
@Glob2 = internal dso_local global %struct.S* null, align 8

define internal void @foo() {
entry:
  %call = call i8* @malloc(i64 4)
  %0 = bitcast i8* %call to %struct.S*
  store %struct.S* %0, %struct.S** @Glob1, align 8
  %1 = load i64, i64* bitcast (%struct.S** @Glob1 to i64*), align 8
  store i64 %1, i64* bitcast (%struct.S** @Glob2 to i64*), align 8
  ret void
}
declare dso_local i8* @malloc(i64)

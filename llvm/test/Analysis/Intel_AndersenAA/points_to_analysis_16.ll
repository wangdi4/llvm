; CMPLRLLVM-8888: Verifies that points-to info for bar:%1 is not incorrectly
; computed as empty.
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; Andersens Analysis shouldn't ignore store instruction since it is actually
; storing pointer. Points-to info of bar:%1 shouldn't be computed as empty.
; Currently, Andersen computes points-to info of bar:%1 as <universal> but
; the analysis can be improved to compute points-to info of bar:%1 as "foo:%call"
; more accurately.

; CHECK: bar:%1        --> (0): <universal>
; CHECK-NOT: [0] bar:%1        -->

%struct.S = type { i32* }

@p = internal dso_local global %struct.S* null, align 8

define dso_local i32* @bar() {
entry:
  call void @foo(%struct.S** @p)
  %0 = load %struct.S*, %struct.S** @p, align 8
  %ptr = getelementptr inbounds %struct.S, %struct.S* %0, i32 0, i32 0
  %1 = load i32*, i32** %ptr, align 8
  ret i32* %1
}

define internal void @foo(%struct.S** %q) {
entry:
  %call = call i8* @malloc(i64 4)
  %0 = bitcast i8* %call to %struct.S*
  %int = ptrtoint %struct.S* %0 to i64
  %ptr1 = bitcast %struct.S** %q to i64*
  store i64 %int, i64* %ptr1, align 8
  ret void
}

declare dso_local i8* @malloc(i64) #1


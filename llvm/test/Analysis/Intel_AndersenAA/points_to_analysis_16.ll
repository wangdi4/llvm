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

%struct.S = type { ptr }

@p = internal dso_local global ptr null, align 8

define dso_local ptr @bar() {
entry:
  call void @foo(ptr @p)
  %0 = load ptr, ptr @p, align 8
  %ptr = getelementptr inbounds %struct.S, ptr %0, i32 0, i32 0
  %1 = load ptr, ptr %ptr, align 8
  ret ptr %1
}

define internal void @foo(ptr %q) {
entry:
  %call = call ptr @malloc(i64 4)
  %0 = bitcast ptr %call to ptr
  %int = ptrtoint ptr %0 to i64
  %ptr1 = bitcast ptr %q to ptr
  store i64 %int, ptr %ptr1, align 8
  ret void
}

declare dso_local ptr @malloc(i64) #1

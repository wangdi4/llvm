; Verifies that points-to analysis doesn't crash.

; RUN: opt < %s -passes='require<anders-aa>' -disable-output 2>/dev/null

%struct.S = type { ptr }

@p = internal global ptr null, align 8
@q = internal global ptr null, align 8

define internal void @foo(ptr %q, ptr %p) {
entry:
  %ptr1 = getelementptr inbounds %struct.S, ptr %p, i32 0, i32 0
  %0 = bitcast ptr %ptr1 to ptr
  %int = load i64, ptr %0, align 8
  %ptr2 = bitcast ptr %q to ptr
  store i64 %int, ptr %ptr2, align 8
  ret void
}

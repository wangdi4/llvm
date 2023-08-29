; RUN: opt --passes='default<O2>' %s -S 2>&1 | FileCheck %s

; This test case checks if the attribute "mustprogress" was added when
; the TargetLibraryInfo analysis runs.

; CHECK: declare dso_local ptr @__dynamic_cast(ptr, ptr, ptr, i64) local_unnamed_addr #0
; CHECK: attributes #0 = { mustprogress nofree willreturn memory(read) }

%test.class = type { i8 }

declare dso_local ptr @__dynamic_cast(ptr, ptr, ptr, i64)

define void @foo(%test.class* %a, ptr %b, ptr %c) {
  %temp0 = bitcast %test.class* %a to ptr
  %temp1 = call ptr @__dynamic_cast(ptr %temp0, ptr %b, ptr %b, i64 0)
  ret void
}


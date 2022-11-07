; RUN: opt --O2 %s -S 2>&1 | FileCheck %s
; RUN: opt --passes='default<O2>' %s -S 2>&1 | FileCheck %s

; This test case checks if the attribute "mustprogress" was added when
; the TargetLibraryInfo analysis runs.

; CHECK: declare dso_local i8* @__dynamic_cast(i8*, i8*, i8*, i64) local_unnamed_addr #0
; CHECK: attributes #0 = { mustprogress nofree willreturn memory(read) }

%test.class = type { i8 }

declare dso_local i8* @__dynamic_cast(i8*, i8*, i8*, i64)

define void @foo(%test.class* %a, i8* %b, i8* %c) {
  %temp0 = bitcast %test.class* %a to i8*
  %temp1 = call i8* @__dynamic_cast(i8* %temp0, i8* %b, i8* %b, i64 0)
  ret void
}


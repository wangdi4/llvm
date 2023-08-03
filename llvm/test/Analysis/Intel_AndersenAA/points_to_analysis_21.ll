; This test verifies that anders-aa should be able to disambiguate stdout
; and local allocated pointer (i.e %p1) when whole_program_safe is true.

; RUN: opt < %s -whole-program-assume -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK: NoAlias:        ptr* %gep, ptr* %p1

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@stdout = external dso_local local_unnamed_addr global ptr, align 8
@plist = internal unnamed_addr global ptr null, align 8

define dso_local void @bar() {
  %call1 = tail call noalias ptr @malloc(i64 40)
  %1 = bitcast ptr %call1 to ptr
  store ptr %1, ptr @plist
  ret void
}

define dso_local i32 @main() {
entry:
  %p0 = load ptr, ptr @stdout, align 8
  %gep = getelementptr inbounds %struct._IO_FILE, ptr %p0, i32 0, i32 1
  %ld.gep = load ptr, ptr %gep
  %call0 = tail call i32 @fflush(ptr %p0)
  tail call void @bar()
  %p1 = load ptr, ptr @plist
  %ld.p1 = load ptr, ptr %p1
  ret i32 0
}

declare dso_local i32 @fflush(ptr nocapture)
declare noalias ptr @malloc(i64)

; CMPLRLLVM-24304: This test verifies that "nabout" in load instructions are
; replaced with "stdout" by globalopt.
; This is same as intel-global-opt-stdout-1.ll except BitCasts are not
; involved while modifying "nabout".

; RUN: opt < %s -passes='require<wholeprogram>,globalopt' -whole-program-assume -S | FileCheck %s

; CHECK-DAG: %l1 = load ptr, ptr @stdout, align 8
; CHECK-DAG: %l3 = load ptr, ptr @stdout, align 8

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@nabout = internal dso_local local_unnamed_addr global ptr null, align 8
@stdout = external dso_local local_unnamed_addr global ptr, align 8

define dso_local void @bar() {
entry:
  %l1 = load ptr, ptr @nabout, align 8
  %call1 = tail call i32 @fflush(ptr %l1)
  ret void
}

define dso_local i32 @main() {
entry:
  %l2 = load ptr, ptr @stdout, align 8
  store ptr %l2, ptr @nabout, align 8
  tail call void @bar()
  %l3 = load ptr, ptr @nabout, align 8
  %call1 = tail call i32 @fflush(ptr %l3)
  ret i32 0
}

declare dso_local i32 @fflush(ptr nocapture)

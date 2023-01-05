; This test verifies that "nabout" in load instructions are NOT replaced
; with "stdout" by globalopt when whole-program-safe is true since "stdout"
; is modified.

; RUN: opt < %s -passes='require<wholeprogram>,globalopt' -whole-program-assume -S | FileCheck %s

; CHECK-NOT: %0 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
; CHECK-NOT: %1 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@nabout = internal dso_local local_unnamed_addr global %struct._IO_FILE* null, align 8
@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8

define dso_local void @bar() {
entry:
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** @nabout, align 8
  %call1 = tail call i32 @fflush(%struct._IO_FILE* %0)
  ret void
}

define dso_local i32 @main() {
entry:
  %0 = load i64, i64* bitcast (%struct._IO_FILE** @stdout to i64*), align 8
  store i64 %0, i64* bitcast (%struct._IO_FILE** @nabout to i64*), align 8
  tail call void @bar()
  %1 = load %struct._IO_FILE*, %struct._IO_FILE** @nabout, align 8
  %call1 = tail call i32 @fflush(%struct._IO_FILE* %1)
; stdout is modified.
  store i64 0, i64* bitcast (%struct._IO_FILE** @stdout to i64*), align 8
  ret i32 0
}

declare dso_local i32 @fflush(%struct._IO_FILE* nocapture)

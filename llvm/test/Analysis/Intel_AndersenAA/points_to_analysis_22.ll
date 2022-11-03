; This test verifies that anders-aa shouldn't be able to disambiguate
; stdout and nout, which are external variables.

; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -whole-program-assume -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -whole-program-assume -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK-NOT: NoAlias:     %struct._IO_FILE* %p0, %struct._IO_FILE* %p1

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@nout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8

define dso_local i32 @main() {
entry:
  %p0 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %call0 = tail call i32 @fflush(%struct._IO_FILE* %p0)
  %p1 = load %struct._IO_FILE*, %struct._IO_FILE** @nout, align 8
  %call1 = tail call i32 @fflush(%struct._IO_FILE* %p1)
  ret i32 0
}

declare dso_local i32 @fflush(%struct._IO_FILE* nocapture)

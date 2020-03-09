; This test verifies that anders-aa should NOT be able to disambiguate stdout
; and external global variable pointer (i.e %p1) when whole_program_safe
; is true.

; This test is same as points_to_analysis_25.ll except @plist is declared
; as external.

; RUN: opt < %s -wholeprogramanalysis -anders-aa -whole-program-assume -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -whole-program-assume -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK-NOT: NoAlias:      %struct._IO_FILE* %p0, i64* %p1

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@plist = external unnamed_addr global i64*, align 8

define dso_local i32 @main() {
entry:
  %p0 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %call0 = tail call i32 @fflush(%struct._IO_FILE* %p0)
  %p1 = load i64*, i64** @plist
  tail call void @bar(i64* %p1)
  ret i32 0
}

declare dso_local i32 @fflush(%struct._IO_FILE* nocapture)
declare dso_local void @bar(i64*)

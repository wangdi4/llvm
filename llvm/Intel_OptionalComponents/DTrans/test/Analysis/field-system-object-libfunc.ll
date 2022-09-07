; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test checks that the argument regexe_t is marked as system object
; since it is used in a libfunc.

; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

; RUN: opt  < %s -whole-program-assume  -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

%struct.regex_t = type { i32, i64, i8*, %struct.re_guts* }
%struct.re_guts = type { i32, i64*, i32, i32, %struct.cset*, i8*, i32, i64, i64, i64, i32, i32, i32, i32, i8*, i8*, i32, i64, i32, i64, [1 x i8] }
%struct.cset = type { i8*, i8, i8, i64, i8* }
@expbuf = internal global %struct.regex_t zeroinitializer, align 8
@rexpr = internal global [1000 x i8] zeroinitializer, align 16

define dso_local i32 @regcomp(%struct.regex_t* nocapture, i8*, i32) {
  ret i32 1
}

define i32 @main() {
  %1 = tail call i32 @regcomp(%struct.regex_t* nonnull @expbuf, i8* getelementptr inbounds ([1000 x i8], [1000 x i8]* @rexpr, i64 0, i64 0), i32 4)
  ret i32 %1
}

; Check the main structure
; CHECK: dtrans-safety-detail: %struct.regex_t = type { i32, i64, i8*, %struct.re_guts* }
; CHECK: dtrans-safety: System object: argument used in a library function

; Check the embedded structures
; CHECK: dtrans-safety-detail: %struct.re_guts = type { i32, i64*, i32, i32, %struct.cset*, i8*, i32, i64, i64, i64, i32, i32, i32, i32, i8*, i8*, i32, i64, i32, i64, [1 x i8] } :: System object
; CHECK: dtrans-safety-detail: %struct.cset = type { i8*, i8, i8, i64, i8* } :: System object

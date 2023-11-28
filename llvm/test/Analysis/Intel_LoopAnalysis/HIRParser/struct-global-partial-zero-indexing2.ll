; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; Verify that we add zero index dims/offsets for all the struct/array types
; until we reach the load type of [10 x i64]. Adding the extra dims/offsets
; should help loopopt utilities analyze the ref better as it recontructs the
; zero GEP indices which existed in typed pointer mode.
; Note that we do not index into the innermost indexable type which is i64 as
; that would result in the ref having a BitCastDestTy of [10 x i64]* and
; would likely inhibit optimizations.

; CHECK: %ld = (@x)[0].0.0;
; CHECK: ret %ld;


%struct.inner = type { [10 x i64] }
%struct.outer = type { %struct.inner, i32 }

@x = dso_local global %struct.outer zeroinitializer, align 4

define dso_local noundef [10 x i64] @foo() {
entry:
  br label %bb

bb:
  %ld = load [10 x i64], ptr @x, align 4
  ret [10 x i64] %ld
}



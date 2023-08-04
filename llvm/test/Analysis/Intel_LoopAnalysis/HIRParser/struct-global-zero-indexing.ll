; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; Verify that we add zero index dims/offsets for all the struct/array types on
; encountering a global value. GEPs with zero indices may be optimized away in
; opaque ptr mode. Adding the extra dims/offsets should help loopopt utilities
; analyze the ref better as it recontructs the zero GEP indices which existed
; in typed pointer mode.


; CHECK: %ld = (@x)[0].0.0[0];
; CHECK: ret %ld;


%struct.inner = type { [100 x i32] }
%struct.outer = type { %struct.inner, i32 }

@x = dso_local global %struct.outer zeroinitializer, align 4

define dso_local noundef i32 @foo() {
entry:
  br label %bb

bb:
  %ld = load i32, ptr @x, align 4
  ret i32 %ld
}



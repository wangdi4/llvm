; RUN: opt < %s -globals-aa -aa-eval -print-all-alias-modref-info 2>&1 | FileCheck %s
; TODO: New pass manager pipeline is not working, need to fix.
; RU: opt < %s -aa-pipeline=globals-aa -aa-eval -print-all-alias-modref-info | FileCheck %s

; Verify that globals-aa can dismabiguate between %ig (heap memory belonging to
; @IndirectGlob) and @Glob even though @Glob's address is taken.

; CHECK: NoAlias:      i32* %ig, i32* @Glob

@Glob = internal global i32 0
@IndirectGlob = internal global i32* null

declare noalias i8* @malloc(i32)
declare void @bar(i32*)

define void @test() {
  %a = call i8* @malloc(i32 4)
  call void @bar(i32* @Glob)
  store i8* %a, i8** bitcast (i32** @IndirectGlob to i8**)
  ret void
}

define void @test1() {
  %ig = load i32*, i32** @IndirectGlob
  store i32 123, i32* @Glob
  %ld.ig = load i32, i32* %ig, align 8
  ret void
}

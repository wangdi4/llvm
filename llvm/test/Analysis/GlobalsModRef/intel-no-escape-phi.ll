; RUN: opt < %s -globals-aa -aa-eval -print-all-alias-modref-info 2>&1 | FileCheck %s
; TODO: New pass manager pipeline is not working, need to fix.
; RU: opt < %s -aa-pipeline=globals-aa -aa-eval -print-all-alias-modref-info | FileCheck %s

; Verify that globals-aa can disambiguate between %p and @Glob by handling phi
; use of @glob in %phi.

; CHECK: NoAlias:      i32* %p, i32* @Glob

@Glob = internal global i32 0

define void @test(i32* %p) {
entry:
  br label %bb

bb:
  %phi = phi i32* [ @Glob , %entry ]
  store i32 0, i32* %p
  ret void
}


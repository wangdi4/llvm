; RUN: opt < %s -S -passes='require<globals-aa>,function(aa-eval)' -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; Verify that globals-aa can disambiguate between %p and @Glob by handling phi
; use of @glob in %phi.

; CHECK: NoAlias: i32* %p, i32* @Glob

@Glob = internal global i32 0

define void @test(i32* %p) {
entry:
  br label %bb

bb:
  %phi = phi i32* [ @Glob , %entry ]
  %ld.Glob = load i32, i32* @Glob, align 8
  store i32 0, i32* %p
  ret void
}


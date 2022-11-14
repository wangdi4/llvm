; RUN: opt -disable-output -passes=aa-eval -evaluate-loopcarried-alias -print-all-alias-modref-info < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes=print-alias-sets -print-loopcarried-alias-sets < %s 2>&1 | FileCheck --check-prefix=AST %s

; This spot checks some of BasicAA's loopCarriedAlias-related
; behavior. BasicAA's ::loopCarriedAlias reuses only parts of ::alias;
; we are trying to verify this here.

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Check that loopCarriedAlias benefits from 'noalias' parameter attributes,
; which are handled by BasicAA.
; Verify the AST-based analysis:
; AST-LABEL: function 'params':
; AST-DAG: alias, Mod Pointers: (i32* %par0, LocationSize::precise(4))
; AST-DAG: alias, Mod Pointers: (i32* %par1, LocationSize::precise(4))
; ...and also the pairwise analysis:
; CHECK-LABEL: Function: params:
; CHECK-DAG: NoAlias: i32* %par0, i32* %par1
define void @params(i32* noalias %par0, i32* noalias %par1) {
  store i32 0, i32* %par0
  store i32 0, i32* %par1
  ret void
}

; Check that loopCarriedAlias benefits from parts of BasicAA, namely
; "isGEPBaseAtNegativeOffset".  This uses inbounds GEPs to prove no
; aliasing by showing that an alias would imply a negative GEP index. (See
; "isGEPBaseAtNegativeOffset" in BasicAA for a better description.)
; Verify the AST-based analysis:
; AST-LABEL: function 'global':
; AST-DAG: 2 alias sets for 3 pointer values
; AST-DAG: alias, Mod Pointers: (i32* %p0, LocationSize::precise(4)), (i32* @gv, LocationSize::precise(4))
; AST-DAG: alias, Mod Pointers: (i32* %p1, LocationSize::precise(4))
; ...and also the pairwise analysis:
; CHECK-LABEL: Function: global:
; CHECK-DAG: MayAlias: i32* %p0, i32* @gv
; CHECK-DAG: NoAlias: i32* %p0, i32* %p1

declare i32* @random.i32(i32* %ptr)
@gv = global i32 1

define void @global(i32* %random) {
  %p0 = getelementptr inbounds i32, i32* %random, i32 0
  %p1 = getelementptr inbounds i32, i32* %random, i32 1
  store i32 0, i32* %p0
  store i32 0, i32* %p1
  store i32 0, i32* @gv
  ret void
}

; Check that loopCarriedAlias does not claim 'NoAlias' incorrectly
; for the infamous pointer swapping example. (NoAlias would be correct
; for ::alias, but not for ::loopCarriedAlias.)
; AST-LABEL: function 'bug42143':
; AST-DAG: alias, Mod/Ref Pointers: (float* %p, LocationSize::precise(4)), (float* %q, LocationSize::precise(4))
; CHECK-LABEL: Function: bug42143:
; CHECK-DAG: MayAlias: float* %p, float* %q
define float @bug42143() {
entry:
  %g = alloca float, align 4
  %h = alloca float, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %p = phi float* [ %g, %entry ], [ %q, %for.body ]
  %q = phi float* [ %h, %entry ], [ %p, %for.body ]
  %0 = load float, float* %p, align 4
  store float undef, float* %q, align 4
  %branch_cond = fcmp ugt float %0, 0.0
  br i1 %branch_cond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret float undef
}

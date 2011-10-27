; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @getAndMoveToFrontDecode
; Nothing to do here
; CHECK-NOT: footer
; CHECK: ret
define void @getAndMoveToFrontDecode() {
	br label %endif.2

endif.2:		; preds = %loopexit.5, %0
	br i1 false, label %loopentry.5, label %UnifiedExitNode

loopentry.5:		; preds = %loopexit.6, %endif.2
	br i1 false, label %loopentry.6, label %UnifiedExitNode

loopentry.6:		; preds = %loopentry.7, %loopentry.5
	br i1 false, label %loopentry.7, label %loopexit.6

loopentry.7:		; preds = %loopentry.7, %loopentry.6
	br i1 false, label %loopentry.7, label %loopentry.6

loopexit.6:		; preds = %loopentry.6
	br i1 false, label %loopentry.5, label %loopexit.5

loopexit.5:		; preds = %loopexit.6
	br i1 false, label %endif.2, label %UnifiedExitNode

UnifiedExitNode:		; preds = %loopexit.5, %loopentry.5, %endif.2
	ret void
}

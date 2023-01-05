; REQUIRES: intel_feature_markercount
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -stop-before=tailduplication | FileCheck --check-prefix=TAIL %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -stop-after=x86-pseudo | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -x86-expand-pseudo-marker-count=false | FileCheck --check-prefix=COMMENT %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- | FileCheck %s

; TAIL: f
; TAIL-NOT: PSEUDO_LOOP_HEADER
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL: INLINEASM
; TAIL: PSEUDO_FUNCTION_EPILOG

; TAIL: g
; TAIL: bb.0.entry
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL-NOT: PSEUDO_FUNCTION_PROLOG
; TAIL: bb.{{.*}}.exit
; TAIL: PSEUDO_FUNCTION_EPILOG
; TAIL-NOT: PSEUDO_FUNCTION_EPILOG

; PSEUDO-NOT: PSEUDO_FUNCTION_PROLOG
; PSEUDO-NOT: PSEUDO_FUNCTION_EPILOG
; PSEUDO: f
; PSEUDO: MARKER_COUNT_FUNCTION
; PSEUDO: MARKER_COUNT_FUNCTION

; PSEUDO: g
; PSEUDO: bb.0.entry
; PSEUDO: MARKER_COUNT_FUNCTION
; PSEUDO: bb.{{.*}}.exit
; PSEUDO: MARKER_COUNT_FUNCTION
; PSEUDO-NOT: MARKER_COUNT_FUNCTION

; COMMENT: f
; COMMENT: #FUNCTION_PROLOG
; COMMENT: #FUNCTION_EPILOG
; COMMENT: retq

; COMMENT: g
; COMMENT: #FUNCTION_PROLOG
; COMMENT: #FUNCTION_EPILOG
; COMMENT: retq

; CHECK: f
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG
; CHECK: retq

; CHECK: g
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG
; CHECK: retq

define i32 @f() {
entry:
  call void asm sideeffect "nop", ""()
  ret i32 0
}

define i32 @g(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}

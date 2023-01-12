; REQUIRES: intel_feature_markercount
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -stop-before=tailduplication | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -stop-after=block-placement | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -x86-expand-pseudo-marker-count=false | FileCheck --check-prefix=COMMENT %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- | FileCheck %s

; PSEUDO: f
; PSEUDO-NOT: PSEUDO_LOOP_HEADER
; PSEUDO: PSEUDO_FUNCTION_PROLOG
; PSEUDO: INLINEASM
; PSEUDO: PSEUDO_FUNCTION_EPILOG

; PSEUDO: g
; PSEUDO: bb.0.entry
; PSEUDO: PSEUDO_FUNCTION_PROLOG
; PSEUDO-NOT: PSEUDO_FUNCTION_PROLOG
; PSEUDO: bb.{{.*}}.exit
; PSEUDO: PSEUDO_FUNCTION_EPILOG
; PSEUDO-NOT: PSEUDO_FUNCTION_EPILOG

; COMMENT: f
; COMMENT: #FUNCTION_PROLOG
; COMMENT: #FUNCTION_EPILOG
; COMMENT: retq

; COMMENT: g
; COMMENT: #FUNCTION_PROLOG
; COMMENT: #FUNCTION_EPILOG
; COMMENT: retq

; CHECK: f
; CHECK: marker_function                    # PROLOG
; CHECK: marker_function                    # EPILOG
; CHECK: retq

; CHECK: g
; CHECK: marker_function                    # PROLOG
; CHECK: marker_function                    # EPILOG
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

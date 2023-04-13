; REQUIRES: intel_feature_markercount
; RUN: llc --stop-after=finalize-isel < %s | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s | FileCheck %s

; PSEUDO: name:            g
; PSEUDO: bb.0.entry:
; PSEUDO: PSEUDO_FUNCTION_PROLOG
; PSEUDO: bb.2.loop:
; PSEUDO: PSEUDO_LOOP_HEADER
; PSEUDO: bb.3.exit:
; PSEUDO: PSEUDO_FUNCTION_EPILOG

; CHECK: g:
; CHECK: marker_function
; CHECK: marker_loop
; CHECK: marker_function

define i32 @g(i1 %cmp) local_unnamed_addr #0 {
entry:
  tail call void @llvm.mark.prolog()
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %entry, %loop
  tail call void @llvm.mark.loop.header()
  br label %loop

exit:                                         ; preds = %entry
  tail call void @llvm.mark.epilog()
  ret i32 0
}

declare void @llvm.mark.prolog()

declare void @llvm.mark.loop.header()

declare void @llvm.mark.epilog()

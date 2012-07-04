; RUN: llvm-as %s -o %t.bc
; RUN: opt  -SimplifyGEP -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'

declare i32 @get_global_id(i32) nounwind readnone
declare i64 @get_local_id(i64) nounwind readnone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP by calculating its indices into 
;; one single index, when GEP has two i64 indices, 
;; first one is not uniform and the second is uniform
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main7
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i64
; CHECK-NEXT: mulIndex = mul
; CHECK-NEXT: addIndex = add
; CHECK-NEXT: ptrTypeCast = bitcast
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main7([4 x float] * %memA, float * nocapture %memB, i64 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i64 @get_local_id(i64 0) 
  %arrayidx = getelementptr [4 x float] * %memA, i64 %i3, i64 %i1
  %A = load float * %arrayidx, align 4
  %arrayidx1 = getelementptr float * %memB, i64 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

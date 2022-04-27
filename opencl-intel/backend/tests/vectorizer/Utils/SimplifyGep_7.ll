; RUN: %oclopt -SimplifyGEP -verify %s -S | FileCheck %s
; RUN: %oclopt -opaque-pointers -SimplifyGEP -verify %s -S | FileCheck %s

; ModuleID = 'Program'

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i64 @_Z12get_local_idj(i64) nounwind readnone

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
  %i3 = call i64 @_Z12get_local_idj(i64 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i64 %i3, i64 %i1
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i64 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}

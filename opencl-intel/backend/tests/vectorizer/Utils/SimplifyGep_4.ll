; RUN: %oclopt -SimplifyGEP -verify %s -S | FileCheck %s
; RUN: %oclopt -opaque-pointers -SimplifyGEP -verify %s -S | FileCheck %s

; ModuleID = 'Program'

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i64 @_Z12get_local_idj(i64) nounwind readnone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check simplified GEP when GEP has two 32 indices,
;; first one is uniform and the second is not uniform
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: @main4
; CHECK-NEXT: entry:
; CHECK-NEXT: %i3 = call i32
; CHECK-NEXT: arrayidx = getelementptr
; CHECK-NEXT: simplifiedGEP = getelementptr
; CHECK-NEXT: A = load float
; CHECK-NEXT: arrayidx1 
; CHECK-NEXT: store float 
; CHECK-NEXT: ret void

define void @main4([4 x float] * %memA, float * nocapture %memB, i32 %i1, i32 %i2) nounwind {
entry:
  %i3 = call i32 @_Z13get_global_idj(i32 0) 
  %arrayidx = getelementptr [4 x float], [4 x float] * %memA, i32 %i1, i32 %i3
  %A = load float , float * %arrayidx, align 4
  %arrayidx1 = getelementptr float, float * %memB, i32 %i1
  store float %A, float * %arrayidx1, align 4
  ret void
}
